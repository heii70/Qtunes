######################################################################
# Automatically generated by qmake (3.0) Sun Mar 8 01:33:31 2015
######################################################################

QT += multimedia opengl
QT += widgets
QT += gui

RESOURCES += Icons.qrc

INCLUDEPATH += -I C:\Qt\Tools\taglib_1.9.1\Dynamic
INCLUDEPATH += -I C:\Qt\Tools\taglib_1.9.1\Dynamic\include\Headers
INCLUDEPATH += -I C:\MinGW\i686-w64-mingw32\include\GL
LIBS += -LC:\Qt\Tools\taglib_1.9.1\Dynamic\lib
LIBS += -LC:\Qt\Tools\taglib_1.9.1\Dynamic\bin
LIBS += -ltag
CONFIG += console
TEMPLATE = app
TARGET = qtunes

# Input
HEADERS += MainWindow.h  squareswidget.h visualizer.h
SOURCES += main.cpp MainWindow.cpp  squareswidget.cpp visualizer.cpp
