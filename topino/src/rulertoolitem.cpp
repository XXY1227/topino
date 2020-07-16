#include "include/rulertoolitem.h"

RulerToolItem::RulerToolItem(QGraphicsItem* parent) : QGraphicsLineItem(parent) {
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);

    /* Set standard visual appearance of this rubberband */
    terminalBrush = QBrush(QColor(0, 135, 215));
    linePen = QPen(QColor(0, 135, 215));
    lineWidth = 2;
    linePen.setWidth(lineWidth);

    offset = 5;
}

RulerToolItem::~RulerToolItem() {

}

QRectF RulerToolItem::boundingRect() const {
    QPoint p1 = line().toLine().p1();
    QPoint p2 = line().toLine().p2();

    return QRectF(qMin(p1.x(), p2.x()) - offset, qMin(p1.y(), p2.y()) - offset,
                  qAbs(p1.x() - p2.x()) + 2 * offset, qAbs(p1.y() - p2.y()) + 2 * offset);
}

bool RulerToolItem::contains(const QPointF& point) const {
    return inTerminalPoint1(point.toPoint()) ||
           inTerminalPoint2(point.toPoint()) ||
           QGraphicsLineItem::contains(point);
}

void RulerToolItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    /* Draw a circle inside the given square (enforced by resizeEvent) instead of the default behaviour */
    painter->setRenderHint(QPainter::Antialiasing);

    QPoint p1 = line().toLine().p1();
    QPoint p2 = line().toLine().p2();

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
        painter->setPen(QPen(Qt::white, 1, Qt::DashLine));
        painter->drawPolygon(boundingRect());
    }
}

QPainterPath RulerToolItem::shape() const {
    QPoint p1 = line().toLine().p1();
    QPoint p2 = line().toLine().p2();
    QPainterPath path;

    path.addEllipse(p1, offset, offset);
    path.addEllipse(p2, offset, offset);
    path.moveTo(p1);
    path.lineTo(p2);

    return path;
}

bool RulerToolItem::inTerminalPoint1(const QPoint& point) const {
    return QRect(0, 0, 2 * offset, 2 * offset).contains(point);
}

bool RulerToolItem::inTerminalPoint2(const QPoint& point) const {
    return QRect(boundingRect().width() - 2 * offset, boundingRect().height() - 2 * offset,
                 boundingRect().width(), boundingRect().height()).contains(point);
}

