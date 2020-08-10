#ifndef IMAGEANALYSISVIEW_H
#define IMAGEANALYSISVIEW_H

#include <QGraphicsView>
#include <QScrollBar>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>


#include "include/topinorubberband.h"
#include "include/linerubberband.h"
#include "include/circlerubberband.h"
#include "include/iobserver.h"
#include "include/topinodocument.h"

#include "include/topinographicsitem.h"
#include "include/rulertoolitem.h"
#include "include/polarcircletoolitem.h"

class ImageAnalysisView : public QGraphicsView, public IObserver {
    Q_OBJECT

  public:
    ImageAnalysisView(QWidget *parent, TopinoDocument &doc);
    ~ImageAnalysisView();

    void modelHasChanged();

    void resetView();

    void setImage(const QImage &image);

    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool viewportEvent(QEvent *event) override;    

    void onSelectionChange();
    void onItemChanged(const TopinoGraphicsItem* item);

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
    void selectionHasChanged();
    void itemHasChanged(int itemID);

  private:
    TopinoDocument &document;

    QGraphicsScene *imagescene = nullptr;
    QGraphicsPixmapItem *currentimage = nullptr;

    QPoint translateOrigin;

    QPoint rubberBandOrigin;
    TopinoRubberBand *rubberBand = nullptr;

    ImageAnalysisView::tools currentTool = ImageAnalysisView::tools::selection;
};

#endif // IMAGEANALYSISVIEW_H
