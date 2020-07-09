#include "include/imageanalysisview.h"

#include <math.h>

ImageAnalysisView::ImageAnalysisView(QWidget *parent, TopinoDocument &doc) :
    QGraphicsView(parent), document(doc) {
    setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );

    currentimage = new QGraphicsPixmapItem();

    imagescene = new QGraphicsScene(contentsRect(), this);
    imagescene->addItem(currentimage);

    setScene(imagescene);
}

ImageAnalysisView::~ImageAnalysisView() {
}

void ImageAnalysisView::modelHasChanged() {
    showImage(document.getImage());

    setSceneRect(currentimage->boundingRect());
}

void ImageAnalysisView::showImage(const QImage& image) {
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
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    QRectF viewrect = contentsRect();

    if (horizontalScrollBar()->isVisible())
        viewrect.setHeight(viewrect.height() - horizontalScrollBar()->height());

    if (verticalScrollBar()->isVisible())
        viewrect.setHeight(viewrect.width() - verticalScrollBar()->width());

    setSceneRect(currentimage->boundingRect());

    QGraphicsView::resizeEvent(event);
}

void ImageAnalysisView::mousePressEvent(QMouseEvent* event) {
    /* Translation event: press and hold middle mouse button to move around */
    if (event->button() == Qt::MiddleButton) {
        translateOrigin = event->pos();
    }

    /* Selection event: press and hold left mouse button to start selection/rubber band process */
    if (event->button() == Qt::LeftButton) {
        rubberBandOrigin = event->pos();

        if (rubberBand == nullptr)
            rubberBand = new CircleRubberBand(this);

        rubberBand->setGeometry(QRect(rubberBandOrigin, QSize()));
        rubberBand->show();
    }

    QGraphicsView::mousePressEvent(event);
}

void ImageAnalysisView::mouseMoveEvent(QMouseEvent* event) {
    /* Translation event: moving the mouse around while holding (only!) the middle mouse button */
    if (event->buttons() == Qt::MidButton) {
        QPointF oldpoint = mapToScene(translateOrigin);
        QPointF newpoint = mapToScene(event->pos());
        QPointF translation = newpoint - oldpoint;

        translate(translation.x(), translation.y());
        translateOrigin = event->pos();
    }

    /* Selection event */
    if (event->buttons() == Qt::LeftButton) {
        if (rubberBand)
            rubberBand->setGeometry(QRect(rubberBandOrigin, event->pos()).normalized());
    }

    QGraphicsView::mouseMoveEvent(event);
}

void ImageAnalysisView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        if (rubberBand) {
            rubberBand->hide();
            qDebug("Rubber band released at %d %d %d %d", rubberBand->geometry().x(), rubberBand->geometry().y(),
                   rubberBand->geometry().width(), rubberBand->geometry().height());
        }
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

void ImageAnalysisView::drawForeground(QPainter* painter, const QRectF& rect) {
    QGraphicsView::drawForeground(painter, rect);

    /* Temporary Zoom factor view */
    QTransform trf = transform();
    painter->setTransform(QTransform());

    painter->drawText(QRect(10,10, 100, 50), QString("Zoom: %1%").arg(getZoomFactor()*100.0));

    painter->setTransform(trf);
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
