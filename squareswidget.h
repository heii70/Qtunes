#ifndef SQUARESWIDGET_H
#define SQUARESWIDGET_H

#include <QOpenGLWidget>

class SquaresWidget : public QOpenGLWidget
{
	Q_OBJECT
public:
	SquaresWidget();
	~SquaresWidget();
protected:
	void initializeGL();
	void resizeGL(int w, int h);
    void paintGL();
	
private:
	QTimer *m_timer; //automatically resets itself
};
#endif