#include <iostream>
#include <fstream>
#include <cstdlib>
#include <assert.h>
#include "visualizer.h"
#include <QOpenGLFunctions>
#include <glu.h>
#include <QtWidgets>
#include <QGLWidget>
using namespace std;

VisualizerWidget::VisualizerWidget(){
    m_width = 3;
    m_height = 1;
    m_scale = 250;
    setMinimumSize(m_width*m_scale,m_height*m_scale);
    resize(m_width*m_scale,m_height*m_scale);
    setMaximumSize(m_width*m_scale,m_height*m_scale);
    setSizeIncrement(m_width,m_height);
    for(int i = 0; i < 9; i++){
        list[i] = 1;
    }
    // Add qtimer that paints the bars of the visualizer
    m_painter = new QTimer(this);
    m_painter->start(50);
    connect(m_painter, SIGNAL(timeout()), this, SLOT(update()));
    // This generates new random values for the bars
    m_newRandNum = new QTimer(this);
    m_newRandNum->start(500);
    connect(m_newRandNum, SIGNAL(timeout()), this, SLOT(s_randomizer()));
}

VisualizerWidget::~VisualizerWidget(){
}

void VisualizerWidget::initializeGL(){
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
}

void VisualizerWidget::resizeGL(int w, int h){
    resize(w,h);
    glMatrixMode(GL_PROJECTION);
    glOrtho(0, 10, -0.1, 1.1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void VisualizerWidget::s_randomizer(){
    for(int i = 0; i < 9; i++){
        float random = rand() % 100 + 1;
        if(list[i] < random)
            list[i] = random;
    }

}

void VisualizerWidget::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    for(int i = 0; i < 9; i++){
        float barval = list[i]/100;
        glTranslatef(1,0,0);
        glColor3f((float)i/10 * barval,(1 - (float)i/10) * barval,(1 - (float)i/10) * barval);
        glPushMatrix();
        glBegin(GL_QUADS);
        glVertex3f(-0.5,0,0);
        glVertex3f(-0.5,barval,0);
        glVertex3f(0.5,barval,0);
        glVertex3f(0.5,0,0);
        glEnd();
        glPopMatrix();
        list[i] = list[i] - 1.5;
    }
    glFlush();
}
