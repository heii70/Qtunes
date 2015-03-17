#include "squareswidget.h"
#include <QOpenGLFunctions>

SquaresWidget::SquaresWidget(){
	//initializeGL();
	//resize(600, 200);
	setMinimumSize(600,200);
	resize(600,200);
	setMaximumSize(600,200);
}

SquaresWidget::~SquaresWidget(){
}

void SquaresWidget::initializeGL(){
	QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-6, 6, -2, 2, -1, 1);
	//gluPerspective(150,2.5,5,2);
	//f->gluOrtho2D(-2.0, 2.0, -2.0, 2.0);
	glMatrixMode(GL_MODELVIEW);
	//f->glViewport(-300,-125,-6,-2.5);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

void SquaresWidget::resizeGL(int w, int h){
}

void SquaresWidget::paintGL(){
	glClear(GL_COLOR_BUFFER_BIT);
	for(float i = 0; i < 5; i++){
		glBegin(GL_POLYGON);
		glVertex3f(-4.5 + 2*i, -0.5, 0);
		glVertex3f(-4.5 + 2*i, 0.5, 0);
		glVertex3f(-3.5 + 2*i, 0.5, 0);
		glVertex3f(-3.5 + 2*i, -0.5, 0);
	glEnd();
	//glTranslatef(i, 0.0, 0.0);
	}
	glFlush();
}