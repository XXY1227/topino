#include "include/mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), view(this, document) {
    /* Basic layout/UI setup */
    ui->setupUi(this);
    setCentralWidget(&view);

    /* All tools are exclusive to select */
    ui->action_group_tools->setExclusive(true);

    /* Add a label to the zoom toolbar that is updated with the current zoom level */
    zoomlabel.setText(tr("Zoom: 100%"));
    //zoomlabel.setStyleSheet("QLabel {color: #B8B8B8;}");
    ui->zoomBar->addWidget(&zoomlabel);

    /* Prepare all things for the object properties */
    ui->propertiesPages->setCurrentIndex(objectPages::general);
    updateObjectPage(objectPages::general);

    /* Prepare the thumbnail miniview */
    miniImage = new QGraphicsPixmapItem();
    miniScene = new QGraphicsScene(ui->miniView->contentsRect(), ui->miniView);
    miniRect = new QGraphicsRectItem();
    miniScene->addItem(miniImage);
    miniScene->addItem(miniRect);
    ui->miniView->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    ui->miniView->setScene(miniScene);
    ui->miniView->installEventFilter(this);

    /* Prepare the angulagram micro view */
    angulascene = new QGraphicsScene(ui->microView->contentsRect(), ui->microView);

    /* The document is a reference to the model; here, all observers (mainwindow and view) are added, so that
     * they get notified if the model changes */
    document.addObserver(this);
    document.addObserver(&view);
    document.notifyAllObserver();

    /* If the view has been updated (e.g. zoomed in or the like) we need to know this, too. Here we implemeted
     * a signal-slot pair for this case */
    connect(&view, &ImageAnalysisView::viewHasChanged, this, &MainWindow::onViewHasChanged);
    connect(&view, &ImageAnalysisView::selectionHasChanged, this, &MainWindow::onSelectionHasChanged);
    connect(&view, &ImageAnalysisView::itemHasChanged, this, &MainWindow::onItemHasChanged);
}

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    /* Events for the miniview */
    if (dynamic_cast<QGraphicsView*>(watched) && (watched == ui->miniView)) {
        /* Miniview: click mouse to change the view viewport */
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mevent = dynamic_cast<QMouseEvent*>(event);            
            view.putImagePointInView(ui->miniView->mapToScene(mevent->pos()));
        }
    }

    return QMainWindow::eventFilter(watched, event);
}


void MainWindow::modelHasChanged() {
    /* Set title of the main window to include the filename and an asterisk */
    QString newtitle;
    newtitle = document.getFilename() + (document.hasChanged() ? tr("*") : tr("")) + tr(" - Topino");
    setWindowTitle(newtitle);

    /* Update the general page of the object properties dock widget */
    updateObjectPage(objectPages::general);

    /* Set image for the mini and micro view */
    miniImage->setPixmap(QPixmap());
    QImage image = document.getData().getImage();
    if (!image.isNull()) {
        /* Extract Pixmap from image */
        miniImage->setPixmap(QPixmap::fromImage(image));        

        /* Adjust rect; the pen width needs to be scaled with the whole scene, otherwise it will
         * not be visible; 1% of the whole width is ok */
        QPen minipen = QPen(QColor(255,0,0));
        minipen.setWidth(int(miniImage->boundingRect().width() * 0.01));
        miniRect->setPen(minipen);
        miniRect->setRect(miniImage->boundingRect());

        /* Fit the image into the view */
        ui->miniView->fitInView(miniScene->itemsBoundingRect(), Qt::KeepAspectRatio);
        ui->miniView->setSceneRect(miniImage->boundingRect());
    }

    update();
}

void MainWindow::onViewHasChanged() {
    /* Update zoom level */
    zoomlabel.setText(QString("%1: %2%").arg(tr("Zoom")).arg(view.getZoomFactor()*100.0));

    /* Set the view rectangle */    
    QRectF viewport = view.getImageViewPoint();
    if (!viewport.contains(miniImage->boundingRect())) {
        miniRect->setRect(viewport);
    } else {
        miniRect->setRect(miniImage->boundingRect());
    }

    /* Set the current tool based on the view */
    switch(view.getCurrentTool()) {
    case ImageAnalysisView::tools::selection:
        ui->action_selection_tool->setChecked(true);
        break;
    case ImageAnalysisView::tools::ruler:
        ui->action_ruler_tool->setChecked(true);
        break;
    case ImageAnalysisView::tools::inletCircle:
        ui->action_inlet_tool->setChecked(true);
        break;
    }
}

void MainWindow::onSelectionHasChanged() {
    int selitems = view.scene()->selectedItems().size();

    /* Nothing selected -> show general page */
    if (selitems == 0) {
        ui->propertiesPages->setCurrentIndex(objectPages::general);
        return;
    }

    /* Exactly one item selected: find the type, select and update the respective object page */
    if (selitems == 1) {
        /* This is not really the best OO implementation, but straightforward; in future it might be replaced
         * by visitor pattern or the like */
        QGraphicsItem *widget = view.scene()->selectedItems()[0];
        RulerToolItem *ruler = dynamic_cast<RulerToolItem*>(widget);

        /* Ruler tool selected */
        if (ruler != nullptr) {
            updateObjectPage(objectPages::ruler);
            ui->propertiesPages->setCurrentIndex(objectPages::ruler);
        }

        return;
    }

    /* More items are selected: check all selected items and count the different types  */
    /*for (auto iter = view.scene()->selectedItems().begin(); iter != view.scene()->selectedItems().end(); ++iter) {

    }*/
}

