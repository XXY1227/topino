#ifndef RADIALGRAMDIALOG_H
#define RADIALGRAMDIALOG_H

#include <QAbstractButton>
#include <QChart>
#include <QDialog>
#include <QLineSeries>
#include <QSvgGenerator>
#include <QValueAxis>


namespace Ui {
class RadialgramDialog;
}

class RadialgramDialog : public QDialog {
    Q_OBJECT

  public:
    explicit RadialgramDialog(QWidget *parent = 0);
    ~RadialgramDialog();

    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent *event) override;

    void setDataPoints(const QVector<QPointF>& value);

    qreal getScalingFactor() const;
    void setScalingFactor(const qreal& value);

    /* Exports the image/data to a file */
    void onExport();

  public slots:
    void buttonClicked(QAbstractButton *button);

  private:
    /* User interface definition */
    Ui::RadialgramDialog *ui;

    /* Scene and chart items */
    QtCharts::QChart *chart = nullptr;

    /* Data points and functions */
    QVector<QPointF> dataPoints;

    /* This is the scaling factor for the data on the y-axis. All data in the document/data
     * object is scaled by this factor before it is added to the chart. */
    qreal scalingFactor;

    /* Re-creates the axes and series of the chart */
    void createAxes();
    void createSeries();

    /* Update the view */
    void updateView();
};

#endif // RADIALGRAMDIALOG_H
