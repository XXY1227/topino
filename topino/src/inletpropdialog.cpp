#include "include/inletpropdialog.h"
#include "ui_inletpropdialog.h"

InletPropDialog::InletPropDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InletPropDialog) {
    ui->setupUi(this);

    /* Initialize all main inlet properties with disabled state. We have to
     * manually call the toggle function here, since the false state might
     * already be set on the checkbox itself - so there is no change and no
     * signal emitted. Calling both to be sure. */
    ui->checkMainInlet->setChecked(false);
    on_checkMainInlet_toggled(false);

    /* Set CCW as default. It is important to call both functions here for
     * the same reasons as above. */
    ui->checkCCW->setChecked(true);
    on_checkCCW_toggled(true);

    /* Prepare the image in the preview */
    pixmap = new QGraphicsPixmapItem();
    scene = new QGraphicsScene(ui->imageView->contentsRect(), ui->imageView);
    scene->addItem(pixmap);

    /* Prepare the inlet tool with a fixed scaling factor (since there is no zoom or the like here) */
    tool = new PolarCircleToolItem(0);
    tool->showSegments(false);
    tool->setScaling(10.0);
    scene->addItem(tool);

    /* Prepare view and scene */
    ui->imageView->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    ui->imageView->setScene(scene);
}

InletPropDialog::~InletPropDialog() {
    delete ui;
}

void InletPropDialog::resizeEvent(QResizeEvent* event) {
    updateView();

    /* Call the basis class show event */
    QDialog::resizeEvent(event);
}

void InletPropDialog::showEvent(QShowEvent* event) {
    updateView();

    /* Call the basis class show event */
    QDialog::showEvent(event);
}

void InletPropDialog::setBackgroundImage(const QImage& image) {
    /* Set image in pixmap */
    pixmap->setPixmap(QPixmap::fromImage(image));

    /* Update view */
    updateView();
}

void InletPropDialog::setPosition(const QPointF& pos) {
    ui->labelPosition->setText(QString("%1, %2").arg(pos.toPoint().x()).arg(pos.toPoint().y()));
}

int InletPropDialog::getInnerRadius() const {
    return tool->getInnerRadius();
}

void InletPropDialog::setInnerRadius(int value) {
    ui->spinInnerRadius->setValue(value);
}

int InletPropDialog::getOuterRadius() const {
    return tool->getOuterRadius();
}

void InletPropDialog::setOuterRadius(int value) {
    ui->spinOuterRadius->setValue(value);
}

int InletPropDialog::getRefAngle() const {
    return tool->getZeroAngle();
}

void InletPropDialog::setRefAngle(int value) {
    ui->spinRefAngle->setValue(value);
}

int InletPropDialog::getCCWAngle() const {
    return tool->getMaxAngle();
}

void InletPropDialog::setCCWAngle(int value) {
    ui->spinCCWAngle->setValue(qAbs(value));
}

int InletPropDialog::getCWAngle() const {
    return tool->getMinAngle();
}

void InletPropDialog::setCWAngle(int value) {
    ui->spinCWAngle->setValue(qAbs(value));
}

bool InletPropDialog::isMainInlet() const {
    return tool->segmentsVisible();
}

void InletPropDialog::setMainInlet(bool value) {
    ui->checkMainInlet->setChecked(value);
}

bool InletPropDialog::isCCW() const {
    return tool->getCounterClockwise();
}

void InletPropDialog::setCCW(bool value) {
    ui->checkCCW->setChecked(value);
}

int InletPropDialog::getSectors() const {
    return tool->getSegments();
}

void InletPropDialog::setSectors(int value) {
    ui->spinSectors->setValue(value);
}

int InletPropDialog::getSectorAngle() const {
    return tool->getDiffAngle();
}

void InletPropDialog::setSectorAngle(int value) {
    ui->spinSectorSize->setValue(value);
}

