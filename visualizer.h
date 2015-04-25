#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <iostream>
#include <fstream>
#include <assert.h>
#include <QtWidgets>
#include <QGLWidget>
#include <QOpenGLWidget>

class VisualizerWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
    VisualizerWidget();
    ~VisualizerWidget();
public slots:
    void s_randomizer();
protected:
	void initializeGL();
	void resizeGL(int w, int h);
    void paintGL();
	
private:
    QList<int> *list;
    int m_width, m_height, m_scale;
    float lowerbar;
    QTimer *m_newRandNum;
    QTimer *m_painter;
};
#endif
