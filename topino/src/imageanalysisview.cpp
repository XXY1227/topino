#include "include/imageanalysisview.h"

#include <math.h>

ImageAnalysisView::ImageAnalysisView(QWidget *parent, TopinoDocument &doc) :
    TopinoAbstractView(parent, doc) {
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
    if (sourceImageShown) {
        setImage(document.getData().getImage());
    } else {
        setImage(document.getData().getProcessedImage());
    }
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

    /* Save the dimensions of the old image for comparison later */
    int oldWidth = inputImage->getPixmap().width();
    int oldHeight = inputImage->getPixmap().height();

    /* Extract Pixmap from image */
    inputImage->setPixmap(QPixmap::fromImage(image));

    /* Should the image dimensions have changed, then update scene rect, zoom, etc. */
    if ((inputImage->getPixmap().width() != oldWidth) || (inputImage->getPixmap().height() != oldHeight)) {
        /* Fit the image into the view, but make sure that the minimum and maximum zoom level is not violated */
        setImageBasedSceneRect();
        fitInView(imagescene->itemsBoundingRect(), Qt::KeepAspectRatio);

        /* Zoom factor should be between 3.125% and 6400%. In contrast to the setZoomFactor function we can use
         * more precise numbers here. */
        double zoomFactor = getZoomFactor();

        if (zoomFactor < 0.03) {
            setZoomFactor(0.03125);
        } else if (zoomFactor > 64.0) {
            setZoomFactor(64.0);
        }
    }

    /* Redraw view and tell everyone */
    viewport()->update();
    emit viewHasChanged();
}

void ImageAnalysisView::setImageBasedSceneRect() {
    /* Get the boundary rect of the image; this is the focus point after all */
    QRectF rect = inputImage->boundingRect();

    /* Border added to the image is a minimum of 20 pixels, or 5% of the width
     * or height of the image - whatever is larger */
    qreal border = qMax(qMax(0.05 * rect.width(), 0.05 * rect.height()), 20.0);

    /* Negative coordinate, because we want the top-left of the image to be (0,0);
     * also, border needs to be added only one time to width and height! */
    rect.setTopLeft(QPointF(-border, -border));
    rect.setWidth(rect.width() + border);
    rect.setHeight(rect.height() + border);

    /* Set the modified scene rect */
    setSceneRect(rect);

    /* For some reason the scenerect of the scene itself does not get updated;
     * let's do it manually, so that items can rely on this */
    imagescene->setSceneRect(rect);
}

const QRectF ImageAnalysisView::getImageViewPoint() {
    return mapToScene(viewport()->geometry()).boundingRect();
}

bool ImageAnalysisView::event(QEvent* event) {
    /* Catch some events that might not be catched otherwise here */
    switch (event->type()) {
    /* Catch tab key here and forward it to our key-press event */
    case QEvent::KeyPress: {
        QKeyEvent *k = dynamic_cast<QKeyEvent *>(event);

        if (k == nullptr)
            break;

        if ((k->key() == Qt::Key_Backtab) || (k->key() == Qt::Key_Tab)
                || (k->key() == Qt::Key_Tab && (k->modifiers() & Qt::ShiftModifier))) {
            /* Key press handling */
            keyPressEvent(k);

            /* Make sure there are no side effects by removing this event from the event
             * queue */
            event->accept();
            return true;
        }

        break;
    }

    /* Default behaviour is to do nothing */
    default:
        break;
    }

    /* Default behaviour */
    return TopinoAbstractView::event( event );
}

void ImageAnalysisView::resizeEvent(QResizeEvent *event) {
    /* Make sure that resizing checks and shows scrollbars if needed */
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    QRectF viewrect = contentsRect();

    if (horizontalScrollBar()->isVisible())
        viewrect.setHeight(viewrect.height() - horizontalScrollBar()->height());

    if (verticalScrollBar()->isVisible())
        viewrect.setHeight(viewrect.width() - verticalScrollBar()->width());

    setImageBasedSceneRect();

    TopinoAbstractView::resizeEvent(event);

    emit viewHasChanged();
}

