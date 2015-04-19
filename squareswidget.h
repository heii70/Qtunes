#ifndef SQUARESWIDGET_H
#define SQUARESWIDGET_H

#include <QOpenGLWidget>

class SquaresWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
	SquaresWidget();
	~SquaresWidget();
public slots:
	void s_shiftleft();
	void s_shiftright();
	void s_loadart();
	void s_mp3art(QList<QImage> *);
	//void s_jumpto(int);
protected:
	void initializeGL();
	void resizeGL(int w, int h);
    void paintGL();
	
private:
	void traverseDirs(QString path);
	int readPPM(char *file, int &width, int &height,
		unsigned char* &image);
	float m_translate, m_translatebuffer;
	float m_shift;
	bool m_recordsloaded, m_frommp3;
	int m_numrecords;
	int m_width, m_height, m_scale;
	float m_currentrecord, m_albumheight;
	bool m_movingleft, m_movingright, m_initialcall;
	QString m_directory;
	QTimer *m_timer; //automatically resets itself
};
#endif