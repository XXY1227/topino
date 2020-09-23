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

  private slots:
    /* Widget events, state changes, etc. */
    void invertBoxChanged(int state);
    void checkBoxChanged(int state);
    void levelsChanged(int min, int max);
    void minLevelChanged(int value);
    void maxLevelChanged(int value);

    private:
    Ui::ImageEditDialog *ui;

    QImage sourceImage;
    QImage workingImage;
    QImage leveledImage;
    QGraphicsScene *scene = nullptr;
    QGraphicsPixmapItem *pixmap = nullptr;

    /* This function will calculate the working image to show */
    void applyEditsToImage();

    /* Show image in preview */
    void showPreviewImage(const QImage& img);
};

#endif // IMAGEEDITDIALOG_H
