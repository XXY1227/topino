#ifndef POLARCIRCLETOOLITEM_H
#define POLARCIRCLETOOLITEM_H

#include <QtMath>
#include <QCursor>
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

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

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

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

    int getZeroAngle() const;
    void setZeroAngle(int value);

    int getMinAngle() const;
    void setMinAngle(int value);

    int getMaxAngle() const;
    void setMaxAngle(int value);

    int getDiffAngle() const;
    void setDiffAngle(int value);

    bool getCounterClockwise() const;
    void setCounterClockwise(bool value);

  private:
    /* Individual parts of the inlet item to keep track of what the user is interacting with
     * at the moment. */
    enum parts {
        none = 0,
        center = 1,
        centerBorder = 2,
        segmentOuterBorder = 3,
        segmentRight = 4,
        segmentLeft = 5
    };
    parts partClicked = parts::none;

    /* Check if the given coordinates are inside different parts of the tool */
    bool inOriginCenter(const QPointF &pos) const;
    bool inOriginBorder(const QPointF &pos) const;
    bool inSegmentOuterBorder(const QPointF &pos) const;

    /* Returns the sector (0-7) in which a position is relative to the origin and a zero plane
     * at 12'o clock (shifted by 22.5°); sector indizes are counter clockwise */
    int inSector(const QPointF &pos) const;
    QCursor sectorCursors[8];

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

    /* This is the angular zero axis (relative to 3'o clock), and the min and max angle to draw;
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
