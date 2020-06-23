#include "include/imageanalysisview.h"

ImageAnalysisView::ImageAnalysisView(QWidget *parent, TopinoDocument &document) :
    QGraphicsView(parent), document(document) {
    setAlignment( Qt::AlignTop | Qt::AlignLeft );

    currentimage = new QGraphicsPixmapItem();

    imagescene = new QGraphicsScene(contentsRect(), this);
    imagescene->addItem(currentimage);

    setScene(imagescene);
}

ImageAnalysisView::~ImageAnalysisView() {
}

void ImageAnalysisView::modelHasChanged() {
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
    QRectF viewrect = contentsRect();

    if (horizontalScrollBar()->isVisible())
        viewrect.setHeight(viewrect.height() - horizontalScrollBar()->height());

    if (verticalScrollBar()->isVisible())
        viewrect.setHeight(viewrect.width() - verticalScrollBar()->width());

    imagescene->setSceneRect(QRectF());

    QGraphicsView::resizeEvent(event);
}

void ImageAnalysisView::mouseReleaseEvent(QMouseEvent *event) {
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
}
