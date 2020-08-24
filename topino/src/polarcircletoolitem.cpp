#include "include/polarcircletoolitem.h"

PolarCircleToolItem::PolarCircleToolItem(int newitemid, QGraphicsItem* parent) :
    TopinoGraphicsItem(newitemid, parent) {
    setFlags(QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemSendsGeometryChanges);

    setAcceptHoverEvents(true);

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

    /* Initialize the cursors used for resizing the circles */
    sectorCursors[0] = QCursor(QPixmap(":/ui/cursors/bd_double_arrow.png"), 24, 24);    /* top-left */
    sectorCursors[1] = QCursor(QPixmap(":/ui/cursors/hori_double_arrow.png"), 24, 24);  /* left */
    sectorCursors[2] = QCursor(QPixmap(":/ui/cursors/fb_double_arrow.png"), 24, 24);    /* bottom-left */
    sectorCursors[3] = QCursor(QPixmap(":/ui/cursors/vert_double_arrow.png"), 24, 24);  /* bottom */
    sectorCursors[4] = QCursor(QPixmap(":/ui/cursors/bd_double_arrow.png"), 24, 24);    /* bottom-right */
    sectorCursors[5] = QCursor(QPixmap(":/ui/cursors/hori_double_arrow.png"), 24, 24);  /* right */
    sectorCursors[6] = QCursor(QPixmap(":/ui/cursors/fb_double_arrow.png"), 24, 24);    /* top-right */
    sectorCursors[7] = QCursor(QPixmap(":/ui/cursors/vert_double_arrow.png"), 24, 24);  /* top */
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
            painter->drawArc(origin.x() - innerRadius - segmentSize * c, origin.y() - innerRadius - segmentSize * c,
                             2 * segmentSize * c + innerRadius * 2, 2 * segmentSize * c + innerRadius * 2,
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

void PolarCircleToolItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QPointF pos = event->pos();

    /* Clicked in one of the terminal points of the ruler? Then remember this
     * for future mouse events */
    if (event->buttons() & Qt::LeftButton) {
        if (inOriginCenter(pos)) {
            partClicked = parts::center;
        } else if (inOriginBorder(pos)) {
            partClicked = parts::centerBorder;
        } else if (inSegmentOuterBorder(pos)) {
            partClicked = parts::segmentOuterBorder;
        }
    }

    /* Continue with processing (this handles selection etc.) */
    TopinoGraphicsItem::mousePressEvent(event);
}

void PolarCircleToolItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    /* Calculcate distance from origin point */
    double distance = qSqrt(qPow(origin.x() - event->pos().x(), 2.0) + qPow(origin.y() - event->pos().y(), 2.0));

    /* Depending on which part clicked, the position of the points is updated */
    switch (partClicked) {
    case parts::center:
        origin = event->pos();
        break;
    case parts::centerBorder:
        if ((distance > 0) && (!drawSegments || (distance < outerRadius))) {
            innerRadius = distance;
        }
        break;
    case parts::segmentOuterBorder:
        if (distance > innerRadius) {
            outerRadius = distance;
        }
        break;
    default:
        /* If clicked just on the line part, run the default movement-code */
        TopinoGraphicsItem::mouseMoveEvent(event);
        break;
    }

    /* Update segments here, regardless off they are drawn/showing or not */
    calculateBoundingRect();
    calculateSegmentSize();

    /* Update the scene (drawing etc.) and emit signal to view*/
    scene()->update();
    emit itemPosChanged(this);
}

void PolarCircleToolItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    /* Release the part clicked */
    partClicked = parts::none;

    /* Send notice that the data of this tool changed */
    emit itemDataChanged(this);

    /* Continue with processing (this handles selection etc.) */
    TopinoGraphicsItem::mouseReleaseEvent(event);
}

void PolarCircleToolItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    QPointF pos = event->pos();
    TopinoGraphicsItem::hoverMoveEvent(event);

    /* Adjust cursor when over terminal points to a grabbing hand / 4-direction-moving arrow
     * depending on platform */
    if (inOriginCenter(pos)) {
        setCursor(QCursor(Qt::SizeAllCursor));
    } else if (inOriginBorder(pos) || inSegmentOuterBorder(pos)) {
        setCursor(sectorCursors[inSector(pos)]);
    } else {
        setCursor(QCursor(Qt::ArrowCursor));
    }
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
    calculateBoundingRect();
    calculateSegmentSize();
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

bool PolarCircleToolItem::getCounterClockwise() const {
    return counterClockwise;
}

void PolarCircleToolItem::setCounterClockwise(bool value) {
    counterClockwise = value;
}

bool PolarCircleToolItem::inOriginCenter(const QPointF& pos) const {
    /* Origin center is defined as inside the circle with half radius */
    return (qSqrt(qPow(origin.x() - pos.x(), 2.0) + qPow(origin.y() - pos.y(), 2.0)) <= (innerRadius/2.0));
}

bool PolarCircleToolItem::inOriginBorder(const QPointF& pos) const {
    /* Origin center border is around the the innerRadius +- linewidth */
    double distance = qSqrt(qPow(origin.x() - pos.x(), 2.0) + qPow(origin.y() - pos.y(), 2.0));
    return (distance >= (innerRadius - coordlineWidth) && (distance <= (innerRadius + coordlineWidth)));
}

bool PolarCircleToolItem::inSegmentOuterBorder(const QPointF& pos) const {
    /* Origin center border is around the the outerRadius +- linewidth */
    double distance = qSqrt(qPow(origin.x() - pos.x(), 2.0) + qPow(origin.y() - pos.y(), 2.0));
    return (distance >= (outerRadius - coordlineWidth) && (distance <= (outerRadius + coordlineWidth)));

}

int PolarCircleToolItem::inSector(const QPointF& pos) const {
    /* Position is relative to the origin */
    QPointF relPos = pos - origin;
    double length = qSqrt(qPow(relPos.x(), 2.0) + qPow(relPos.y(), 2.0));

    /* Get angle relative to 12'o clock */
    double angle = (int(relPos.x() > 0)/0.5 - 1) * qAcos(QPointF::dotProduct(relPos, QPointF(0.0, 1.0)) /
                   (length * 1.0)) * 180.0 / M_PI + 180.0;

    /* Shift angle by 22.5 */
    angle -= 22.5;

    qDebug("pure angle %.f", angle);

    if (angle > 360.0) {
        angle -= 360.0;
    } else if (angle < 0.0) {
        angle += 360.0;
    }

    qDebug("adapted angle %.f", angle);

    /* Every 45° there is a sector (0-7), 0 is top-left, 1 is left, ... 7 is top;
     * the modulo will make sure that only values of 0 to 7 are returned */
    return int(angle / 45.0) % 8;
}

int PolarCircleToolItem::getDiffAngle() const {
    return diffAngle;
}

void PolarCircleToolItem::setDiffAngle(int value) {
    diffAngle = value;
}

int PolarCircleToolItem::getMaxAngle() const {
    return maxAngle;
}

void PolarCircleToolItem::setMaxAngle(int value) {
    maxAngle = value;
}

int PolarCircleToolItem::getMinAngle() const {
    return minAngle;
}

void PolarCircleToolItem::setMinAngle(int value) {
    minAngle = value;
}

int PolarCircleToolItem::getZeroAngle() const {
    return zeroAngle;
}

void PolarCircleToolItem::setZeroAngle(int value) {
    zeroAngle = value;
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
