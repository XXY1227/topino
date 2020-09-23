#ifndef IMAGEEDITDIALOG_H
#define IMAGEEDITDIALOG_H

#include <QDialog>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

#include <include/topinotool.h>

namespace Ui {
class ImageEditDialog;
}

class ImageEditDialog : public QDialog {
    Q_OBJECT

  public:
    explicit ImageEditDialog(QWidget *parent = 0);
    ~ImageEditDialog();

    /* General dialog events */
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

    /* Set the image to show in the dialog and work with */
    void setImage(const QImage& value);

    /* Set and get the values for the user */
    int getDesaturationMode() const;
    void setDesaturationMode(int value);

    int getLevelMin() const;
    void setLevelMin(int value);

    int getLevelMax() const;
    void setLevelMax(int value);

    bool getInvert() const;
    void setInvert(bool value);

  private slots:
    /* Widget events, state changes, etc. */
    void previewModeChanged(int index);

    void desaturationModeChanged(int index);
    void invertBoxChanged(int state);

    void levelsChanging(int min, int max);
    void levelsChanged(int min, int max);
    void minLevelChanged(int value);
    void maxLevelChanged(int value);

  private:
    /* All the UI data */
    Ui::ImageEditDialog *ui;

    /* Preview modes */
    enum previewModes {
        showProcessedImage = 0,
        showSourceImage = 1,
        showHalfhalf = 2
    };

    /* Desaturation modes */
    enum desaturationModes {
        desatLightness = 0,
        desatLuminance = 1,
        desatAverage = 2,
        desatMaximum = 3,
        desatRed = 4,
        desatGreen = 5,
        desatBlue = 6
    };

    /* Source and preview images */
    QImage sourceImage;
    QImage processedImage;
    QGraphicsScene *scene = nullptr;
    QGraphicsPixmapItem *pixmap = nullptr;

    /* Spin boxes were changed by the program */
    bool spinChanged = false;

    /* This function will calculate the working image to show */    
    void applyEditsToImage();

    /* Show image in preview */
    void showPreviewImage();
};

#endif // IMAGEEDITDIALOG_H
