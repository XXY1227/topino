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

    chart = new QtCharts::QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->axisX()->setTitleText("Angle (°)");
    chart->axisY()->setTitleText("Intensity (a.u.)");
    chart->setTheme(QtCharts::QChart::ChartThemeDark);

    chartScene->addItem(chart);
}

AngulagramView::~AngulagramView() {
}

void AngulagramView::modelHasChanged() {
}

bool AngulagramView::isToolSupported(const TopinoAbstractView::tools& value) const {
    /* Only support selection tool at the moment */
    return (value == tools::selection);
}

void AngulagramView::resizeEvent(QResizeEvent* event) {
    /* Resize the chart and adapt the scene */
    chart->resize(event->size());
    chartScene->setSceneRect(chart->boundingRect());
    fitInView(chart->boundingRect(), Qt::KeepAspectRatio);

    /* Call the original implementation */
    QGraphicsView::resizeEvent(event);
}
