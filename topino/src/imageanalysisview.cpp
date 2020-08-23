#include "include/imageanalysisview.h"

#include <math.h>

ImageAnalysisView::ImageAnalysisView(QWidget *parent, TopinoDocument &doc) :
    QGraphicsView(parent), document(doc) {
    setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

    currentimage = new QGraphicsPixmapItem();

    imagescene = new QGraphicsScene(contentsRect(), this);
    imagescene->addItem(currentimage);
    currentimage->setEnabled(false);
    currentimage->setFlags(0);

    setScene(imagescene);

    /* Check if selection changed and eventually propagate */
    connect(imagescene, &QGraphicsScene::selectionChanged, this, &ImageAnalysisView::onSelectionChange);
}

ImageAnalysisView::~ImageAnalysisView() {
}

void ImageAnalysisView::modelHasChanged() {
    setImage(document.getData().getImage());

    setSceneRect(currentimage->boundingRect());
}

void ImageAnalysisView::resetView() {
    /* Resets the view by clearing all items from the scenes, setting the tool to the first one, etc */
    imagescene->clear();

    currentimage = new QGraphicsPixmapItem();
    imagescene->addItem(currentimage);
    currentimage->setEnabled(false);
    currentimage->setFlags(0);

    setCurrentTool(tools::selection);
    setZoomFactor(1.0);

    emit viewHasChanged();
}

void ImageAnalysisView::setImage(const QImage& image) {
    /* If the image is empty, the zoom factor is set to 100% */
    if (image.isNull()) {
        currentimage->setPixmap(QPixmap());
        setZoomFactor(1.0);

        return;
    }

    /* Extract Pixmap from image */
    currentimage->setPixmap(QPixmap::fromImage(image));

    /* Fit the image into the view, but make sure that the minimum and maximum zoom level is not violated */
    fitInView(imagescene->itemsBoundingRect(), Qt::KeepAspectRatio);

    /* Zoom factor should be between 3.125% and 6400%. In contrast to the setZoomFactor function we can use
     * more precise numbers here. */
    double zoomFactor = getZoomFactor();

    if (zoomFactor < 0.03) {
        setZoomFactor(0.03125);
    } else if (zoomFactor > 64.0) {
        setZoomFactor(64.0);
    }

    /* Redraw view and tell everyone */
    emit viewHasChanged();
}

void ImageAnalysisView::resizeEvent(QResizeEvent *event) {
    /* Make sure that resizing checks and shows scrollbars if needed */
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    QRectF viewrect = contentsRect();

    if (horizontalScrollBar()->isVisible())
        viewrect.setHeight(viewrect.height() - horizontalScrollBar()->height());

    if (verticalScrollBar()->isVisible())
        viewrect.setHeight(viewrect.width() - verticalScrollBar()->width());

    setSceneRect(currentimage->boundingRect());

    QGraphicsView::resizeEvent(event);

    emit viewHasChanged();
}

void ImageAnalysisView::mousePressEvent(QMouseEvent* event) {
    /* Translation event: press and hold right mouse button to move around; this works regardless of the tool */
    if (event->buttons() == Qt::RightButton) {
        translateOrigin = event->pos();
    }

    /* Ignore the tool behaviour if the user clicked on anything other than the background (image) */
    if (itemAt(event->pos()) != nullptr && itemAt(event->pos()) != currentimage)
        return QGraphicsView::mousePressEvent(event);

    switch (currentTool) {
    /* Selection event for selection, ruler, and inlet tools: press and hold left mouse button to
     * start selection/rubber band process */
    case tools::selection:
    case tools::ruler:
    case tools::inletCircle:
        if (event->button() == Qt::LeftButton) {
            if (rubberBand == nullptr) {
                switch (currentTool) {
                case tools::ruler:
                    rubberBand = new LineRubberBand(this);
                    break;
                case tools::inletCircle:
                    rubberBand = new CircleRubberBand(this);
                    break;
                default:
                    rubberBand = new TopinoRubberBand(this);
                    break;
                }
            }

            rubberBand->setSrcPoint(event->pos());
            rubberBand->setDestPoint(event->pos());
            rubberBand->show();
        }
        break;
    }

    QGraphicsView::mousePressEvent(event);
}

