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
protected:
	void initializeGL();
	void resizeGL(int w, int h);
    void paintGL();
	
private:
	void traverseDirs(QString path);
	int readPPM(char *file, int &width, int &height,
		unsigned char* &image);
	float m_translate, m_translatebuffer;
	bool m_recordsloaded;
	int m_numrecords;
	int m_width, m_height, m_scale;
	QString m_directory;
	QTimer *m_timer; //automatically resets itself
};
#endif