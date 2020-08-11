#include "include/rulertoolitem.h"

RulerToolItem::RulerToolItem(int newitemid, QGraphicsItem* parent) : TopinoGraphicsItem(newitemid, parent) {
    setFlags(QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemSendsGeometryChanges);

    setAcceptHoverEvents(true);

    /* Set standard visual appearance of this rubberband */
    terminalBrush = QBrush(QColor(0, 135, 215, 128));
    linePen = QPen(QColor(0, 135, 215));
    lineWidth = 2;
    linePen.setWidth(lineWidth);

    offset = 5;
}

RulerToolItem::~RulerToolItem() {
}

QRectF RulerToolItem::boundingRect() const {
    QPoint p1 = getLine().toLine().p1();
    QPoint p2 = getLine().toLine().p2();

    return QRectF(qMin(p1.x(), p2.x()) - offset, qMin(p1.y(), p2.y()) - offset,
                  qAbs(p1.x() - p2.x()) + 2 * offset, qAbs(p1.y() - p2.y()) + 2 * offset);
}

void RulerToolItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    QPoint p1 = line.toLine().p1();
    QPoint p2 = line.toLine().p2();

    /* Drawing line */
    painter->setPen(linePen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(p1, p2);

    /* Drawing terminal points */
    painter->setPen(Qt::NoPen);
    painter->setBrush(terminalBrush);
    painter->drawEllipse(p1, offset, offset);
    painter->drawEllipse(p2, offset, offset);

    /* Drawing selection rectangle */
    if (isSelected()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(penSelection);
        painter->drawPath(shape());
    }
}

QPainterPath RulerToolItem::shape() const {
    /* Create the shape out of the invidual elements, first the two circles at the end */
    QPainterPath path;
    QPoint p1 = line.toLine().p1();
    QPoint p2 = line.toLine().p2();

    path.addEllipse(p1, offset, offset);
    path.addEllipse(p2, offset, offset);

    /* Calculate the points for the bounding rectangle of the line; it is a little bit
     * wider than the actual line, so that the user does not exactly have to click on
     * the 1px-line */
    QPainterPath pathLine;
    double radAngle = line.angle() * M_PI / 180;
    double dx = offset * sin(radAngle);
    double dy = offset * cos(radAngle);

    pathLine.moveTo(p1.x() + dx, p1.y() + dy);
    pathLine.lineTo(p2.x() + dx, p2.y() + dy);
    pathLine.lineTo(p2.x() - dx, p2.y() - dy);
    pathLine.lineTo(p1.x() - dx, p1.y() - dy);

    /* Unify the areas from the middle part and the two circles at the end */
    return path.united(pathLine).simplified();
}

QLineF RulerToolItem::getLine() const {
    return line;
}

void RulerToolItem::setLine(const QLineF& value) {
    line = value;
}

TopinoGraphicsItem::itemtype RulerToolItem::getItemType() const {
    return itemtype::ruler;
}

void RulerToolItem::updateScale() {
    double scaling = getScaling();

    offset = offset * scaling;
    lineWidth = lineWidth * scaling;
    linePen.setWidth(lineWidth);
}

void RulerToolItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    /* Clicked in one of the terminal points of the ruler? Then remember this
     * for future mouse events */
    if (event->buttons() & Qt::LeftButton) {
        if (inTerminalPoint(line.p1(), event->pos())) {
            qDebug("clicked in first point");
            partClicked = parts::point1;
        } else if (inTerminalPoint(line.p2(), event->pos())) {
            qDebug("clicked in second point");
            partClicked = parts::point2;
        }
    }

    /* Continue with processing (this handles selection etc.) */
    TopinoGraphicsItem::mousePressEvent(event);
}


void RulerToolItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    /* Depending on which part clicked, the position of the points is updated */
    switch (partClicked) {
    case parts::point1:
        line.setP1(event->pos());
        break;
    case parts::point2:
        line.setP2(event->pos());
        break;
    default:
        /* If clicked just on the line part, run the default movement-code */
        TopinoGraphicsItem::mouseMoveEvent(event);
        break;
    }

    /* Update the scene (drawing etc.) and emit signal to view*/
    scene()->update();
    emit itemHasChanged(this);
}

void RulerToolItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    /* Release the part clicked; return mouse cursor to normal version */
    partClicked = parts::none;

    /* Process all release events */
    TopinoGraphicsItem::mouseReleaseEvent(event);
}

void RulerToolItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    TopinoGraphicsItem::hoverMoveEvent(event);

    /* Adjust cursor when over terminal points to a grabbing hand / 4-direction-moving arrow
     * depending on platform */
    if (inTerminalPoint(line.p1(), event->pos()) || inTerminalPoint(line.p2(), event->pos())) {
        setCursor(QCursor(Qt::SizeAllCursor));
    } else {
        setCursor(QCursor(Qt::ArrowCursor));
    }
}

bool RulerToolItem::inTerminalPoint(const QPointF& termPoint, const QPointF& pos) const {
    return QRectF(termPoint.x() - offset, termPoint.y() - offset, 2 * offset, 2 * offset).contains(pos);
}


