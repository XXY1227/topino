#include "include/radialgramdialog.h"
#include "ui_radialgramdialog.h"

RadialgramDialog::RadialgramDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RadialgramDialog) {
    /* Set up the user interface */
    ui->setupUi(this);
    ui->buttonBox->clear();
    ui->buttonBox->addButton(tr("Close"), QDialogButtonBox::AcceptRole);
    ui->buttonBox->addButton(tr("Export..."), QDialogButtonBox::ActionRole);

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

RadialgramDialog::~RadialgramDialog() {
    delete ui;
}

void RadialgramDialog::resizeEvent(QResizeEvent* event) {
    updateView();

    QDialog::resizeEvent(event);
}

void RadialgramDialog::showEvent(QShowEvent* event) {
    updateView();

    QDialog::showEvent(event);
}

void RadialgramDialog::setDataPoints(const QVector<QPointF>& value) {
    dataPoints = value;

    createAxes();
    createSeries();

    updateView();
}

qreal RadialgramDialog::getScalingFactor() const {
    return scalingFactor;
}

void RadialgramDialog::setScalingFactor(const qreal& value) {
    /* Since we are scaling by this factor (i.e. dividing) we need to make sure
     * that the factor is never be zero! */
    if (value == 0.0) {
        scalingFactor = 1.0;
    } else {
        scalingFactor = value;
    }
}

void RadialgramDialog::onExport() {
    /* If there is no data, then let's leave here */
    if (dataPoints.length() == 0)
        return;

    /* Let the user choose a name */
    QString filename = QFileDialog::getSaveFileName(this, tr("Export radialgram chart/data"), "radialgram.png",
                       tr("Raster image file (*.png);;Scalable Vector files (*.svg);;Text file (*.txt);;All files (*.*)"));

    if (filename.length() == 0)
        return;

    qDebug("File exported: %s", filename.toStdString().c_str());

    /* Depending on the extension, we save either as vector image (SVG) or
     * as raster image (whatever QImage/QPixmap supports). */
    if (filename.endsWith(".svg", Qt::CaseInsensitive)) {
        qDebug("Save as vector image");

        /* Create a SVG generator, let the Angulagram view draw into it and
         * save the result. */
        QSvgGenerator generator;
        generator.setFileName(filename);
        generator.setTitle("Radialgram graph");
        generator.setDescription(tr("Radialgram graph generated at ") + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
        generator.setSize(ui->previewView->size());
        generator.setViewBox(QRect(QPoint(0, 0), ui->previewView->size()));

        QPainter paintVector(&generator);
        ui->previewView->render(&paintVector);
        paintVector.end();

    /* Text file */
    } else if (filename.endsWith(".txt", Qt::CaseInsensitive)) {
        qDebug("Save text file");

        /* Prepare all the data in text form, line by line */
        QStringList textdata;
        textdata.append("Radius (Px)\tCircular intensity (a.u.)");
        for(int i = 0; i < dataPoints.length(); ++i) {
            QString line;
            line.sprintf("%.0f\t%.3f", dataPoints[i].x(), dataPoints[i].y());
            textdata.append(line);
        }

        /* Write it to the given file */
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            file.write(textdata.join("\n").toUtf8());
            file.close();
        } else {
            qDebug("Could not open file %s", filename.toStdString().c_str());
        }

    /* Raster image */
    } else {
        qDebug("Save as raster image");

        /* Create an image, let the Angulagram view draw into it, and
         * save the result as a file. */
        QImage imageData(ui->previewView->size(), QImage::Format_ARGB32);
        QPainter paintRaster(&imageData);
        paintRaster.setRenderHint(QPainter::Antialiasing);
        paintRaster.setCompositionMode (QPainter::CompositionMode_Source);
        paintRaster.fillRect(QRectF(QPointF(0, 0), ui->previewView->size()), Qt::transparent);
        paintRaster.setCompositionMode (QPainter::CompositionMode_SourceOver);
        ui->previewView->render(&paintRaster);
        paintRaster.end();

        imageData.save(filename);
    }
}

void RadialgramDialog::buttonClicked(QAbstractButton* button) {
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ActionRole) {
        onExport();
    }
}

void RadialgramDialog::createAxes() {
    /* Reset the axes ranges; it is important to create new axes here for the new data by
     * calling createDefaultAxes() - otherwise the data will be shown at the wrong positions. */
    chart->createDefaultAxes();

    /* Here the x-axis is the radius axis, so the x-values of the data */
    QtCharts::QValueAxis *xaxis = dynamic_cast<QtCharts::QValueAxis*>(chart->axisX());

    if (xaxis != nullptr) {
        xaxis->setTitleText("Radius (Px)");
        xaxis->setLabelFormat("%d");
        xaxis->setMinorTickCount(3);

        /* We assume that dataPoints are sorted and the last element holds therefore the
         * maximal x value. */
        if (dataPoints.length() > 0) {
            xaxis->setRange(0, dataPoints.last().x());
        } else {
            xaxis->setRange(0, 100);
        }
    }

    /* Here, the y-axis is the integrated signal over angle. We call it here circular intensity
     * to distinguish it from the "normal" intensity in an angulagram. */
    QtCharts::QValueAxis *yaxis = dynamic_cast<QtCharts::QValueAxis*>(chart->axisY());

    if (yaxis != nullptr) {
        yaxis->setTitleText("Circular intensity (a.u.)");
        yaxis->setLabelFormat("%.1f");

        yaxis->setRange(0.0, 1.2);
        yaxis->setTickCount(7);
        yaxis->setMinorTickCount(3);
    }

    /* Prepare the font for both axes based on pixel size (might change in future and be configurable). */
    QFont font;
    font.setPixelSize(18);
    chart->axisX()->setTitleFont(font);
    chart->axisY()->setTitleFont(font);

    font.setPixelSize(16);
    chart->axisX()->setLabelsFont(font);
    chart->axisY()->setLabelsFont(font);
}

void RadialgramDialog::createSeries() {
    /* If there are no data points, simply exit here. */
    if (dataPoints.length() == 0)
        return;

    /* Let's calculate a scaling factor from this data to scale it to relative
     * intensities (makes the y-axis way more clear!). */
    QPointF maxPoint = *std::max_element(dataPoints.constBegin(), dataPoints.constEnd(),
    [](const QPointF& a,const QPointF& b) {
        return a.y() < b.y();
    });
    setScalingFactor(maxPoint.y());

    /* Let's create a line series first with all the data points. */
    QtCharts::QLineSeries *series = new QtCharts::QLineSeries(chart);

    for (auto iter = dataPoints.begin(); iter != dataPoints.end(); ++iter) {
        series->append(iter->x(), iter->y() / scalingFactor);
    }
    chart->addSeries(series);
}

void RadialgramDialog::updateView() {
    /* Update viewport */
    ui->previewView->viewport()->update();
}
