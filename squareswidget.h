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
    void s_shiftLeft();
    void s_shiftRight();
    void s_MP3Art(QList<QImage> *, QList<QString> *);
protected:
	void initializeGL();
	void resizeGL(int w, int h);
    void paintGL();
	
private:
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void defaultImage();

    float m_translate, m_translateBuffer, m_shift,
        m_centerRegion, m_albumDepth;
    bool m_recordsLoaded, m_fromMP3, m_movingLeft,
        m_movingRight, m_initialCall, m_doubleClicked;
    int m_numRecords, m_width, m_height, m_scale,
        m_xpos, m_ypos, m_albumsShown, m_mainAlbum;
	QString m_directory;
	QTimer *m_timer; //automatically resets itself
    QList<QString> *m_albumList;
};
#endif
