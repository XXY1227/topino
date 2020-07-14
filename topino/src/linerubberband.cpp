#include "include/linerubberband.h"

LineRubberBand::LineRubberBand(QWidget *parent) : QRubberBand(QRubberBand::Line, parent) {
    /* Set standard visual appearance of this rubberband */
    terminalBrush = QBrush(QColor(0, 135, 215));
    offset = 5;
    linePen = QPen(QColor(0, 135, 215));
    lineWidth = 2;
    linePen.setWidth(lineWidth);
}

LineRubberBand::~LineRubberBand() {

}

void LineRubberBand::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);

    /* Draw a circle inside the given square (enforced by resizeEvent) instead of the default behaviour */
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Difference);

    /* Drawing line */
    painter.setPen(linePen);
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(rect().topLeft() + QPoint(offset, offset), rect().bottomRight() - QPoint(offset, offset));

    /* Drawing terminal points */
    painter.setPen(Qt::NoPen);
    painter.setBrush(terminalBrush);
    painter.drawEllipse(rect().topLeft() + QPoint(offset, offset), offset, offset);
    painter.drawEllipse(rect().bottomRight() - QPoint(offset, offset), offset, offset);

    painter.restore();
}
