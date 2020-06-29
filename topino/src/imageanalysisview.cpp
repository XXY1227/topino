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
    if (image.isNull()) {
        currentimage->setPixmap(QPixmap());
    } else {
        currentimage->setPixmap(QPixmap::fromImage(image));
    }

    fitInView(imagescene->itemsBoundingRect(), Qt::KeepAspectRatio);
    update();
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
            rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

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

    // Scale the view / do the zoom
    double scaleFactor = 1.15;

    if(event->delta() > 0) {
        // Zoom in
        scale(scaleFactor, scaleFactor);
    } else {
        // Zooming out
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }

    setSceneRect(currentimage->boundingRect());

    update();
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
