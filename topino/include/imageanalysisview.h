#ifndef IMAGEANALYSISVIEW_H
#define IMAGEANALYSISVIEW_H

#include <QGraphicsView>
#include <QScrollBar>
#include <QGraphicsItem>
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

    void createToolsFromDocument();

  signals:
    void viewHasChanged();
    void selectionHasChanged();
    void itemHasChanged(int itemID);

  private:
    /* Reference to the document to get/set data */
    TopinoDocument &document;

    /* The basic scene object + the image at the bottom */
    QGraphicsScene *imagescene = nullptr;
    QGraphicsPixmapItem *currentimage = nullptr;

    /* Point used for translating/moving the scene by mouse */
    QPoint translateOrigin;

    /* Rubber band data */
    QPoint rubberBandOrigin;
    TopinoRubberBand *rubberBand = nullptr;

    /* Tool data and functions */
    ImageAnalysisView::tools currentTool = ImageAnalysisView::tools::selection;

    RulerToolItem* createRulerToolItem(QPointF srcPoint, QPointF destPoint);
    PolarCircleToolItem* createInletToolItem(QPointF srcPoint, int radius);

    int counterToolItemByType(TopinoGraphicsItem::itemtype type);
    void deleteToolItemByType(TopinoGraphicsItem::itemtype type);
};

#endif // IMAGEANALYSISVIEW_H
