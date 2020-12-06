#include "include/evalangulagramdialog.h"
#include "ui_evalangulagramdialog.h"

EvalAngulagramDialog::EvalAngulagramDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EvalAngulagramDialog) {
    /* Setup UI */
    ui->setupUi(this);

    /* Clear the error message */
    ui->labelError->setText("");

    /* Default values */
    scalingFactor = 1.0;

    /* Set some visual standard for this chart; in particular, scrollbars off! */
    ui->previewView->setBackgroundRole(QPalette::Window);
    ui->previewView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->previewView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->previewView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->previewView->setRenderHint(QPainter::Antialiasing);

    /* Get the chart from the view and set the theme; important: the theme has to be set BEFORE
     * everything else, otherwise it will overwrite font sizes, etc. Also, we need to add an
     * empty series here before the creation of the axes otherwise this will crash. */
    chart = ui->previewView->chart();
    chart->setTheme(QtCharts::QChart::ChartThemeDark);
    chart->legend()->hide();

    QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
    chart->addSeries(series);
    createAxes();
}

EvalAngulagramDialog::~EvalAngulagramDialog() {
    delete ui;
}

void EvalAngulagramDialog::resizeEvent(QResizeEvent* event) {
    updateView();

    QDialog::resizeEvent(event);
}

void EvalAngulagramDialog::showEvent(QShowEvent* event) {
    updateView();

    QDialog::showEvent(event);
}

void EvalAngulagramDialog::on_spinSmoothSize_valueChanged(int value) {
    Q_UNUSED(value);

    updateData();
    updateView();
}

void EvalAngulagramDialog::on_spinSmoothSigma_valueChanged(double value) {
    Q_UNUSED(value);

    updateData();
    updateView();
}

void EvalAngulagramDialog::updateView() {
    /* Update viewport */
    ui->previewView->viewport()->update();
}

void EvalAngulagramDialog::updateData() {
    /* Remove the old series and clear errors */
    chart->removeAllSeries();
    ui->labelError->setText("");

    /* Process the data */
    processData();

    /* Update all labels and counters */
    updateLabels();

    /* Update raw but smoothened series */
    createDataSeries();

    /* Update axes */
    createAxes();
}

void EvalAngulagramDialog::updateLabels() {
    /* Number of total points in the raw data set */
    ui->labelDataPoints->setText(QString("%1").arg(dataPoints.length()));

    /* Number of smoothened data points */
    ui->labelSmoothenedDataPoints->setText(QString("%1").arg(smoothenedDataPoints.length()));

    /* Number of minima and maxima */
    ui->labelMinima->setText(QString("%1").arg(TopinoTools::countExtrema(extrema, TopinoTools::extremaMinimum)));
    ui->labelMaxima->setText(QString("%1").arg(TopinoTools::countExtrema(extrema, TopinoTools::extremaMaximum)));
}

qreal EvalAngulagramDialog::getScalingFactor() const {
    return scalingFactor;
}

void EvalAngulagramDialog::setScalingFactor(const qreal& value) {
    /* Since we are scaling by this factor (i.e. dividing) we need to make sure
     * that the factor is never be zero! */
    if (value == 0.0) {
        scalingFactor = 1.0;
    } else {
        scalingFactor = value;
    }
}

bool EvalAngulagramDialog::getOrientationRTL() const {
    return orientationRTL;
}

void EvalAngulagramDialog::setOrientationRTL(bool value) {
    orientationRTL = value;
}

QPair<int, int> EvalAngulagramDialog::getAngularRange() const {
    return angularRange;
}

void EvalAngulagramDialog::setAngularRange(const QPair<int, int>& value) {
    angularRange = value;

    updateData();
    updateView();
}

