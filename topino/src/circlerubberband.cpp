#include "include/circlerubberband.h"

CircleRubberBand::CircleRubberBand(QWidget *parent) : QRubberBand(QRubberBand::Rectangle, parent) {
    /* Set standard visual appearance of this rubberband */
    backgroundBrush = QBrush(QColor(100, 230, 255), Qt::Dense6Pattern);
    borderPen = QPen(QColor(0, 135, 215));
    borderWidth = 2;
    borderPen.setWidth(borderWidth);
}


CircleRubberBand::~CircleRubberBand() {
}

void CircleRubberBand::paintEvent(QPaintEvent* event) {    
    Q_UNUSED(event);
    QPainter painter(this);

    /* Draw a circle inside the given square (enforced by resizeEvent) instead of the default behaviour */
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Difference);
    painter.setPen(borderPen);
    painter.setBrush(backgroundBrush);
    painter.drawEllipse(rect().adjusted(borderWidth, borderWidth, -borderWidth, -borderWidth));
    painter.restore();
}

void CircleRubberBand::resizeEvent(QResizeEvent* event) {
    /* To create a (perfect) circle, the surrounding geometry must be a square; this is enforced here
     * by using the max of width and height for both the width and the height; maximum of both feels
     * more natural when selecting than the minimum */
    if (event->size().width() != event->size().height()) {
        int newsize = qMax(event->size().width(), event->size().height());

        /* We do not want to further process this event; resizing the rubberband will fire a new event */
        resize(newsize, newsize);
        return;
    }

    /* Only do full resizing when both width and height are equal (avoid unnecessary redrawing, hopefully) */
    QRubberBand::resizeEvent(event);
}
