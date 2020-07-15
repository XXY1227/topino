#include "include/circlerubberband.h"

#include <QtMath>

CircleRubberBand::CircleRubberBand(QWidget *parent) : TopinoRubberBand(parent) {
    /* Set standard visual appearance of this rubberband */
    markerBrush = QBrush(QColor(0, 135, 215));
    backgroundBrush = QBrush(QColor(100, 230, 255), Qt::Dense6Pattern);
    borderPen = QPen(QColor(0, 135, 215));

    borderPen.setWidth(borderWidth);
    setOffset(borderWidth);
}


CircleRubberBand::~CircleRubberBand() {
}

void CircleRubberBand::setDestPoint(const QPoint& value) {
    /* Recalculates the geometry using the source point as center point of the surrounding rectangle
     * and the distance between destination and source point as radius */
    destPoint = value;

    circleRadius = qSqrt(qPow(srcPoint.x() - destPoint.x(), 2.0) + qPow(srcPoint.y() - destPoint.y(), 2.0));

    setGeometry(srcPoint.x() - circleRadius - offset, srcPoint.y() - circleRadius - offset,
                (circleRadius + offset) * 2, (circleRadius + offset) * 2);
}

void CircleRubberBand::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);

    /* Draw a circle inside the given square (enforced by resizeEvent) instead of the default behaviour */
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Difference);

    /* Drawing the actual circle */
    painter.setPen(borderPen);
    painter.setBrush(backgroundBrush);
    painter.drawEllipse(getRelativeSrcPoint(), circleRadius, circleRadius);

    /* Drawing the marker point in the middle of the circle (if the circle radius is large enough) */
    if (circleRadius > markerRadius) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(markerBrush);
        painter.drawEllipse(getRelativeSrcPoint(), markerRadius, markerRadius);
    }

    painter.restore();
}

