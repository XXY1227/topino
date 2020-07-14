#ifndef IMAGEANALYSISVIEW_H
#define IMAGEANALYSISVIEW_H

#include <QGraphicsView>
#include <QScrollBar>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>

#include "include/linerubberband.h"
#include "include/circlerubberband.h"
#include "include/iobserver.h"
#include "include/topinodocument.h"

class ImageAnalysisView : public QGraphicsView, public IObserver {
    Q_OBJECT

  public:
    ImageAnalysisView(QWidget *parent, TopinoDocument &doc);
    ~ImageAnalysisView();

    void modelHasChanged();

    void showImage(const QImage &image);

    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void drawForeground(QPainter *painter, const QRectF &rect) override;

    double getZoomFactor() const;
    void setZoomFactor(const double zoomTo);
    void zoomByFactor(const double factor);

    enum tools {
        selection = 0,
        ruler = 1,
        inletCircle = 2
    };

    ImageAnalysisView::tools getCurrentTool() const;
    void setCurrentTool(const ImageAnalysisView::tools& value);

  signals:
    void viewHasChanged();

  private:
    TopinoDocument &document;

    QGraphicsScene *imagescene = nullptr;
    QGraphicsPixmapItem *currentimage = nullptr;

    QPoint translateOrigin;

    QPoint rubberBandOrigin;
    QRubberBand *rubberBand = nullptr;

    ImageAnalysisView::tools currentTool = ImageAnalysisView::tools::selection;
};

#endif // IMAGEANALYSISVIEW_H
