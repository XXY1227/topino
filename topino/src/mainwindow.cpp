#include "include/mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow),
    imageView(this, document), angulagramView(this, document) {
    /* Basic layout/UI setup */
    ui->setupUi(this);

    /* Set up our "view manager" (a simple stacked widget that allows to switch between all views) and
     * the views themselves */
    setCentralWidget(&viewManager);
    viewManager.addWidget(&imageView);
    viewManager.addWidget(&angulagramView);
    viewManager.setCurrentIndex(viewPages::image);

    /* All tools are exclusive to select */
    ui->action_group_tools->setExclusive(true);

    /* Add these shortcuts 'manually' and just as text to these action items since they are
     * handled by the view(s) themselves (if supported/available). Otherwise, these keyboard
     * shortcuts are globally registered for Topino (which is a bad idea for standard keys
     * such as TAB, ESC, etc.). */
    ui->action_delete->setText(ui->action_delete->text() + "\t" + tr("Delete"));
    ui->action_select_none->setText(ui->action_select_none->text() + "\t" + tr("Escape"));
    ui->action_next_item->setText(ui->action_next_item->text() + "\t" + tr("Tab"));

    /* Add a label to the zoom toolbar that is updated with the current zoom level */
    zoomlabel.setText(tr("Zoom: 100%"));
    //zoomlabel.setStyleSheet("QLabel {color: #B8B8B8;}");
    ui->zoomBar->addWidget(&zoomlabel);

    /* Prepare all things for the object properties */
    ui->propertiesPages->setCurrentIndex(objectPages::imageProps);
    updateObjectPage(objectPages::imageProps);

    /* Prepare the thumbnail miniview */
    miniImage = new QGraphicsPixmapItem();
    miniScene = new QGraphicsScene(ui->miniView->contentsRect(), ui->miniView);
    miniRect = new QGraphicsRectItem();
    miniScene->addItem(miniImage);
    miniScene->addItem(miniRect);
    ui->miniView->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    ui->miniView->setScene(miniScene);
    ui->miniView->installEventFilter(this);

    /* The document is a reference to the model; here, all observers (mainwindow and view) are added, so that
     * they get notified if the model changes */
    document.addObserver(this);
    document.addObserver(&imageView);
    document.notifyAllObserver();

    /* If the view has been updated (e.g. zoomed in or the like) we need to know this, too. Here we implemeted
     * a signal-slot pair for this case */
    connect(&imageView, &ImageAnalysisView::viewHasChanged, this, &MainWindow::onViewHasChanged);
    connect(&imageView, &ImageAnalysisView::selectionHasChanged, this, &MainWindow::onSelectionHasChanged);
    connect(&imageView, &ImageAnalysisView::itemHasChanged, this, &MainWindow::onItemHasChanged);
}