void EvalAngulagramDialog::createAxes() {
    /* Reset the axes ranges; it is important to create new axes here for the new data by
     * calling createDefaultAxes() - otherwise the data will be shown at the wrong positions. */
    chart->createDefaultAxes();

    /* The x-axis is the angular axis from min to max angle. We will reverse the direction based on the
     * direction of the coordinate system on the image (it is made so that CCW is from right to left on
     * the graph, which seemed to be the most natural one). */
    QtCharts::QValueAxis *xaxis = dynamic_cast<QtCharts::QValueAxis*>(chart->axisX());

    if (xaxis != nullptr) {
        xaxis->setTitleText("Angle (°)");
        xaxis->setLabelFormat("%+.1f");

        xaxis->setRange(angularRange.first, angularRange.second);

        xaxis->setMinorTickCount(3);
        xaxis->setReverse(orientationRTL);
    }

    /* The y-axis is a relative intensity axis (better and clearer than showing the absolute values).
     * Therefore, the range is always simply 0.0 to 1.2 with 0.2 steps (6+1 steps). */
    QtCharts::QValueAxis *yaxis = dynamic_cast<QtCharts::QValueAxis*>(chart->axisY());

    if (yaxis != nullptr) {
        yaxis->setTitleText("Intensity (a.u.)");
        yaxis->setLabelFormat("%.1f");

        yaxis->setRange(0.0, 1.2);
        yaxis->setTickCount(7);
        yaxis->setMinorTickCount(3);
    }

    /* Prepare the font for both axes based on pixel size (might change in future and be configurable). */
    QFont font;
    font.setPixelSize(16);
    chart->axisX()->setTitleFont(font);
    chart->axisY()->setTitleFont(font);

    font.setPixelSize(16);
    chart->axisX()->setLabelsFont(font);
    chart->axisY()->setLabelsFont(font);
}

void EvalAngulagramDialog::createDataSeries() {
    /* If there are no smoothened data points, simply exit here. */
    if (smoothenedDataPoints.length() == 0) {
        return;
    }

    /* Let's create a mini-series for the "threshold" bar */
    QtCharts::QLineSeries *threshSeries = new QtCharts::QLineSeries(chart);
    threshSeries->append(angularRange.first, ui->spinThreshold->value() / 100.0);
    threshSeries->append(angularRange.second, ui->spinThreshold->value() / 100.0);
    threshSeries->setPen(TopinoTools::colorsTableau10[4]);
    chart->addSeries(threshSeries);

    /* Let's calculate a scaling factor from this data to scale it to relative
     * intensities (makes the y-axis way more clear!). */
    QPointF maxPoint = *std::max_element(smoothenedDataPoints.constBegin(), smoothenedDataPoints.constEnd(),
    [](const QPointF& a,const QPointF& b) {
        return a.y() < b.y();
    });
    setScalingFactor(maxPoint.y());

    /* Let's create a line series first with all the data points. We multiply
     * the x-values with either -1.0 or 1.0 depending on the orientation (CCW or CW)
     * of the coordinate system on the image. We devide the y-values by the scaling
     * factor to just get relative numbers. */
    QtCharts::QLineSeries *series = new QtCharts::QLineSeries(chart);

    qreal xFactor = orientationRTL ? 1.0 : -1.0;
    for (auto iter = smoothenedDataPoints.begin(); iter != smoothenedDataPoints.end(); ++iter) {
        series->append(iter->x() * xFactor, iter->y() / scalingFactor);
    }

    /* Let's create an area series based on this line series to fill the area
     * below. This will render the raw data in the "background" using a gray partly
     * transparent color here (and white as border). */
    QtCharts::QAreaSeries *areaseries = new QtCharts::QAreaSeries(series, threshSeries);

    QColor colorseries = TopinoTools::colorsTableau10[7];
    areaseries->setPen(QColor(255, 255, 255));
    colorseries.setAlpha(50);
    areaseries->setBrush(QBrush(colorseries));

    chart->addSeries(areaseries);

    /* Next, let's add a series each for the minima and maxima, respectively. There are points
     * each. For the minima, we also add dashed lines from the point to the x-axis. */
    if (extrema.length() > 0) {
        QtCharts::QScatterSeries *minima = new QtCharts::QScatterSeries(chart);
        QtCharts::QScatterSeries *maxima = new QtCharts::QScatterSeries(chart);
        minima->setPen(Qt::NoPen);
        minima->setBrush(QBrush(TopinoTools::colorsTableau10[6]));
        maxima->setPen(Qt::NoPen);
        maxima->setBrush(QBrush(TopinoTools::colorsTableau10[9]));

        for(auto it = extrema.begin(); it != extrema.end(); ++it) {
            if (it->type == TopinoTools::extremaMinimum) {
                minima->append(it->pos.x(), it->pos.y() / scalingFactor);

                QtCharts::QLineSeries *minimaLine = new QtCharts::QLineSeries(chart);
                minimaLine->setPen(QPen(Qt::DashLine));
                minimaLine->setColor(QColor(255, 255, 255));
                minimaLine->append(it->pos.x(), 0.0);
                minimaLine->append(it->pos.x(), it->pos.y() / scalingFactor);
                chart->addSeries(minimaLine);

            } else if (it->type == TopinoTools::extremaMaximum) {
                maxima->append(it->pos.x(), it->pos.y() / scalingFactor);
            }
        }

        chart->addSeries(minima);
        chart->addSeries(maxima);
    }

    /* Finally, let's add Lorentzian curves for each Lorentzian fit */
    for(int i = 0; i < lorentzians.length(); ++i) {
        qDebug("Adding Lorentzian %d to chart", i+1);

        /* Create a new line for each Lorentzian and choose one from seven colours
         * in the Tableau color data. */
        QtCharts::QLineSeries *lorentzLine = new QtCharts::QLineSeries(chart);
        lorentzLine->setPen(TopinoTools::colorsTableau10[i % 7]);

        /* Calculate data based on the x-values of the smoothened data */
        for(int j = 0; j < smoothenedDataPoints.length(); ++j) {
            qreal x = smoothenedDataPoints[j].x() * xFactor;
            qreal y = lorentzians[i].f(x) / scalingFactor;
            lorentzLine->append(x, y);
        }

        chart->addSeries(lorentzLine);
    }
}

