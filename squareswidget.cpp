#include <iostream>
#include <fstream>
#include <assert.h>
#include "squareswidget.h"
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
    setSizeIncrement(m_width,m_height);
	m_translate = 0;
	m_translateBuffer = 0;
	m_recordsLoaded = false;
    m_fromMP3 = false;
    m_numRecords = 0;
	m_directory = ".";
    m_centerRegion = 0;
    m_shift = 2.000;
    m_albumDepth = 1.0;
    m_initialCall = true;
    m_albumsShown = 0;
    m_doubleClicked = false;
    m_mainAlbum = 0;
	m_timer = new QTimer(this);
	m_timer->start(16);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

/* SquaresWidget deconstructor*/
SquaresWidget::~SquaresWidget(){
}

/* s_shiftLeft() is a slot function that increments m_translate
 * by 1. This slot function is connected to a signal from
 * MainWindow that will be emitted when the m_albumleft button
 * is clicked*/
void SquaresWidget::s_shiftLeft(){
    if(m_numRecords == 0) return;
    if(m_translate - 0.1 > m_shift*(m_albumsShown/2-1) ) return;
	m_translate += m_shift;
}
/* s_shiftRight() is a slot function that decrements m_translate
 * by 1. This slot function is connected to a signal from
 * MainWindow that will be emitted when the m_albumright button
 * is clicked
 * */
