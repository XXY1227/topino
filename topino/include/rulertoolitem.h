#ifndef RULERTOOLITEM_H
#define RULERTOOLITEM_H

#include <QtMath>
#include <QCursor>
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "include/topinotool.h"
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

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

  private:
    /* Individual parts of the rubber item to keep track of what the user is interacting with
     * at the moment. */
    enum parts {
        none = 0,
        point1 = 1,
        point2 = 2,
        middleline = 3
    };
    parts partClicked = parts::none;

    /* Line itself is defined as float line to position and interact with it more exactly */
    QLineF line;

    /* Brushes, pens, and options/dimensions used to draw the rubber item */
    QBrush terminalBrush;
    QPen linePen;

    int lineWidth;
    int offset = 0;

    /* Checks if the given position is _inside_ on of the terminal points or not */
    bool inTerminalPoint(const QPointF &termPoint, const QPointF &pos) const;

    /* Swaps the first with the second point of the line */
    void swapTerminalPoints();

    /* Checks which point is left/top and which is right/bottom and sets them to
     * point1 and point2 of the line, respectively. */
    void checkTerminalPointsOrder();

    /* Checks if the given point is inside the boundary of the scene. */
    bool isTerminalPointInsideScene(const QPointF &pos);

};

#endif // RULERTOOLITEM_H
