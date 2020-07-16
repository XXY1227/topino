#ifndef RULERTOOLITEM_H
#define RULERTOOLITEM_H

#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsLineItem>

class RulerToolItem : public QGraphicsLineItem {
  public:
    RulerToolItem(QGraphicsItem *parent = nullptr);
    ~RulerToolItem();

    bool contains(const QPointF &point) const override;
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;
    QPainterPath shape() const override;

  private:
    QBrush terminalBrush;
    QPen linePen;
    int lineWidth;
    int offset = 0;

    bool inTerminalPoint1(const QPoint &point) const;
    bool inTerminalPoint2(const QPoint &point) const;
};

#endif // RULERTOOLITEM_H
