#include "include/polarimagedialog.h"
#include "ui_polarimagedialog.h"

PolarImageDialog::PolarImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PolarImageDialog) {

    /* Setup user interface */
    ui->setupUi(this);
    ui->buttonBox->addButton(tr("Export..."), QDialogButtonBox::ActionRole);

    ui->imageView->setBackgroundRole(QPalette::Window);
    ui->imageView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->imageView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->imageView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->imageView->setRenderHint(QPainter::Antialiasing);

    /* Prepare the image view */
    pixmap = new QGraphicsPixmapItem();
    scene = new QGraphicsScene(ui->imageView->contentsRect(), ui->imageView);

    scene->addItem(pixmap);
    ui->imageView->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    ui->imageView->setScene(scene);

    updateLabels();
}

PolarImageDialog::~PolarImageDialog() {
    delete ui;
}

void PolarImageDialog::resizeEvent(QResizeEvent* event) {
    /* Adjust the image view to show the picture */
    ui->imageView->fitInView(scene->itemsBoundingRect(), Qt::IgnoreAspectRatio);
    ui->imageView->setSceneRect(pixmap->boundingRect());
    ui->imageView->viewport()->update();

    /* Call the basis class show event */
    QDialog::resizeEvent(event);
}

void PolarImageDialog::showEvent(QShowEvent* event) {
    /* Fit the image into the view */
    ui->imageView->fitInView(scene->itemsBoundingRect(), Qt::IgnoreAspectRatio);
    ui->imageView->setSceneRect(pixmap->boundingRect());
    ui->imageView->viewport()->update();

    /* Call the basis class show event */
    QDialog::showEvent(event);
}

void PolarImageDialog::onExport() {
    /* If there is no image, then let's leave here */
    if (polarImage.isNull())
        return;

    /* Let the user choose a name */
    QString filename = QFileDialog::getSaveFileName(this, tr("Export polar image"), "polarimage.png",
                       tr("Raster image file (*.png);;All files (*.*)"));

    if (filename.length() == 0)
        return;

    qDebug("File exported: %s", filename.toStdString().c_str());

    /* Export as raster image */
    qDebug("Save as raster image");

    /* Create an image, let the Angulagram view draw into it, and
     * save the result as a file. */
    polarImage.save(filename);
}

void PolarImageDialog::updateLabels() {
    ui->labelAngleMin->setText(QString::number(angleRange.first));
    ui->labelAngleMax->setText(QString::number(angleRange.second));
    ui->labelRadiusMax->setText(QString::number(polarImage.width()));
}

void PolarImageDialog::setAngleRange(const QPair<int, int>& value) {
    angleRange = value;

    updateLabels();
}

void PolarImageDialog::buttonClicked(QAbstractButton* button) {
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ActionRole) {
        onExport();
    }
}

void PolarImageDialog::setPolarImage(const QImage& value) {
    polarImage = value;
    pixmap->setPixmap(QPixmap::fromImage(polarImage));

    updateLabels();
}
