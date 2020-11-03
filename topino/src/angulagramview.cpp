#include "include/angulagramview.h"

AngulagramView::AngulagramView(QWidget* parent, TopinoDocument& doc) : TopinoAbstractView(parent, doc) {
    /* Create and set a scene */
    chartScene = new QGraphicsScene(contentsRect(), this);
    setScene(chartScene);

    /* Set some visual standard for this chart; in particular, scrollbars off! */
    setFrameShape(QFrame::NoFrame);
    setBackgroundRole(QPalette::Window);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setRenderHint(QPainter::Antialiasing);
    //chartView.setRubberBand(QtCharts::QChartView::RectangleRubberBand););

    QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
    series->append(0, 6);
    series->append(2, 4);
    series->append(3, 8);
    series->append(7, 4);
    series->append(10, 5);
    *series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);

    /* Create a new chart and set the theme; important: the theme has to be set BEFORE everything else,
     * otherwise it will overwrite font sizes, etc. */
    chart = new QtCharts::QChart();
    chart->setTheme(QtCharts::QChart::ChartThemeDark);
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->axisX()->setTitleText("Angle (°)");
    chart->axisY()->setTitleText("Intensity (a.u.)");

    QFont font;
    font.setPixelSize(36);
    chart->axisX()->setTitleFont(font);
    chart->axisY()->setTitleFont(font);

    font.setPixelSize(22);
    chart->axisX()->setLabelsFont(font);
    chart->axisY()->setLabelsFont(font);

    chartScene->addItem(chart);
}

AngulagramView::~AngulagramView() {
}

void AngulagramView::modelHasChanged() {
    /* Remove the old series and get the new one from the data */
    chart->removeAllSeries();

    QtCharts::QLineSeries *series = new QtCharts::QLineSeries(chart);
    QVector<QPointF> dataPoints = document.getData().getAngulagramPoints();
    for (auto iter = dataPoints.begin(); iter != dataPoints.end(); ++iter) {
        qDebug("Add point %.1f %.1f", iter->x(), iter->y());
        series->append(iter->x(), iter->y());
    }

    chart->addSeries(series);

    /* Reset the axes ranges; it is important to create new axes here for the new data by
     * calling createDefaultAxes() - otherwise the data will be shown at the wrong positions. */
    chart->createDefaultAxes();
    chart->axisX()->setRange(document.getData().getCoordMinAngle(), document.getData().getCoordMaxAngle());
    /* TODO: support relative intensities */
    //double max = *std::max_element(dataPoints.constBegin(), dataPoints.constEnd());
    //chart->axisY()->setRange(0.0, 1.0);

    chart->axisX()->setTitleText("Angle (°)");
    chart->axisY()->setTitleText("Intensity (a.u.)");

    QFont font;
    font.setPixelSize(36);
    chart->axisX()->setTitleFont(font);
    chart->axisY()->setTitleFont(font);

    font.setPixelSize(22);
    chart->axisX()->setLabelsFont(font);
    chart->axisY()->setLabelsFont(font);
}

bool AngulagramView::isToolSupported(const TopinoAbstractView::tools& value) const {
    /* Only support selection tool at the moment */
    return (value == tools::selection);
}

void AngulagramView::cut() {
    /* Not supported yet. */
}

void AngulagramView::copy() {
    /* Not supported yet. */
}

void AngulagramView::paste() {
    /* Not supported yet. */
}

void AngulagramView::erase() {
    /* Not supported yet. */
}

void AngulagramView::selectAll() {
    /* Not supported yet. */
}

void AngulagramView::selectNone() {
    /* Not supported yet. */
}

void AngulagramView::selectNext() {
    /* Not supported yet. */
}

bool AngulagramView::isEditFunctionSupported(const TopinoAbstractView::editfunc& value) const {
    Q_UNUSED(value);

    /* None of the edit functions is (yet) supported by this view */
    return false;
}

void AngulagramView::resizeEvent(QResizeEvent* event) {
    /* Resize the chart and adapt the scene */
    chart->resize(event->size());
    chartScene->setSceneRect(chart->boundingRect());
    fitInView(chart->boundingRect(), Qt::KeepAspectRatio);

    /* Call the original implementation */
    QGraphicsView::resizeEvent(event);
}
