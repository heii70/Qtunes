#include <iostream>
#include <fstream>
#include <assert.h>
#include "squareswidget.h"
#include <QOpenGLFunctions>
//#include <QtOpenGL>
#include <glu.h>
#include <QtWidgets>
#include <QGLWidget>
using namespace std;

typedef struct {
    int		width;
    int		height;
    GLuint	texId;
    char	imageFilename[512];
} Record;
Record  *records;
Record def_record;

/*SquaresWidget constructor that sets the default values of sizes,
variables, etc
 * Also initializes a QTimer and connects it such that it will
 * call the slot function update() every 16ms. update() will call
 * updateGL(), which will call paintGL()
 * */
SquaresWidget::SquaresWidget(){
	m_width = 3;
	m_height = 1;
	m_scale = 250;
	setMinimumSize(m_width*m_scale,m_height*m_scale);
	resize(m_width*m_scale,m_height*m_scale);
	setMaximumSize(m_width*m_scale,m_height*m_scale);
	setGeometry(0,0,m_width*m_scale,m_height*m_scale);
	//heightForWidth(3);
	setSizeIncrement(m_width,m_height);
	m_translate = 0;
	m_translatebuffer = 0;
	m_recordsloaded = false;
	m_frommp3 = false;
	m_numrecords = 0;
	m_directory = ".";
	m_currentrecord = 0;
	m_shift = 2;
	m_albumheight = 1.0;
	m_initialcall = true;
    m_albums_shown = 0;
    m_main_album = 0;
	
	m_timer = new QTimer(this);
	m_timer->start(16);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

/* SquaresWidget deconstructor*/
SquaresWidget::~SquaresWidget(){
}

/* s_shiftleft() is a slot function that increments m_translate
 * by 1. This slot function is connected to a signal from
 * MainWindow that will be emitted when the m_albumleft button
 * is clicked*/
void SquaresWidget::s_shiftleft(){
    if(m_numrecords == 0) return;
    if(m_translate > m_shift*(m_albums_shown/2-1) ) return;
    if(m_main_album > 0 && m_translate > (m_albums_shown-m_numrecords-2.0)) m_main_album -= 1;
	m_translate += m_shift;
}
/* s_shiftright() is a slot function that decrements m_translate
 * by 1. This slot function is connected to a signal from
 * MainWindow that will be emitted when the m_albumright button
 * is clicked
 * */
void SquaresWidget::s_shiftright(){
    if(m_numrecords == 0) return;
    //m_albums_shown switched with m_numrecords
    if(m_translate < -1*m_shift*(m_numrecords/2)) return;
    //if(m_translate < -1*m_shift*(m_numrecords/2) && m_numrecords%2 == 0) return;
    if(m_main_album < m_numrecords - m_albums_shown && m_translate < 0) m_main_album += 1;
	m_translate -= m_shift;
}
/* s_loadart() is a slot function that is connected to a signal
 * from MainWindow that is emitted when the m_loadart button is 
 * pressed. This button is the button with the folder icon and 
 * this function will create a new dialog that can be used to
 * choose a folder. Calls function traverseDirs once a folder is
 * selected
 * NOTE: used in getting album art from .ppm files
 * */

/* s_mp3art is a slot function that is connected to a signal
 * from MainWindow that is emitted when the traverseDirs function
 * is called. It takes in a QList of QImages that have been "given"
 * to it by MainWindow's traverseDirs function and sets up the array
 * of records to have elements with OpenGL textures from the QImages
 * */
void SquaresWidget::s_mp3art(QList<QImage> *artlist){
	qDebug("s_mp3art");
	m_frommp3 = true;
	//if(artlist.size()>0){
		m_recordsloaded = true;
		m_numrecords = artlist->size();
	//}
	records = (Record*) malloc(sizeof(Record) * m_numrecords);
	
	glEnable(GL_TEXTURE_2D);
	int		ww, hh;
	//unsigned char  *texData;
	for(int i=0; i < m_numrecords; i++) {
		QString temp_string = QString("record %1").arg(i+1);
        //sprintf(records[i].imageFilename,"%s",temp_string.toStdString().c_str());
		records[i].width  = 4;
		records[i].height = 4;
		//readPPM(records[i].imageFilename, ww, hh, texData);

	    glGenTextures  (1, &records[i].texId);
	    glBindTexture  (GL_TEXTURE_2D,  records[i].texId);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
		QImage image_gl = QGLWidget::convertToGLFormat(artlist->at(i));
	    glTexImage2D   (GL_TEXTURE_2D, 0, GL_RGBA, image_gl.width(), image_gl.height(), 0, GL_RGBA,
			    GL_UNSIGNED_BYTE, image_gl.bits());
	}
	glDisable(GL_TEXTURE_2D);
    //printf("number of records = %d\n", m_numrecords);
}

void SquaresWidget::mousePressEvent(QMouseEvent *event){
    m_xpos = event->x();
    m_ypos = event->y();
    //printf("mouse x = %d, y = %d\n",m_xpos,m_ypos);
    /*if(m_xpos < 135) m_translate += 2*m_shift;
    else if(m_xpos <= 256) m_translate += m_shift;
    else if(m_xpos >= 500 && m_xpos < 620) m_translate -= m_shift;
    else if(m_xpos >= 620 && m_xpos <= 750) m_translate -= 2*m_shift;*/
    if(m_xpos < 135){
        s_shiftleft();
        s_shiftleft();
    }
    else if(m_xpos <= 256) s_shiftleft();
    else if(m_xpos >= 500 && m_xpos < 620) s_shiftright();
    else if(m_xpos >= 620 && m_xpos <= 750){
        s_shiftright();
        s_shiftright();
    }
}

/*void SquaresWidget::s_jumpto(int x){
	m_translate = 
}*/

void SquaresWidget::default_image(){
    QImage def_image, resized_image;
    //def_image.load("/Users/matt/Desktop/csc221/qtunes/noload.jpg");
    def_image.load(":/Resources/noload.jpg");
    resized_image = def_image.scaled(250,250,Qt::KeepAspectRatio);

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1,&def_record.texId);
    glBindTexture  (GL_TEXTURE_2D, def_record.texId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    QImage def_gl = QGLWidget::convertToGLFormat(resized_image);
    glTexImage2D   (GL_TEXTURE_2D, 0, GL_RGBA, def_gl.width(), def_gl.height(), 0, GL_RGBA,
            GL_UNSIGNED_BYTE, def_gl.bits());
    glDisable(GL_TEXTURE_2D);
}

/* initializeGL() is a function that sets up the widget to be used.
 * It clears the widget of any previous colors, enables the widget
 * to display things in 3D correctly, and enables certain shapes to
 * have smooth lines
 * */
void SquaresWidget::initializeGL(){
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);

    default_image();
}

