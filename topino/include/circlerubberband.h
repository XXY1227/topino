#ifndef CIRCLERUBBERBAND_H
#define CIRCLERUBBERBAND_H

#include <QRubberBand>

class CircleRubberBand : public QRubberBand {
  public:
    CircleRubberBand(QWidget *parent = nullptr);
    ~CircleRubberBand();

    void setGeometry(const QRect &geom);
    void setGeometry(int x, int y, int w, int h);

    void paintEvent(QPaintEvent *event) override;
};

#endif // CIRCLERUBBERBAND_H
