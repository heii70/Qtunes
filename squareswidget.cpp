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
	m_translate = 0;
	m_translatebuffer = 0;
	m_timer = new QTimer(this);
	m_timer->start(50);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

SquaresWidget::~SquaresWidget(){
}

void SquaresWidget::s_shiftleft(){
	m_translate += 1;
}
void SquaresWidget::s_shiftright(){
	m_translate -= 1;
}

void SquaresWidget::initializeGL(){
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //make last parameter 1
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
}

void SquaresWidget::resizeGL(int w, int h){
	resize(w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float) w/h, 1, 1000);
	gluLookAt(0,0,3.5,
				0,0,1,
				0,1,0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void SquaresWidget::paintGL(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	if(m_translatebuffer != m_translate){
		if(m_translate - m_translatebuffer < -0.05) m_translatebuffer -= 0.1;
		else if(m_translate - m_translatebuffer > 0.05) m_translatebuffer += 0.1;
	}
	glTranslatef(m_translatebuffer,0,0);
	printf("b:%f | t:%f\n",m_translatebuffer, m_translate);
	glTranslatef(-6,0,0);
	for(int i = 1; i < 12; i++){
		glTranslatef(1,0,0);
		glColor3f((float)i/11,1-(float)i/11,1-(float)i/11);
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