#include "include/rulertoolitem.h"

RulerToolItem::RulerToolItem(int newitemid, QGraphicsItem* parent) : TopinoGraphicsItem(newitemid, parent) {
    setFlags(QGraphicsItem::ItemIsSelectable |
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
    return shape().boundingRect();
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
            partClicked = parts::point1;
        } else if (inTerminalPoint(line.p2(), event->pos())) {
            partClicked = parts::point2;
        }
    }

    /* Continue with processing (this handles selection etc.) */
    TopinoGraphicsItem::mousePressEvent(event);
}


void RulerToolItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    QPointF pos = event->pos();

    /* Depending on which part clicked, the position of the points is updated; let's
     * not forget to call prepareGeometryChange() here - otherwise, the scene does
     * not update properly and the ruler item becomes unmovable at some point (also,
     * it will draw artifacts, etc.)! */
    switch (partClicked) {
    case parts::point1:
        if (isTerminalPointInsideScene(pos)) {
            line.setP1(pos);
            prepareGeometryChange();
        }
        break;
    case parts::point2:
        if (isTerminalPointInsideScene(pos)) {
            line.setP2(pos);
            prepareGeometryChange();
        }
        break;
    default:
        /* If clicked just on the line part, run the default movement-code */
        TopinoGraphicsItem::mouseMoveEvent(event);
        break;
    }

    /* Update the scene (drawing etc.) and emit signal to view*/
    scene()->update();
    emit itemPosChanged(this);
}

void RulerToolItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    /* Release the part clicked */
    partClicked = parts::none;

    /* Check the order of the terminal points and swap if necessary */
    checkTerminalPointsOrder();

    /* Send notice that the data of this tool changed */
    emit itemDataChanged(this);

    /* Process all release events */
    TopinoGraphicsItem::mouseReleaseEvent(event);
}

void RulerToolItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
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

void RulerToolItem::swapTerminalPoints() {
    QPointF tempP1 = line.p1();
    QPointF tempP2 = line.p2();
    line.setPoints(tempP2, tempP1);
}

void RulerToolItem::checkTerminalPointsOrder() {
    /* Check if point2 is now left and/or top of point1. If yes,
     * swap the points. */
    if ((line.p2().x() < line.p1().x()) || (line.p2().y() < line.p1().y())) {
        swapTerminalPoints();
    }
}

bool RulerToolItem::isTerminalPointInsideScene(const QPointF& pos) {
    /* Creating a rectangle around the terminal point; then, we only have to check
     * if the top-left point and the bottom-right point are inside the scene
     * rect (since both rectangles are aligned). */
    QRectF terminalRect = QRectF(pos - QPointF(offset, offset), pos + QPointF(offset, offset));
    QRectF sceneRect = scene()->sceneRect();

    return (sceneRect.contains(terminalRect.topLeft()) && sceneRect.contains(terminalRect.bottomRight()));
}

