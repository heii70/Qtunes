######################################################################
# Automatically generated by qmake (3.0) Mon Mar 16 20:40:13 2015
######################################################################

QT -= core gui
QT += multimedia
QT += widgets

INCLUDEPATH += -I /usr/local/include/taglib -I /opt/X11
LIBS += -L /usr/local/lib 
LIBS += -ltag 
CONFIG += console

TEMPLATE = app
TARGET = qtunes

# Input
HEADERS += MainWindow.h squareswidget.h
SOURCES += main.cpp MainWindow.cpp squareswidget.cpp