void InletPropDialog::on_checkMainInlet_toggled(bool checked) {
    /* Show the segments (if the tool was already created) */
    if (tool != nullptr) {
        tool->showSegments(checked);
    }

    /* This just enables/disables the properties for the main inlet */
    ui->labelOuterRadius->setEnabled(checked);
    ui->spinOuterRadius->setEnabled(checked);

    ui->labelRefAngle->setEnabled(checked);
    ui->spinRefAngle->setEnabled(checked);
    ui->labelCCWAngle->setEnabled(checked);
    ui->spinCCWAngle->setEnabled(checked);
    ui->labelCWAngle->setEnabled(checked);
    ui->spinCWAngle->setEnabled(checked);
    ui->checkCCW->setEnabled(checked);

    ui->labelSectors->setEnabled(checked);
    ui->spinSectors->setEnabled(checked);
    ui->labelSectorSize->setEnabled(checked);
    ui->spinSectorSize->setEnabled(checked);
}

void InletPropDialog::updateView() {
    /* Fit the image into the view */
    ui->imageView->fitInView(pixmap, Qt::KeepAspectRatio);
    ui->imageView->setSceneRect(pixmap->boundingRect());

    /* Adjust tool item: it should always be in the middle of the pixmap
     * since this is how the background image was created/adjusted. */
    tool->setOrigin(pixmap->boundingRect().center());

    /* Update viewport */
    ui->imageView->viewport()->update();
}

void InletPropDialog::checkToolValues() {
    /* This only works if the tool is already created */
    if (tool == nullptr)
        return;

    /* Block signals, otherwise they will emit value changed signals and
     * then create an infinite loop. */
    ui->spinInnerRadius->blockSignals(true);
    ui->spinOuterRadius->blockSignals(true);
    ui->spinRefAngle->blockSignals(true);
    ui->spinCCWAngle->blockSignals(true);
    ui->spinCWAngle->blockSignals(true);

    /* Check values */
    ui->spinInnerRadius->setValue(tool->getInnerRadius());
    ui->spinOuterRadius->setValue(tool->getOuterRadius());
    ui->spinRefAngle->setValue(tool->getZeroAngle());
    ui->spinCCWAngle->setValue(tool->getMaxAngle());        /* max angle counterclockwise, i.e. ccw boundary */
    ui->spinCWAngle->setValue(qAbs(tool->getMinAngle()));   /* min angle counterclockwise, i.e. cw boundary */

    /* De-Block signals */
    ui->spinInnerRadius->blockSignals(false);
    ui->spinOuterRadius->blockSignals(false);
    ui->spinRefAngle->blockSignals(false);
    ui->spinCCWAngle->blockSignals(false);
    ui->spinCWAngle->blockSignals(false);

    /* Updates the view */
    updateView();
}

void InletPropDialog::on_spinInnerRadius_valueChanged(int value) {
    tool->setInnerRadius(value);
    checkToolValues();
}

void InletPropDialog::on_spinOuterRadius_valueChanged(int value) {
    tool->setOuterRadius(value);
    checkToolValues();
}


void InletPropDialog::on_spinRefAngle_valueChanged(int value) {
    tool->setZeroAngle(value);
    checkToolValues();
}

void InletPropDialog::on_spinCCWAngle_valueChanged(int value) {
    tool->setMaxAngle(qAbs(value));
    checkToolValues();
}

void InletPropDialog::on_spinCWAngle_valueChanged(int value) {
    tool->setMinAngle(-1 * qAbs(value));
    checkToolValues();
}

void InletPropDialog::on_checkCCW_toggled(bool checked) {
    if (tool != nullptr) {
        tool->setCounterClockwise(checked);
    }

    if (checked) {
        ui->spinCCWAngle->setPrefix("＋");
        ui->spinCWAngle->setPrefix("－");
    } else {
        ui->spinCCWAngle->setPrefix("－");
        ui->spinCWAngle->setPrefix("＋");
    }

    checkToolValues();
}

void InletPropDialog::on_spinSectors_valueChanged(int value) {
    tool->setSegments(value);
    checkToolValues();
}

void InletPropDialog::on_spinSectorSize_valueChanged(int value) {
    tool->setDiffAngle(value);
    checkToolValues();
}
