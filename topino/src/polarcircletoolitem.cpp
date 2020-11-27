#include "include/polarcircletoolitem.h"

PolarCircleToolItem::PolarCircleToolItem(int newitemid, QGraphicsItem* parent) :
    TopinoGraphicsItem(newitemid, parent) {
    setFlags(QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemSendsGeometryChanges);

    setAcceptHoverEvents(true);

    /* Set standard angles to draw */
    zeroAngle = 90;
    minAngle = -30;  /* the min angle _counterclockwise_ (i.e. cw boundary) */
    maxAngle = +30;  /* the max angle _counterclockwise_ (i.e. ccw boundary) */
    diffAngle = 15;

    /* This parameter is only used for _displaying_ the angle to the user */
    counterClockwise = true;

    drawSegments = true;
    segments = 0;
    segmentSize = 0;

    /* Set standard visual appearance of this polar circle item */
    offset = 5;
    QColor colorOdd = TopinoTools::colorsTableau10[1];
    colorOdd.setAlpha(50);
    QColor colorEven = TopinoTools::colorsTableau10[1].darker(250);
    colorEven.setAlpha(50);
    QColor colorInner = TopinoTools::colorsTableau10[1];
    colorInner.setAlpha(100);

    innerCircleBrush = QBrush(colorInner);
    segmentBrushOdd = QBrush(colorOdd);
    segmentBrushEven = QBrush(colorEven);

    coordlinePen = QPen(TopinoTools::colorsTableau10[1]);
    coordlineWidth = 2;
    coordlinePen.setWidth(coordlineWidth);

    grabLinePen = QPen(TopinoTools::colorsTableau10[1].darker(300));
    grabLinePen.setWidth(coordlineWidth);

    fontAngleLabels = QFont("Helvetica", 16, QFont::Bold);
    fontOutline = QPen(Qt::black);
    fontOutline.setWidth(coordlineWidth / 2);
    fontFilling = QBrush(Qt::white);

    /* Initialize the cursors used for resizing the circles */
    sectorCursors[0] = QCursor(QPixmap(":/ui/cursors/bd_double_arrow.png"), 24, 24);    /* top-left */
    sectorCursors[1] = QCursor(QPixmap(":/ui/cursors/hori_double_arrow.png"), 24, 24);  /* left */
    sectorCursors[2] = QCursor(QPixmap(":/ui/cursors/fb_double_arrow.png"), 24, 24);    /* bottom-left */
    sectorCursors[3] = QCursor(QPixmap(":/ui/cursors/vert_double_arrow.png"), 24, 24);  /* bottom */
    sectorCursors[4] = QCursor(QPixmap(":/ui/cursors/bd_double_arrow.png"), 24, 24);    /* bottom-right */
    sectorCursors[5] = QCursor(QPixmap(":/ui/cursors/hori_double_arrow.png"), 24, 24);  /* right */
    sectorCursors[6] = QCursor(QPixmap(":/ui/cursors/fb_double_arrow.png"), 24, 24);    /* top-right */
    sectorCursors[7] = QCursor(QPixmap(":/ui/cursors/vert_double_arrow.png"), 24, 24);  /* top */

    /* Initialize the cursors used for grabbing/moving the min, max, and zero angles */
    grabberCursors[0] = QCursor(QPixmap(":/ui/cursors/fb_double_arrow.png"), 24, 24);    /* top-left */
    grabberCursors[1] = QCursor(QPixmap(":/ui/cursors/vert_double_arrow.png"), 24, 24);  /* left */
    grabberCursors[2] = QCursor(QPixmap(":/ui/cursors/bd_double_arrow.png"), 24, 24);    /* bottom-left */
    grabberCursors[3] = QCursor(QPixmap(":/ui/cursors/hori_double_arrow.png"), 24, 24);  /* bottom */
    grabberCursors[4] = QCursor(QPixmap(":/ui/cursors/fb_double_arrow.png"), 24, 24);    /* bottom-right */
    grabberCursors[5] = QCursor(QPixmap(":/ui/cursors/vert_double_arrow.png"), 24, 24);  /* right */
    grabberCursors[6] = QCursor(QPixmap(":/ui/cursors/bd_double_arrow.png"), 24, 24);    /* top-right */
    grabberCursors[7] = QCursor(QPixmap(":/ui/cursors/hori_double_arrow.png"), 24, 24);  /* top */
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

        /* Calculate the fixed min and max angles - those that are fixed at "diffangles" */
        int fixMinAngle = (minAngle / diffAngle) * diffAngle;
        int fixMaxAngle = (maxAngle / diffAngle) * diffAngle;
        int n = (fixMaxAngle - fixMinAngle) / diffAngle;

        /* Select drawing tools */
        painter->setPen(coordlinePen);
        painter->setBrush(segmentBrushEven);
        int drawn = 0;

        if ((fixMinAngle / diffAngle) % 2 == 0) {
            painter->setBrush(segmentBrushOdd);
            drawn++;
        }

        /* Draw a first segment if there is any residue angle */
        if (minAngle < fixMinAngle) {
            int size = fixMinAngle - minAngle;

            /* Brush is switched and then switched back for the first peace to not disturb the order */
            painter->setBrush(drawn % 2 == 0 ? segmentBrushOdd : segmentBrushEven);
            painter->drawPie(drawRect, (zeroAngle + minAngle) * 16, 16 * size);
            painter->setBrush(drawn % 2 == 0 ? segmentBrushEven : segmentBrushOdd);
        }

        /* Draw the outer circle using "pies" to draw the radius lines at the same time */
        for (int p = 0; p < n; ++p) {
            /* This drawing function take 1/16th of an angle (given in degrees) */
            painter->drawPie(drawRect, (zeroAngle + fixMinAngle + diffAngle * p) * 16, 16 * diffAngle);
            drawn++;

            /* By setting the brush for the _next_ piece, we can draw the residue piece in the right shape */
            if ((drawn % 2) == 0) {
                painter->setBrush(segmentBrushEven);
            } else {
                painter->setBrush(segmentBrushOdd);
            }
        }

        if (maxAngle > fixMaxAngle) {
            int size = maxAngle - fixMaxAngle;
            painter->drawPie(drawRect, (zeroAngle + maxAngle) * 16, - 16 * size);
        }

        /* Draw the labels for every angle line */
        painter->setPen(Qt::black);
        painter->setFont(fontAngleLabels);
        for (int a = fixMinAngle; a <= fixMaxAngle; a+=diffAngle) {
            drawAngleLabel(painter, a);
        }

        /* Remove clipping again */
        painter->setClipRect(fullRect);
        painter->setClipping(false);

        /* Draw inner circle arcs; start at 1 and draw one less then needed since we have drawn the inner and
         * outer most circle already */
        painter->setPen(coordlinePen);
        painter->setBrush(Qt::NoBrush);
        for (int c = 1; c < segments; ++c ) {
            /* These drawing function take 1/16th of an angle (given in degrees) */
            painter->drawArc(origin.x() - innerRadius - segmentSize * c, origin.y() - innerRadius - segmentSize * c,
                             2 * segmentSize * c + innerRadius * 2, 2 * segmentSize * c + innerRadius * 2,
                             16 * (zeroAngle + minAngle), 16 * (maxAngle - minAngle));
        }

        /* Draw "grabbing" lines at the outer and the neutral line ONLY if the inlet is selected */
        if (isSelected()) {
            painter->setPen(grabLinePen);

            /* Draw new outer lines and a new zero line that is darker to indicate interaction */
            painter->drawLine(polarToCartesianCoords(minAngle, innerRadius), polarToCartesianCoords(minAngle, outerRadius));
            painter->drawLine(polarToCartesianCoords(       0, innerRadius), polarToCartesianCoords(       0, outerRadius));
            painter->drawLine(polarToCartesianCoords(maxAngle, innerRadius), polarToCartesianCoords(maxAngle, outerRadius));

            /* Draw a new outer arc to indicate interaction */
            painter->drawArc(drawRect, (zeroAngle + fixMinAngle) * 16, 16 * (fixMaxAngle - fixMinAngle));
        }
    }

    /* Draw the inner circle, the actual inlet (basically what the user selected with the rubber band) */
    painter->setPen(coordlinePen);
    if (isSelected()) {
        /* Use the darker pen for the inner circle if the inlet is selected
         * to indicate interaction */
        painter->setPen(grabLinePen);
    }
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
        pathLine.arcTo(drawRect, zeroAngle + minAngle, maxAngle - minAngle);
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
    grabLinePen.setWidth(coordlineWidth);

    fontOutline.setWidth(coordlineWidth / 2);
    fontAngleLabels.setPointSizeF(fontAngleLabels.pointSizeF() * scaling);
    prepareGeometryChange();
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
        } else if (inAngleGrabber(pos, minAngle)) {
            partClicked = parts::grabAngleMin;
        } else if (inAngleGrabber(pos, maxAngle)) {
            partClicked = parts::grabAngleMax;
        } else if (inAngleGrabber(pos, 0)) {
            partClicked = parts::grabAngleZero;
        }
    }

    /* Continue with processing (this handles selection etc.) */
    TopinoGraphicsItem::mousePressEvent(event);
}

void PolarCircleToolItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    /* Calculcate distance from origin point */
    QPointF polar = cartesianToPolarCoords(event->pos());
    int angle = (int)polar.x();
    int anglefixed = (angle / diffAngle) * diffAngle;
    int distance = (int)polar.y();

    /* Depending on which part clicked, the position of the points is updated */
    switch (partClicked) {
    case parts::center:
        origin = event->pos();
        prepareGeometryChange();
        break;
    case parts::centerBorder:
        if ((distance > 0) && (!drawSegments || (distance < outerRadius))) {
            innerRadius = distance;
        }
        prepareGeometryChange();
        break;
    case parts::segmentOuterBorder:
        if (distance > innerRadius) {
            outerRadius = distance;
        }
        prepareGeometryChange();
        break;
    case parts::grabAngleZero:
        if (event->modifiers() == Qt::ShiftModifier) {
            zeroAngle += angle;
        } else {
            zeroAngle += anglefixed;
        }
        if (zeroAngle > 360) {
            zeroAngle -= 360;
        } else if (zeroAngle < 0) {
            zeroAngle += 360;
        }
        prepareGeometryChange();
        break;
    case parts::grabAngleMin:
        if ((anglefixed >= -180) && (anglefixed < 0)) {
            if (event->modifiers() == Qt::ShiftModifier) {
                minAngle = angle;
            } else {
                minAngle = anglefixed;
            }
        }
        prepareGeometryChange();
        break;
    case parts::grabAngleMax:
        if ((anglefixed <= 180) && (anglefixed > 0)) {
            if (event->modifiers() == Qt::ShiftModifier) {
                maxAngle = angle;
            } else {
                maxAngle = anglefixed;
            }
        }
        prepareGeometryChange();
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
    } else if (inAngleGrabber(pos, 0) || inAngleGrabber(pos, minAngle) || inAngleGrabber(pos, maxAngle)) {
        setCursor(grabberCursors[inSector(pos)]);
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
    prepareGeometryChange();
}

