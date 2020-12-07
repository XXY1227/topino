#ifndef EVALANGULAGRAMDIALOG_H
#define EVALANGULAGRAMDIALOG_H

#include <QAreaSeries>
#include <QChartView>
#include <QChart>
#include <QDialog>
#include <QLineSeries>
#include <QPair>
#include <QScatterSeries>
#include <QValueAxis>

#include "include/topinotool.h"

namespace Ui {
class EvalAngulagramDialog;
}

class EvalAngulagramDialog : public QDialog {
    Q_OBJECT

  public:
    explicit EvalAngulagramDialog(QWidget *parent = 0);
    ~EvalAngulagramDialog();

    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent *event) override;

    QPair<int, int> getAngularRange() const;
    void setAngularRange(const QPair<int, int>& value);

    bool getOrientationRTL() const;
    void setOrientationRTL(bool value);

    void setDataPoints(const QVector<QPointF>& value);

    qreal getScalingFactor() const;
    void setScalingFactor(const qreal& value);

    QVector<TopinoTools::Lorentzian> getLorentzians() const;

  private slots:
    void on_spinSmoothSize_valueChanged(int value);
    void on_spinSmoothSigma_valueChanged(double value);
    void on_spinThreshold_valueChanged(double value);

  private:
    /* Interface definitions */
    Ui::EvalAngulagramDialog *ui;

    /* Chart items */
    QtCharts::QChart *chart = nullptr;

    QPair<int, int> angularRange;
    bool orientationRTL;

    /* Update the view and data */
    void updateView();
    void updateData();
    void updateLabels();

    /* This is the scaling factor for the data on the y-axis. All data in the document/data
     * object is scaled by this factor before it is added to the chart. */
    qreal scalingFactor;

    /* Re-creates the axes of the chart */
    void createAxes();

    /* Create the diffent series for the chart */
    void createDataSeries();

    /* Data points and functions */
    QVector<QPointF> dataPoints;
    QVector<QPointF> smoothenedDataPoints;
    QVector<TopinoTools::Extrema> extrema;
    QVector<TopinoTools::Lorentzian> lorentzians;

    /* Processed the data */
    void processData();
};

#endif // EVALANGULAGRAMDIALOG_H
