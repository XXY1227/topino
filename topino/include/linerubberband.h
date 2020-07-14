#ifndef LINERUBBERBAND_H
#define LINERUBBERBAND_H

#include <QRubberBand>
#include <QPainter>
#include <QPaintEvent>

class LineRubberBand : public QRubberBand {
  public:
    LineRubberBand(QWidget *parent = nullptr);
    ~LineRubberBand();

    void paintEvent(QPaintEvent *event) override;

  private:
    QBrush terminalBrush;
    int offset;
    QPen linePen;
    int lineWidth;
};

#endif // LINERUBBERBAND_H
