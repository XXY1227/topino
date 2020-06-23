#include "include/mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), view(this, document) {
    ui->setupUi(this);
    setCentralWidget(&view);

    document.addObserver(this);
    document.addObserver(&view);
    document.notifyAllObserver();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::modelHasChanged() {

}

void MainWindow::onNew() {

}

void MainWindow::onOpen() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open file to analyze"), "",
                       tr("Topino files (*.topxml);;"
                          "Image files/CFE image files (*.png *.jpg *.bmp *.FFE.png *.CFE.png);;"
                          "All files (*.*)"));

    if (filename.length() == 0)
        return;

    QMessageBox::information(this, "Open filename", filename);

    QImage image(filename);
    view.showImage(image);
}

void MainWindow::onSave() {

}

void MainWindow::onSaveAs() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Save file and analysis"), "",
                       tr("Topino files (*.topxml);;"
                          "CFE image files (*.CFE.png)"));

    if (filename.length() == 0)
        return;

    QMessageBox::information(this, "Save filename", filename);
}

void MainWindow::onCloseFile() {

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

