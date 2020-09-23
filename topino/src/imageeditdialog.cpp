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

    /* Connect the histogram widget */
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

    /* Apply edits */
    applyEditsToImage();
}

void ImageEditDialog::checkBoxChanged(int state) {
    Q_UNUSED(state);

    applyEditsToImage();
}

void ImageEditDialog::levelsChanged(int min, int max) {
    ui->levelMin->setValue(min);
    ui->levelMax->setValue(max);
}

void ImageEditDialog::minLevelChanged(int value) {
    /* Setting and reading out the value immediately makes sure that the
     * value is legit */
    ui->histogram->setMinSelValue(value);
    ui->levelMin->setValue(ui->histogram->getMinSelValue());
}

void ImageEditDialog::maxLevelChanged(int value) {
    /* Setting and reading out the value immediately makes sure that the
     * value is legit */
    ui->histogram->setMaxSelValue(value);
    ui->levelMax->setValue(ui->histogram->getMaxSelValue());
}

void ImageEditDialog::applyEditsToImage() {
    /* Start with the source image */
    workingImage = sourceImage;

    /* Invert if needed */
    if (ui->checkInvert->isChecked()) {
        workingImage.invertPixels();
    }

    /* Iterate over all pixels and apply desaturation method (at the moment
     * by the lightness/average method) if needed. Count the values for the
     * histogram in any case. */
    int pixelCount = workingImage.width() * workingImage.height();
    QRgb *pixels = reinterpret_cast<QRgb *>(workingImage.bits());

    /* Create a vector for 256 (0-255) values to count. Fill with zeros. */
    QVector<int> histogram(256);
    histogram.fill(0);

    bool desaturate = ui->checkDesaturate->isChecked();
    for (int p = 0; p < pixelCount; ++p) {
        /* Get the color by directly accessing the data. Basically the fastest method. */
        int lightness = TopinoTools::qLightness(pixels[p]);

        /* Count in any case */
        histogram[lightness]++;

        /* Desaturate if selected */
        if (desaturate)
            pixels[p] = QColor(lightness, lightness, lightness).rgb();
    }

    /* Set levels and apply */
    ui->histogram->setHistogram(histogram);
    int minValue = ui->histogram->getMinSelValue();
    int maxValue = ui->histogram->getMaxSelValue();
    qreal scale = 255.0 / (qreal)(maxValue - minValue);

    for (int p = 0; p < pixelCount; ++p) {
        /* Subtract the bottom value */
        int value = qMax(0, qGreen(pixels[p]) - minValue);

        /* Multiply with the scale */
        value = qMin(255, (int)(value * scale));

        /* Set the pixel */
        pixels[p] = qRgb(value, value, value);
    }

    /* Show working image */
    showPreviewImage(workingImage);
}

void ImageEditDialog::showPreviewImage(const QImage& img) {
    /* Empty image handling */
    if (img.isNull()) {
        pixmap->setPixmap(QPixmap());
        return;
    }

    /* Update the image view */
    pixmap->setPixmap(QPixmap::fromImage(img));

    /* Fit the image into the view */
    ui->imageView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    ui->imageView->setSceneRect(pixmap->boundingRect());
}

void ImageEditDialog::setImage(const QImage& value) {
    sourceImage = value;
}
