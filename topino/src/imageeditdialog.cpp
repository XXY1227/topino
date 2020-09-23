#include "include/imageeditdialog.h"
#include "ui_imageeditdialog.h"

ImageEditDialog::ImageEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageEditDialog) {
    ui->setupUi(this);

    /* Prepare the mini image view */
    pixmap = new QGraphicsPixmapItem();
    scene = new QGraphicsScene(ui->imageView->contentsRect(), ui->imageView);

    scene->addItem(pixmap);
    ui->imageView->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    ui->imageView->setScene(scene);

    /* Insert separator into the desaturation-mode combobox */
    ui->desaturateModes->insertSeparator(4);

    /* Connect the histogram widget */
    connect(ui->histogram, &HistogramWidget::valuesChanging, this, &ImageEditDialog::levelsChanging);
    connect(ui->histogram, &HistogramWidget::valuesChanged, this, &ImageEditDialog::levelsChanged);
}

ImageEditDialog::~ImageEditDialog() {
    delete ui;
}

void ImageEditDialog::resizeEvent(QResizeEvent* event) {
    /* Adjust the image view to show the picture */
    ui->imageView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    ui->imageView->setSceneRect(pixmap->boundingRect());

    /* Call the basis class show event */
    QDialog::resizeEvent(event);
}

void ImageEditDialog::showEvent(QShowEvent* event) {
    /* Calculate a working and leveled image; will also show the image */
    applyEditsToImage();

    /* Call the basis class show event */
    QDialog::showEvent(event);
}

void ImageEditDialog::invertBoxChanged(int state) {
    Q_UNUSED(state);

    /* Mirror min and max values of the histogram widget so that the
     * selection window "inverts" in the same way */
    ui->histogram->mirrorMinMaxValue();

    spinChanged = true;
    ui->levelMin->setValue(ui->histogram->getMinSelValue());
    spinChanged = true;
    ui->levelMax->setValue(ui->histogram->getMaxSelValue());

    /* Apply edits */
    applyEditsToImage();
}

void ImageEditDialog::levelsChanging(int min, int max) {
    spinChanged = true;
    ui->levelMin->setValue(min);
    spinChanged = true;
    ui->levelMax->setValue(max);

    /* Apply edits */
    applyEditsToImage();
}

void ImageEditDialog::levelsChanged(int min, int max) {
    spinChanged = true;
    ui->levelMin->setValue(min);
    spinChanged = true;
    ui->levelMax->setValue(max);

    /* Apply edits */
    applyEditsToImage();
}

void ImageEditDialog::minLevelChanged(int value) {
    /* Setting the value to the histogram */
    ui->histogram->setMinSelValue(value);

    /* Apply edits */
    if (!spinChanged) {
        applyEditsToImage();
    }

    /* Reading out the value immediately makes sure the
     * value is legit */
    if (ui->histogram->getMinSelValue() != value) {
        spinChanged = true;
        ui->levelMin->setValue(ui->histogram->getMinSelValue());
    }

    spinChanged = false;
}

void ImageEditDialog::maxLevelChanged(int value) {
    /* Setting the value to the histogram */
    ui->histogram->setMaxSelValue(value);

    /* Apply edits */
    if (!spinChanged) {
        applyEditsToImage();
    }

    /* Reading out the value immediately makes sure the
     * value is legit */
    if (ui->histogram->getMaxSelValue() != value) {
        spinChanged = true;
        ui->levelMax->setValue(ui->histogram->getMaxSelValue());
    }

    spinChanged = false;
}

bool ImageEditDialog::getInvert() const {
    return ui->checkInvert->isChecked();
}

void ImageEditDialog::setInvert(bool value) {
    ui->checkInvert->setChecked(value);
}

int ImageEditDialog::getLevelMax() const {
    return ui->histogram->getMaxSelValue();
}

void ImageEditDialog::setLevelMax(int value) {
    ui->histogram->setMaxSelValue(value);
    ui->levelMax->setValue(value);
}

int ImageEditDialog::getLevelMin() const {
    return ui->histogram->getMinSelValue();
}

void ImageEditDialog::setLevelMin(int value) {
    ui->histogram->setMinSelValue(value);
    ui->levelMin->setValue(value);
}

int ImageEditDialog::getDesaturationMode() const {
    return ui->desaturateModes->currentIndex();
}

void ImageEditDialog::setDesaturationMode(int value) {
    ui->desaturateModes->setCurrentIndex(value);
}

