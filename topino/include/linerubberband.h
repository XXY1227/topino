#ifndef LINERUBBERBAND_H
#define LINERUBBERBAND_H

#include <QRubberBand>
#include <QPainter>
#include <QPaintEvent>

#include "include/topinorubberband.h"

class LineRubberBand : public TopinoRubberBand {
  public:
    LineRubberBand(QWidget *parent = nullptr);
    ~LineRubberBand();

    void paintEvent(QPaintEvent *event) override;

  private:
    QBrush terminalBrush;
    QPen linePen;
    int lineWidth;
};

#endif // LINERUBBERBAND_H