void ImageAnalysisView::keyPressEvent(QKeyEvent* event) {
    qDebug("Key pressed on view %d", event->key());

    /* Check keys */
    switch(event->key()) {
    /* Tab key => Select prev/next item (and bring it to the foreground) */
    case Qt::Key_Tab:
        selectNext();
        break;

    /* Escape key => Clear selection/Unselect everything */
    case Qt::Key_Escape:
        selectNone();
        break;

    /* Delete key => Remove all currently selected items */
    case Qt::Key_Delete:
        erase();
        break;

    default:
        break;
    }

    /* Call abstract class implementation */
    TopinoAbstractView::keyPressEvent(event);
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
    case tools::rulerLine:
    case tools::inletCircle:
        if (event->button() == Qt::LeftButton) {
            if (rubberBand == nullptr) {
                switch (currentTool) {
                case tools::rulerLine:
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
    default:
        break;
    }

    TopinoAbstractView::mousePressEvent(event);
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
    case tools::rulerLine:
    case tools::inletCircle:
        if (event->buttons() == Qt::LeftButton) {
            if (rubberBand)
                rubberBand->setDestPoint(event->pos());
        }
        break;
    default:
        break;
    }

    TopinoAbstractView::mouseMoveEvent(event);
}

void ImageAnalysisView::mouseReleaseEvent(QMouseEvent *event) {
    switch (currentTool) {
    /* Selection event for selection, ruler, and inlet tools: press and hold left mouse button to
     * start selection/rubber band process */
    case tools::selection:
        if (event->button() == Qt::LeftButton) {
            if (rubberBand) {
                /* Delete this rubber band and free it */
                delete rubberBand;
                rubberBand = nullptr;

                emit viewHasChanged();

                return;
            }
        }
        break;
    case tools::rulerLine:
        if (event->button() == Qt::LeftButton) {
            if (rubberBand) {
                /* Create the actual tool, select it, and return to selection tool */
                RulerToolItem *tool = createRulerToolItem(mapToScene(rubberBand->getSrcPoint()),
                                      mapToScene(rubberBand->getDestPoint()));

                /* Select only the new item */
                scene()->clearSelection();
                selectItem(tool);

                setCurrentTool(tools::selection);

                /* Delete the rubber band itself and free it */
                delete rubberBand;
                rubberBand = nullptr;

                /* View has definitely changed */
                emit viewHasChanged();

                return;
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

                /* Select only the new item */
                scene()->clearSelection();
                selectItem(tool);

                setCurrentTool(tools::selection);

                /* Delete this rubber band and free it */
                delete rubberBand;
                rubberBand = nullptr;

                emit viewHasChanged();

                return;
            }
        }
        break;
    default:
        break;
    }

    TopinoAbstractView::mouseReleaseEvent(event);
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

    return TopinoAbstractView::viewportEvent(event);
}

void ImageAnalysisView::onSelectionChange() {
    /* If the selection contains more than one item, then unselected the image tool,
     * which acts as a background and is not directly visible to the user */
    if (scene()->selectedItems().count() > 0) {
        inputImage->setSelected(false);
    }

    /* Put all items in the foreground that are selected */
    QList<QGraphicsItem *> items = scene()->selectedItems();
    for(auto iter = items.begin(); iter != items.end(); ++iter) {
        TopinoGraphicsItem *item = dynamic_cast<TopinoGraphicsItem *>(*iter);

        if ((item != nullptr) && (item != inputImage)) {
            selectItem(item);
        }
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

    emit itemHasChanged(item->getItemid());
}

QRectF ImageAnalysisView::getFocusArea() const {
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

bool ImageAnalysisView::isToolSupported(const TopinoAbstractView::tools& value) const {
    Q_UNUSED(value);

    /* Support all tools at the moment */
    return true;
}

void ImageAnalysisView::cut(QClipboard *clipboard) {
    /* Cut = Copy + Erase */
    copy(clipboard);
    erase();
}

void ImageAnalysisView::copy(QClipboard *clipboard) {
    qDebug("ImageAnalysisView copy");

    /* Create a mimedata object that will store our items in different formats, so
     * that they can be used in different programs */
    QMimeData *mimeData = new QMimeData();

    /* For creating images, etc. We need actually to get the boundary rect of all
     * selected items */
    QRectF selectRect = getSelectionBoundary();

    /* These are the different data streams/objects that will represent the selected
     * items. The simplest form is the textual data, which is just a set of lines
     * describing the various items. */
    QStringList textData;

    /* Prepare everything for raster painting; don't forget to translate the
     * coordinate system. Otherwise, the items do not get drawn on the right
     * positions. */
    QImage imageData(selectRect.size().toSize(), QImage::Format_ARGB32);
    QPainter paintRaster(&imageData);
    paintRaster.fillRect(QRectF(0, 0, selectRect.width(), selectRect.height()), QBrush(Qt::transparent));
    paintRaster.translate(- selectRect.topLeft().toPoint());

    /* Prepare everything for vector painting; same things as above apply. */
    QSvgGenerator generator;
    QBuffer svgBuffer;
    generator.setOutputDevice(&svgBuffer);
    generator.setSize(selectRect.size().toSize());
    generator.setViewBox(QRect(QPoint(0, 0), selectRect.size().toSize()));
    QPainter paintVector(&generator);

    /* Create a list with all objects selected and iterate through it. We need
     * to ask each item for the representation. */
    QList<QGraphicsItem *> items = scene()->selectedItems();

    for(auto iter = items.begin(); iter != items.end(); ++iter) {
        TopinoGraphicsItem *item = dynamic_cast<TopinoGraphicsItem *>(*iter);

        if ((item != nullptr) && (item != inputImage)) {
            textData.append(item->toString());

            /* For painting, remove the selection */
            item->setSelected(false);
            item->paint(&paintRaster, nullptr);
            item->paint(&paintVector, nullptr);
            item->setSelected(true);
        }
    }

    paintRaster.end();
    paintVector.end();

    /* Add the data to our mime object and feed the clipboard with it. Ownership
     * is transfered to the clipboard. */
    mimeData->setText(textData.join("\n") + "\n");
    mimeData->setImageData(imageData);
    mimeData->setData("image/svg+xml", svgBuffer.buffer());
    clipboard->setMimeData(mimeData);
}

void ImageAnalysisView::paste(QClipboard *clipboard) {
    qDebug("ImageAnalysisView paste");
}

void ImageAnalysisView::erase() {
    qDebug("ImageAnalysisView erase");

    /* Removes all selected item */
    QList<TopinoGraphicsItem *> delitems;

    /* Search all selected items and add them to the del-list if they are
     * a TopinoGraphicsItem and not the background image. This is necessary
     * since by deleting items, the selectedItems list will change. */
    QList<QGraphicsItem *> items = scene()->selectedItems();
    for(auto iter = items.begin(); iter != items.end(); ++iter) {
        TopinoGraphicsItem *item = dynamic_cast<TopinoGraphicsItem *>(*iter);

        if ((item != nullptr) && (item != inputImage)) {
            delitems.push_back(item);
        }
    }

    /* Delete now all items we found */
    for(auto iter = delitems.begin(); iter != delitems.end(); ++iter) {
        removeItem(*iter);
    }

    emit viewHasChanged();
}

void ImageAnalysisView::selectAll() {
    qDebug("ImageAnalysisView selectAll");
    /* Select all _Topino_ items on the view ecept the image itself */
    scene()->clearSelection();

    QList<QGraphicsItem *> items = scene()->items();
    for(auto iter = items.begin(); iter != items.end(); ++iter) {
        TopinoGraphicsItem *item = dynamic_cast<TopinoGraphicsItem *>(*iter);

        if ((item != nullptr) && (item != inputImage)) {
            item->setSelected(true);
        }
    }

    /* Update viewport */
    viewport()->update();

    emit viewHasChanged();
}

void ImageAnalysisView::selectNone() {
    /* Clears the selection */
    scene()->clearSelection();

    emit viewHasChanged();
}

void ImageAnalysisView::selectNext() {
    /* No items in the scene? Leave! */
    if (this->items().size() == 0) {
        return;
    }

    /* Behaviour:
     * - if _none_ or _multiple_ items are selected, select the first element of items()
     * - if exactly _one_ item is selected, select the item with the lowest stacking index. */
    QGraphicsItem *toSelect = this->items()[0];

    if (scene()->selectedItems().size() == 1) {
        /* By using the _ascending_ order, we can make sure to always select an item that
         * has a low stacking index. We use the first non-selected item that is also NOT
         * the background image. */
        QList<QGraphicsItem *> items = scene()->items(Qt::AscendingOrder);
        QListIterator iter(items);
        while(iter.hasNext()) {
            QGraphicsItem *item = iter.next();

            /* This is the item we are look for */
            if ((item != nullptr) && (item != inputImage)) {
                toSelect = item;
                break;
            }
        }
    }

    /* Unselect everything, select new item from above, and
     * notify scene/viewport of changes. */
    scene()->clearSelection();

    if (toSelect != nullptr) {
        selectItem(dynamic_cast<TopinoGraphicsItem *>(toSelect));
    }

    viewport()->update();

    emit viewHasChanged();
}

bool ImageAnalysisView::isEditFunctionSupported(const TopinoAbstractView::editfunc& value) const {
    Q_UNUSED(value)

    /* All edit functions are supported */
    return true;
}

void ImageAnalysisView::createToolsFromDocument() {
    const TopinoData &data = document.getData();

    /* First, create all inlets from the document */
    QList<TopinoData::InletData> indata = data.getInlets();

    for(auto iter = indata.begin(); iter != indata.end(); ++iter) {
        PolarCircleToolItem *tool = createInletToolItem((*iter).coord, (*iter).radius);
        tool->setItemid((*iter).ID);

        if (data.getMainInletID() == (*iter).ID) {
            tool->setMinAngle(data.getCoordMinAngle());
            tool->setMaxAngle(data.getCoordMaxAngle());
            tool->setOuterRadius(data.getCoordOuterRadius());
            tool->setZeroAngle(data.getCoordNeutralAngle());
            tool->setCounterClockwise(data.getCoordCounterClockwise());
            tool->showSegments(true);
        } else {
            tool->showSegments(false);
        }
    }

    emit viewHasChanged();
}

TopinoGraphicsItem*ImageAnalysisView::getToolbyTypeAndId(TopinoGraphicsItem::itemtype type, int id = 0) {
    /* Check every item for type */
    QList<QGraphicsItem *> items = this->items();
    for(auto iter = items.begin(); iter != items.end(); ++iter) {
        TopinoGraphicsItem *item = dynamic_cast<TopinoGraphicsItem *>(*iter);

        if ((item != nullptr) && (item->getItemType() == type)) {
            /* If id > 0, check also for the ID of the item */
            if (((id > 0) && (item->getItemid() == id)) || (id == 0)) {
                return item;
            }
        }
    }

    /* No item found */
    return nullptr;
}

PolarCircleToolItem* ImageAnalysisView::getMainInletTool() {
    /* Convenience function */
    return dynamic_cast<PolarCircleToolItem*>(getToolbyTypeAndId(TopinoGraphicsItem::itemtype::inlet,
            document.getData().getMainInletID()));
}

void ImageAnalysisView::getPointsOfRulerIntersections(QList<QPointF>& list, bool currentSelection) const {
    /* Function to get all intersections of all rulers currently in view */
    QList<RulerToolItem *> rulers;

    /* Search all items in the scene and add them to the ruler-list if they are
     * of the respective class; important is here that we create a temporary
     * QList and not call items() every time - otherwise the iterator will not
     * be constant/fitting and the dynamic cast will crash. */
    QList<QGraphicsItem *> items = this->items();
    for(auto iter = items.begin(); iter != items.end(); ++iter) {
        RulerToolItem *item = dynamic_cast<RulerToolItem *>(*iter);

        /* Check if caller wants just selected items */
        if ((item != nullptr) && ((currentSelection && item->isSelected()) || !currentSelection)) {
            rulers.push_back(item);
        }
    }

    /* Iterate through all rulers and find intersections with all other rulers */
    QPointF pt;
    for(auto ruler1 = rulers.begin(); ruler1 != rulers.end(); ++ruler1) {
        for(auto ruler2 = ruler1; ruler2 != rulers.end(); ++ruler2) {
            /* Only take intersections that are INSIDE the visible lines */
            if ((*ruler1)->getLine().intersect((*ruler2)->getLine(), &pt) == QLineF::BoundedIntersection) {
                list.append(pt);
            }
        }
    }
}

int ImageAnalysisView::getNumberOfRulerIntersections(bool currentSelection) const {
    /* Convenience function that returns simply the number of points for all ruler intersections */
    QList<QPointF> list;
    getPointsOfRulerIntersections(list, currentSelection);
    return list.size();
}

bool ImageAnalysisView::isSourceImageShown() const {
    return sourceImageShown;
}

void ImageAnalysisView::showSourceImage(bool value) {
    /* Switch the image */
    sourceImageShown = value;

    if (sourceImageShown) {
        setImage(document.getData().getImage());
    } else {
        setImage(document.getData().getProcessedImage());
    }
}

void ImageAnalysisView::selectItem(TopinoGraphicsItem* item) {
    /* Select an item and make sure that the item is put in the foreground
     * above all items that might be on top of it. This function does NOT
     * unselect other items. Don't restack the image item. */
    if ((item == nullptr) || (item == inputImage)) {
        return;
    }

    item->setSelected(true);

    /* Get all items that are physically above this item */
    QList<QGraphicsItem *> items = this->items(mapFromScene(item->boundingRect()));

    /* If there is at least one item in this list (that would be the item
     * itself) then stack item before the first element in the list. The list
     * is given in descending stacking order, i.e. the first element is on top
     * of everything else.
     * NOTE: Stack _before_ means to the _BACKGROUND_! Therefore, we need to
     * call the function of the foreground item! */
    if (items.size() > 0) {
        QListIterator iter(items);
        iter.toBack();
        while(iter.hasPrevious()) {
            QGraphicsItem *stackItem = iter.previous();

            if (stackItem != inputImage) {
                stackItem->stackBefore(item);
            }
        }
    }
}

void ImageAnalysisView::selectItemType(TopinoGraphicsItem::itemtype type, bool currentSelection) {
    /* Search all items in the scene and select them if they are of the respective type;
     * important is here that we create a temporary QList and not call items() every time
     * - otherwise the iterator will not be constant/fitting and the dynamic cast will
     * crash. See C++ reference:
     *  https://en.cppreference.com/w/cpp/language/range-for#Temporary_range_expression */
    QList<QGraphicsItem *> items = this->items();

    /* Only from currently selected items */
    if (currentSelection) {
        items = scene()->selectedItems();
    }

    /* Check list */
    for(auto iter = items.begin(); iter != items.end(); ++iter) {
        TopinoGraphicsItem *item = dynamic_cast<TopinoGraphicsItem *>(*iter);

        if (item == nullptr) {
            continue;
        }

        /* Select if of the right type */
        item->setSelected(item->getItemType() == type);
    }
}

void ImageAnalysisView::removeItem(TopinoGraphicsItem* item) {
    /* Remove the item if it exists and it is not the image background. */
    if ((item == nullptr) || (item == inputImage)) {
        return;
    }

    /* It is actually MANDATORY to NOT remove the item from the scene but simply
     * delete it. The reason is not completely clear, but removing it prior
     * deleting results in random crashes (SEGFAULTS) during the runtime of
     * Topino. According to the following forum thread, the problem occurs when
     * scene()->removeItem(...) is called from an event handler and there are
     * still events in to process for this respective (Topino)QGraphicsItem:
     * https://forum.qt.io/topic/87303/crash-in-qgraphicsscene-event-when-removing-and-deleting-items-that-use-sceneeventfilters/4
     * It seems to be more stable after remove scene()->removeitem(...) from
     * here (2020-11-03) - so fixed? */

    /* Specifics for individual item types. For instance, the document
     * needs to remove the inlet entry for an inlet item. */
    switch(item->getItemType()) {
    /* Inlet: remove inlet with ID from document. */
    case TopinoGraphicsItem::inlet: {
        TopinoData data = document.getData();
        data.removeInlet(item->getItemid());
        document.setData(data);
    }
    break;
    /* Default: nothing to do for all other types. */
    default:
        break;
    }

    /* Delete occupied memory of this item */
    item->setVisible(false);
    delete item;
}

TopinoGraphicsItem::itemtype ImageAnalysisView::getItemTypeOfSelection() const {
    /* Check if all items currently selected have the same item type. If yes,
     * return it. If no, return nonspecific. */
    QList<QGraphicsItem *> items = scene()->selectedItems();

    /* There should be at least one item. */
    if (items.size() == 0) {
        return TopinoGraphicsItem::nonspecific;
    }

    TopinoGraphicsItem::itemtype type = TopinoGraphicsItem::nonspecific;
    for(auto iter = items.begin(); iter != items.end(); ++iter) {
        TopinoGraphicsItem *item = dynamic_cast<TopinoGraphicsItem *>(*iter);

        /* If there is an item that could not be casted to TopinoGraphicsItem,
         * then return with nonspecific. */
        if (item == nullptr) {
            return TopinoGraphicsItem::nonspecific;
        }

        /* Use the first itemtype encountered as reference */
        if (type == TopinoGraphicsItem::nonspecific) {
            type = item->getItemType();
            continue;
        }

        /* Check if types are compatible. Return if they are not. */
        if (type != item->getItemType()) {
            return TopinoGraphicsItem::nonspecific;
        }
    }

    /* If we are here, then all items in the selection must have the same itemtype,
     * so let's return it. */
    return type;
}

QRectF ImageAnalysisView::getSelectionBoundary() const {
    QRectF rect;

    /* Create a list with all objects selected and iterate through it. We need
     * to ask each item for the representation. */
    QList<QGraphicsItem *> items = scene()->selectedItems();

    for(auto iter = items.begin(); iter != items.end(); ++iter) {
        TopinoGraphicsItem *item = dynamic_cast<TopinoGraphicsItem *>(*iter);

        if ((item != nullptr) && (item != inputImage)) {
            /* Simply combining both rectangles by | will
             * give the boundary rect of both */
            rect |= item->boundingRect();
        }
    }

    return rect;
}

void ImageAnalysisView::createInletAtPos(const QPointF& pt, int radius) {
    /* Create an inlet at the given coordinates and by taking a standard radius. Also,
     * add the inlet to the document. Should not be called to create inlets from the
     * document. Creates no inlet if there is no image loaded. */
    if (inputImage->getPixmap().isNull()) {
        return;
    }

    /* If the point is outside the scene, then do not create an inlet */
    if (!sceneRect().contains(pt)) {
        return;
    }

    /* If no radius is given, use the image dimensions to get a good one */
    if (radius == 0) {
        int width = inputImage->getPixmap().width();
        int height = inputImage->getPixmap().height();
        radius = qMax((int)(qMin(width, height) * 0.01), 10);
    }

    /* Create tool by using a good guess of the inlet radius */
    PolarCircleToolItem *tool = createInletToolItem(pt, radius, true);

    /* Select only the new item */
    scene()->clearSelection();
    selectItem(tool);
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
    /* Create a new tool, scale it to 0.2% of the image width, and connect it to the event chain */
    RulerToolItem *tool = new RulerToolItem(0);
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

    /* Make sure that the initial direction of the tool is towards the middle of the picture; for
     * this, we separate the picture into four triangles (left, bottom, right, top) and select the
     * respective angle (fixed 90°-wise). Important: take the scenerect, not the image rect, in
     * case the user decides to create an inlet outside of the image */
    QPointF center = sceneRect().center();
    int x = sceneRect().left();
    int y = sceneRect().top();
    int width = sceneRect().width();
    int height = sceneRect().height();
    QPolygonF triangles[4] = {
        QPolygonF({QPointF(x, y), center, QPointF(x, height), QPointF(x, y)}),
        QPolygonF({QPointF(x, height), center, QPointF(width, height), QPointF(x, height)}),
        QPolygonF({QPointF(width, height), center, QPointF(width, y), QPointF(width, height)}),
        QPolygonF({QPointF(x, y), QPointF(width, y), center, QPointF(x, y)})
    };

    int angle = 90;
    for (int a = 0; a < 4; ++a) {
        if (triangles[a].containsPoint(srcPoint, Qt::OddEvenFill)) {
            angle = a * 90;
            break;
        }
    }
    tool->setZeroAngle(angle);

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
        data.setCoordOuterRadius(item->getOuterRadius());
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
