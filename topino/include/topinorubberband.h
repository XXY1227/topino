#ifndef TOPINORUBBERBAND_H
#define TOPINORUBBERBAND_H

#include <QRubberBand>

class TopinoRubberBand : public QRubberBand {
  public:
    TopinoRubberBand(QWidget *parent = nullptr);
    ~TopinoRubberBand();

    QPoint getSrcPoint() const;
    void setSrcPoint(const QPoint& value);

    QPoint getDestPoint() const;
    virtual void setDestPoint(const QPoint& value);

    int getOffset() const;
    void setOffset(int value);

    QPoint getRelativeSrcPoint() const;
    QPoint getRelativeDestPoint() const;

    bool getSquared() const;
    void setSquared(bool value);

  protected:
    QPoint srcPoint = QPoint(0, 0);
    QPoint destPoint = QPoint(0, 0);
    int offset = 0;
};

#endif // TOPINORUBBERBAND_H
