######################################################################
# Automatically generated by qmake (3.0) Sun Mar 8 01:33:31 2015
######################################################################

QT -= core gui
QT += multimedia
QT += widgets

INCLUDEPATH += -I /usr/local/include/taglib
LIBS += -L /usr/local/lib 
LIBS += -ltag
CONFIG += console

TEMPLATE = app
TARGET = qtunes

# Input
HEADERS += MainWindow.h
SOURCES += main.cpp MainWindow.cpp