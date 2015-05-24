#ifndef SQUARESWIDGET_H
#define SQUARESWIDGET_H
#include <QOpenGLWidget>
#include <tag.h>
#include <id3v2tag.h>

///////////////////////////////////////////////////////////////////////////////
///
/// \class SquaresWidget
/// \brief QTunes Coverflow Widget.
///
/// The SquaresWidget class is used to create the coverflow seen in QTunes. It is
/// a subclass of the QOpenGLWidget and can be used to display the album art of
/// the songs loaded into QTunes. Users may browse through the album art of their
/// songs by clicking on the albums to the left or right of coverflow and can
/// filter the songs displayed by QTunes by double-clicking on the center album
/// art.
///
///////////////////////////////////////////////////////////////////////////////

class SquaresWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
	SquaresWidget();
	~SquaresWidget();
    void shiftLeft();
    void shiftRight();
signals:
    void s_albumSelected(QString);
    void s_currentAlbum(QString);
public slots:
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
    int m_numAlbums, m_width, m_height, m_scale,
        m_xpos, m_ypos, m_albumsShown, m_mainAlbum;
	QString m_directory;
    QTimer *m_timer;
    QList<QString> *m_albumList;
    QList<QString> *m_pathList;
    QList<GLuint> *m_glTexID;
    GLuint m_defTexID;
};
#endif
