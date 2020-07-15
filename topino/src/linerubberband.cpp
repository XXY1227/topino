#include "include/linerubberband.h"

LineRubberBand::LineRubberBand(QWidget *parent) : TopinoRubberBand(parent) {
    /* Set standard visual appearance of this rubberband */
    terminalBrush = QBrush(QColor(0, 135, 215));
    linePen = QPen(QColor(0, 135, 215));
    lineWidth = 2;
    linePen.setWidth(lineWidth);

    setOffset(5);
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
    painter.drawLine(getRelativeSrcPoint(), getRelativeDestPoint());

    /* Drawing terminal points */
    painter.setPen(Qt::NoPen);
    painter.setBrush(terminalBrush);
    painter.drawEllipse(getRelativeSrcPoint(), getOffset(), getOffset());
    painter.drawEllipse(getRelativeDestPoint(), getOffset(), getOffset());

    painter.restore();
}
