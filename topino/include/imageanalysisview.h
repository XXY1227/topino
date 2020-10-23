#ifndef IMAGEANALYSISVIEW_H
#define IMAGEANALYSISVIEW_H

#include <QGraphicsView>
#include <QScrollBar>
#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>

#include "include/topinoabstractview.h"

#include "include/topinorubberband.h"
#include "include/linerubberband.h"
#include "include/circlerubberband.h"
#include "include/iobserver.h"
#include "include/topinodocument.h"

#include "include/topinographicsitem.h"
#include "include/inputimagetoolitem.h"
#include "include/rulertoolitem.h"
#include "include/polarcircletoolitem.h"

class ImageAnalysisView : public TopinoAbstractView {
    Q_OBJECT

  public:
    ImageAnalysisView(QWidget *parent, TopinoDocument &doc);
    ~ImageAnalysisView();

    void modelHasChanged();

    void resetView();

    void setImage(const QImage &image);

    void setImageBasedSceneRect();

    const QRectF getImageViewPoint();

    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool viewportEvent(QEvent *event) override;

    void onSelectionChange();
    void onItemPosChanged(const TopinoGraphicsItem* item);
    void onItemDataChanged(const TopinoGraphicsItem* item);

    QRectF getFocusArea() const;

    bool isToolSupported(const TopinoAbstractView::tools& value) const;

    void createToolsFromDocument();
    TopinoGraphicsItem* getToolbyTypeAndId(TopinoGraphicsItem::itemtype type, int id);
    PolarCircleToolItem* getMainInletTool();

    bool isSourceImageShown() const;
    void showSourceImage(bool value);

  signals:
    void viewHasChanged();
    void selectionHasChanged();
    void itemHasChanged(int itemID);

  private:
    /* The basic scene object + the image at the bottom */
    QGraphicsScene *imagescene = nullptr;
    bool sourceImageShown = true;

    /* Point used for translating/moving the scene by mouse */
    QPoint translateOrigin;

    /* Rubber band data */
    QPoint rubberBandOrigin;
    TopinoRubberBand *rubberBand = nullptr;

    /* Tool data and functions */
    InputImageToolItem* inputImage = nullptr;

    InputImageToolItem* createInputImageToolItem(const QPixmap& pixmap);
    RulerToolItem* createRulerToolItem(QPointF srcPoint, QPointF destPoint);
    PolarCircleToolItem* createInletToolItem(QPointF srcPoint, int radius, bool addToDocument = false);

    void synchronizeInletToolItem(const PolarCircleToolItem *item);

    int countToolItemByType(TopinoGraphicsItem::itemtype type);
    void deleteToolItemByType(TopinoGraphicsItem::itemtype type);
};

#endif // IMAGEANALYSISVIEW_H
