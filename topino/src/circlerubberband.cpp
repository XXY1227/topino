#include "include/circlerubberband.h"

#include <QStyle>
#include <QCommonStyle>
#include <QStylePainter>
#include <QStyleOptionRubberBand>

CircleRubberBand::CircleRubberBand(QWidget *parent) : QRubberBand(QRubberBand::Rectangle, parent) {

}


CircleRubberBand::~CircleRubberBand() {

}

void CircleRubberBand::setGeometry(const QRect& geom) {
    setGeometry(geom.x(), geom.y(), geom.width(), geom.height());
}

void CircleRubberBand::setGeometry(int x, int y, int w, int h) {
    /* To create a (perfect) circle, the surrounding geometry must be a square; this is enforced here
     * by using the max of width or height for both the width and the height; maximum of both feels
     * more natural when selecting than the minimum */
    QRubberBand::setGeometry(x, y, qMax(w, h), qMax(w, h));
}

void CircleRubberBand::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QStylePainter painter(this);
    QStyleOptionRubberBand option;
    initStyleOption(&option);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(QColor(0, 135, 215));
    pen.setWidth(3);
    painter.setPen(pen);
    painter.setBrush(QBrush(QColor(100, 190, 230), Qt::Dense5Pattern));
    painter.drawEllipse(rect().adjusted(3, 3, -3, -3));
    painter.restore();
}
