#ifndef IMAGEANALYSISVIEW_H
#define IMAGEANALYSISVIEW_H

#include <QGraphicsView>
#include <QScrollBar>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>

#include "include/iobserver.h"
#include "include/topinodocument.h"

class ImageAnalysisView : public QGraphicsView, public IObserver {
  public:
    ImageAnalysisView(QWidget *parent, TopinoDocument &doc);
    ~ImageAnalysisView();

    void modelHasChanged();

    void showImage(const QImage &image);

    void resizeEvent(QResizeEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

  private:
    TopinoDocument &document;

    QGraphicsScene *imagescene = nullptr;
    QGraphicsPixmapItem *currentimage = nullptr;
};

#endif // IMAGEANALYSISVIEW_H
