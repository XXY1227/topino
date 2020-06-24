#include "include/mainwindow.h"
#include "ui/darkstyle/darkstyle.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[]) {
    /* Set up the Application object with a nice and dark style */
    QApplication a(argc, argv);

    a.setStyle(new DarkStyle());

    QFile f(":ui/darkstyle/darkstyle.qss");
    if (f.exists()) {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        a.setStyleSheet(ts.readAll());
    }

    /* Set up the main window, show it, and run the event loop */
    MainWindow w;
    w.show();

    return a.exec();
}
