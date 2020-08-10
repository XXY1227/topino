#ifndef POLARCIRCLETOOLITEM_H
#define POLARCIRCLETOOLITEM_H

#include <QPainter>
#include <QGraphicsItem>

#include "topinographicsitem.h"

class PolarCircleToolItem : public virtual TopinoGraphicsItem {
  public:
    PolarCircleToolItem(int newitemid, QGraphicsItem *parent = nullptr);
    ~PolarCircleToolItem();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;
    QPainterPath shape() const override;

    itemtype getItemType() const override;
    void updateScale() override;

    QPointF getOrigin() const;
    void setOrigin(const QPointF& value);

    int getSegments() const;
    void setSegments(int value);

    bool segmentsVisible() const;
    void showSegments(bool value);

    int getInnerRadius() const;
    void setInnerRadius(int value);

    int getOuterRadius() const;
    void setOuterRadius(int value);

  private:
    /* These are the dimensions used for drawing the inner circle (= the actual inlet) and the segments
     * on top of it */
    int offset;
    QPointF origin;
    int innerRadius;
    int outerRadius;

    int segments;
    int segmentSize;
    bool drawSegments;
    void calculateSegmentSize();

    /* This is the angular zero axis (relative to 9'o clock), and the min and max angle to draw;
     * all angles are given in degree; every diffAngle a line is drawn from the inner circle to
     * the outer one */
    int zeroAngle;
    int minAngle;
    int maxAngle;
    int diffAngle;
    bool counterClockwise;

    /* This function recalculates the visual-to-the-user angle to the angle used for drawing; will
     * also take care of the sign of angle */
    inline int adjustAngleAbs(int visualAngle) const;
    inline int adjustAngleRel(int visualAngle) const;

    /* Brushes, pens, and options/dimensions used to draw the polarcircle item */
    QBrush innerCircleBrush;
    QBrush segmentBrushOdd;
    QBrush segmentBrushEven;
    QPen coordlinePen;
    int coordlineWidth;

    /* This function calculates the size of the boundingRect if position, etc. was changed */
    QRectF fullRect;
    QRectF drawRect;
    void calculateBoundingRect();
};

#endif // POLARCIRCLETOOLITEM_H
