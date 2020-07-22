#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

#include "include/iobserver.h"
#include "include/topinodocument.h"
#include "include/imageanalysisview.h"

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
        changeTool(ImageAnalysisView::selection);
    }
    void onToolRuler() {
        changeTool(ImageAnalysisView::ruler);
    }
    void onToolInletCircle() {
        changeTool(ImageAnalysisView::inletCircle);
    }

    void onViewHasChanged();
    void onSelectionHasChanged();
    void onItemHasChanged(int itemID);

  private:
    enum objectPages {
        general = 0,
        multiple = 1,
        ruler = 2
    };

    Ui::MainWindow *ui;

    QLabel zoomlabel;

    TopinoDocument document;
    ImageAnalysisView view;

    /* Stuff for the mini/micro views on the top right corner */
    QGraphicsScene *miniScene = nullptr;
    QGraphicsPixmapItem *miniImage = nullptr;
    QGraphicsRectItem *miniRect = nullptr;

    QGraphicsScene *angulascene = nullptr;

    void changeTool(ImageAnalysisView::tools tool);

    void updateObjectPage(objectPages page);
};

#endif // MAINWINDOW_H
