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
    bool eventFilter(QObject *, QEvent *);
    void keyPressEvent(QKeyEvent *);
    bool isSearch;
    int colorval;
signals:
    void s_artLoaded(QList<QImage> *, QList<QString> *);
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
    void s_playButton();
    void s_pauseButton();
    void s_prevSong();
    void s_nextSong();
    void s_repeat();
    void s_shuffle();
    void s_checkboxSelect();
    void s_setVolume(int);
	void s_setPosition(qint64);
    void s_seek(int);
    void s_updateLabel(qint64);
    void s_changeSpeed();
    void s_setDuration(qint64);
    void s_playlistUpdate();
    void s_redrawAlbum(QString);
    void s_albumLabel(QString);
    void s_tableUpdate();
    void s_toggleNightMode();
    void s_searchSongs();
    void s_setSliderColor(QString, QString, QString);
    void s_cycleSliderColor();

private:
	void createActions();
	void createMenus  ();
	void createWidgets();
	
	void createButtons();
	
	void createLayouts();
	void initLists	  ();
	void redrawLists  (QListWidgetItem *, int);
    void redrawLists  (QList<QString> *, int);
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
    QAction     *m_nightmodeAction;
    QAction     *m_slidercolorAction;
    QActionGroup *m_playbackAction;

	// menus
	QMenu		*m_fileMenu;
	QMenu		*m_helpMenu;
    QMenu       *m_playbackMenu;
    QMenu       *m_prefMenu;

	// widgets
	QWidget *m_mainWidget;
	QWidget *m_songSplitter;
	QVBoxLayout *m_mainBox;

    QSplitter *m_split1;
    QSplitter *m_split2;
    QWidget *m_staticImage;
    QWidget *m_musicWidgets;
	
	QLabel		*m_label[3];
	QListWidget 	*m_panel[3];
	QTableWidget	*m_table;

    QTableWidget *m_playlistTable;
    QTableWidget *m_checkboxTable;
    QCheckBox *m_checkbox;
	QProgressDialog	*m_progressBar;
	QToolButton	*m_stop;
	QToolButton *m_play;
    QToolButton *m_pause;
    QToolButton *m_prevSong;
    QToolButton *m_nextSong;
    QToolButton *m_repeat;
    QToolButton *m_shuffle;
    QToolButton *m_checkboxSelect;
    QToolButton *m_loadArt;
    QToolButton *m_search;
	
    QSlider *m_volumeSlider;
	QSlider *m_timeSlider;
    QLabel *m_timeLabel;
    QLabel *m_imageLabel;
    QHBoxLayout *m_buttonLayout;
    QHBoxLayout *m_sliderLayout;
	QImage m_resizedArt; 
	QImage m_tdResizedArt;
        QLineEdit *m_searchbox;
	
	SquaresWidget *m_squares;
    QLabel *m_albumLabel;
	QMediaPlayer *m_mediaplayer;
	
	QList <QImage> *m_artlist;
    QList <QString> *m_albumList;

    QString ts_styleSheet;
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