MainWindow::~MainWindow() {
    delete ui;
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    /* Events for the miniview */
    if (dynamic_cast<QGraphicsView*>(watched) && (watched == ui->miniView)) {
        /* Miniview: click mouse to change the view viewport */
        if ((event->type() == QEvent::MouseButtonPress) && (getCurrentViewIndex() == viewPages::image)) {
            QMouseEvent *mevent = dynamic_cast<QMouseEvent*>(event);
            imageView.centerOn(ui->miniView->mapToScene(mevent->pos()));
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
    updateObjectPage(objectPages::imageProps);

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
    zoomlabel.setText(QString("%1: %2%").arg(tr("Zoom")).arg(imageView.getZoomFactor()*100.0));

    /* Set the view rectangle of the image view */
    QRectF viewport = imageView.getImageViewPoint();
    if (!viewport.contains(miniImage->boundingRect())) {
        miniRect->setRect(viewport);
    } else {
        miniRect->setRect(miniImage->boundingRect());
    }

    /* Set the current tool based on the view */
    switch(getCurrentView()->getCurrentTool()) {
    case TopinoAbstractView::tools::selection:
        ui->action_selection_tool->setChecked(true);
        break;
    case TopinoAbstractView::tools::rulerLine:
        ui->action_ruler_tool->setChecked(true);
        break;
    case TopinoAbstractView::tools::inletCircle:
        ui->action_inlet_tool->setChecked(true);
        break;
    default:
        break;
    }
}

void MainWindow::onSelectionHasChanged() {
    /* This is only for the image view */
    if (getCurrentViewIndex() != viewPages::image)
        return;

    int selitems = imageView.scene()->selectedItems().size();

    /* Nothing selected -> show general/image page */
    if (selitems == 0) {
        ui->propertiesPages->setCurrentIndex(objectPages::imageProps);
        return;
    }

    /* Exactly one item selected: find the type, select and update the respective object page */
    if (selitems == 1) {
        /* This is not really the best OO implementation, but straightforward; in future it might be replaced
         * by visitor pattern or the like */
        QGraphicsItem *widget = imageView.scene()->selectedItems()[0];
        TopinoGraphicsItem *item = dynamic_cast<TopinoGraphicsItem*>(widget);

        /* No graphics item selected -> show general page */
        if (item == nullptr) {
            ui->propertiesPages->setCurrentIndex(objectPages::imageProps);
            return;
        }

        /* Check item type; default is again the general/image page */
        objectPages selectPage = objectPages::imageProps;
        switch(item->getItemType()) {
        case TopinoGraphicsItem::itemtype::ruler:
            selectPage = objectPages::rulerProps;
            break;
        case TopinoGraphicsItem::itemtype::inlet:
            selectPage = objectPages::inletProps;
            break;
        default:
            break;
        }

        /* Update respective page and then make sure it is shown */
        updateObjectPage(selectPage);
        ui->propertiesPages->setCurrentIndex(selectPage);

        return;
    }

    /* More items are selected; check if all are of the same type. If yes,
     * show the respective object page, if it exists. */
    TopinoGraphicsItem::itemtype type = imageView.getItemTypeOfSelection();

    objectPages selectPage = objectPages::multipleProps;
    switch(type) {
    /* Multiple rulers selected */
    case TopinoGraphicsItem::itemtype::ruler:
        selectPage = objectPages::multipleRulerProps;
        break;

    /* That means either there are multiple items of different types
     * selected or there is no page of a group of the same item type,
     * yet. So, let's show the standard page for multiple items. */
    default:
        break;
    }

    /* Update respective page and then make sure it is shown */
    updateObjectPage(selectPage);
    ui->propertiesPages->setCurrentIndex(selectPage);
}

void MainWindow::onItemHasChanged(int itemID) {
    qDebug("Main windows: item %d has changed", itemID);

    onSelectionHasChanged();
}

void MainWindow::changeTool(TopinoAbstractView::tools tool) {
    getCurrentView()->setCurrentTool(tool);
}

void MainWindow::changeToView(const viewPages value) {
    /* Check if value is out of boundaries */
    if (value >= viewPages::countView)
        return;

    /* Set the page in the view manager */
    viewManager.setCurrentIndex(value);

    /* Check the availability of tools for the respective view */
    QAction *tools[TopinoAbstractView::tools::toolCount] = {
        ui->action_selection_tool,
        ui->action_ruler_tool,
        ui->action_inlet_tool
    };
    for (int i = 0; i < TopinoAbstractView::tools::toolCount; ++i) {
        tools[i]->setEnabled(getCurrentView()->isToolSupported(static_cast<TopinoAbstractView::tools>(i)));
    }

    /* Check availability of edit functions for the respective view */
    QAction *editfuncs[TopinoAbstractView::editfunc::editfuncCOUNT] = {
        ui->action_cut,
        ui->action_copy,
        ui->action_paste,
        ui->action_delete,
        ui->action_select_all,
        ui->action_select_none,
        ui->action_next_item
    };
    for (int i = 0; i < TopinoAbstractView::editfunc::editfuncCOUNT; ++i) {
        editfuncs[i]->setEnabled(getCurrentView()->isEditFunctionSupported(static_cast<TopinoAbstractView::editfunc>(i)));
    }

    /* Select the selection tool as standard */
    getCurrentView()->setCurrentTool(TopinoAbstractView::tools::selection);

    /* Page related stuff, e.g. show specific property pages,
     * (re)calculate the angulagram, etc. */
    TopinoData data = document.getData();
    switch(value) {
    /* Angulagram page */
    case viewPages::angulagram:
        updateObjectPage(objectPages::angulagramProps);
        ui->propertiesPages->setCurrentIndex(objectPages::angulagramProps);
        data.calculatePolarImage();
        data.calculateAngulagramPoints();
        document.setData(data);
        break;
    /* Default is the image page */
    case viewPages::image:
    default:
        onSelectionHasChanged();
        break;
    }

    /* View has changed */
    onViewHasChanged();
}

TopinoAbstractView* MainWindow::getCurrentView() {
    /* Get current page from view manager widget */
    viewPages value = getCurrentViewIndex();

    /* Check if value is out of boundaries */
    if (value >= viewPages::countView)
        return nullptr;

    /* Return the respective object */
    switch(value) {
    /* Image analysis page */
    case viewPages::image:
        return &imageView;
    /* Angulagram page */
    case viewPages::angulagram:
        return &angulagramView;
    /* Default is null pointer */
    default:
        return nullptr;
    }
}

MainWindow::viewPages MainWindow::getCurrentViewIndex() const {
    return static_cast<viewPages>(viewManager.currentIndex());
}

void MainWindow::updateObjectPage(MainWindow::objectPages page) {
    ui->propertiesPages->setEnabled(true);

    /* This function updates the information on the respective page of the object properties dock widget */
    switch (page) {
    case imageProps:
        /* First page: general document properties, image sizes, etc. */
        updateImagePage();
        break;
    case multipleProps:
        /* Second page: multiple objects of multiple types selected; we do not test here, just update */
        ui->propObjectsAmount->setText(QString::number(imageView.scene()->selectedItems().size()));
        break;

    case rulerProps:
        /* Third page: ruler properties */
        if (imageView.scene()->selectedItems().size() == 1) {
            QGraphicsItem *widget = imageView.scene()->selectedItems()[0];
            RulerToolItem *ruler = dynamic_cast<RulerToolItem*>(widget);

            if (ruler != nullptr) {
                QPoint p1 = ruler->mapToScene(ruler->getLine().p1().toPoint()).toPoint();
                QPoint p2 = ruler->mapToScene(ruler->getLine().p2().toPoint()).toPoint();
                ui->propRulerP1->setText(QString("%1, %2").arg(p1.x()).arg(p1.y()));
                ui->propRulerP2->setText(QString("%1, %2").arg(p2.x()).arg(p2.y()));
                ui->propRulerAngle->setText(QString("%1°").arg(ruler->getAngleToAbscissa()));
                ui->propRulerLength->setText(QString("%1 pixel").arg(
                                                 QString::number(ruler->getLine().length(), 'f', 1)));
            }
        }
        break;

    case inletProps:
        /* Fourth page: inlet properties */
        if (imageView.scene()->selectedItems().size() == 1) {
            QGraphicsItem *widget = imageView.scene()->selectedItems()[0];
            PolarCircleToolItem *inlet = dynamic_cast<PolarCircleToolItem*>(widget);

            if (inlet != nullptr) {
                /* Position and inner radius */
                QPoint pos = inlet->mapToScene(inlet->getOrigin().toPoint()).toPoint();
                ui->propInletPos->setText(QString("%1, %2").arg(pos.x()).arg(pos.y()));
                ui->propInletInnerRadius->setText(QString("%1 pixel").arg(inlet->getInnerRadius()));

                /* Check for main inlet data */
                if (document.getData().getMainInletID() == inlet->getItemid()) {
                    ui->propInletRefAngle->setText(QString("%1°").arg(inlet->getZeroAngle()));
                    ui->propInletOuterRadius->setText(QString("%1 pixel").arg(inlet->getOuterRadius()));
                    ui->propInletAngleRange->setText(QString("%1° – %2°").arg(inlet->getMinAngle()).arg(inlet->getMaxAngle()));
                    ui->propInletSector->setText(QString("%1 / %2°").arg(inlet->getSegments()).arg(inlet->getDiffAngle()));
                } else {
                    ui->propInletRefAngle->setText("-");
                    ui->propInletOuterRadius->setText("-");
                    ui->propInletAngleRange->setText("-");
                    ui->propInletSector->setText("-");
                }
            }
        }
        break;

    case angulagramProps:
        /* Fifth page: angulagram properties */
        break;

    case multipleRulerProps:
        /* Sixth page: multiple rulers selected */
        ui->propRulersAmount->setText(QString::number(imageView.scene()->selectedItems().size()));
        ui->propRulersIntersections->setText(QString::number(imageView.getNumberOfRulerIntersections(true)));
        break;

    default:
        break;
    }
}

void MainWindow::updateImagePage() {
    /* Get data */
    const TopinoData &data = document.getData();

    /* Check if there is an image in the data */
    if (!data.getImage().isNull()) {
        /* Pixel size */
        ui->propImageDimension->setText(QString("%1 × %2 Px²").arg(data.getImage().width()).arg(data.getImage().height()));

        /* Calculate the memory size. It's two times the bytesize of one image since there is
         * a processed image as well. */
        qreal byteSize = (qreal)data.getImage().byteCount() * 2.0;
        QString prefix = TopinoTools::getUnitPrefix(byteSize);
        ui->propImageSize->setText(QString::number(byteSize, 'f', 1) + " " + prefix + "Bytes");

        /* Processing parameters */
        ui->propImageDesatMode->setText(TopinoTools::getDesaturationModeName(data.getDesatMode()));
        ui->propImageInversion->setText(data.getInversion() ? tr("applied") : tr("not applied"));
        ui->propImageLevels->setText(QString::number(data.getLevelMin()) + " – " + QString::number(data.getLevelMax()));
    } else {
        ui->propertiesPages->setEnabled(false);
        ui->propImageDimension->setText("---");
        ui->propImageSize->setText("---");
        ui->propImageDesatMode->setText("---");
        ui->propImageInversion->setText("---");
        ui->propImageLevels->setText("---");
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
    imageView.resetView();
    changeToView(viewPages::image);

    document = TopinoDocument();
    document.addObserver(this);
    document.addObserver(&imageView);
    document.addObserver(&angulagramView);
    document.notifyAllObserver();
}

void MainWindow::onOpen() {
    /* Let the user select a filename and try to open this file as TopinoXML file */
    QString filename = QFileDialog::getOpenFileName(this, tr("Open file to analyze"), "",
                       tr("Topino files (*.topxml);;All files (*.*)"));

    if (filename.length() == 0)
        return;

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
    imageView.resetView();
    changeToView(viewPages::image);

    document = newdoc;
    document.addObserver(this);
    document.addObserver(&imageView);
    document.addObserver(&angulagramView);
    document.notifyAllObserver();

    /* Create objects from the document and set the document to saved state */
    imageView.createToolsFromDocument();
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

    /* Change to default view */
    changeToView(viewPages::image);
}

void MainWindow::onExportImage() {

}


void MainWindow::onQuit() {
    this->close();
}

void MainWindow::onCut() {
    getCurrentView()->cut();
}

void MainWindow::onCopy() {
    getCurrentView()->copy();
}

void MainWindow::onPaste() {
    getCurrentView()->paste();
}

void MainWindow::onErase() {
    getCurrentView()->erase();
}

void MainWindow::onSelectAll() {
    getCurrentView()->selectAll();
}

void MainWindow::onSelectNone() {
    getCurrentView()->selectNone();
}

void MainWindow::onSelectNext() {
    getCurrentView()->selectNext();
}

void MainWindow::onAboutQt() {
    QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::onAboutTopino() {
    /* About text of Topino */
    const QString aboutTopinoText = tr(
                                        "<p><b>Topino 1.0</b></p>"
                                        "<p><b>Copyright (C) 2020 by Sven Kochmann</b></p>"
                                        "<p>Topino is an analysis tool for continuous flow electrophoresis (CFE). It provides an easy "
                                        "to use graphical user-interface (GUI) for assessing CFE images by angulagrams [1].</p>"
                                        "<p>Topino was developed by <b>Sven Kochmann</b> in the research group of <b>Prof. Sergey N. "
                                        "Krylov</b>. It is using the Qt framework and, thus, is available for all major desktop operating "
                                        "systems (Linux/Unix, MacOS, Windows). It is open-source software licensed under BSD-3-Clause "
                                        "License; it's source code can be found on <a href=\"https://github.com/Schallaven/topino/\">"
                                        "Github</a>.</p>"
                                        "<p>The development of Topino was supported by a grant from Natural Sciences and Engineering "
                                        "Research Council of Canada to Sergey N. Krylov (grant number STPG-P 521331-2018). Please see "
                                        "<a href=\"https://www.yorku.ca/skrylov/\">Krylov group page</a> for more information on CFE "
                                        "research.</p>"
                                        "<p>References:</p>"
                                        "<style>ol {-qt-list-number-prefix: '['; -qt-list-number-suffix: ']'; -qt-list-indent: 1;}</style>"
                                        "<ol>"
                                        "<li>Kochmann <i>et al.</i>, <i>Anal. Chem.</i> <b>2018</b>, 90, 9504–9509. "
                                        "<a href=\"https://doi.org/10.1021/acs.analchem.8b02186\">Link</a>.</li>"
                                        "</ol>"
                                    );

    /* Present to user */
    QMessageBox::about(this, tr("About Topino"), aboutTopinoText);
}

void MainWindow::onToolInputImage() {
    qDebug("Input image");
    changeToView(viewPages::image);
}

void MainWindow::onToolAngulagram() {
    qDebug("Angulagram");
    changeToView(viewPages::angulagram);
}

void MainWindow::onToolEditImage() {
    /* Copy data */
    TopinoData data = document.getData();

    /* Open the image editing/preparing dialog to adjust saturation, levels, etc. before
     * analysis */
    ImageEditDialog dlg(this);

    /* We either look at the whole image OR cut out the bounding rect of the main inlet,
     * i.e. what the user wants to focus on. */
    QImage image = data.getImage();

    /* The rect can technically be OUTSIDE of the image, if that is the case, adjust */
    QRect rect = imageView.getFocusArea().toRect();
    rect.setLeft(rect.left() < 0 ? 0 : rect.left());
    rect.setTop(rect.top() < 0 ? 0 : rect.top());
    rect.setRight(rect.right() > image.width() ? image.width()-1 : rect.right());
    rect.setBottom(rect.bottom() > image.height() ? image.height()-1 : rect.bottom());

    /* Cut out the image */
    dlg.setImage(image.copy(rect));

    /* Set starting parameters */
    dlg.setInvert(data.getInversion());
    dlg.setDesaturationMode(data.getDesatMode());
    dlg.setLevelMin(data.getLevelMin());
    dlg.setLevelMax(data.getLevelMax());

    /* Execute in a modal format and apply options if accepted */
    if (dlg.exec() == QDialog::DialogCode::Accepted) {
        qDebug("Image edit: invert = %s", dlg.getInvert() ? "true" : "false");
        qDebug("Image edit: desaturation mode %d", dlg.getDesaturationMode());
        qDebug("Image edit: min %d max %d", dlg.getLevelMin(), dlg.getLevelMax());

        /* Process the image */
        data.setInversion(dlg.getInvert());
        data.setDesatMode(dlg.getDesaturationMode());
        data.setLevelMin(dlg.getLevelMin());
        data.setLevelMax(dlg.getLevelMax());
        data.processImage();

        /* Write back the data; set the view to show the processed image */
        imageView.showSourceImage(false);
        document.setData(data);

        /* Update the image page */
        updateImagePage();
    }
}

void MainWindow::onToolSwitchImage() {
    /* Swap processed and source image */
    imageView.showSourceImage(!imageView.isSourceImageShown());
}

void MainWindow::onToolResetImage() {
    /* Copy data */
    TopinoData data = document.getData();

    /* Reset processed image back to source image */
    data.resetProcessing();

    /* Write back the data; set the view to show the source image */
    imageView.showSourceImage(true);
    document.setData(data);

    /* Update the image page */
    updateImagePage();
}

void MainWindow::onToolSnapRulerToCorner() {
    /* Check if there is a single ruler selected */
    if (imageView.scene()->selectedItems().size() != 1)
        return;

    RulerToolItem *ruler = dynamic_cast<RulerToolItem*>(imageView.scene()->selectedItems()[0]);

    if (ruler == nullptr)
        return;

    /* Receive inlet position (if it exists) */
    QPointF inletPos = document.getData().getCoordOrigin();
    QPointF rulerPos[2] = {ruler->getLine().p1(), ruler->getLine().p2()};

    /* Snap both points to a density point near their terminal points (using the respective
     * bounding rectangle to determine the search area). Only exception: one of the points
     * is snapped the main inlet, i.e. inside the inner radius. */
    for (int p = 0; p < 2; ++p) {
        /* Compare point with inlet position; it is better to use the integer version for comparison
         * here (floats are not precise enough for equality-comparison) */
        if ((document.getData().getMainInletID() != 0) && (rulerPos[p].toPoint() == inletPos.toPoint())) {
            continue;
        }

        /* Rectangle as search area, create a sum area table from it and find max value */
        QImage searchImage = document.getData().getImage().copy(ruler->getRectOfTerminalPoint(p).toRect());
        QImage satImage = TopinoTools::imageSumAreaTable(searchImage);
        int maxValue = TopinoTools::imageMaxColorValue(satImage);

        /* Count number of points on the image with this max value. Should this be over 50% of the total
         * pixel number, then we invert the image here and calculate again (that usually is the case with
         * reflectometric images in which the background is very bright). */
        QList<QPointF> list;
        TopinoTools::imagePointsGrayValue(satImage, list, maxValue);

        if (list.size() > ((satImage.width() * satImage.height()) / 2)) {
            searchImage.invertPixels();
            satImage = TopinoTools::imageSumAreaTable(searchImage);
            maxValue = TopinoTools::imageMaxColorValue(satImage);

            list.clear();
            TopinoTools::imagePointsGrayValue(satImage, list, maxValue);
        }

        /* Calculate the mass center of the points found and set the ruler position
         * to this new center. Don't forget to add the top-left corner of the rectangle
         * to adjust for the origin! */
        rulerPos[p] = TopinoTools::getMassCenter(list) + ruler->getRectOfTerminalPoint(p).toRect().topLeft();
    }

    /* Set ruler positions */
    ruler->setLine(QLineF(rulerPos[0], rulerPos[1]));

    /* Update the ruler page and the view(port) */
    updateObjectPage(rulerProps);
    getCurrentView()->viewport()->update();
}

void MainWindow::onToolSnapRulerToInlet() {
    /* Check if there is a single ruler selected */
    if (imageView.scene()->selectedItems().size() != 1)
        return;

    RulerToolItem *ruler = dynamic_cast<RulerToolItem*>(imageView.scene()->selectedItems()[0]);

    if (ruler == nullptr)
        return;

    /* Let's check if there is a main inlet defined yet,
     * otherwise we will just leave here */
    if (document.getData().getMainInletID() == 0)
        return;

    /* Receive inlet and ruler position */
    QPointF inletPos = document.getData().getCoordOrigin();
    QLineF rulerLine = ruler->getLine();

    /* Calculate the distance of both points of the ruler item
     * to the main inlet. */
    qreal dist1 = qSqrt(qPow(inletPos.x() - rulerLine.p1().x(), 2.0) + qPow(inletPos.y() - rulerLine.p1().y(), 2.0));
    qreal dist2 = qSqrt(qPow(inletPos.x() - rulerLine.p2().x(), 2.0) + qPow(inletPos.y() - rulerLine.p2().y(), 2.0));

    /* Decide which distance is smaller and snap respective point to the inlet */
    if (dist1 < dist2) {
        rulerLine.setP1(inletPos);
    } else {
        rulerLine.setP2(inletPos);
    }
    ruler->setLine(rulerLine);

    /* Update the ruler page and the view(port) */
    updateObjectPage(rulerProps);
    getCurrentView()->viewport()->update();
}

void MainWindow::onToolExtendInletToRuler() {
    /* Check if there is a single ruler selected */
    if (imageView.scene()->selectedItems().size() != 1)
        return;

    /* Let's check if there is a main inlet defined yet,
     * otherwise we will just leave here */
    if (document.getData().getMainInletID() == 0)
        return;

    /* Get ruler and main inlet pointers */
    RulerToolItem *ruler = dynamic_cast<RulerToolItem*>(imageView.scene()->selectedItems()[0]);
    PolarCircleToolItem *inlet = imageView.getMainInletTool();

    if ((ruler == nullptr) || (inlet == nullptr))
        return;

    /* Receive inlet and ruler position */
    QPointF inletPos = document.getData().getCoordOrigin();
    QLineF rulerLine = ruler->getLine();

    /* Special case: ruler line is not a line but a single point - in this case, the distance
     * from the ruler to the inlet position is simply the distance between one of the points
     * and the inlet position. */
    int dist = 0;
    if (rulerLine.p1() == rulerLine.p2()) {
        dist = qSqrt(qPow(inletPos.x() - rulerLine.p1().x(), 2.0) + qPow(inletPos.y() - rulerLine.p1().y(), 2.0));

        /* Special case: ruler is a vertical line (both points have same x values); in this case,
         * the distance to the inlet position is simply the difference between the x values. */
    } else if (rulerLine.p1().x() == rulerLine.p2().x()) {
        dist = qAbs(inletPos.x() - rulerLine.p1().x());

        /* Special case: ruler is a horizontal line (both points have same y values); in this case,
         * the distance to the inlet position is simply the difference between the y values. */
    } else if (rulerLine.p1().y() == rulerLine.p2().y()) {
        dist = qAbs(inletPos.y() - rulerLine.p1().y());

        /* Otherwise, simply calculcate the slope and intercept of the ruler line, find the normal line to it, and
         * calculate the distance from the intercept to the inlet position. */
    } else {
        qreal rulerSlope = (rulerLine.p1().y() - rulerLine.p2().y()) / (rulerLine.p1().x() - rulerLine.p2().x());
        qreal rulerIntercept = rulerLine.p1().y() - rulerSlope * rulerLine.p1().x();

        qreal iX = (inletPos.x() + rulerSlope * (inletPos.y() - rulerIntercept)) / (qPow(rulerSlope, 2.0) + 1);
        qreal iY = rulerSlope * iX + rulerIntercept;

        dist = qSqrt(qPow(inletPos.x() - iX, 2.0) + qPow(inletPos.y() - iY, 2.0));
    }

    /* Update the inlet item and tell the view that it changed; it will synchronize
     * the data then automatically with the document */
    inlet->setOuterRadius(dist);
    imageView.onItemDataChanged(inlet);

    /* Update the ruler page and the view(port) */
    updateObjectPage(rulerProps);
    getCurrentView()->viewport()->update();
}

void MainWindow::onToolRulerAsRefAngle() {
    /* Check if there is a single ruler selected */
    if (imageView.scene()->selectedItems().size() != 1)
        return;

    /* Let's check if there is a main inlet defined yet,
     * otherwise we will just leave here */
    if (document.getData().getMainInletID() == 0)
        return;

    /* Get ruler and main inlet pointers */
    RulerToolItem *ruler = dynamic_cast<RulerToolItem*>(imageView.scene()->selectedItems()[0]);
    PolarCircleToolItem *inlet = imageView.getMainInletTool();

    if ((ruler == nullptr) || (inlet == nullptr))
        return;

    /* Receive inlet reference angle and the angle of the ruler */
    int refAngle = inlet->getZeroAngle();
    int rulerAngle = ruler->getAngleToAbscissa();

    /* If the reference angle and the ruler angle are more than 90° away from
     * each other, then add 180° to the ruler angle to not inverse the inlet;
     * if the angles are 270° from each other, this means that one is <90° and
     * the other one >270° - so they are actually closer than 90° to each other */
    int diffAngle = qAbs(refAngle - rulerAngle);
    if ((diffAngle > 90) && (diffAngle < 270)) {
        rulerAngle += 180;
    }

    /* Update the inlet item and tell the view that it changed; it will synchronize
     * the data then automatically with the document */
    inlet->setZeroAngle(rulerAngle);
    imageView.onItemDataChanged(inlet);

    /* Update the ruler page and the view(port) */
    updateObjectPage(rulerProps);
    getCurrentView()->viewport()->update();
}

void MainWindow::onToolRulerAsMinBoundary() {
    qDebug("Use as left boundary");
    onToolRulerAsMinMaxBoundary(false);
}

void MainWindow::onToolRulerAsMaxBoundary() {
    qDebug("Use as right boundary");
    onToolRulerAsMinMaxBoundary(true);
}

void MainWindow::onToolRulerAsMinMaxBoundary(bool max) {
    /* Check if there is a single ruler selected */
    if (imageView.scene()->selectedItems().size() != 1)
        return;

    /* Let's check if there is a main inlet defined yet,
     * otherwise we will just leave here */
    if (document.getData().getMainInletID() == 0)
        return;

    /* Get ruler and main inlet pointers */
    RulerToolItem *ruler = dynamic_cast<RulerToolItem*>(imageView.scene()->selectedItems()[0]);
    PolarCircleToolItem *inlet = imageView.getMainInletTool();

    if ((ruler == nullptr) || (inlet == nullptr))
        return;

    /* Get all inlet angles and the angle of the ruler */
    int refAngle = inlet->getZeroAngle();
    int minAngle = inlet->getMinAngle() + refAngle;
    int maxAngle = inlet->getMaxAngle() + refAngle;
    int rulerAngle = ruler->getAngleToAbscissa();

    qDebug("Angles (ref, min, max, ruler): %d %d %d %d", refAngle, minAngle, maxAngle, rulerAngle);

    /* If the reference angle and the ruler angle are more than 90° away from
     * each other, then add 180° to the ruler angle to not inverse the inlet;
     * if the angles are 270° from each other, this means that one is <90° and
     * the other one >270° - so they are actually closer than 90° to each other */
    int diffAngle = qAbs(minAngle - rulerAngle);
    if ((diffAngle > 90) && (diffAngle < 270)) {
        rulerAngle -= 180;
    }

    /* Update the inlet item and tell the view that it changed; it will synchronize
     * the data then automatically with the document */
    if (max) {
        inlet->setMaxAngle(rulerAngle - refAngle);
    } else {
        inlet->setMinAngle(rulerAngle - refAngle);
    }
    imageView.onItemDataChanged(inlet);

    /* Update the ruler page and the view(port) */
    updateObjectPage(rulerProps);
    getCurrentView()->viewport()->update();
}

void MainWindow::onToolSelectOnlyRulers() {
    imageView.selectItemType(TopinoGraphicsItem::ruler, true);
}

void MainWindow::onToolSelectOnlyInlets() {
    imageView.selectItemType(TopinoGraphicsItem::inlet, true);
}

void MainWindow::onToolInletAtIntersection() {
    /* Get a list of intersection points and calculate the mass center
     * of these points. This mass center will be used to create a new
     * inlet. */
    QList<QPointF> list;
    imageView.getPointsOfRulerIntersections(list, true);
    QPointF center = TopinoTools::getMassCenter(list);

    /* Create an inlet a this center position. We do not provide a radius
     * here, so the view will just try to make one up. */
    imageView.createInletAtPos(center);
}



