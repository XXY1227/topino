#ifndef RULERTOOLITEM_H
#define RULERTOOLITEM_H

#include <QtMath>
#include <QCursor>
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "include/topinographicsitem.h"

class RulerToolItem : public TopinoGraphicsItem {
  public:
    RulerToolItem(int newitemid, QGraphicsItem *parent = nullptr);
    ~RulerToolItem();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;
    QPainterPath shape() const override;

    QLineF getLine() const;
    void setLine(const QLineF& value);

    itemtype getItemType() const override;
    void updateScale() override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

  private:
    enum parts {
        none = 0,
        point1 = 1,
        point2 = 2,
        middleline = 3
    };

    parts partClicked = parts::none;

    QLineF line;
    QBrush terminalBrush;
    QPen linePen;
    int lineWidth;
    int offset = 0;

    bool inTerminalPoint(const QPointF &termPoint, const QPointF &pos) const;
};

#endif // RULERTOOLITEM_H