void ImageAnalysisView::mouseMoveEvent(QMouseEvent* event) {
    /* Translation event: moving the mouse around while holding (only!) the right mouse button */
    if (event->buttons() == Qt::RightButton) {
        QPointF oldpoint = mapToScene(translateOrigin);
        QPointF newpoint = mapToScene(event->pos());
        QPointF translation = newpoint - oldpoint;

        translate(translation.x(), translation.y());
        translateOrigin = event->pos();
    }

    switch (currentTool) {
    /* Selection event for selection, ruler, and inlet tools: press and hold left mouse button to
     * start selection/rubber band process */
    case tools::selection:
        if (event->buttons() == Qt::LeftButton) {
            if (rubberBand) {
                rubberBand->setDestPoint(event->pos());

                QPainterPath path;
                path.addRect(QRect(rubberBand->getSrcPoint(), rubberBand->getDestPoint()));
                imagescene->setSelectionArea(mapToScene(path));
            }
        }
        break;
    case tools::ruler:
    case tools::inletCircle:
        if (event->buttons() == Qt::LeftButton) {
            if (rubberBand)
                rubberBand->setDestPoint(event->pos());
        }
        break;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void ImageAnalysisView::mouseReleaseEvent(QMouseEvent *event) {
    switch (currentTool) {
    /* Selection event for selection, ruler, and inlet tools: press and hold left mouse button to
     * start selection/rubber band process */
    case tools::selection:
        if (event->button() == Qt::LeftButton) {
            if (rubberBand) {
                rubberBand->hide();
                qDebug("Rubber band released at %d %d %d %d", rubberBand->geometry().x(), rubberBand->geometry().y(),
                       rubberBand->geometry().width(), rubberBand->geometry().height());
            }

            /* Delete this rubber band and free it */
            delete rubberBand;
            rubberBand = nullptr;

            emit viewHasChanged();
        }
        break;
    case tools::ruler:
        if (event->button() == Qt::LeftButton) {
            if (rubberBand) {
                /* Create the actual tool, select it, and return to selection tool */
                RulerToolItem *tool = createRulerToolItem(mapToScene(rubberBand->getSrcPoint()),
                                                          mapToScene(rubberBand->getDestPoint()));

                QPainterPath path;
                path.addRect(tool->boundingRect());
                imagescene->setSelectionArea(path);

                setCurrentTool(tools::selection);

                /* Delete the rubber band itself and free it */
                delete rubberBand;
                rubberBand = nullptr;

                /* View has definitely changed */
                emit viewHasChanged();
            }
        }
        break;
    case tools::inletCircle:
        if (event->button() == Qt::LeftButton) {
            if (rubberBand) {                
                /* Create the actual tool, select it, and return to selection tool */
                QPointF srcPoint = mapToScene(rubberBand->getSrcPoint());
                QPointF destPoint = mapToScene(rubberBand->getDestPoint());
                int radius = qSqrt(qPow(srcPoint.x() - destPoint.x(), 2.0) + qPow(srcPoint.y() - destPoint.y(), 2.0));
                PolarCircleToolItem *tool = createInletToolItem(srcPoint, radius, true);

                QPainterPath path;
                path.addRect(tool->boundingRect());
                imagescene->setSelectionArea(path);

                setCurrentTool(tools::selection);

                /* Delete this rubber band and free it */
                delete rubberBand;
                rubberBand = nullptr;

                emit viewHasChanged();
            }
        }
        break;
    }

    return QGraphicsView::mouseReleaseEvent(event);
}

void ImageAnalysisView::wheelEvent(QWheelEvent *event) {
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    /* Zoom In */
    if (event->delta() > 0) {
        zoomByFactor(2.0);
        /* Zoom Out */
    } else {
        zoomByFactor(0.5);
    }
}

bool ImageAnalysisView::viewportEvent(QEvent* event) {
    emit viewHasChanged();

    return QGraphicsView::viewportEvent(event);
}

void ImageAnalysisView::onSelectionChange() {
    emit selectionHasChanged();
}

void ImageAnalysisView::onItemChanged(const TopinoGraphicsItem* item) {
    if (item == nullptr)
        return;

    qDebug("View: Item %d changed", item->getItemid());
    emit itemHasChanged(item->getItemid());
}

double ImageAnalysisView::getZoomFactor() const {
    /* Use determinant of the transformation matrix as the zoom factor (determinant = scaling of the _area_) */
    return transform().det();
}

void ImageAnalysisView::setZoomFactor(const double zoomTo) {
    /* Zoom factor should be between 3.125% and 6400%. Due to double innaccuracies the check is actually less
     * than 6500% (65.0) and greater than 3% (0.03). */
    if ((zoomTo < 0.03) || (zoomTo > 65.0))
        return;

    /* New zoom factor over current zoom factor is the factor we need to scale the view; since we are using
     * the area as the reference, the actual scale factor is the square root of the ratio! */
    double scaleFactor = sqrt(zoomTo / getZoomFactor());
    scale(scaleFactor, scaleFactor);

    /* Adjust the scene, redraw the view, and tell everyone that the view has changed */
    setSceneRect(currentimage->boundingRect());

    //update();
    emit viewHasChanged();
}

void ImageAnalysisView::zoomByFactor(const double factor) {
    /* Technically the scale function allows simply to forward the factor; however, it scales by sides while
     * factor is the scaling of the area; so, we just forward the factor to setZoomFactor, which also checks
     * for the right range. */
    setZoomFactor(getZoomFactor() * factor);
}

ImageAnalysisView::tools ImageAnalysisView::getCurrentTool() const {
    return currentTool;
}

void ImageAnalysisView::setCurrentTool(const ImageAnalysisView::tools& value) {
    currentTool = value;
}

void ImageAnalysisView::createToolsFromDocument() {
    const TopinoData &data = document.getData();

    /* First, create all inlets from the document */
    QList<TopinoData::InletData> indata = data.getInlets();

    for(auto iter = indata.begin(); iter != indata.end(); ++iter) {
        PolarCircleToolItem *tool = createInletToolItem((*iter).coord, (*iter).radius);

        qDebug("Add ID %d. MainID = %d", (*iter).ID, data.getMainInletID());
        if (data.getMainInletID() == (*iter).ID) {
            tool->showSegments(true);
        } else {
            tool->showSegments(false);
        }
    }

    emit viewHasChanged();
}

RulerToolItem* ImageAnalysisView::createRulerToolItem(QPointF srcPoint, QPointF destPoint) {
    /* First of all we remove all other ruler tools (only one allowed!) */
    deleteToolItemByType(TopinoGraphicsItem::itemtype::ruler);

    /* Create a new tool, scale it to 0.2% of the image width, and connect it to the event chain */
    RulerToolItem *tool = new RulerToolItem(0);
    tool->setScaling(currentimage->pixmap().width() * 0.002);
    tool->setLine(QLineF(srcPoint, destPoint));
    imagescene->addItem(tool);
    connect(tool, &TopinoGraphicsItem::itemHasChanged, this, &ImageAnalysisView::onItemChanged);

    return tool;
}

PolarCircleToolItem*ImageAnalysisView::createInletToolItem(QPointF srcPoint, int radius, bool addToDocument) {
    /* Create a new tool, scale it to 0.2% of the image width, and connect it to the event chain */
    PolarCircleToolItem *tool = new PolarCircleToolItem(0);
    tool->setScaling(currentimage->pixmap().width() * 0.002);

    tool->setOrigin(srcPoint);
    tool->setInnerRadius(radius);
    tool->setOuterRadius(currentimage->pixmap().width() / 2);
    tool->setSegments(3);

    /* If there are already other inlets out there, then new ones will not show any segments but
     * only the cirlce itself; only one inlet will be the "main" inlet and used for origins */
    int count = counterToolItemByType(TopinoGraphicsItem::itemtype::inlet);

    if (count > 0) {
        tool->showSegments(false);
    }

    /* Create an inlet item and add it to the document - if needed */
    if (addToDocument) {
        TopinoData::InletData indata;
        indata.ID = 0;
        indata.coord = srcPoint;
        indata.radius = radius;

        /* Create a document inlet object, receive ID, and connect to the tool */
        TopinoData data = document.getData();
        int ID = data.updateInlet(indata, true);
        tool->setItemid(ID);

        /* If no other inlet has been created yet, this will be the main inlet */
        if (count == 0) {
            data.setMainInletID(ID);
        }

        /* Add the new inlet data to the document */
        document.setData(data);
    }

    /* Add the tool itself to the scene and connect it to the event chain */
    imagescene->addItem(tool);
    connect(tool, &TopinoGraphicsItem::itemHasChanged, this, &ImageAnalysisView::onItemChanged);

    return tool;
}

int ImageAnalysisView::counterToolItemByType(TopinoGraphicsItem::itemtype type) {
    int counter = 0;

    /* Search all items in the scene and count them if they are of the respective
     * type; important is here that we create a temporary QList and not call items()
     * every time - otherwise the iterator will not be constant/fitting and the
     * dynamic cast will crash. See C++ reference:
     * https://en.cppreference.com/w/cpp/language/range-for#Temporary_range_expression */
    QList<QGraphicsItem *> items = this->items();
    for(auto iter = items.begin(); iter != items.end(); ++iter) {
        TopinoGraphicsItem *item = dynamic_cast<TopinoGraphicsItem *>(*iter);

        if ((item != nullptr) && (item->getItemType() == type)) {
            counter++;
        }
    }

    return counter;
}

void ImageAnalysisView::deleteToolItemByType(TopinoGraphicsItem::itemtype type) {
    QList<TopinoGraphicsItem *> delitems;

    /* Search all items in the scene and add them to the del-list if they are
     * of the respective type; important is here that we create a temporary
     * QList and not call items() every time - otherwise the iterator will not
     * be constant/fitting and the dynamic cast will crash. See C++ reference:
     * https://en.cppreference.com/w/cpp/language/range-for#Temporary_range_expression */
    QList<QGraphicsItem *> items = this->items();
    for(auto iter = items.begin(); iter != items.end(); ++iter) {
        TopinoGraphicsItem *item = dynamic_cast<TopinoGraphicsItem *>(*iter);

        if ((item != nullptr) && (item->getItemType() == type)) {
            delitems.push_back(item);
        }
    }

    /* Delete now all items we found */
    for(auto iter = delitems.begin(); iter != delitems.end(); ++iter) {
        scene()->removeItem(*iter);
    }
}