void MainWindow::onItemHasChanged(int itemID) {
    qDebug("Main windows: item %d has changed", itemID);

    //onSelectionHasChanged();
}

void MainWindow::changeTool(ImageAnalysisView::tools tool) {
    view.setCurrentTool(tool);
}

void MainWindow::updateObjectPage(MainWindow::objectPages page) {
    const TopinoData &data = document.getData();

    /* This function updates the information on the respective page of the object properties dock widget */
    switch (page) {
    case general:
        /* First page: general document properties, image sizes, etc. */
        if (!data.getImage().isNull()) {
            ui->propImageDimension->setText(QString("%1 × %2 Px²").
                                            arg(data.getImage().width()).
                                            arg(data.getImage().height()));
        } else {
            ui->propImageDimension->setText("---");
        }
        break;
    case multiple:
        /* Second page: multiple objects of multiple types selected; we do not test here, just update */
        ui->propObjectsAmount->setText(QString("%1 objects selected").arg(view.scene()->selectedItems().size()));
        break;
    case ruler:
        /* Third page: ruler properties */
        if (view.scene()->selectedItems().size() == 1) {
            QGraphicsItem *widget = view.scene()->selectedItems()[0];
            RulerToolItem *ruler = dynamic_cast<RulerToolItem*>(widget);

            if (ruler != nullptr) {
                QPoint p1 = ruler->mapToScene(ruler->getLine().p1().toPoint()).toPoint();
                QPoint p2 = ruler->mapToScene(ruler->getLine().p2().toPoint()).toPoint();
                ui->propRulerP1->setText(QString("%1, %2").arg(p1.x()).arg(p1.y()));
                ui->propRulerP2->setText(QString("%1, %2").arg(p2.x()).arg(p2.y()));
                ui->propRulerLength->setText(QString("%1 pixel").arg(
                                                 QString::number(ruler->getLine().length(), 'f', 1)));
            }
        }
        break;
    }
}

void MainWindow::onNew() {
    /* If the document has changed the user needs to be asked if to proceed */
    if (document.hasChanged()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::question(this, tr("The current document was not saved."),
                                    tr("Do you want to save it before creating a new one?"),
                                    QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort)
                                   );

        if (ret == QMessageBox::Yes) {
            onSave();
        } else if (ret == QMessageBox::Abort) {
            return;
        }
    }

    /* Create an empty document, override the old one, reset the view, notify everyone */
    view.resetView();

    document = TopinoDocument();
    document.addObserver(this);
    document.addObserver(&view);
    document.notifyAllObserver();
}

void MainWindow::onOpen() {
    /* If the document has changed the user needs to be asked if to proceed */
    if (document.hasChanged()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::question(this, tr("The current document was not saved."),
                                    tr("Do you want to save it before open another file?"),
                                    QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort)
                                   );

        if (ret == QMessageBox::Yes) {
            onSave();
        } else if (ret == QMessageBox::Abort) {
            return;
        }
    }

    /* Let the user select a filename and try to open this file as TopinoXML file */
    QString filename = QFileDialog::getOpenFileName(this, tr("Open file to analyze"), "",
                       tr("Topino files (*.topxml);;All files (*.*)"));

    if (filename.length() == 0)
        return;

    TopinoDocument newdoc;
    TopinoDocument::FileError err = newdoc.loadFromXML(filename);

    if (err != TopinoDocument::FileError::NoFailure) {
        qDebug("Loading '%s' was not successful. Error = %d.", filename.toStdString().c_str(), int(err));
        QMessageBox::warning(this, tr("File could not be loaded"),
                             tr("The processing of the file as TopinoXML was not successful."));

        return;
    }

    /* Only override the open document if loading of the new document was successful; don't forget to reset
     * the view! */
    view.resetView();
    document = newdoc;
    document.addObserver(this);
    document.addObserver(&view);
    document.notifyAllObserver();

    /* Create objects from the document and set the document to saved state */
    view.createToolsFromDocument();
    document.saveChanges();
}

void MainWindow::onSave() {
    /* Check if the user has chosen a filename already for the document */
    if (!document.hasFileName()) {
        onSaveAs();
        return;
    }

    /* Save and notify everyone */
    document.saveToXML();
    document.notifyAllObserver();
}

void MainWindow::onSaveAs() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Save file and analysis"), document.getFilename(),
                       tr("Topino files (*.topxml);;All files (*.*)"));

    if (filename.length() == 0)
        return;

    QMessageBox::information(this, "Blabla", filename);

    /* Save and notify everyone */
    document.setFullFilename(filename);
    document.saveToXML();
    document.notifyAllObserver();
}

void MainWindow::onImportImage() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open file to analyze"), "",
                       tr("Image files/CFE image files (*.png *.jpg *.bmp *.FFE.png *.CFE.png);;All files (*.*)"));

    if (filename.length() == 0)
        return;

    /* Try to load it, if it does not success abort the whole process */
    QImage img(filename);

    if (img.isNull()) {
        QMessageBox::critical(this, tr("Failed to load image file"), tr("The image file could not be loaded."));
        return;
    }

    /* Modify data */
    TopinoData data;
    document.getData(data);
    data.setImage(img);
    document.setData(data);
    document.notifyAllObserver();
}

void MainWindow::onExportImage() {

}


void MainWindow::onQuit() {
    this->close();
}

void MainWindow::onAboutQt() {
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::onAboutTopino() {
    QMessageBox::about(this, "About Topino", "Test");
}