void SquaresWidget::s_shiftRight(){
    if(m_numRecords == 0) return;
    if(m_translate + 0.1 < -1*m_shift*(m_numRecords/2)-1) return;
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

/* s_MP3Art is a slot function that is connected to a signal
 * from MainWindow that is emitted when the traverseDirs function
 * is called. It takes in a QList of QImages that have been "given"
 * to it by MainWindow's traverseDirs function and sets up the array
 * of records to have elements with OpenGL textures from the QImages
 * */
void SquaresWidget::s_MP3Art(QList<QImage> *artlist, QList<QString> *albumlist){
    m_albumList = albumlist;
    m_fromMP3 = true;
    if(artlist->size() == 0) return;
    m_recordsLoaded = true;
    m_numRecords = artlist->size();
    records = (Record*) malloc(sizeof(Record) * m_numRecords);
	
	glEnable(GL_TEXTURE_2D);
    for(int i=0; i < m_numRecords; i++) {
		QString temp_string = QString("record %1").arg(i+1);
		records[i].width  = 4;
		records[i].height = 4;
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

    if(m_numRecords < 8) m_albumsShown = m_numRecords;
    else m_albumsShown = 7;
    m_mainAlbum = (float)(m_numRecords-1-m_albumsShown)*0.5;
}

void SquaresWidget::mousePressEvent(QMouseEvent *event){
    m_xpos = event->x();
    m_ypos = event->y();
    if(m_xpos < 0.175*width()){
        s_shiftLeft();
        s_shiftLeft();
    }
    else if(m_xpos <= 0.34*width()) s_shiftLeft();
    else if(m_xpos >= 0.66*width() && m_xpos < 0.825*width()) s_shiftRight();
    else if(m_xpos >= 0.825*width() && m_xpos <= width()){
        s_shiftRight();
        s_shiftRight();
    }
}

void SquaresWidget::mouseDoubleClickEvent(QMouseEvent *event){
    if(event->x() > 0.34*width() && event->x() < 0.66*width()) m_doubleClicked = true;
}

void SquaresWidget::defaultImage(){
    QImage def_image, resized_image;
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

    defaultImage();
}

/* resizeGL(int w, int h) is a function that ensures that, when
 * the widget is resized, it will scale the widget and what it
 * displays appropriately (THIS PART NEEDS TO BE WORKED ON).
 * In addition, this function sets up the perspective view, the
 * camera view, and loads the identity matrix into the widget
 * */
void SquaresWidget::resizeGL(int w, int h){
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float) w/h, 1, 1000);
    gluLookAt(0,-0.05, 2.95, //2.8
                0,-0.05,1,
                0,1,0);
    /*gluLookAt(0,0,3.05,
              0,0,1,
              0,1,0);*/
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
	if(m_translateBuffer != m_translate){
		if(m_translate - m_translateBuffer < -0.001){
			m_translateBuffer -= 0.05;
            m_movingRight = true;
		}
        else m_movingRight = false;
		if(m_translate - m_translateBuffer > 0.001){
			m_translateBuffer += 0.05;
            m_movingLeft = true;
		}
        else m_movingLeft = false;
	}
	glTranslatef(m_translateBuffer,0,0);
    if(m_numRecords == 0){
        glTranslatef(0,0,m_albumDepth/2);
        glEnable(GL_TEXTURE_2D);
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
        //m_centerRegion is used to calculate the index*m_shift of the center album art
        if(m_albumsShown%2 == 0) m_centerRegion = (float)m_shift*(m_albumsShown)/2 - m_translateBuffer;
            else m_centerRegion = (float)m_shift*(m_albumsShown)/2 - m_translateBuffer - 1;

        m_mainAlbum = 0.5*(m_centerRegion-m_shift*m_albumsShown/2);
        if(m_mainAlbum < 0) m_mainAlbum = 0;

        //this translation will shift the coverflow such that the middle album art
        //will be the center album
        glTranslatef((int)-1*m_shift*(m_albumsShown/2+1),0,0);

        for(int i = 0; i < m_numRecords; i++){
            glTranslatef(m_shift,0,0);

            //if the record is not within the desired range of records, continue
            if(i < m_mainAlbum || i >= m_mainAlbum + m_albumsShown + m_shift/4) continue;

			glEnable(GL_TEXTURE_2D);
			glPushMatrix();
			glColor3f(0.8,0.8,0.7);

            //if the position of the album art is not currently in the center
            if(i*m_shift >= m_centerRegion + 0.01 || i*m_shift <= m_centerRegion - 0.01){

                //temp_rotation will represent the desired angle of rotation
                //the desired angle of rotation depends on how far things have been shifted
                //m_translate-m_translatebuffer is equal to the difference between the "desired"
                //translation of the coverflow and the actual position of the coverflow
                //I divide it by m_shift to then figure out the position of the center album within
                //the region where it would be rotating (from -m_shift to +m_shift)
                //it is also subtracted by the integer version of this to ensure that the number
                //multiplied by 90 is between 0 and 1
                float regionMoved = ((m_translate-m_translateBuffer)/m_shift-
                                     (int)((m_translate-m_translateBuffer)/m_shift));
                float temp_rotation = 90*regionMoved;
                if(m_movingLeft){

                    //This if statement checks to see if the position of the record being created in this
                    //iteration of the for loop is in the region on the right of the center album
                    //that would need to be rotated as the coverflow is moving
                    if(i*m_shift >= m_centerRegion + 0.01 && i*m_shift <= m_centerRegion + (m_shift-0.039)){
                        //translated in the positive z-direction based on how far the album has moved
                        //through the region where things must be rotated
                        glTranslatef(0,0,m_albumDepth*regionMoved);
                        glRotatef(temp_rotation + 90,0,1,0); //+90
					}

                    //Else, we need to check if the position of the record being created in this iteration
                    //is in the region on the left of the center album that would need to be rotated
                    //as the coverflow is moving
                    else if(i*m_shift >= m_centerRegion - (m_shift-0.039) && i*m_shift <= m_centerRegion - 0.01){
                        glTranslatef(0,0,m_albumDepth-m_albumDepth*regionMoved);
						glRotatef(temp_rotation,0,1,0);
					}

                    //Else, if it's not in the region that needs to be rotated, then these albums need to be rotated
                    //90 degrees to get the coverflow perspective
					else glRotatef(90,0,1,0);
				}
                else if(m_movingRight){
                    if(i*m_shift >= m_centerRegion + 0.01 && i*m_shift <= m_centerRegion + (m_shift-0.039)){
                        glTranslatef(0,0,m_albumDepth-m_albumDepth*(-1)*regionMoved);
                        glRotatef(temp_rotation + 180,0,1,0);
					}
                    else if(i*m_shift >= m_centerRegion - (m_shift-0.039) && i*m_shift <= m_centerRegion - 0.01){
                        glTranslatef(0,0,m_albumDepth*(-1)*regionMoved);
						glRotatef(temp_rotation + 90,0,1,0);
					}
					else glRotatef(90,0,1,0);
				}

                //This else statement handles the case where the album art being created in this iteration
                //of the for loop is not in the region that art needs to be rotated in
				else glRotatef(90,0,1,0);
			}
            else{
                if(m_doubleClicked){
                    emit s_albumSelected(m_albumList->at(i));
                    m_doubleClicked = false;
                }
                emit s_currentAlbum(m_albumList->at(i));
                glTranslatef(0,0,m_albumDepth);
            }
			glBindTexture(GL_TEXTURE_2D, records[i].texId);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            if(i*m_shift <= m_centerRegion + 0.01){
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
