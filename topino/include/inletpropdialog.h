#ifndef INLETPROPDIALOG_H
#define INLETPROPDIALOG_H

#include <QDialog>

#include "include/polarcircletoolitem.h"

namespace Ui {
class InletPropDialog;
}

class InletPropDialog : public QDialog {
    Q_OBJECT

  public:
    explicit InletPropDialog(QWidget *parent = 0);
    ~InletPropDialog();

    /* General dialog events */
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

    /* Set background image and "real" position (to display) */
    void setBackgroundImage(const QImage &image);
    void setPosition(const QPointF &pos);

    /* Setter/Getter functions */
    int getInnerRadius() const;
    void setInnerRadius(int value);

    int getOuterRadius() const;
    void setOuterRadius(int value);

    int getRefAngle() const;
    void setRefAngle(int value);

    int getCCWAngle() const;
    void setCCWAngle(int value);

    int getCWAngle() const;
    void setCWAngle(int value);

    bool isMainInlet() const;
    void setMainInlet(bool value);

    bool isCCW() const;
    void setCCW(bool value);

    int getSectors() const;
    void setSectors(int value);

    int getSectorAngle() const;
    void setSectorAngle(int value);

  private slots:
    void on_checkMainInlet_toggled(bool checked);

    void on_spinInnerRadius_valueChanged(int value);
    void on_spinOuterRadius_valueChanged(int value);

    void on_spinRefAngle_valueChanged(int value);
    void on_spinCCWAngle_valueChanged(int value);
    void on_spinCWAngle_valueChanged(int value);
    void on_checkCCW_toggled(bool checked);

    void on_spinSectors_valueChanged(int value);
    void on_spinSectorSize_valueChanged(int value);

  private:
    /* UI elements */
    Ui::InletPropDialog *ui;

    /* Background image of the preview */
    QGraphicsScene *scene = nullptr;
    QGraphicsPixmapItem *pixmap = nullptr;

    /* Polar circle tool item is used for saving the
     * states as well as for preview purpose. */
    PolarCircleToolItem *tool = nullptr;

    /* Recalculates the view, tool positions, etc. */
    void updateView();

    /* Checks if all values could be applies to the tool and
     * readjusts the spin controls, etc. respectively. */
    void checkToolValues();
};

#endif // INLETPROPDIALOG_H