/* resizeGL(int w, int h) is a function that ensures that, when
 * the widget is resized, it will scale the widget and what it
 * displays appropriately (THIS PART NEEDS TO BE WORKED ON).
 * In addition, this function sets up the perspective view, the
 * camera view, and loads the identity matrix into the widget
 * */
void SquaresWidget::resizeGL(int w, int h){
	resize(w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float) w/h, 1, 1000);
    gluLookAt(0,0, 2.8, //2.8
				0,0,1,
				0,1,0);
	//glOrtho(-6, 6, -2, 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/* paintGL() is a function that is updated every 16ms (subject to
 * change). This function creates album coverflow from .ppm files or
 * from .mp3 files.
 * */
void SquaresWidget::paintGL(){
	// clears 2D and 3D
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	if(m_translatebuffer != m_translate){
		if(m_translate - m_translatebuffer < -0.001){
			m_translatebuffer -= 0.05;
			m_movingright = true;
		}
		else m_movingright = false;
		if(m_translate - m_translatebuffer > 0.001){
			m_translatebuffer += 0.05;
			m_movingleft = true;
		}
		else m_movingleft = false;
	}
    //printf("m_translate = %f",m_translate);
	glTranslatef(m_translatebuffer,0,0);
    if(m_numrecords == 0){
        glTranslatef(0,0,m_albumheight/2);
        glEnable(GL_TEXTURE_2D);
        //glColor3f(0.8,0.8,0.7);
        glBindTexture(GL_TEXTURE_2D, def_record.texId);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_QUADS);
            glTexCoord2f(1, 0);	glVertex3f(1,-1,0);
            glTexCoord2f(0, 0);	glVertex3f(-1,-1,0);
            glTexCoord2f(0, 1);	glVertex3f(-1,1,0);
            glTexCoord2f(1, 1);	glVertex3f(1,1,0);
        glEnd();
        glDisable(GL_TEXTURE_2D);
	}
	else{
        if(m_numrecords < 8) m_albums_shown = m_numrecords; //12
        else m_albums_shown = 7; //11
        //m_albums_shown = m_numrecords;

        if(m_albums_shown%2 == 0) m_currentrecord = (float)m_shift*(m_albums_shown)/2 - m_translatebuffer;//no (m_albums_shown+m_main_album)
        else m_currentrecord = (float)m_shift*(m_albums_shown)/2 - m_translatebuffer - 1;
        glTranslatef((int)-1*m_shift*(m_albums_shown/2+1),0,0);
        //printf("current record = %f\n",m_currentrecord);

        //for(int i = m_main_album; i < m_albums_shown + m_main_album; i++){
        for(int i = 0; i < m_numrecords; i++){
            glTranslatef(m_shift,0,0);
            if(i < m_main_album || i >= m_main_album + m_albums_shown) continue;

			glEnable(GL_TEXTURE_2D);
			glPushMatrix();
			glColor3f(0.8,0.8,0.7);
			//if(i >= m_currentrecord + 0.001 || i <= m_currentrecord - 0.001){
            if(i*m_shift >= m_currentrecord + 0.01 || i*m_shift <= m_currentrecord - 0.01){
				float temp_rotation = 90*((m_translate-m_translatebuffer)/m_shift-(int)((m_translate-m_translatebuffer)/m_shift));
				if(m_movingleft){
					if(i*m_shift >= m_currentrecord + 0.01 && i*m_shift <= m_currentrecord + (m_shift-0.039)){
						glTranslatef(0,0,m_albumheight-m_albumheight*(i*m_shift-m_currentrecord)/m_shift);
						glRotatef(temp_rotation + 90,0,1,0); //+90
					}
					else if(i*m_shift >= m_currentrecord - (m_shift-0.039) && i*m_shift <= m_currentrecord - 0.01){
						glTranslatef(0,0,m_albumheight-m_albumheight*(m_currentrecord-i*m_shift)/m_shift);
						glRotatef(temp_rotation,0,1,0);
					}
					else glRotatef(90,0,1,0);
				}
				else if(m_movingright){
					if(i*m_shift >= m_currentrecord + 0.01 && i*m_shift <= m_currentrecord + (m_shift-0.039)){
						glTranslatef(0,0,m_albumheight-m_albumheight*(i*m_shift-m_currentrecord)/m_shift);
						glRotatef(180,0,1,0);
						glRotatef(temp_rotation,0,1,0);
					}
					else if(i*m_shift >= m_currentrecord - (m_shift-0.039) && i*m_shift <= m_currentrecord - 0.01){
						glTranslatef(0,0,m_albumheight-m_albumheight*(m_currentrecord-i*m_shift)/m_shift);
						glRotatef(temp_rotation + 90,0,1,0);
					}
					else glRotatef(90,0,1,0);
				}
				else glRotatef(90,0,1,0);
			}
            else glTranslatef(0,0,m_albumheight);
			glBindTexture(GL_TEXTURE_2D, records[i].texId);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			//if(i <= (float)m_numrecords/2 - m_translatebuffer){
			if(i*m_shift <= m_currentrecord + 0.01){
					glBegin(GL_QUADS);
						glTexCoord2f(1, 0);	glVertex3f(1,-1,0);
						glTexCoord2f(0, 0);	glVertex3f(-1,-1,0);
						glTexCoord2f(0, 1);	glVertex3f(-1,1,0);
						glTexCoord2f(1, 1);	glVertex3f(1,1,0);
					glEnd();
			}
			else{
					glBegin(GL_QUADS);
						glTexCoord2f(1, 0);	glVertex3f(-1,-1,0);
						glTexCoord2f(0, 0);	glVertex3f(1,-1,0);
						glTexCoord2f(0, 1);	glVertex3f(1,1,0);
						glTexCoord2f(1, 1);	glVertex3f(-1,1,0);
					glEnd();
			}
			glDisable(GL_TEXTURE_2D);
			glColor3f(1, (float)215/255, 0);
			glBegin(GL_LINE_LOOP);
				glVertex3f(-1, -1, 0);
				glVertex3f(-1, 1, 0);
				glVertex3f(1, 1, 0);
				glVertex3f(1, -1, 0);
			glEnd();
			glPopMatrix();
		}
	}
	/* glFlush() is needed to essentially draw what was just written
	 * above
	 * */
	glFlush();
}
