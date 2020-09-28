#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QStackedWidget>

#include "include/iobserver.h"
#include "include/topinodocument.h"

#include "include/imageanalysisview.h"
#include "include/imageeditdialog.h"

#include "include/angulagramview.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public IObserver {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool eventFilter(QObject *watched, QEvent *event) override;

    void modelHasChanged() final;

  private slots:
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onImportImage();
    void onExportImage();
    void onQuit();

    void onAboutQt();
    void onAboutTopino();

    void onToolSelection() {
        changeTool(TopinoAbstractView::selection);
    }
    void onToolRuler() {
        changeTool(TopinoAbstractView::rulerLine);
    }
    void onToolInletCircle() {
        changeTool(TopinoAbstractView::inletCircle);
    }

    void onToolInputImage();
    void onToolAngulagram();

    void onToolEditImage();
    void onToolSwitchImage();
    void onToolResetImage();

    void onViewHasChanged();
    void onSelectionHasChanged();
    void onItemHasChanged(int itemID);

  private:
    enum objectPages {
        general = 0,
        multiple = 1,
        ruler = 2,
        inlet = 3,
        countPropPages = 4
    };

    enum viewPages {
        image = 0,
        angulagram = 1,
        countView = 2
    };

    Ui::MainWindow *ui;

    QLabel zoomlabel;

    TopinoDocument document;
    QStackedWidget viewManager;
    ImageAnalysisView imageView;
    AngulagramView angulagramView;

    /* Stuff for the mini/micro views on the top right corner */
    QGraphicsScene *miniScene = nullptr;
    QGraphicsPixmapItem *miniImage = nullptr;
    QGraphicsRectItem *miniRect = nullptr;

    QGraphicsScene *angulascene = nullptr;

    void changeTool(TopinoAbstractView::tools tool);
    void changeToView(const viewPages value);
    TopinoAbstractView *getCurrentView();
    viewPages getCurrentViewIndex() const;

    void updateObjectPage(objectPages page);
    void updateImagePage();
};

#endif // MAINWINDOW_H
