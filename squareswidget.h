#ifndef SQUARESWIDGET_H
#define SQUARESWIDGET_H

#include <QOpenGLWidget>
#include "MainWindow.h"
#include <fileref.h>
#include <tag.h>
#include <tpropertymap.h>
#include <tbytevector.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <id3v2frame.h>
#include <id3v2header.h>
#include <attachedpictureframe.h>

class SquaresWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
	SquaresWidget();
	~SquaresWidget();
signals:
    void s_albumSelected(QString);
    void s_currentAlbum(QString);
public slots:
    void s_shiftLeft();
    void s_shiftRight();
    void s_MP3Art(QList<QString> *, QList<QString> *);
protected:
	void initializeGL();
	void resizeGL(int w, int h);
    void paintGL();
	
private:
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent* event);
    void defaultImage();
    QImage imageForTag(TagLib::ID3v2::Tag *tag);

    float m_translate, m_translateBuffer, m_shift,
        m_centerRegion, m_albumDepth;
    bool m_recordsLoaded, m_fromMP3, m_movingLeft,
        m_movingRight, m_initialCall, m_doubleClicked;
    int m_numRecords, m_width, m_height, m_scale,
        m_xpos, m_ypos, m_albumsShown, m_mainAlbum;
	QString m_directory;
	QTimer *m_timer; //automatically resets itself
    QList<QString> *m_albumList;
    QList<QString> *m_pathList;
};
#endif
