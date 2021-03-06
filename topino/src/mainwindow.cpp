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

    /* Add the zoom level to the Overview docking window */
    ui->dockViewTools->setWindowTitle(QString("%1 - %2: %3%").arg(tr("Overview")).arg(tr("Zoom")).arg(100));

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
    document.addObserver(&angulagramView);
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

    /* Update the general pages of the object properties dock widget */
    updateObjectPage(objectPages::imageProps);
    updateObjectPage(objectPages::angulagramProps);

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
    ui->dockViewTools->setWindowTitle(QString("%1 - %2: %3%").arg(tr("Overview")).arg(tr("Zoom")).arg(imageView.getZoomFactor()*100.0));

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

    /* If this is the angulagram view, then update the respective
     * object page. */
    if(viewManager.currentIndex() == viewPages::angulagram) {
        updateObjectPage(objectPages::angulagramProps);
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

    /* Tell view that is about to show */
    getCurrentView()->showView();

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
                    bool ccw = inlet->getCounterClockwise();
                    ui->propInletRefAngle->setText(QString("%1°").arg(inlet->getZeroAngle()));
                    ui->propInletOuterRadius->setText(QString("%1 pixel").arg(inlet->getOuterRadius()));
                    ui->propInletAngleRange->setText(QString("%1° – %2°")
                                                     .arg(ccw ? inlet->getMinAngle() : -inlet->getMaxAngle())
                                                     .arg(ccw ? inlet->getMaxAngle() : -inlet->getMinAngle()));
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

    /* Fifth page: angulagram properties */
    case angulagramProps:
        /* Clear all data in the table first */
        ui->tableAnguStreams->clearContents();
        ui->tableAnguStreams->setRowCount(0);
        ui->tableAnguResolution->clearContents();
        ui->tableAnguResolution->setRowCount(0);

        /* Visible? */
        if(viewManager.currentIndex() == viewPages::angulagram) {
            ui->propAnguDataPoints->setText(QString::number(document.getData().getAngulagramPoints().length()));
            ui->propAnguStreams->setText(QString::number(document.getData().getStreamParameters().length()));

            QVector<AngulagramView::LegendItem> legendItems = angulagramView.getLegendItems();

            if (legendItems.length() == 0) {
                break;
            }

            /* For each item, we create a row and include all the data */
            ui->tableAnguStreams->setRowCount(legendItems.length());

            for(int i = 0; i < legendItems.length(); ++i) {
                /* Create a simple icon that will hold the color */
                QPixmap pixmap(14, 14);
                pixmap.fill(legendItems[i].color);

                QString posLabel;
                posLabel.sprintf("%+.1f", legendItems[i].pos);
                QTableWidgetItem *pos = new QTableWidgetItem(QIcon(pixmap), posLabel);
                pos->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

                QString widthLabel;
                widthLabel.sprintf("%.1f", legendItems[i].width);
                QTableWidgetItem *width = new QTableWidgetItem(widthLabel);
                width->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

                QString linLabel;
                linLabel.sprintf("%.2f", legendItems[i].rsquare);
                QTableWidgetItem *lin = new QTableWidgetItem(linLabel);
                lin->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

                ui->tableAnguStreams->setItem(i, 0, pos);
                ui->tableAnguStreams->setItem(i, 1, width);
                ui->tableAnguStreams->setItem(i, 2, lin);
            }

            /* For each item, calculate the resolution between itself and all other ones. Combining them
             * without having one resolution line doubled means we have (n * (n-1) / 2) lines! */
            ui->tableAnguResolution->setRowCount(legendItems.length() * (legendItems.length() - 1) / 2);

            int row = 0;
            for(int i = 0; i < legendItems.length(); ++i) {
                for(int j = (i+1); j < legendItems.length(); ++j) {
                    /* Create a simple icon for both streams */
                    QPixmap pixmap1(14, 14);
                    pixmap1.fill(legendItems[i].color);
                    QPixmap pixmap2(14, 14);
                    pixmap2.fill(legendItems[j].color);

                    /* For some reason, the QTableWidget does not allow to simply center the
                     * icon in the table. So, we use a QLabel here, give it the icon and put
                     * it in the respective cell. */
                    QLabel *stream1 = new QLabel();
                    stream1->setPixmap(pixmap1);
                    stream1->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                    QLabel *stream2 = new QLabel();
                    stream2->setPixmap(pixmap2);
                    stream2->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

                    QString resLabel;
                    resLabel.sprintf("%.2f", TopinoTools::calculateResolution(
                                         legendItems[i].pos, legendItems[i].width,
                                         legendItems[j].pos, legendItems[j].width));
                    QTableWidgetItem *res = new QTableWidgetItem(resLabel);
                    res->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

                    ui->tableAnguResolution->setCellWidget(row, 0, stream1);
                    ui->tableAnguResolution->setCellWidget(row, 1, stream2);
                    ui->tableAnguResolution->setItem(row, 2, res);

                    row++;
                }
            }
        }
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

bool MainWindow::isImageAvailable() const {
    if (document.getData().getImage().isNull()) {
        QMessageBox::information(nullptr, tr("No image data available"),
                                 tr("You need to import an image first before you can use the edit functions."));
        return false;
    }

    return true;
}

bool MainWindow::isAngulagramAvailable() const {
    if (!document.getData().isAngulagramAvailable()) {
        QMessageBox::information(nullptr, tr("No angulagram data available"),
                                 tr("You need to create a main inlet with polar coordinate system (in the image view) to "
                                    "acquire data for an angulagram."));
        return false;
    }

    return true;
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
    angulagramView.resetView();
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
    angulagramView.resetView();
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

    /* Save and notify everyone */
    document.setFullFilename(filename);
    document.saveToXML();
    document.notifyAllObserver();
}

void MainWindow::onImportImage() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open file to analyze"), "",
                       tr("Image files/CFE image files (*.png *.jpg *.FFE.png *.CFE.png "
                          "*.bmp *.gif *.jpeg *.tga *.tiff *.pbm *.pgm *.ppm *.xbm "
                          "*.xpm);;All files (*.*)"));

    if (filename.length() == 0)
        return;

    /* Try to load it, if it does not success abort the whole process */
    QImage img(filename);

    if (img.isNull()) {
        QMessageBox::critical(this, tr("Failed to load image file"), tr("The image file could not be loaded."));        
        return;
    }

    /* Check if the new image dimensions are the same as the old ones (but only if another image is
     * loaded of course!) - if not, we have to reset the view (remove all inlets, etc.) to avoid
     * some glitches. */
    const QImage &docimage = document.getData().getImage();
    if (!docimage.isNull() && ((docimage.width() != img.width()) || (docimage.height() != img.height()))) {
        qDebug("New image has different dimensions. Resetting free.");

        imageView.resetView();
        angulagramView.resetView();
        changeToView(viewPages::image);
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

void MainWindow::onQuit() {
    this->close();
}

void MainWindow::onCut() {
    getCurrentView()->cut(QGuiApplication::clipboard());
}

void MainWindow::onCopy() {
    getCurrentView()->copy(QGuiApplication::clipboard());
}

void MainWindow::onPaste() {
    getCurrentView()->paste(QGuiApplication::clipboard());
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
                                        "<p><b>Topino 1.1</b></p>"
                                        "<p><b>Copyright (C) 2021 by Sven Kochmann</b></p>"
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

    /* Calculate the integral of the angulagram points and see if it is above
     * 80% of the integral of (maxAngle-minAngle) × maxIntensity. If yes, this
     * means that the user probably did NOT prepare the image before proceeding. */
    QVector<QPointF> dataPoints = document.getData().getAngulagramPoints();

    if (dataPoints.length() == 0)
        return;

    /* Integral and maximum of data points */
    qreal int_datapoints = 0.0;
    qreal maximum = 0.0;
    for(auto it = dataPoints.begin(); it != dataPoints.end(); ++it) {
        int_datapoints += it->y();

        if (it->y() > maximum)
            maximum = it->y();
    }

    /* Divide by 10 because dataPoints are divided in 0.1° steps not 1.0° */
    int_datapoints /= 10.0;

    qDebug("Maximum: %.2f", maximum);
    qDebug("Integral datapoints: %.2f", int_datapoints);

    /* Integral of integral of (maxAngle-minAngle) × maxIntensity */
    qreal int_rectangle = (dataPoints.last().x() - dataPoints.first().x()) * maximum;
    qDebug("Integral rectangle: %.2f (x1: %.2f, x2: %.2f)", int_rectangle, dataPoints.first().x(), dataPoints.last().x());

    if (int_rectangle == 0.0)
        return;

    /* Ratio between both integrals should ba above > 0.90 */
    qreal ratio = int_datapoints / int_rectangle;
    qDebug("Ratio: %.2f", ratio);

    if (ratio >= 0.80) {
        qDebug("Ratio is above 80%%. user probably did not adjust the image.");

        QMessageBox::information(nullptr, tr("High background detected (integral > 80%)"),
                                 tr("It seems that the angulagram contains a huge amount of background (and probably"
                                    "negative peaks). Usually, such background is caused by not selecting any or "
                                    "non-optimal desaturation parameters.\n\n"
                                    "Please change back to the image view and select 'Preprocess image' from "
                                    "the object properties of the image."));
    }
}

void MainWindow::onToolEditImage() {
    /* Check if image is available */
    if (!isImageAvailable())
        return;

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
    /* Check if image is available */
    if (!isImageAvailable())
        return;

    /* Swap processed and source image */
    imageView.showSourceImage(!imageView.isSourceImageShown());
}

void MainWindow::onToolResetImage() {
    /* Check if image is available */
    if (!isImageAvailable())
        return;

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

void MainWindow::onToolExportImage() {
    qDebug("Exporting analysis image");

    if (!isImageAvailable())
        return;

    QString filename = document.getFilename();

    /* Remove the extension if it exists and add a new one */
    filename.replace(".topxml", "", Qt::CaseInsensitive);
    filename += ".png";

    filename = QFileDialog::getSaveFileName(this, tr("Export analysis image"), filename,
                                            tr("Raster image file (*.png);;Scalable Vector files (*.svg);;All files (*.*)"));

    if (filename.length() == 0)
        return;

    qDebug("File exported: %s", filename.toStdString().c_str());

    /* Depending on the extension, we save either as vector image (SVG) or
     * as raster image (whatever QImage/QPixmap supports). */
    imageView.selectNone();
    QRectF rect = imageView.scene()->sceneRect();

    if (filename.endsWith(".svg", Qt::CaseInsensitive)) {
        qDebug("Save as vector image");

        /* Create a SVG generator, let the Angulagram view draw into it and
         * save the result. */
        QSvgGenerator generator;
        generator.setFileName(filename);
        generator.setTitle(document.getFilename());
        generator.setDescription(tr("Analysis image generated at ") + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
        generator.setSize(rect.size().toSize());
        generator.setViewBox(rect);

        QPainter paintVector(&generator);
        paintVector.translate(rect.topLeft().toPoint());
        imageView.scene()->render(&paintVector);
        paintVector.end();

        /* Raster image */
    } else {
        qDebug("Save as raster image");

        /* Create an image, let the Angulagram view draw into it, and
         * save the result as a file. */
        QImage imageData(rect.size().toSize(), QImage::Format_ARGB32);
        QPainter paintRaster(&imageData);
        paintRaster.setRenderHint(QPainter::Antialiasing);
        paintRaster.setCompositionMode (QPainter::CompositionMode_Source);
        paintRaster.fillRect(QRectF(QPointF(0, 0), rect.size()), Qt::transparent);
        paintRaster.setCompositionMode (QPainter::CompositionMode_SourceOver);
        imageView.scene()->render(&paintRaster);
        paintRaster.end();

        imageData.save(filename);
    }
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

void MainWindow::onToolRulerAsCCWBoundary() {
    qDebug("Use as left (ccw) boundary");
    onToolRulerAsBoundary(true);
}

void MainWindow::onToolRulerAsCWBoundary() {
    qDebug("Use as right (cw) boundary");
    onToolRulerAsBoundary(false);
}

void MainWindow::onToolRulerAsBoundary(bool ccw) {
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

    /* Get min/max line of inlet and calculcate the angle to the ruler line. For the
     * ruler line make sure that p1 is on the inlet */
    QLineF inletLine = ccw ? inlet->getMaxLine() : inlet->getMinLine();
    QLineF rulerLine = ruler->getLine();
    if (rulerLine.p2().toPoint() == inletLine.p1().toPoint()) {
        rulerLine.setPoints(rulerLine.p2(), rulerLine.p1());
    }
    int angle = (int)inletLine.angleTo(rulerLine);

    /* Revert the angle if it is above 180° */
    if (angle > 180) {
        angle -= 360;
    }
    qDebug("Angle between inlet min/max and ruler: %d", angle);

    /* CCW means "max" angle gets set, CW means "min" angle gets set */
    if (ccw) {
        inlet->setMaxAngle(inlet->getMaxAngle() + angle);
    } else {
        inlet->setMinAngle(inlet->getMinAngle() + angle);
    }

    imageView.onItemDataChanged(inlet);

    /* Update the ruler page and the view(port) */
    updateObjectPage(rulerProps);
    getCurrentView()->viewport()->update();
}

void MainWindow::onToolSetMainInlet() {
    /* Check if there is a single inlet selected */
    if (imageView.scene()->selectedItems().size() != 1)
        return;

    /* Get current inlet and main inlet pointer */
    PolarCircleToolItem *inlet = dynamic_cast<PolarCircleToolItem*>(imageView.scene()->selectedItems()[0]);

    if (inlet == nullptr)
        return;

    /* Get data */
    TopinoData data = document.getData();

    /* Is there a main inlet already defined? In this case, we need to
     * hide its segments drawing (i.e. the coordinate system). However,
     * we save the segments + diff angle and transfer it to the new one. */
    int diffangle = 0;
    int segments = 0;
    if (data.getMainInletID() > 0) {
        imageView.getMainInletTool()->showSegments(false);
        diffangle = imageView.getMainInletTool()->getDiffAngle();
        segments = imageView.getMainInletTool()->getSegments();
    }

    /* Change new item id in data */
    data.setMainInletID(inlet->getItemid());
    document.setData(data);

    /* Set main inlet ID to the ID of the currently selected inlet. Also
     * transfer data to new main inlet */
    inlet->showSegments(true);
    inlet->setZeroAngle(data.getCoordNeutralAngle());
    inlet->setMinAngle(data.getCoordMinAngle());
    inlet->setMaxAngle(data.getCoordMaxAngle());
    inlet->setDiffAngle(data.getCoordDiffAngle());
    inlet->setCounterClockwise(data.getCoordCounterClockwise());
    inlet->setOuterRadius(data.getCoordOuterRadius());
    inlet->setSegments(data.getCoordSectors());

    /* Transfer diff angle and segments if possible */
    if (segments > 0) {
        inlet->setSegments(segments);
    }

    if (diffangle > 0) {
        inlet->setDiffAngle(diffangle);
    }

    /* Update object page and view port */
    imageView.onItemDataChanged(inlet);
    updateObjectPage(inletProps);
    getCurrentView()->viewport()->update();

}

void MainWindow::onToolSwapInletBoundaries() {
    /* Check if there is a single inlet selected */
    if (imageView.scene()->selectedItems().size() != 1)
        return;

    /* Get inlet pointer */
    PolarCircleToolItem *inlet = dynamic_cast<PolarCircleToolItem*>(imageView.scene()->selectedItems()[0]);

    if (inlet == nullptr)
        return;

    /* Reverse direction */
    inlet->setCounterClockwise(!inlet->getCounterClockwise());

    /* Update item on view, object page, and view port */
    imageView.onItemDataChanged(inlet);
    updateObjectPage(inletProps);
    getCurrentView()->viewport()->update();
}

void MainWindow::onToolSnapInletToImage() {
    /* Right now, this code is disabled. Too buggy for now. Maybe next release! */
    qDebug("Snap inlet to image");
    /* Check if there is a single inlet selected */
    if (imageView.scene()->selectedItems().size() != 1)
        return;

    /* Get inlet pointer */
    PolarCircleToolItem *inlet = dynamic_cast<PolarCircleToolItem*>(imageView.scene()->selectedItems()[0]);

    if (inlet == nullptr)
        return;

    /* Use the rectange of the inner circle as search image - use processed image if available! */
    QImage searchImage = document.getData().getProcessedImage().copy(inlet->getInnerRect().toRect());
    QImage grayImage = searchImage.convertToFormat(QImage::Format_Grayscale8);
    grayImage.save("test2.png");

    /* Calculate centroid and use it as starting point for a set of 4 lines (8 directions) */
    QPointF centroid = TopinoTools::imageCentroid(grayImage);
    qDebug("Centroid: %.1f %.1f", centroid.x(), centroid.y());

    QList<QPointF> points = TopinoTools::imageSlopePoints(grayImage, centroid);
    QPointF massCenter = TopinoTools::getMassCenter(points);

    qDebug("Mass center: %.1f %.1f", massCenter.x(), massCenter.y());
    for (auto iter = points.begin(); iter != points.end(); ++iter) {
        qDebug("Point: %.1f %.1f", (*iter).x(), (*iter).y());

        searchImage.setPixelColor((*iter).toPoint(), QColor(255, 0, 0));
    }
    searchImage.save("test1.png");

    /* Less than three points make no sense to calculcate a circle and we will leave here. */
    if (points.size() < 3)
        return;

    /* Calculate the smallest circle for all points */
    qreal circleRadius = 0.0;
    QPointF circleCenter;
    TopinoTools::calculateSmallestCircle(points, circleCenter, circleRadius);
    qDebug("Smallest circle at %.1f %.1f with radius %.1f", circleCenter.x(), circleCenter.y(), circleRadius);

    /* Adjust the inlet and tell everyone */
    inlet->setOrigin(circleCenter + inlet->getInnerRect().topLeft());
    inlet->setInnerRadius(qRound(circleRadius));

    /* Update item on view, object page, and view port */
    imageView.onItemDataChanged(inlet);
    updateObjectPage(inletProps);
    getCurrentView()->viewport()->update();
}

void MainWindow::onToolEditInlet() {
    qDebug("Edit inlet props");

    /* Check if there is a single inlet selected */
    if (imageView.scene()->selectedItems().size() != 1)
        return;

    /* Get inlet pointer */
    PolarCircleToolItem *inlet = dynamic_cast<PolarCircleToolItem*>(imageView.scene()->selectedItems()[0]);

    if (inlet == nullptr)
        return;

    /* Use the outer rectangle of the inlet as background image */
    QImage backgroundImage = document.getData().getImage().copy(inlet->boundingRect().toRect());

    /* Opens the inlet properties dialog */
    InletPropDialog dlg(this);

    /* Copy the data from the inlet to the dialog */
    dlg.setBackgroundImage(backgroundImage);
    dlg.setPosition(inlet->getOrigin());

    dlg.setInnerRadius(inlet->getInnerRadius());

    if (inlet->segmentsVisible()) {
        dlg.setMainInlet(true);

        dlg.setOuterRadius(inlet->getOuterRadius());

        dlg.setRefAngle(inlet->getZeroAngle());
        dlg.setCCWAngle(inlet->getMaxAngle());
        dlg.setCWAngle(inlet->getMinAngle());
        dlg.setCCW(inlet->getCounterClockwise());

        dlg.setSectors(inlet->getSegments());
        dlg.setSectorAngle(inlet->getDiffAngle());
    }

    /* Execute in a modal format and apply options if accepted */
    if (dlg.exec() == QDialog::DialogCode::Accepted) {
        qDebug("Dialog accepted");

        /* Update the inlet properties */
        inlet->setInnerRadius(dlg.getInnerRadius());
        inlet->showSegments(dlg.isMainInlet());

        inlet->setOuterRadius(dlg.getOuterRadius());
        inlet->setZeroAngle(dlg.getRefAngle());
        inlet->setMinAngle(dlg.getCWAngle());
        inlet->setMaxAngle(dlg.getCCWAngle());
        inlet->setCounterClockwise(dlg.isCCW());

        inlet->setSegments(dlg.getSectors());
        inlet->setDiffAngle(dlg.getSectorAngle());

        /* Update item on view, object page, and view port */
        imageView.onItemDataChanged(inlet);
        updateObjectPage(inletProps);
        getCurrentView()->viewport()->update();
    }
}

void MainWindow::onToolEvaluateAngulagram() {
    qDebug("Evaluate angulagram!");

    if (!isAngulagramAvailable())
        return;

    /* Opens the evaluation dialog for processing and setting parameters */
    EvalAngulagramDialog dlg(this);

    /* Set data values and some options */
    dlg.setDataPoints(document.getData().getAngulagramPoints());
    dlg.setOrientationRTL(document.getData().getCoordCounterClockwise());
    dlg.setAngularRange(QPair<int, int>(document.getData().getCoordMinAngle(), document.getData().getCoordMaxAngle()));

    /* Execute in a modal format and apply options if accepted */
    if (dlg.exec() == QDialog::DialogCode::Accepted) {
        qDebug("Fitting accepted");

        /* Save the fits found as stream parameters in data. */
        TopinoData data = document.getData();
        data.setStreamParameters(dlg.getLorentzians());
        document.setData(data);

        updateObjectPage(angulagramProps);
        getCurrentView()->viewport()->update();
    }
}

void MainWindow::onToolExportAngulagram() {
    qDebug("Exporting angulagram image");

    if (!isAngulagramAvailable())
        return;

    QString filename = document.getFilename();

    /* Remove the extension if it exists and add a new one */
    filename.replace(".topxml", "", Qt::CaseInsensitive);
    filename += ".png";

    filename = QFileDialog::getSaveFileName(this, tr("Export angulagram graph"), filename,
                                            tr("Raster image file (*.png);;Scalable Vector files (*.svg);;All files (*.*)"));

    if (filename.length() == 0)
        return;

    qDebug("File exported: %s", filename.toStdString().c_str());

    /* Depending on the extension, we save either as vector image (SVG) or
     * as raster image (whatever QImage/QPixmap supports). */
    if (filename.endsWith(".svg", Qt::CaseInsensitive)) {
        qDebug("Save as vector image");

        /* Create a SVG generator, let the Angulagram view draw into it and
         * save the result. */
        QSvgGenerator generator;
        generator.setFileName(filename);
        generator.setTitle(document.getFilename());
        generator.setDescription(tr("Angulagram graph generated at ") + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
        generator.setSize(angulagramView.size());
        generator.setViewBox(QRect(QPoint(0, 0), angulagramView.size()));

        QPainter paintVector(&generator);
        angulagramView.render(&paintVector);
        paintVector.end();

        /* Raster image */
    } else {
        qDebug("Save as raster image");

        /* Create an image, let the Angulagram view draw into it, and
         * save the result as a file. */
        QImage imageData(angulagramView.size(), QImage::Format_ARGB32);
        QPainter paintRaster(&imageData);
        paintRaster.setRenderHint(QPainter::Antialiasing);
        paintRaster.setCompositionMode (QPainter::CompositionMode_Source);
        paintRaster.fillRect(QRectF(QPointF(0, 0), angulagramView.size()), Qt::transparent);
        paintRaster.setCompositionMode (QPainter::CompositionMode_SourceOver);
        angulagramView.render(&paintRaster);
        paintRaster.end();

        imageData.save(filename);
    }
}

void MainWindow::onToolExportAngulagramData() {
    qDebug("Exporting angulagram data");

    if (!isAngulagramAvailable())
        return;

    QString filename = document.getFilename();

    /* Remove the extension if it exists and add a new one */
    filename.replace(".topxml", "", Qt::CaseInsensitive);
    filename += ".txt";

    filename = QFileDialog::getSaveFileName(this, tr("Export angulagram data"), filename,
                                            tr("Text files (*.txt);;All files (*.*)"));

    if (filename.length() == 0)
        return;

    document.exportDataToText(filename);
}

void MainWindow::onToolShowPolarImage() {
    qDebug("Show polar image");

    /* Only works if there is a polar image available. */
    if (document.getData().getPolarImage().isNull() || (document.getData().getMainInletID() == 0)) {
        QMessageBox::information(nullptr, tr("No polar image data available"),
                                 tr("You need to process the image first by creating an inlet with a polar coordinate system before using"
                                    "this function."));
        return;
    }

    /* Setup the dialog and give it the image data it needs */
    PolarImageDialog dlg(this);

    dlg.setPolarImage(document.getData().getPolarImage());
    int sign = document.getData().getCoordCounterClockwise() ? -1 : 1;
    dlg.setAngleRange(QPair<int, int>(sign * qAbs(document.getData().getCoordMinAngle()),
                                      -1 * sign * qAbs(document.getData().getCoordMaxAngle())));

    if (dlg.exec() == QDialog::DialogCode::Accepted) {
        qDebug("Accepted (but useless in this case).");
    }
}

void MainWindow::onToolShowRadialgram() {
    qDebug("Show radialgram");

    /* Only works if there is a polar image available. */
    if (document.getData().getPolarImage().isNull() || (document.getData().getMainInletID() == 0)) {
        QMessageBox::information(nullptr, tr("No polar image data available"),
                                 tr("You need to process the image first by creating an inlet with a polar coordinate system before using"
                                    "this function."));
        return;
    }

    /* Setup the dialog and give it radial data it needs */
    RadialgramDialog dlg(this);

    /* Calculate the radialgram data */
    TopinoData data = document.getData();
    data.calculateRadialgramPoints();
    dlg.setDataPoints(data.getRadialgramPoints());

    if (dlg.exec() == QDialog::DialogCode::Accepted) {
        qDebug("Accepted (but useless in this case).");
    }
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

