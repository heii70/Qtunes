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
protected:
	void initializeGL();
	void resizeGL(int w, int h);
    void paintGL();
	
private:
	float m_translate, m_translatebuffer;
	QTimer *m_timer; //automatically resets itself
};
#endif