int PolarCircleToolItem::getSegments() const {
    return segments;
}

void PolarCircleToolItem::setSegments(int value) {
    segments = value;
    calculateSegmentSize();
    prepareGeometryChange();
}

bool PolarCircleToolItem::segmentsVisible() const {
    return drawSegments;
}

void PolarCircleToolItem::showSegments(bool value) {
    drawSegments = value;
    prepareGeometryChange();
}

int PolarCircleToolItem::getInnerRadius() const {
    return innerRadius;
}

void PolarCircleToolItem::setInnerRadius(int value) {
    innerRadius = value;
    calculateBoundingRect();
    calculateSegmentSize();
    prepareGeometryChange();
}

int PolarCircleToolItem::getOuterRadius() const {
    return outerRadius;
}

void PolarCircleToolItem::setOuterRadius(int value) {
    /* Outer radius should be larger than the inner radius */
    if (value > innerRadius) {
        outerRadius = value;
        calculateBoundingRect();
        calculateSegmentSize();
    }
    prepareGeometryChange();
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
    prepareGeometryChange();
}

QString PolarCircleToolItem::toString() const {
    QPoint pt = origin.toPoint();
    QString text = QString("Inlet: (%1, %2) with %3 pixel inner radius").arg(pt.x()).arg(pt.y()).arg(innerRadius);

    if (drawSegments) {
        text += "\t" + QString("defines coord system (ref: %1°, range: %2° – %3° %4, %5 pixel outer radius)")
                .arg(zeroAngle)
                .arg(counterClockwise ? minAngle : -maxAngle)
                .arg(counterClockwise ? maxAngle : -minAngle)
                .arg(counterClockwise ? tr("ccw") : tr("cw"))
                .arg(outerRadius);
    }
    return text;
}