void ImageEditDialog::applyEditsToImage() {
    /* Start with the source image */
    processedImage = sourceImage;

    /* Invert if needed */
    if (ui->checkInvert->isChecked()) {
        processedImage.invertPixels();
    }

    /* Iterate over all pixels and apply desaturation method. Count the
     * values for the histogram in the same run. */
    int pixelCount = processedImage.width() * processedImage.height();
    QRgb *pixels = reinterpret_cast<QRgb *>(processedImage.bits());

    /* Create a vector for 256 (0-255) values to count. Fill with zeros. */
    QVector<int> histogram(256);
    histogram.fill(0);

    int desatmode = ui->desaturateModes->currentIndex();
    for (int p = 0; p < pixelCount; ++p) {
        /* The value calculated depends on the method selected */
        int value = 0;

        switch(desatmode) {
        /* Luminance method */
        case desaturationModes::desatLuminance:
            value = TopinoTools::qLuminance(pixels[p]);
            break;

        /* Average method */
        case desaturationModes::desatAverage:
            value = TopinoTools::qAverage(pixels[p]);
            break;

        /* Maximum method */
        case desaturationModes::desatMaximum:
            value = TopinoTools::qMaximum(pixels[p]);
            break;

        /* Red channel */
        case desaturationModes::desatRed:
            value = qRed(pixels[p]);
            break;

        /* Green channel */
        case desaturationModes::desatGreen:
            value = qGreen(pixels[p]);
            break;

        /* Blue channel */
        case desaturationModes::desatBlue:
            value = qBlue(pixels[p]);
            break;

        /* Lightness method (default) */
        case desaturationModes::desatLightness:
        default:
            value = TopinoTools::qLightness(pixels[p]);
            break;
        }

        /* Count in any case */
        histogram[value]++;

        /* Apply new desaturated value */
        pixels[p] = QColor(value, value, value).rgb();
    }

    /* Set levels and apply */
    ui->histogram->setHistogram(histogram);
    int minValue = ui->histogram->getMinSelValue();
    int maxValue = ui->histogram->getMaxSelValue();
    qreal scale = 255.0 / (qreal)(maxValue - minValue);

    for (int p = 0; p < pixelCount; ++p) {
        /* Subtract the bottom value; since all the channels are the same
         * it does not really matter which channel we use here. */
        int value = qMax(0, qGreen(pixels[p]) - minValue);

        /* Multiply with the scale */
        value = qMin(255, (int)(value * scale));

        /* Set the pixel */
        pixels[p] = qRgb(value, value, value);
    }

    /* Show working image */
    showPreviewImage();
}

void ImageEditDialog::showPreviewImage() {
    /* Depending on mode either the processed image, the source image, or
     * a half-half version is shown */
    switch(ui->previewModes->currentIndex()) {
    /* show source image */
    case previewModes::showSourceImage:
        if (sourceImage.isNull()) {
            pixmap->setPixmap(QPixmap());
            return;
        }

        pixmap->setPixmap(QPixmap::fromImage(sourceImage));
        break;

    /* show half-half version */
    case previewModes::showHalfhalf: {
        /* Prepare half image and rectangle */
        QImage halfhalfImage = QImage(sourceImage.size(), QImage::Format_ARGB32_Premultiplied);
        QRect halfRect = sourceImage.rect();
        halfRect.setX(halfRect.width()/2);

        QPainter painter(&halfhalfImage);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawImage(0, 0, sourceImage);
        painter.fillRect(halfRect, Qt::transparent);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
        painter.drawImage(0, 0, processedImage);
        painter.end();

        pixmap->setPixmap(QPixmap::fromImage(halfhalfImage));
    }
    break;

    /* show processed image (default mode) */
    case previewModes::showProcessedImage:
    default:
        if (processedImage.isNull()) {
            pixmap->setPixmap(QPixmap());
            return;
        }

        pixmap->setPixmap(QPixmap::fromImage(processedImage));
        break;
    }

    /* Fit the image into the view */
    ui->imageView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    ui->imageView->setSceneRect(pixmap->boundingRect());
}

void ImageEditDialog::setImage(const QImage& value) {
    sourceImage = value;
}

void ImageEditDialog::previewModeChanged(int index) {
    Q_UNUSED(index);

    showPreviewImage();
}

void ImageEditDialog::desaturationModeChanged(int index) {
    Q_UNUSED(index);

    applyEditsToImage();
}
