#include "include/imageanalysisview.h"

#include <math.h>

ImageAnalysisView::ImageAnalysisView(QWidget *parent, TopinoDocument &doc) :
    QGraphicsView(parent), document(doc) {
    setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

    /* Create scene and input image tool */
    imagescene = new QGraphicsScene(contentsRect(), this);
    inputImage = createInputImageToolItem(QPixmap());
    setScene(imagescene);

    /* Check if selection changed and eventually propagate */
    connect(imagescene, &QGraphicsScene::selectionChanged, this, &ImageAnalysisView::onSelectionChange);
}

ImageAnalysisView::~ImageAnalysisView() {
}

void ImageAnalysisView::modelHasChanged() {
    setImage(document.getData().getImage());

    setSceneRect(inputImage->boundingRect());
}

void ImageAnalysisView::resetView() {
    /* Resets the view by clearing all items from the scenes, setting the tool to the first one, etc */
    imagescene->clear();

    inputImage = createInputImageToolItem(QPixmap());

    setCurrentTool(tools::selection);
    setZoomFactor(1.0);

    emit viewHasChanged();
}

void ImageAnalysisView::setImage(const QImage& image) {
    /* If the image is empty, the zoom factor is set to 100% */
    if (image.isNull()) {
        inputImage->setPixmap(QPixmap());
        setZoomFactor(1.0);

        return;
    }

    /* Extract Pixmap from image */
    inputImage->setPixmap(QPixmap::fromImage(image));

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

void ImageAnalysisView::putImagePointInView(const QPointF& pos) {
    /* The point given is relative to the image, but the image tool has a border,
     * so pos needs to be adjusted for the border. */
    QPointF border = inputImage->getBorderSize();

    /* Add border to the pos and center on this point */
    centerOn(inputImage->mapToScene(pos + border));
}

const QRectF ImageAnalysisView::getImageViewPoint() {
    /* The returning point needs to be adjusted for the border of the image */
    QPointF border = inputImage->getBorderSize();
    QRectF rect = mapToScene(viewport()->geometry()).boundingRect();

    /* Both points need to be adjusted in the same way (translating to the left/top) */
    return rect.adjusted(-border.x(), -border.y(), -border.x(), -border.y());
}

void ImageAnalysisView::resizeEvent(QResizeEvent *event) {
    /* Make sure that resizing checks and shows scrollbars if needed */
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    QRectF viewrect = contentsRect();

    if (horizontalScrollBar()->isVisible())
        viewrect.setHeight(viewrect.height() - horizontalScrollBar()->height());

    if (verticalScrollBar()->isVisible())
        viewrect.setHeight(viewrect.width() - verticalScrollBar()->width());

    setSceneRect(inputImage->boundingRect());

    QGraphicsView::resizeEvent(event);

    emit viewHasChanged();
}

void ImageAnalysisView::mousePressEvent(QMouseEvent* event) {
    /* Translation event: press and hold right mouse button to move around; this works regardless of the tool */
    if (event->buttons() == Qt::RightButton) {
        translateOrigin = event->pos();
    }

    /* Ignore the tool behaviour if the user clicked on anything other than the background (image) */
    if (itemAt(event->pos()) != nullptr && itemAt(event->pos()) != inputImage)
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
    /* If the selection contains more than one item, then unselected the image tool,
     * which acts as a background and is not directly visible to the user */
    if (scene()->selectedItems().count() > 0) {
        inputImage->setSelected(false);
    }

    /* Tell everyone about the selection change */
    emit selectionHasChanged();
}

void ImageAnalysisView::onItemPosChanged(const TopinoGraphicsItem* item) {
    if (item == nullptr)
        return;

    qDebug("View: Item pos %d of type %d changed", item->getItemid(), item->getItemType());

    emit itemHasChanged(item->getItemid());
}

void ImageAnalysisView::onItemDataChanged(const TopinoGraphicsItem* item) {
    if (item == nullptr)
        return;

    qDebug("View: Item data %d of type %d changed", item->getItemid(), item->getItemType());

    /* Check what the item is */
    switch (item->getItemType()) {
    /* Update the inlet item of the document */
    case TopinoGraphicsItem::itemtype::inlet:
        synchronizeInletToolItem(dynamic_cast<const PolarCircleToolItem*>(item));
        break;
    default:
        break;
    }
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
    setSceneRect(inputImage->boundingRect());

    //update();
    emit viewHasChanged();
}

void ImageAnalysisView::zoomByFactor(const double factor) {
    /* Technically the scale function allows simply to forward the factor; however, it scales by sides while
     * factor is the scaling of the area; so, we just forward the factor to setZoomFactor, which also checks
     * for the right range. */
    setZoomFactor(getZoomFactor() * factor);
}

QRectF ImageAnalysisView::getFocusArea() const
{
    const TopinoData &data = document.getData();

    /* Let's find the main inlet and return the bounding rect of that as focus area */
    QList<QGraphicsItem *> items = this->items();
    for(auto iter = items.begin(); iter != items.end(); ++iter) {
        PolarCircleToolItem *item = dynamic_cast<PolarCircleToolItem *>(*iter);

        /* We need to use the bounding rect of the shape of PolarCircleToolItem to get
         * an accurate focus area. The boundingrect of PolarCircleToolItem itself will
         * return a rect for the FULL circle not only the part that is visible! */
        if ((item != nullptr) && (item->getItemid() == data.getMainInletID())) {
            return item->shape().boundingRect();
        }
    }

    /* If there is no main inlet yet, the whole image rect is the focus area */
    return inputImage->boundingRect();
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
        tool->setItemid((*iter).ID);

        if (data.getMainInletID() == (*iter).ID) {
            tool->showSegments(true);
        } else {
            tool->showSegments(false);
        }
    }

    emit viewHasChanged();
}

