#include "include/topinorubberband.h"

TopinoRubberBand::TopinoRubberBand(QWidget *parent) : QRubberBand(QRubberBand::Rectangle, parent) {
}

TopinoRubberBand::~TopinoRubberBand() {
}

QPoint TopinoRubberBand::getSrcPoint() const {
    return srcPoint;
}

void TopinoRubberBand::setSrcPoint(const QPoint& value) {
    srcPoint = value;
    destPoint = value;
}

QPoint TopinoRubberBand::getDestPoint() const {
    return destPoint;
}

void TopinoRubberBand::setDestPoint(const QPoint& value) {
    /* Recalculates the geometry based on source and destination point using the given offset */
    destPoint = value;

    setGeometry(qMin(srcPoint.x(), destPoint.x()) - offset, qMin(srcPoint.y(), destPoint.y()) - offset,
                qAbs(srcPoint.x() - destPoint.x()) + 2 * offset, qAbs(srcPoint.y() - destPoint.y()) + 2 * offset);
}

int TopinoRubberBand::getOffset() const {
    return offset;
}

void TopinoRubberBand::setOffset(int value) {
    offset = value;
}

QPoint TopinoRubberBand::getRelativeSrcPoint() const {
    return srcPoint - pos();
}

QPoint TopinoRubberBand::getRelativeDestPoint() const {
    return destPoint - pos();
}


