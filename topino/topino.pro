#-------------------------------------------------
#
# Project created by QtCreator 2020-06-17T17:11:27
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets charts

# This flag is needed to prevent gcc from producing "shared object"
# binary files. Such shared object files are wrongly identfied as
# shared libraries in desktop based file managers (e.g. Linux Mint/Caja)
# not allowing the user to double click the file
QMAKE_LFLAGS += -no-pie

# Version of Topino
VERSION = 1.0.0

# Add an icon for the Windows exe file and the definitions for the
# resource file
RC_ICONS = ui/topino.ico
VERSION_PE_HEADER = 1.0
QMAKE_TARGET_DESCRIPTION = "Program to create and evaluate angulagrams"
QMAKE_TARGET_COPYRIGHT = "Sven Kochmann, 2020"

# Adding the include path for eigen3, which is in parallel to the topino
# source directory. Please see license, etc. in the eigen3 directory.
INCLUDEPATH += ../eigen3

# Select C++17
QMAKE_CXXFLAGS += -std=c++17

TARGET = topino
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/iobserver.cpp \
    src/topinodocument.cpp \
    src/imageanalysisview.cpp \
    ui/darkstyle/darkstyle.cpp \
    src/circlerubberband.cpp \
    src/linerubberband.cpp \
    src/topinorubberband.cpp \
    src/topinodata.cpp \
    src/rulertoolitem.cpp \
    src/topinographicsitem.cpp \
    src/polarcircletoolitem.cpp \
    src/inputimagetoolitem.cpp \
    src/imageeditdialog.cpp \
    src/histogramwidget.cpp \
    src/topinotool.cpp \
    src/topinoabstractview.cpp \
    src/angulagramview.cpp \
    src/inletpropdialog.cpp \
    src/evalangulagramdialog.cpp \
    src/polarimagedialog.cpp

HEADERS += \
    include/mainwindow.h \
    include/iobserver.h \
    include/topinodocument.h \
    include/imageanalysisview.h \
    ui/darkstyle/darkstyle.h \
    include/circlerubberband.h \
    include/linerubberband.h \
    include/topinorubberband.h \
    include/topinodata.h \
    include/rulertoolitem.h \
    include/topinographicsitem.h \
    include/polarcircletoolitem.h \
    include/inputimagetoolitem.h \
    include/imageeditdialog.h \
    include/topinotool.h \
    include/histogramwidget.h \
    include/topinoabstractview.h \
    include/angulagramview.h \
    include/inletpropdialog.h \
    include/evalangulagramdialog.h \
    include/polarimagedialog.h

FORMS += \
    ui/mainwindow.ui \
    ui/imageeditdialog.ui \
    ui/inletpropdialog.ui \
    ui/evalangulagramdialog.ui \
    ui/polarimagedialog.ui

RESOURCES += \
    topino.qrc
