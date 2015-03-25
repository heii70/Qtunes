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
class SquaresWidget;
class QMediaPlayer;

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

	// actions
	QAction		*m_loadAction;
	QAction		*m_quitAction;
	QAction		*m_aboutAction;

	// menus
	QMenu		*m_fileMenu;
	QMenu		*m_helpMenu;

	// widgets
	QWidget *m_mainWidget;
	QWidget *m_songSplitter;
	QVBoxLayout *m_mainBox;
	//QSplitter	*m_mainSplit;
	//QSplitter	*m_leftSplit;
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
	QToolButton *m_albumleft;
	QToolButton *m_albumright;
    QSlider *m_volumeSlider;
    QSlider *m_timeSlider;
    QLabel *m_timeLabel;
	QHBoxLayout *m_buttonlayout;
    QHBoxLayout *m_sliderlayout;
	
	SquaresWidget *m_squares;
	QMediaPlayer *m_mediaplayer;

	// string lists
	QString		   m_directory;
	QStringList	   m_listGenre;
	QStringList	   m_listArtist;
	QStringList	   m_listAlbum;
	QList<QStringList> m_listSongs;
};

#endif // MAINWINDOW_H