QVector<TopinoTools::Lorentzian> EvalAngulagramDialog::getLorentzians() const {
    return lorentzians;
}

void EvalAngulagramDialog::processData() {
    /* First step: copy raw data points into smoothened data points vector and empty all other
     * data. */
    smoothenedDataPoints = dataPoints;
    extrema.clear();
    lorentzians.clear();

    /* Second step: smoothen the data with the provided parameters */
    TopinoTools::smoothByGaussianKernel(smoothenedDataPoints, ui->spinSmoothSize->value(), ui->spinSmoothSigma->value());

    /* Third step: find extrema points */
    TopinoTools::getExtrema(smoothenedDataPoints, extrema);

    qDebug("Found %d extrema:", extrema.length());
    qreal threshold = (ui->spinThreshold->value() / 100.0) * scalingFactor;
    TopinoTools::filterExtrema(extrema, threshold);

    qDebug("Filtered to %d extrema:", extrema.length());

    for(int i = 0; i < extrema.length(); ++i) {
        qDebug("%3d: at index %d (%1.f, %.1f) type %d", i+1, extrema[i].index, extrema[i].pos.x(), extrema[i].pos.y(), extrema[i].type);
    }
    int minima = TopinoTools::countExtrema(extrema, TopinoTools::extremaMinimum);
    int maxima = TopinoTools::countExtrema(extrema, TopinoTools::extremaMaximum);

    if (minima != (maxima-1)) {
        ui->labelError->setText(QString(tr("There should be exactly one maxima more than minima! "
                                           "Please change parameters (smoothing, threshold) to achieve this!")));

        return;
    }

    /* Forth step: get sections and fit every section to a Lorentzian curve */
    QVector<TopinoTools::Section> sections = TopinoTools::getSections(smoothenedDataPoints, extrema, threshold);
    for(int i = 0; i < sections.length(); ++i) {
        qDebug("Section %2d: from %d (%.1f, %.1f) to %d (%.1f, %.1f) with maximum at %d (%.1f, %.1f)", i+1,
               sections[i].indexLeft, smoothenedDataPoints[sections[i].indexLeft].x(), smoothenedDataPoints[sections[i].indexLeft].y(),
               sections[i].indexRight, smoothenedDataPoints[sections[i].indexRight].x(), smoothenedDataPoints[sections[i].indexRight].y(),
               sections[i].indexMax, smoothenedDataPoints[sections[i].indexMax].x(), smoothenedDataPoints[sections[i].indexMax].y());
    }

    lorentzians = TopinoTools::calculateLorentzians(smoothenedDataPoints, sections, threshold);
    for(int i = 0; i < lorentzians.length(); ++i) {
        qDebug("Lorentzian %2d: pos %.1f, width %.1f, height %.1f, offset %.1f, r-square %.2f", i+1,
               lorentzians[i].pos, lorentzians[i].width, lorentzians[i].height, lorentzians[i].offset, lorentzians[i].rsquare);
    }
}

void EvalAngulagramDialog::setDataPoints(const QVector<QPointF>& value) {
    /* Save data points */
    dataPoints = value;
    smoothenedDataPoints = value;

    /* The maximum of smoothing is to take half the points on the left and half
     * the points on the right side. */
    ui->spinSmoothSize->setMaximum(dataPoints.length() / 2);

    /* Update everything */
    updateData();
    updateView();
}


void EvalAngulagramDialog::on_spinThreshold_valueChanged(double value) {
    Q_UNUSED(value);

    updateData();
    updateView();
}
