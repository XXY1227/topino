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

    /* Unfortunately, the layout designer of Qt Creator does not allow to add widgets to a toolbar; therefore,
     * an empty widget with the QHBoxLayout (which is in turn designed/layouted in MainWindow.ui) is added to
     * the toolbox-toolbar instead here */
    /*QWidget *emptyWidget = new QWidget();
    emptyWidget->setLayout(ui->layoutToolbox);
    ui->toolbox->addWidget(emptyWidget);*/

    /* The document is a reference to the model; here, all observers (mainwindow and view) are added, so that
     * they get notified if the model changes */
    document.addObserver(this);
    document.addObserver(&view);
    document.notifyAllObserver();

    /* If the view has been updated (e.g. zoomed in or the like) we need to know this, too. Here we implemeted
     * a signal-slot pair for this case */
    connect(&view, SIGNAL(viewHasChanged()), this, SLOT(onViewHasChanged()));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::modelHasChanged() {
    /* Set title of the main window to include the filename and an asterisk */
    QString newtitle;
    newtitle = document.getFilename() + (document.hasChanged() ? tr("*") : tr("")) + tr(" - Topino");
    setWindowTitle(newtitle);

    update();
}

void MainWindow::onViewHasChanged() {
    /* Update zoom level */
    zoomlabel.setText(QString("%1: %2%").arg(tr("Zoom")).arg(view.getZoomFactor()*100.0));
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

    /* Create an empty document, override the old one, notify everyone */
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

    /* Only override the open document if loading of the new document was successful */
    document = newdoc;
    document.addObserver(this);
    document.addObserver(&view);
    document.notifyAllObserver();
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

    document.setImage(img);
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

