#-------------------------------------------------
#
# Project created by QtCreator 2013-10-25T19:52:44
#
#-------------------------------------------------

QT       += core gui

OBJECTS_DIR = build
MOC_DIR = build
RCC_DIR = build
UI_DIR = build
DESTDIR = bin

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QWurthEasyScan
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qwurthscanner.cpp \
    formabout.cpp

HEADERS  += mainwindow.h \
    qwurthscanner.h \
    formabout.h

FORMS    += mainwindow.ui \
    formabout.ui
LIBS += -lQt5ExtSerialPort
