#-------------------------------------------------
#
# Project created by QtCreator 2018-05-23T10:29:06
#
#-------------------------------------------------

QT       += core gui
QT       += xlsx


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Spline_demo
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    spline.cpp

HEADERS  += mainwindow.h \
    spline.h

FORMS    += mainwindow.ui
