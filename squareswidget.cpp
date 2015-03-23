#include "squareswidget.h"
#include <QOpenGLFunctions>
//#include <QtOpenGL>
#include <glu.h>
#include <QtWidgets>

SquaresWidget::SquaresWidget(){
	setMinimumSize(600,200);
	resize(600,200);
	setMaximumSize(600,200);
	setGeometry(0,0,600,200);
	//heightForWidth(3);
	setSizeIncrement(3,1);
	m_timer = new QTimer(this);
	m_timer->start(100);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(paintGL()));
}

SquaresWidget::~SquaresWidget(){
}

void SquaresWidget::initializeGL(){
	//gluPerspective(150,2.5,5,2); need glu library
	//can use look at to chnage things
	//f->gluOrtho2D(-2.0, 2.0, -2.0, 2.0);
	//f->glViewport(-300,-125,-6,-2.5);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //make last parameter 1
}

//system automatically gives you w and h?
void SquaresWidget::resizeGL(int w, int h){
	//resize(w,w/3);
	//setMinimumSize(600,200);
	//setMaximumSize(600,200);
	//resize(w,h);
	resize(w,h);
	glViewport(0,0,h*3,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//glOrtho(-6, 6, -2, 2, -0.5, 0.5);
	//glPerspective(90, 2, 0.1, 1000);
	//glFrustum(-12,12,-4, 4, 0.1, 1000);
	gluPerspective(60, 3, 0.1, 100);
	gluLookAt(0,0,3.5,
				0,0,1,
				0,1,0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
}

void SquaresWidget::paintGL(){
	//glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	/* 5 squares next to each other
	glTranslatef(-6,0,0);
	for(float i = 0; i < 5; i++){
		glTranslatef(2, 0.0, 0.0); 
		glColor3f(0.6,0.2*i,0.2*i);
		glBegin(GL_POLYGON);
		glVertex3f(-1, -1, 0);
		glVertex3f(-1, 1, 0);
		glVertex3f(1, 1, 0);
		glVertex3f(1, -1, 0);
		glEnd();
	}*/
	glPushMatrix();
	glColor3f(0,0,1);
	glBegin(GL_POLYGON);
		glVertex3f(-1, -1, 0);
		glVertex3f(-1, 1, 0);
		glVertex3f(1, 1, 0);
		glVertex3f(1, -1, 0);
		glEnd();
	glEnd();
	glColor3f(1,1,1);
	glBegin(GL_LINE_LOOP);
		glVertex3f(-1.01, -1.01, 0);
		glVertex3f(-1.01, 1.01, 0);
		glVertex3f(1.01, 1.01, 0);
		glVertex3f(1.01, -1.01, 0);
		glEnd();
	glEnd();
	glPopMatrix();
	glPushMatrix();
	for(float i = 1; i <= 3; i++){
		glColor3f(0.3,0,0.9/i);
		glTranslatef(2,0,0);
		glPushMatrix();
		glRotatef(90,0,1,0);
		glBegin(GL_POLYGON);
		glVertex3f(-1, -1, 0);
		glVertex3f(-1, 1, 0);
		glVertex3f(1, 1, 0);
		glVertex3f(1, -1, 0);
		glEnd();
		glColor3f(1,1,1);
		glBegin(GL_LINE_LOOP);
		glVertex3f(-1.01, -1.01, 0);
		glVertex3f(-1.01, 1.01, 0);
		glVertex3f(1.01, 1.01, 0);
		glVertex3f(1.01, -1.01, 0);
		glEnd();
	glEnd();
		glPopMatrix();
	}
	glPopMatrix();
	glPushMatrix();
	for(float i = 1; i <= 3; i++){
		glColor3f(0.3,0,0.9/i);
		glTranslatef(-2,0,0);
		glPushMatrix();
		glRotatef(90,0,1,0);
		glBegin(GL_POLYGON);
		glVertex3f(-1, -1, 0);
		glVertex3f(-1, 1, 0);
		glVertex3f(1, 1, 0);
		glVertex3f(1, -1, 0);
		glEnd();
		glColor3f(1,1,1);
		glBegin(GL_LINE_LOOP);
		glVertex3f(-1.01, -1.01, 0);
		glVertex3f(-1.01, 1.01, 0);
		glVertex3f(1.01, 1.01, 0);
		glVertex3f(1.01, -1.01, 0);
		glEnd();
		glPopMatrix();
	}
	
	glFlush();
}