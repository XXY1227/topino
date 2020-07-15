#ifndef CIRCLERUBBERBAND_H
#define CIRCLERUBBERBAND_H

#include <QRubberBand>
#include <QPainter>
#include <QPaintEvent>

#include "include/topinorubberband.h"

class CircleRubberBand : public TopinoRubberBand {
  public:
    CircleRubberBand(QWidget *parent = nullptr);
    ~CircleRubberBand();

    void setDestPoint(const QPoint& value) override;

    void paintEvent(QPaintEvent *event) override;

  private:
    QBrush markerBrush;
    QBrush backgroundBrush;
    QPen borderPen;
    int circleRadius = 0;
    int markerRadius = 10;
    int borderWidth = 2;
};

#endif // CIRCLERUBBERBAND_H
