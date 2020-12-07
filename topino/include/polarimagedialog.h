#ifndef POLARIMAGEDIALOG_H
#define POLARIMAGEDIALOG_H

#include <QAbstractButton>
#include <QDialog>
#include <QFileDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>

namespace Ui {
class PolarImageDialog;
}

class PolarImageDialog : public QDialog {
    Q_OBJECT

  public:
    explicit PolarImageDialog(QWidget *parent = 0);
    ~PolarImageDialog();

    /* General dialog events */
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

    /* Exports the image to a file */
    void onExport();

    /* Set the image and data to show */
    void setPolarImage(const QImage& value);
    void setAngleRange(const QPair<int, int>& value);

  public slots:
    void buttonClicked(QAbstractButton *button);

  private:
    /* Setup the user interface and so on */
    Ui::PolarImageDialog *ui;
    QGraphicsScene *scene = nullptr;
    QGraphicsPixmapItem *pixmap = nullptr;

    void updateLabels();

    /* Image and data to show */
    QImage polarImage;
    QPair<int, int> angleRange;
};

#endif // POLARIMAGEDIALOG_H
