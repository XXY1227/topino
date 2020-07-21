#ifndef RULERTOOLITEM_H
#define RULERTOOLITEM_H

#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsLineItem>

#include "include/topinographicsitem.h"

class RulerToolItem : public TopinoGraphicsItem {
  public:
    RulerToolItem(int newitemid, QGraphicsItem *parent = nullptr);
    ~RulerToolItem();

    bool contains(const QPointF &point) const override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;
    QPainterPath shape() const override;

    QLineF getLine() const;
    void setLine(const QLineF& value);

    itemtype getItemType() const override;

  private:
    QLineF line;
    QBrush terminalBrush;
    QPen linePen;
    int lineWidth;
    int offset = 0;

    bool inTerminalPoint1(const QPoint &point) const;
    bool inTerminalPoint2(const QPoint &point) const;
};

#endif // RULERTOOLITEM_H
