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
    /* Calculate a working image */
    applyEditsToImage();

    /* Set image of imageView if the image given is not null */
    pixmap->setPixmap(QPixmap());
    if (!workingImage.isNull()) {
        /* Extract Pixmap from image */
        pixmap->setPixmap(QPixmap::fromImage(workingImage));

        /* Fit the image into the view */
        ui->imageView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
        ui->imageView->setSceneRect(pixmap->boundingRect());
    }

    /* Call the basis class show event */
    QDialog::showEvent(event);
}

void ImageEditDialog::checkBoxChanged(int state) {
    Q_UNUSED(state);

    applyEditsToImage();
}

void ImageEditDialog::applyEditsToImage() {
    /* Start with the source image */
    workingImage = sourceImage;

    /* Invert if needed */
    if (ui->checkInvert->isChecked()) {
        workingImage.invertPixels();
    }

    /* Desaturate the image */
    if (ui->checkDesaturate->isChecked()) {
        /* Iterate over all pixels and apply desaturation method. At the moment the resulting
         * gray value is calculated by the Lightness method, i.e. the average of the max and
         * min values of the color values (already implemented in QColor):
         *      Lightness = 0.5 × (max(R,G,B) + min(R,G,B))
         * In future we might implement further desaturation methods. */
        int pixelCount = workingImage.width() * workingImage.height();
        QRgb *pixels = reinterpret_cast<QRgb *>(workingImage.bits());

        for (int p = 0; p < pixelCount; ++p) {
            /* Get the color; need to test how fast this is and if this can be optimized */
            QColor color = workingImage.pixelColor(p % workingImage.width(), p / workingImage.width());
            pixels[p] = QColor(color.lightness(), color.lightness(), color.lightness()).rgb();
        }
    }

    /* TODO: Levels */

    /* Update the image view */
    pixmap->setPixmap(QPixmap::fromImage(workingImage));

    /* Fit the image into the view */
    ui->imageView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    ui->imageView->setSceneRect(pixmap->boundingRect());
}

void ImageEditDialog::setImage(const QImage& value) {
    sourceImage = value;
}
