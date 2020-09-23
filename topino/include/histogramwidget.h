#ifndef HISTOGRAMWIDGET_H
#define HISTOGRAMWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QPixmap>
#include <QCursor>

class HistogramWidget : public QWidget {
    Q_OBJECT
  public:
    explicit HistogramWidget(QWidget *parent = nullptr);
    ~HistogramWidget();

    QVector<int> getHistogram() const;
    void setHistogram(const QVector<int>& value);

    int getMinSelValue() const;
    void setMinSelValue(int value);

    int getMaxSelValue() const;
    void setMaxSelValue(int value);

    void mirrorMinMaxValue();

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;

  signals:
    void valuesChanged(int min, int max);

  public slots:

  private:
    /* Data for the histogram */
    QVector<int> histogram;
    int maxIntensityValue;

    int minSelValue;
    int maxSelValue;

    /* Data for drawing */
    QBrush backgroundBrush;
    QBrush selectionBrush;
    QBrush barBrush;
    QBrush barBrushSelected;
    QCursor cursorSelection;

    /* Individual parts of the histogram widget clicked */
    enum parts {
        none = 0,
        minborder = 1,
        maxborder = 2
    };
    parts partClicked = parts::none;

    bool inMinBorder(QPointF pt) const;
    bool inMaxBorder(QPointF pt) const;
};

#endif // HISTOGRAMWIDGET_H
