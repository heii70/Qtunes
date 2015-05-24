#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <iostream>
#include <fstream>
#include <assert.h>
#include <QtWidgets>
#include <QGLWidget>
#include <QOpenGLWidget>
#include <QMediaPlayer>

///////////////////////////////////////////////////////////////////////////////
///
/// \class VisualizerWidget
/// \brief QTunes Visualizer Widget.
///
/// The VisualizerWidget class is used to create the visualizer seen in QTunes.
/// It is a subclass of the QOpenGLWidget and displays bars that represent
/// frequency bands of a song being played. These values are, however, randomly
/// generated (may be connected to frequency data from songs in the future).
/// The visualizer only updates the values of the bars if QTunes is playing
/// music. In addition, the visualizer is affected by the current speed of
/// playback. It can be set to update the values faster or slower based on
/// what speed is selected.
///
///////////////////////////////////////////////////////////////////////////////

class VisualizerWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    VisualizerWidget();
    ~VisualizerWidget();
public slots:
    void s_randomizer();
    void s_toggle(QMediaPlayer::State state);
    void s_changeSpeed(signed int s);
protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    float numArray[10];
    int m_width, m_height, m_scale;
    QTimer *m_newRandNum;
    QTimer *m_painter;
};
#endif
