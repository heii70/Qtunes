#include <iostream>
#include <fstream>
#include <assert.h>
#include "squareswidget.h"
#include <QOpenGLFunctions>
//#include <QtOpenGL>
#include <glu.h>
#include <QtWidgets>
using namespace std;

typedef struct {
	int		width;
	int		height;
	GLuint	texId;
	char	imageFilename[512];
} Record;
Record  *records;

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
	m_numrecords = 0;
	m_directory = ".";
	
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
void SquaresWidget::s_loadart(){
	QFileDialog *fd = new QFileDialog(this, Qt::Window);
	fd->setFileMode(QFileDialog::Directory);
	QString s = fd->getExistingDirectory(0, "Select Folder", m_directory,
			 QFileDialog::ShowDirsOnly |
			 QFileDialog::DontResolveSymlinks);
	if(s == NULL) return;
	traverseDirs(s);
}
void SquaresWidget::traverseDirs(QString path){
	//Setting up the traversal of directories
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
	gluLookAt(0,0,3,
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
	if(!m_recordsloaded){
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
				glVertex3f(-1, -1, 0);
				glVertex3f(-1, 1, 0);
				glVertex3f(1, 1, 0);
				glVertex3f(1, -1, 0);
			glEnd();
			glPopMatrix();
		}
	}
	else{
		glTranslatef(-(m_numrecords/2+1),0,0);
		for(int i = 0; i < m_numrecords; i++){
			glTranslatef(1,0,0);
			glEnable(GL_TEXTURE_2D);
			glPushMatrix();
			glRotatef(90,0,1,0);
			glColor3f(0.8,0.8,0.7);
			glBindTexture(GL_TEXTURE_2D, records[i].texId);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			if(i <= (float)m_numrecords/2 - m_translatebuffer){
				glBegin(GL_QUADS);
					glTexCoord2f(1, 1);	glVertex3f(1,-1,0);
					glTexCoord2f(0, 1);	glVertex3f(-1,-1,0);
					glTexCoord2f(0, 0);	glVertex3f(-1,1,0);
					glTexCoord2f(1, 0);	glVertex3f(1,1,0);
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
	glFlush();
}
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