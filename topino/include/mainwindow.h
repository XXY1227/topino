#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QImage>
#include <QPainter>
#include <QStackedWidget>
#include <QSvgGenerator>

#include "include/iobserver.h"
#include "include/topinodocument.h"

#include "include/angulagramview.h"
#include "include/evalangulagramdialog.h"
#include "include/imageanalysisview.h"
#include "include/imageeditdialog.h"
#include "include/inletpropdialog.h"
#include "include/polarimagedialog.h"

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
    void onQuit();

    void onCut();
    void onCopy();
    void onPaste();
    void onErase();
    void onSelectAll();
    void onSelectNone();
    void onSelectNext();

    void onAboutQt();
    void onAboutTopino();

    /* Change tool buttons */
    void onToolSelection() {
        changeTool(TopinoAbstractView::selection);
    }
    void onToolRuler() {
        changeTool(TopinoAbstractView::rulerLine);
    }
    void onToolInletCircle() {
        changeTool(TopinoAbstractView::inletCircle);
    }

    /* Viewing buttons/functions */
    void onToolInputImage();
    void onToolAngulagram();

    /* Image buttons/functions */
    void onToolEditImage();
    void onToolSwitchImage();
    void onToolResetImage();
    void onToolExportImage();

    /* Ruler buttons/functions */
    void onToolSnapRulerToCorner();
    void onToolSnapRulerToInlet();
    void onToolExtendInletToRuler();
    void onToolRulerAsRefAngle();
    void onToolRulerAsCCWBoundary();
    void onToolRulerAsCWBoundary();
    void onToolRulerAsBoundary(bool ccw);

    /* Inlet buttons/functions */
    void onToolSetMainInlet();
    void onToolSwapInletBoundaries();
    void onToolSnapInletToImage();
    void onToolEditInlet();

    /* Angulagram functions */
    void onToolEvaluateAngulagram();
    void onToolExportAngulagram();
    void onToolExportAngulagramData();
    void onToolShowPolarImage();
    void onToolShowRadialgram();

    /* Multiple object functions */
    void onToolSelectOnlyRulers();
    void onToolSelectOnlyInlets();
    void onToolInletAtIntersection();

    /* Event slots */
    void onViewHasChanged();
    void onSelectionHasChanged();
    void onItemHasChanged(int itemID);

    private:
    enum objectPages {
        imageProps = 0,
        multipleProps = 1,
        rulerProps = 2,
        inletProps = 3,
        angulagramProps = 4,
        multipleRulerProps = 5,
        countPropPages = 6
    };

    enum viewPages {
        image = 0,
        angulagram = 1,
        countView = 2
    };

    Ui::MainWindow *ui;

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

    /* Checks if an image is available in the document. If not, it will show
     * a message to the user. */
    bool isImageAvailable() const;

    /* Checks if angulagram data exists in the document. If not, it will show
     * a message to the user. */
    bool isAngulagramAvailable() const;
};

#endif // MAINWINDOW_H
