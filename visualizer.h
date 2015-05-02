#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <iostream>
#include <fstream>
#include <assert.h>
#include <QtWidgets>
#include <QGLWidget>
#include <QOpenGLWidget>
#include <QMediaPlayer>

class VisualizerWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    VisualizerWidget();
    ~VisualizerWidget();
public slots:
    void s_randomizer();
    void s_toggle(QMediaPlayer::State state);
    void s_changeSpeed(signed int s);
protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    //QList<float> *list;
    float numArray[10];
    int m_width, m_height, m_scale;
    QTimer *m_newRandNum;
    QTimer *m_painter;
};
#endif