void PolarCircleToolItem::fromString(const QString& value) {
    Q_UNUSED(value)

    /* TODO: Implement */
}

QLineF PolarCircleToolItem::getZeroLine() const {
    /* First point is always the origin; second point calculcated
     * by angle and max radius */
    return QLineF(origin, polarToCartesianCoords(0, outerRadius));
}

QLineF PolarCircleToolItem::getMinLine() const {
    /* First point is always the origin; second point calculcated
     * by angle and max radius */
    return QLineF(origin, polarToCartesianCoords(minAngle, outerRadius));
}

QLineF PolarCircleToolItem::getMaxLine() const {
    /* First point is always the origin; second point calculcated
     * by angle and max radius */
    return QLineF(origin, polarToCartesianCoords(maxAngle, outerRadius));
}

QRectF PolarCircleToolItem::getInnerRect() const {
    return QRectF(origin.x() - innerRadius, origin.y() - innerRadius, 2 * innerRadius, 2 * innerRadius);
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

bool PolarCircleToolItem::inAngleGrabber(const QPointF& pos, int angle) const {
    /* Set up the polygon to check for interaction */
    QPolygonF polygon;
    polygon.append(polarToCartesianCoords(angle - 1, innerRadius));
    polygon.append(polarToCartesianCoords(angle - 1, outerRadius));
    polygon.append(polarToCartesianCoords(angle + 1, outerRadius));
    polygon.append(polarToCartesianCoords(angle + 1, innerRadius));
    polygon.append(polarToCartesianCoords(angle - 1, innerRadius));

    /* While the polygon class has a "contains" function, it is important to use
     * this specific one here - otherwise it will practically always return false */
    return polygon.containsPoint(pos, Qt::OddEvenFill);
}

int PolarCircleToolItem::inSector(const QPointF& pos) const {
    /* Position is relative to the origin */
    QPointF relPos = pos - origin;
    double length = qSqrt(qPow(relPos.x(), 2.0) + qPow(relPos.y(), 2.0));

    /* Get angle relative to 12'o clock */
    double angle = ((relPos.x() > 0)/0.5 - 1) *
                   qRadiansToDegrees(qAcos(QPointF::dotProduct(relPos, QPointF(0.0, 1.0)) / (length * 1.0))) + 180.0;

    /* Shift angle by 22.5 */
    angle -= 22.5;

    if (angle > 360.0) {
        angle -= 360.0;
    } else if (angle < 0.0) {
        angle += 360.0;
    }

    /* Every 45° there is a sector (0-7), 0 is top-left, 1 is left, ... 7 is top;
     * the modulo will make sure that only values of 0 to 7 are returned */
    return int(angle / 45.0) % 8;
}

void PolarCircleToolItem::drawAngleLabel(QPainter* painter, int angle) {
    /* Center point of label to draw */
    QPointF pt = polarToCartesianCoords(angle, 0.9 * outerRadius);

    /* Prepare label text with respective signs (the minus is actually a minus not a dash!) */
    QString label = ((angle == 0) ? "±" : ((angle < 0) ? "+" : "−")) + QString::number(qAbs(angle));
    if (counterClockwise) {
        label = ((angle == 0) ? "±" : ((angle < 0) ? "−" : "+")) + QString::number(qAbs(angle));
    }

    /* Calculate font metrics */
    QFontMetrics fm(painter->font());
    int half_width = fm.width(label) / 2;

    /* Draw the text */
    QPainterPath fontPath;
    fontPath.addText(pt.x() - half_width, pt.y(), fontAngleLabels, label);
    painter->setPen(fontOutline);
    painter->setBrush(fontFilling);
    painter->drawPath(fontPath);
}

QPointF PolarCircleToolItem::polarToCartesianCoords(int angle, int radius) const {
    /* We need the angle in radians */
    double angleRadians = qDegreesToRadians((double)(zeroAngle + angle));

    /* Point relative to the origin; y needs to be negative since it is from up to down */
    QPointF rel(radius * qCos(angleRadians), - radius * qSin(angleRadians));

    /* Return absolute point */
    return origin + rel;
}

QPointF PolarCircleToolItem::cartesianToPolarCoords(const QPointF& pos) const {
    /* Relative point to the origin, rotate by reference plane */
    QTransform matrix;
    matrix.rotate(zeroAngle);
    QPointF relPos = (pos - origin) * matrix;

    /* Length of this relative point is the radius */
    double radius = qSqrt(qPow(relPos.x(), 2.0) + qPow(relPos.y(), 2.0));

    /* Get the angle relative to the zero angle */
    double angle = 0;

    if (relPos.x() == 0 ) {
        relPos.setX(0.00001);
    }

    angle = qRadiansToDegrees(qAtan(-relPos.y() / relPos.x()));
    if ((relPos.x() < 0) && (-relPos.y() > 0)) {
        angle += 180;
    } else if ((relPos.x() < 0) && (-relPos.y() <= 0)) {
        angle -= 180;
    }

    /* Return the angle in "x" of the point and the radius in "y" of the point;
     * in future, we might just add a polar point class */
    return QPointF(angle, radius);
}

int PolarCircleToolItem::getDiffAngle() const {
    return diffAngle;
}

void PolarCircleToolItem::setDiffAngle(int value) {
    diffAngle = value;
    prepareGeometryChange();
}

int PolarCircleToolItem::getMaxAngle() const {
    return maxAngle;
}

void PolarCircleToolItem::setMaxAngle(int value) {
    /* The new maximum angle should be larger than zero and
     * larger than the minimum angle */
    if ((value > 0) && (value > minAngle) && (value <= 180)) {
        maxAngle = value;
    }
    prepareGeometryChange();
}

int PolarCircleToolItem::getMinAngle() const {
    return minAngle;
}

void PolarCircleToolItem::setMinAngle(int value) {
    /* The new minimum angle should be smaller than zero and
     * smaller than the maximum angle */
    if ((value < 0) && (value < maxAngle) && (value >= -180)) {
        minAngle = value;
    }
    prepareGeometryChange();
}

int PolarCircleToolItem::getZeroAngle() const {
    return zeroAngle;
}

void PolarCircleToolItem::setZeroAngle(int value) {
    /* Only allow values between -359 and +359 */
    if (value > -359 && value < 359) {
        zeroAngle = value;
    }
    prepareGeometryChange();
}

void PolarCircleToolItem::calculateBoundingRect() {
    /* Calculates the bounding rectangle of the circle */
    fullRect = QRectF(origin.x() - outerRadius - offset, origin.y() - outerRadius - offset,
                      2 * outerRadius + 2 * offset, 2 * outerRadius + 2 * offset);
    drawRect = fullRect.adjusted(offset, offset, -offset, -offset);
}
