#include "include/mainwindow.h"
#include "ui/darkstyle/darkstyle.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>

void logMessageOutputHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    /* Create a message handler that redirects all qDebug, qInfo, etc to standard error instead of
     * the console; taken from the Qt5 example at https://doc.qt.io/qt-5/qtglobal.html */
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "<none>";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    }
}

int main(int argc, char *argv[]) {
    /* Install a message handler that redirects all qDebug, qInfo, etc to standard error instead of
     * the console */
    qInstallMessageHandler(&logMessageOutputHandler);

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