InputImageToolItem* ImageAnalysisView::createInputImageToolItem(const QPixmap& pixmap) {
    /* First of all we remove all other image tools (only one allowed!) */
    deleteToolItemByType(TopinoGraphicsItem::itemtype::image);

    /* Create a new tool, save the pointer, and connect it to the event chain */
    InputImageToolItem *tool = new InputImageToolItem(0);
    tool->setPixmap(pixmap);
    imagescene->addItem(tool);

    return tool;
}

RulerToolItem* ImageAnalysisView::createRulerToolItem(QPointF srcPoint, QPointF destPoint) {
    /* First of all we remove all other ruler tools (only one allowed!) */
    deleteToolItemByType(TopinoGraphicsItem::itemtype::ruler);

    /* Create a new tool, scale it to 0.2% of the image width, and connect it to the event chain */
    RulerToolItem *tool = new RulerToolItem(0);
    //tool->setScaling(currentimage->pixmap().width() * 0.002);
    tool->setScaling(inputImage->getPixmap().width() * 0.002);
    tool->setLine(QLineF(srcPoint, destPoint));
    imagescene->addItem(tool);
    connect(tool, &TopinoGraphicsItem::itemPosChanged, this, &ImageAnalysisView::onItemPosChanged);
    connect(tool, &TopinoGraphicsItem::itemDataChanged, this, &ImageAnalysisView::onItemDataChanged);

    return tool;
}

PolarCircleToolItem* ImageAnalysisView::createInletToolItem(QPointF srcPoint, int radius, bool addToDocument) {
    /* Create a new tool, scale it to 0.2% of the image width, and connect it to the event chain */
    PolarCircleToolItem *tool = new PolarCircleToolItem(0);
    //tool->setScaling(currentimage->pixmap().width() * 0.002);
    tool->setScaling(inputImage->getPixmap().width() * 0.002);

    tool->setOrigin(srcPoint);
    tool->setInnerRadius(radius);
    tool->setOuterRadius(inputImage->getPixmap().width() / 2);
    tool->setSegments(3);

    /* If there are already other inlets out there, then new ones will not show any segments but
     * only the cirlce itself; only one inlet will be the "main" inlet and used for origins */
    int count = countToolItemByType(TopinoGraphicsItem::itemtype::inlet);

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
        qDebug("Created new inlet with ID %d", ID);
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
    connect(tool, &TopinoGraphicsItem::itemPosChanged, this, &ImageAnalysisView::onItemPosChanged);
    connect(tool, &TopinoGraphicsItem::itemDataChanged, this, &ImageAnalysisView::onItemDataChanged);

    return tool;
}

void ImageAnalysisView::synchronizeInletToolItem(const PolarCircleToolItem* item) {
    if (item == nullptr)
        return;

    qDebug("Synchronize data for ID %d", item->getItemid());

    TopinoData data = document.getData();

    /* Update data for every inlet type */
    TopinoData::InletData indata;
    indata.ID = item->getItemid();
    indata.coord = item->getOrigin();
    indata.radius = item->getInnerRadius();

    qDebug("Coordinates: %.0f x %.0f", indata.coord.x(), indata.coord.y());

    /* Only for main inlets */
    if (indata.ID == data.getMainInletID()) {
        data.setCoordNeutralAngle(item->getZeroAngle());
        data.setCoordMinAngle(item->getMinAngle());
        data.setCoordMaxAngle(item->getMaxAngle());
        data.setCoordCounterClockwise(item->getCounterClockwise());
    }

    /* Update */
    data.updateInlet(indata);
    document.setData(data);
}

int ImageAnalysisView::countToolItemByType(TopinoGraphicsItem::itemtype type) {
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
