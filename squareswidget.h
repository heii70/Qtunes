#ifndef SQUARESWIDGET_H
#define SQUARESWIDGET_H

#include <QOpenGLWidget>

class SquaresWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
	SquaresWidget();
	~SquaresWidget();
signals:
    void s_albumSelected(QString);
public slots:
	void s_shiftleft();
	void s_shiftright();
    void s_mp3art(QList<QImage> *, QList<QString> *);
protected:
	void initializeGL();
	void resizeGL(int w, int h);
    void paintGL();
	
private:
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void default_image();

	float m_translate, m_translatebuffer;
	float m_shift;
	bool m_recordsloaded, m_frommp3;
	int m_numrecords;
	int m_width, m_height, m_scale;
	float m_currentrecord, m_albumheight;
	bool m_movingleft, m_movingright, m_initialcall;
	QString m_directory;
	QTimer *m_timer; //automatically resets itself
    int m_xpos, m_ypos;
    int m_albums_shown, m_main_album;

    QList<QString> *m_albumList;
    bool m_doubleClicked;
};
#endif
