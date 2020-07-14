#ifndef CIRCLERUBBERBAND_H
#define CIRCLERUBBERBAND_H

#include <QRubberBand>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

class CircleRubberBand : public QRubberBand {
  public:
    CircleRubberBand(QWidget *parent = nullptr);
    ~CircleRubberBand();

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private:
    QBrush backgroundBrush;
    QPen borderPen;
    int borderWidth;
};

#endif // CIRCLERUBBERBAND_H
