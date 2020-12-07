#include "include/angulagramview.h"

AngulagramView::AngulagramView(QWidget* parent, TopinoDocument& doc) : TopinoAbstractView(parent, doc) {
    /* Default values */
    scalingFactor = 1.0;

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

    /* Create a new chart and set the theme; important: the theme has to be set BEFORE everything else,
     * otherwise it will overwrite font sizes, etc. Also, we need to add an empty series here before
     * the creation of the axes otherwise this will crash. */
    chart = new QtCharts::QChart();
    chart->setTheme(QtCharts::QChart::ChartThemeDark);

    /* Prepare legend (but it is hidden by default) */
    chart->legend()->hide();
    chart->legend()->setBackgroundVisible(true);
    chart->legend()->setBrush(QBrush(QColor(128, 128, 128, 128)));
    chart->legend()->setPen(QPen(QColor(192, 192, 192, 192)));
    chart->legend()->setShowToolTips(true);
    QFont font;
    font.setPixelSize(12);
    chart->legend()->setFont(font);

    QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
    chart->addSeries(series);

    createAxes();

    /* Add chart to the scene */
    chartScene->addItem(chart);
}

AngulagramView::~AngulagramView() {
}

void AngulagramView::modelHasChanged() {
    /* Remove the old series and get the new one from the data */
    chart->removeAllSeries();
    legendItems.clear();

    /* First, let's add the data as an area series (basically as background) and
     * add all stream fitting functions to it. */
    if (document.getData().getMainInletID() != 0) {
        createDataSeries();
    }

    /* Recreate the axes after adding all series so that it is ensured that
     * all points/lines show at the correct positions. */
    createAxes();

    /* View has changed */
    emit viewHasChanged();
}

bool AngulagramView::isToolSupported(const TopinoAbstractView::tools& value) const {
    /* Only support selection tool at the moment */
    return (value == tools::selection);
}

void AngulagramView::showView() {
    /* Not supported yet. */
}

void AngulagramView::cut(QClipboard *clipboard) {
    Q_UNUSED(clipboard);
    /* Not supported yet. */
}

void AngulagramView::copy(QClipboard *clipboard) {
    qDebug("AngulagramView copy");

    /* Create a mimedata object that will store our chart in different formats, so
     * that they can be used in different programs */
    QMimeData *mimeData = new QMimeData();

    /* First, let's turn the chart into a transparent pixel image (PNG) */
    QImage imageData(this->size(), QImage::Format_ARGB32);
    QPainter paintRaster(&imageData);
    paintRaster.setRenderHint(QPainter::Antialiasing);
    paintRaster.setCompositionMode (QPainter::CompositionMode_Source);
    paintRaster.fillRect(QRectF(QPointF(0, 0), this->size()), Qt::transparent);
    paintRaster.setCompositionMode (QPainter::CompositionMode_SourceOver);
    this->render(&paintRaster);
    paintRaster.end();

    /* Second, let's make a vector painting; same things as above apply. */
    QSvgGenerator generator;
    QBuffer svgBuffer;
    generator.setOutputDevice(&svgBuffer);
    generator.setSize(chart->size().toSize());
    generator.setViewBox(QRect(QPoint(0, 0), this->size()));
    QPainter paintVector(&generator);
    this->render(&paintVector);
    paintVector.end();

    /* Create the text item consisting of a general overview of the raw data and
     * potential stream functions. Additionally, the rawdata + calculated stream
     * curves are added. */
    QStringList textData;
    document.createDataHeader(textData);
    textData.append("");
    document.createDataTable(textData);

    /* Add the data to our mime object and feed the clipboard with it. Ownership
     * is transfered to the clipboard. */
    mimeData->setImageData(imageData);
    mimeData->setData("image/svg+xml", svgBuffer.buffer());
    mimeData->setData("text/csv", textData.join("\n").toUtf8());
    mimeData->setText(textData.join("\n") + "\n");
    clipboard->setMimeData(mimeData);
}

void AngulagramView::paste(QClipboard *clipboard) {
    Q_UNUSED(clipboard);
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

    /* Only support copy function */
    return (value == TopinoAbstractView::editCopy);
}

void AngulagramView::resizeEvent(QResizeEvent* event) {
    /* Resize the chart and adapt the scene */
    chart->resize(event->size());
    chartScene->setSceneRect(chart->boundingRect());
    fitInView(chart->boundingRect(), Qt::KeepAspectRatio);

    /* Call the original implementation */
    QGraphicsView::resizeEvent(event);
}

QVector<AngulagramView::LegendItem> AngulagramView::getLegendItems() const {
    return legendItems;
}

