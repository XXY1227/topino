#ifndef IMAGEEDITDIALOG_H
#define IMAGEEDITDIALOG_H

#include <QDialog>
#include <QImage>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

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
    void checkBoxChanged(int state);

  private:
    Ui::ImageEditDialog *ui;

    QImage sourceImage;
    QImage workingImage;
    QGraphicsScene *scene = nullptr;
    QGraphicsPixmapItem *pixmap = nullptr;

    /* This function will calculate the working image to show */
    void applyEditsToImage();
};

#endif // IMAGEEDITDIALOG_H
