#-------------------------------------------------
#
# Project created by QtCreator 2014-12-04T15:35:27
#
#-------------------------------------------------

QT       += core gui webkitwidgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += release
QMAKE_CXXFLAGS += -std=c++11

TARGET = "Vocabulary"

TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    auth.cpp \
    table.cpp

HEADERS  += mainwindow.h \
    auth.h \
    table.h

FORMS    += mainwindow.ui \
    auth.ui \
    table.ui
