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
	m_translate += 1;
}
/* s_shiftright() is a slot function that decrements m_translate
 * by 1. This slot function is connected to a signal from
 * MainWindow that will be emitted when the m_albumright button
 * is clicked
 * */
void SquaresWidget::s_shiftright(){
	m_translate -= 1;
}
/* s_loadart() is a slot function that is connected to a signal
 * from MainWindow that is emitted when the m_loadart button is 
 * pressed. This button is the button with the folder icon and 
 * this function will create a new dialog that can be used to
 * choose a folder. Calls function traverseDirs once a folder is
 * selected
 * NOTE: used in getting album art from .ppm files
 * */
void SquaresWidget::s_loadart(){
	QFileDialog *fd = new QFileDialog(this, Qt::Window);
	fd->setFileMode(QFileDialog::Directory);
	QString s = fd->getExistingDirectory(0, "Select Folder", m_directory,
			 QFileDialog::ShowDirsOnly |
			 QFileDialog::DontResolveSymlinks);
	if(s == NULL) return;
	traverseDirs(s);
}
/* traverseDirs is a function called by s_loadart() once a folder
 * has been selected. This function will traverse through the directory
 * and its subdirectories and will set up the array of records to have
 * elements which contain a texture that can be used with OpenGL
 * */
void SquaresWidget::traverseDirs(QString path){
	//Setting up the traversal of directories
	m_frommp3 = false;
	QString		key, val;
	QStringList	list;
	QDir dir(path);
	dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
	QFileInfoList listDirs = dir.entryInfoList();
	QDir files(path);
	files.setFilter(QDir::Files);
	files.setNameFilters(QStringList("*.ppm"));
	QFileInfoList listFiles = files.entryInfoList();
	records = (Record*) malloc(sizeof(Record) * listFiles.size());
	if(listFiles.size() > 0){
		m_recordsloaded = true;
		m_numrecords = listFiles.size();
	}
	
	//Setting up loading of textures
	glEnable(GL_TEXTURE_2D);
	int		ww, hh;
	unsigned char  *texData;
	
	//Traversal:
	for(int i=0; i < listFiles.size(); i++) {
		QFileInfo fileInfo = listFiles.at(i);
		sprintf(records[i].imageFilename,"%s",fileInfo.filePath().toStdString().c_str());
		//records[i].imageFilename = fileInfo.filePath().toStdString().c_str();
		records[i].width  = 4;
		records[i].height = 4;
		readPPM(records[i].imageFilename, ww, hh, texData);
	    glGenTextures  (1, &records[i].texId);
	    glBindTexture  (GL_TEXTURE_2D,  records[i].texId);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	    glTexImage2D   (GL_TEXTURE_2D, 0, 3, ww, hh, 0, GL_RGB,
			    GL_UNSIGNED_BYTE, texData);
	}
	glDisable(GL_TEXTURE_2D);
	if(listDirs.size() == 0) return;
	// recursively descend through all subdirectories
	for(int i=0; i<listDirs.size(); i++) {
		QFileInfo fileInfo = listDirs.at(i);
		traverseDirs( fileInfo.filePath() );
	}
	return;
}

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
		sprintf(records[i].imageFilename,"%s",temp_string.toStdString().c_str());
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
	gluLookAt(0,0,2.8,
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
	/* Variables m_translatebuffer and m_translate are used to allow for
	 * side to side motion in coverflow. If the two variables are not equal,
	 * then m_translatebuffer is incremented or decremented in small steps
	 * until it is equal to m_translate. This process allows for sliding
	 * album art. A translation is then performed on m_translatebuffer.
	 * */
	/* NOTE: must be changed-> will set it to be 
	 * glTranslatef(m_translate+m_translatebuffer) will only be from 0 to 1
	 * */
	if(m_translatebuffer != m_translate){
		if(m_translate - m_translatebuffer < -0.001) m_translatebuffer -= 0.04;
		else if(m_translate - m_translatebuffer > 0.001) m_translatebuffer += 0.04;
	}
	glTranslatef(m_translatebuffer,0,0);
	/* This if statement contains code that will create something that looks like
	 * coverflow, but just with squares (not actual album art). Currently left to
	 * be false so it does not execute this code. Still, I must stress that this 
	 * code should be read as this is fundamental in terms of understanding how 
	 * coverflow is being created
	 * */
	if(false){
		/* translation of -6 in order to center the squares*/
		glTranslatef(-6,0,0);
		/* for loop that will create 11 squares to be displayed in the coverflow
		 * style
		 * */
		for(int i = 1; i < 12; i++){
			/* squares must be translated by 1 to the right so that they do not
			 * overlap*/
			glTranslatef(1,0,0);
			glColor3f((float)i/11,1-(float)i/11,1-(float)i/11);
			/* glPushMatrix() and glPopMatrix() are used in order to ensure that
			 * whatever code is executed inbetween these two functions does not
			 * affect everything else. For example, this ensures that the
			 * rotation that the glRotatef only rotates the square that is created
			 * here around the y-axis (and not everything else).
			 * */
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
				glVertex3f(-1, -1, 0);
				glVertex3f(-1, 1, 0);
				glVertex3f(1, 1, 0);
				glVertex3f(1, -1, 0);
			glEnd();
			glPopMatrix();
		}
	}
	/* Everything in this else statement handles the case in which
	 * album coverflow must be created from .ppm files or from .mp3 files
	 * */
	else{
		/* translation here to center the coverflow*/
		glTranslatef(-(m_numrecords/2+1),0,0);
		for(int i = 0; i < m_numrecords; i++){
			glTranslatef(1,0,0);
			glEnable(GL_TEXTURE_2D);
			glPushMatrix();
			glRotatef(90,0,1,0);
			glColor3f(0.8,0.8,0.7);
			/* glBindTexture will bind the texture of the record
			 * to the square created
			 * */
			glBindTexture(GL_TEXTURE_2D, records[i].texId);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			/* Handles the case in which the square to be created
			 * will be to the left of the center square
			 * */
			if(i <= (float)m_numrecords/2 - m_translatebuffer){
				/* Slightly different coordinates for textures from
				 * .mp3 files and textures from .ppm files
				 * */
				if(m_frommp3){
					glBegin(GL_QUADS);
						glTexCoord2f(1, 0);	glVertex3f(1,-1,0);
						glTexCoord2f(0, 0);	glVertex3f(-1,-1,0);
						glTexCoord2f(0, 1);	glVertex3f(-1,1,0);
						glTexCoord2f(1, 1);	glVertex3f(1,1,0);
					glEnd();
				}
				else{
					glBegin(GL_QUADS);
						glTexCoord2f(1, 1);	glVertex3f(1,-1,0);
						glTexCoord2f(0, 1);	glVertex3f(-1,-1,0);
						glTexCoord2f(0, 0);	glVertex3f(-1,1,0);
						glTexCoord2f(1, 0);	glVertex3f(1,1,0);
					glEnd();
				}
			}
			/* Handles the case in which the square to be created
			 * will be to the right of the center square
			 * */
			else{
				if(m_frommp3){
					glBegin(GL_QUADS);
						glTexCoord2f(1, 0);	glVertex3f(-1,-1,0);
						glTexCoord2f(0, 0);	glVertex3f(1,-1,0);
						glTexCoord2f(0, 1);	glVertex3f(1,1,0);
						glTexCoord2f(1, 1);	glVertex3f(-1,1,0);
					glEnd();
				}
				else{
					glBegin(GL_QUADS);
						glTexCoord2f(1, 1);	glVertex3f(-1,-1,0);
						glTexCoord2f(0, 1);	glVertex3f(1,-1,0);
						glTexCoord2f(0, 0);	glVertex3f(1,1,0);
						glTexCoord2f(1, 0);	glVertex3f(-1,1,0);
					glEnd();
				}
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

/* readPPM is a function used to generate textures that can be used
 * with OpenGL
 * */
int SquaresWidget::readPPM(char *file, int &width, int &height, unsigned char* &image){
	// open binary file for reading
	ifstream inFile(file, ios::binary);
	assert(inFile);

	// verify that the image is in PPM format
	// error checking: first two bytes must be code for a raw PPM file
	char buf[80];
	inFile.getline(buf, 3);
	if(strncmp(buf, "P6", 2)) {
		cerr << "The file " << file << " is not in PPM format)\n";
		inFile.close();
		return 0;
	}

	// read width, height, and maximum gray val
	int maxGray;
	inFile >> width >> height >> maxGray;

	// skip over linefeed and carriage return
	inFile.getline(buf, 2);

	// allocate image
	image = new unsigned char[width*height*3];
	assert(image);

	// read entire image data
	inFile.read((char*) image, width*height*3);
	inFile.close();
	return 1;
}