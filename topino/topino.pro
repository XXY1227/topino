#-------------------------------------------------
#
# Project created by QtCreator 2020-06-17T17:11:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# This flag is needed to prevent gcc from producing "shared object"
# binary files. Such shared object files are wrongly identfied as
# shared libraries in desktop based file managers (e.g. Linux Mint/Caja)
# not allowing the user to double click the file
QMAKE_LFLAGS += -no-pie

# Select C++14
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
    ui/darkstyle/darkstyle.cpp

HEADERS += \
    include/mainwindow.h \
    include/iobserver.h \
    include/topinodocument.h \
    include/imageanalysisview.h \
    ui/darkstyle/darkstyle.h

FORMS += \
    ui/mainwindow.ui

RESOURCES += \
    topino.qrc
