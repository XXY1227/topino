#include "include/polarcircletoolitem.h"

PolarCircleToolItem::PolarCircleToolItem(int newitemid, QGraphicsItem* parent) :
    TopinoGraphicsItem(newitemid, parent) {
    setFlags(QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemSendsGeometryChanges);

    /* Set standard angles to draw */
    zeroAngle = -90;
    minAngle = -30;
    maxAngle = +30;
    diffAngle = 15;
    counterClockwise = false;

    drawSegments = true;
    segments = 0;
    segmentSize = 0;

    /* Set standard visual appearance of this polar circle item */
    offset = 5;
    innerCircleBrush = QBrush(QColor(255,0,0, 100));
    segmentBrushOdd = QBrush(QColor(255,0,0, 50));
    segmentBrushEven = QBrush(QColor(255,0,0, 25));
    coordlinePen = QPen(QColor(215, 135, 0));
    coordlineWidth = 2;
    coordlinePen.setWidth(coordlineWidth);
}

PolarCircleToolItem::~PolarCircleToolItem() {
}

QRectF PolarCircleToolItem::boundingRect() const {
    return fullRect;
}

void PolarCircleToolItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    if (drawSegments) {
        /* Set clipping region to not draw into the inner circle; best way to preserve transparency there */
        painter->setClipping(true);
        QRegion reg(origin.x() - innerRadius, origin.y() - innerRadius,
                    2 * innerRadius, 2 * innerRadius, QRegion::Ellipse);
        painter->setClipRegion(QRegion(fullRect.toRect()).subtracted(reg));

        /* Draw the outer circle using "pies" to draw the radius lines at the same time */
        painter->setPen(coordlinePen);
        int n = (maxAngle - minAngle) / diffAngle;
        for (int p = 0; p < n; ++p) {
            if ((p % 2) == 0) {
                painter->setBrush(segmentBrushEven);
            } else {
                painter->setBrush(segmentBrushOdd);
            }

            painter->drawPie(drawRect, adjustAngleAbs(minAngle + diffAngle * p), adjustAngleRel(diffAngle));
        }

        /* Remove clipping again */
        painter->setClipRect(fullRect);
        painter->setClipping(false);

        /* Draw inner circle arcs; start at 1 and draw one less then needed since we have drawn the inner and
         * outer most circle already */
        painter->setPen(coordlinePen);
        painter->setBrush(Qt::NoBrush);
        for (int c = 1; c < segments; ++c ) {
            painter->drawArc(origin.x() - segmentSize * c, origin.y() - segmentSize * c,
                             2 * segmentSize * c, 2 * segmentSize * c,
                             adjustAngleAbs(minAngle), adjustAngleRel(maxAngle - minAngle));
        }
    }

    /* Draw the inner circle, the actual inlet (basically what the user selected with the rubber band) */
    painter->setPen(coordlinePen);
    painter->setBrush(innerCircleBrush);
    painter->drawEllipse(origin, innerRadius, innerRadius);

    /* Drawing selection rectangle */
    if (isSelected()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(penSelection);
        painter->drawPath(shape());
    }
}

QPainterPath PolarCircleToolItem::shape() const {
    /* Create the shape out of two invidual elements, first a circle for the actual inlet and then
     * an pie for the segments (if visible) */
    QPainterPath path;

    path.addEllipse(origin, innerRadius, innerRadius);

    /* Calculate the points for the bounding pie if visible */
    if (drawSegments) {
        QPainterPath pathLine;

        /* In contrast to the drawing functions, the QPainterPath.arcTo function takes
         * FULL degrees and not 1/16th of a degree */
        pathLine.moveTo(origin);
        pathLine.arcTo(drawRect, minAngle - zeroAngle, maxAngle - minAngle);
        pathLine.closeSubpath();

        path = path.united(pathLine);
    }

    /* Unify the areas from the middle part and the two circles at the end */
    return path.simplified();
}

TopinoGraphicsItem::itemtype PolarCircleToolItem::getItemType() const {
    return itemtype::inlet;
}

void PolarCircleToolItem::updateScale() {
    double scaling = getScaling();

    offset = offset * scaling;
    coordlineWidth = coordlineWidth * scaling;
    coordlinePen.setWidth(coordlineWidth);
}

QPointF PolarCircleToolItem::getOrigin() const {
    return origin;
}

void PolarCircleToolItem::setOrigin(const QPointF& value) {
    origin = value;
    calculateBoundingRect();
}

int PolarCircleToolItem::getSegments() const {
    return segments;
}

void PolarCircleToolItem::setSegments(int value) {
    segments = value;
    calculateSegmentSize();
}

bool PolarCircleToolItem::segmentsVisible() const {
    return drawSegments;
}

void PolarCircleToolItem::showSegments(bool value) {
    drawSegments = value;
}

int PolarCircleToolItem::getInnerRadius() const {
    return innerRadius;
}

void PolarCircleToolItem::setInnerRadius(int value) {
    innerRadius = value;
}

int PolarCircleToolItem::getOuterRadius() const {
    return outerRadius;
}

void PolarCircleToolItem::setOuterRadius(int value) {
    outerRadius = value;
    calculateBoundingRect();
    calculateSegmentSize();
}

void PolarCircleToolItem::calculateSegmentSize() {
    if (segments == 0) {
        /* To prevent division by zero */
        segmentSize = 0;
    } else {
        /* Segment size is simply defined by the distance of the outer and inner radius over the number of
         * segements */
        segmentSize = (outerRadius - innerRadius) / segments;
    }
}

int PolarCircleToolItem::adjustAngleAbs(int visualAngle) const {
    return adjustAngleRel(visualAngle + zeroAngle);
}

int PolarCircleToolItem::adjustAngleRel(int visualAngle) const {
    /* For painter, angles are given as 1/16th units, so multiply respectively here */
    return counterClockwise ? visualAngle * 16 : -visualAngle * 16;
}

void PolarCircleToolItem::calculateBoundingRect() {
    /* Calculates the bounding rectangle of the circle */
    fullRect = QRectF(origin.x() - outerRadius - offset, origin.y() - outerRadius - offset,
                      2 * outerRadius + 2 * offset, 2 * outerRadius + 2 * offset);
    drawRect = fullRect.adjusted(offset, offset, -offset, -offset);
}
