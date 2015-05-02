// ======================================================================
// IMPROC: Image Processing Software Package
// Copyright (C) 2015 by George Wolberg
//
// MainWindow.h - Main Window widget class
//
// Written by: George Wolberg, 2015
// ======================================================================

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "qmediaplayer.h"
#include <QtWidgets>
#include "squareswidget.h"
#include <tag.h>
#include <id3v2tag.h>
#include <QTabWidget>
#include <QEvent>
#include "visualizer.h"
class SquaresWidget;
class QMediaPlayer;
class VisualizerWidget;

///////////////////////////////////////////////////////////////////////////////
///
/// \class MainWindow
/// \brief IMPROC main window.
///
/// The IMPROC main window consists of three main areas: a tree view
/// in the top-left corner, an information view directly below the
/// tree view, and a frame array that takes up the entire right side
/// of the screen.
///
///////////////////////////////////////////////////////////////////////////////

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	//! Constructor.
	MainWindow(QString);

	//! Destructor.
    ~MainWindow();

    bool eventFilter(QObject *object, QEvent *event);
    void keyPressEvent(QKeyEvent * event);
signals:
	void s_artLoaded(QList<QImage> *);
    void s_visualizerSpeed(signed int);
public slots:
	// slots
	void s_load  ();
	void s_panel1(QListWidgetItem*);
	void s_panel2(QListWidgetItem*);
	void s_panel3(QListWidgetItem*);
	void s_play  (QTableWidgetItem*);
	void s_about ();
	void statusChanged(QMediaPlayer::MediaStatus status);
	void s_playbutton();
    void s_pausebutton();
	void s_prevsong();
	void s_nextsong();
    void s_setVolume(int);
	void s_setPosition(qint64);
    void s_seek(int);
    void s_updateLabel(qint64);
    void s_changeSpeed();
    void s_setDuration(qint64);
    //void s_clickedTime(QMouseEvent*);

private:
	void createActions();
	void createMenus  ();
	void createWidgets();
	
	void createButtons();
	
	void createLayouts();
	void initLists	  ();
	void redrawLists  (QListWidgetItem *, int);
	void traverseDirs (QString);
	void setSizes	  (QSplitter *, int, int);
	QImage imageForTag(TagLib::ID3v2::Tag *tag);

	// actions
	QAction		*m_loadAction;
	QAction		*m_quitAction;
	QAction		*m_aboutAction;
    QAction     *m_slowestAction;
    QAction     *m_slowerAction;
    QAction     *m_normalAction;
    QAction     *m_fasterAction;
    QAction     *m_fastestAction;
    QActionGroup *m_playbackAction;

	// menus
	QMenu		*m_fileMenu;
	QMenu		*m_helpMenu;
    QMenu       *m_playbackMenu;

	// widgets
	QWidget *m_mainWidget;
	QWidget *m_songSplitter;
	QVBoxLayout *m_mainBox;

	QWidget *m_popup;
	
	QSplitter	*m_rightSplit;
	//QLabel		*m_labelSide[2];
	QLabel		*m_label[3];
	QListWidget 	*m_panel[3];
	QTableWidget	*m_table;
	QProgressDialog	*m_progressBar;
	QToolButton	*m_stop;
	QToolButton *m_play;
    QToolButton *m_pause;
	QToolButton *m_prevsong;
    QToolButton *m_nextsong;
    //QToolButton *m_albumleft;
    //QToolButton *m_albumright;
	QToolButton *m_loadart;
	QToolButton *m_showimage;
	
    QSlider *m_volumeSlider;
    QSlider *m_timeSlider;
    QLabel *m_timeLabel;
    QLabel *m_imagelabel;
	QHBoxLayout *m_buttonlayout;
	QHBoxLayout *m_sliderlayout; 
	QImage m_resizedArt; 
	QImage m_tdResizedArt;
	
	SquaresWidget *m_squares;
	QMediaPlayer *m_mediaplayer;
	
	QList <QImage> *m_artlist;

	// string lists
	QString		   m_directory;
	QStringList	   m_listGenre;
	QStringList	   m_listArtist;
	QStringList	   m_listAlbum;
	QList<QStringList> m_listSongs;

    QTabWidget *m_tabs;
    VisualizerWidget *m_visualizer;
};

#endif // MAINWINDOW_H