qreal AngulagramView::getScalingFactor() const {
    return scalingFactor;
}

void AngulagramView::setScalingFactor(const qreal& value) {
    /* Since we are scaling by this factor (i.e. dividing) we need to make sure
     * that the factor is never be zero! */
    if (value == 0.0) {
        scalingFactor = 1.0;
    } else {
        scalingFactor = value;
    }
}

void AngulagramView::createAxes() {
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

        qreal factor = document.getData().getCoordCounterClockwise() ? -1.0 : 1.0;
        xaxis->setRange(factor * qAbs(document.getData().getCoordMinAngle()),
                        -1.0 * factor * qAbs(document.getData().getCoordMaxAngle()));

        xaxis->setMinorTickCount(3);
        xaxis->setReverse(document.getData().getCoordCounterClockwise());
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
    font.setPixelSize(36);
    chart->axisX()->setTitleFont(font);
    chart->axisY()->setTitleFont(font);

    font.setPixelSize(22);
    chart->axisX()->setLabelsFont(font);
    chart->axisY()->setLabelsFont(font);
}

void AngulagramView::createDataSeries() {
    /* Get the raw data points from the document. If there is nothing, just leave
     * here immediately. */
    QVector<QPointF> dataPoints = document.getData().getAngulagramPoints();

    if (dataPoints.length() == 0) {
        return;
    }

    /* First, let's calculate a scaling factor from this data to scale it to relative
     * intensities (makes the y-axis way more clear!). */
    QPointF maxPoint = *std::max_element(dataPoints.constBegin(), dataPoints.constEnd(),
    [](const QPointF& a,const QPointF& b) {
        return a.y() < b.y();
    });
    setScalingFactor(maxPoint.y());

    /* Second, let's create a line series first with all the data points. We multiply
     * the x-values with either -1.0 or 1.0 depending on the orientation (CCW or CW)
     * of the coordinate system on the image. We devide the y-values by the scaling
     * factor to just get relative numbers. */
    QtCharts::QLineSeries *series = new QtCharts::QLineSeries(chart);

    //qreal xFactor = document.getData().getCoordCounterClockwise() ? 1.0 : -1.0;
    for (auto iter = dataPoints.begin(); iter != dataPoints.end(); ++iter) {
        series->append(iter->x(), iter->y() / scalingFactor);
    }

    /* Third, let's create an area series based on this line series to fill the area
     * below. This will render the raw data in the "background" using a gray partly
     * transparent color here (and white as border). */
    QtCharts::QAreaSeries *areaseries = new QtCharts::QAreaSeries(series);

    QColor colorseries = TopinoTools::colorsTableau10[7];
    areaseries->setPen(QColor(255, 255, 255));
    colorseries.setAlpha(50);
    areaseries->setBrush(QBrush(colorseries));

    chart->addSeries(areaseries);
    chart->legend()->markers(areaseries)[0]->setVisible(false);

    /* Get stream parameters */
    QVector<TopinoTools::Lorentzian> lorentzians = document.getData().getStreamParameters();

    /* Finally, let's add Lorentzian curves for each Lorentzian fit */
    legendItems.clear();
    for(int i = 0; i < lorentzians.length(); ++i) {
        qDebug("Adding Lorentzian %d to chart", i+1);

        /* Create a new line for each Lorentzian and choose one from seven colours
         * in the Tableau color data. */
        QtCharts::QLineSeries *lorentzLine = new QtCharts::QLineSeries(chart);
        lorentzLine->setPen(TopinoTools::colorsTableau10[i % 7]);

        /* Calculate data based on the x-values of the smoothened data */
        for(int j = 0; j < dataPoints.length(); ++j) {
            qreal x = dataPoints[j].x();
            qreal y = lorentzians[i].f(x) / scalingFactor;
            lorentzLine->append(x, y);
        }

        /* Name the series for the legend */
        QString label;
        label.sprintf("<i>𝜑</i> = %+.1f°, <i>𝜔</i> = %.1f°, <i>L</i>² = %.2f",
                      lorentzians[i].pos, lorentzians[i].width, lorentzians[i].rsquare);
        lorentzLine->setName(label);

        chart->addSeries(lorentzLine);

        /* Change label so that every second item contains an "\n" at the end to wrap the
         * legend. */
        chart->legend()->markers(lorentzLine)[0]->setLabel(label);

        /* Add the data to our legend series */
        LegendItem item;
        item.pos     = lorentzians[i].pos;
        item.width   = lorentzians[i].width;
        item.rsquare = lorentzians[i].rsquare;
        item.color   = chart->legend()->markers(lorentzLine)[0]->brush().color();
        legendItems.append(item);
    }
}

