// ======================================================================
// IMPROC: Image Processing Software Package
// Copyright (C) 2015 by George Wolberg
//
// MainWindow.cpp - Main Window widget
//
// Written by: George Wolberg, 2015
// Modified by: Matthew Liu, Kenichi Yamamoto, William Gao
// 				added tag support with TagLib
//				adding mediplayer functionality using a QMediaPlayer object
// ======================================================================
#include <QTextStream>
#include <QtWidgets>
#include <QLabel> 
#include <QSlider>
#include "MainWindow.h"
#include <fileref.h>
#include <tag.h>
#include <tpropertymap.h>
#include <QMediaPlayer>
#include <QtMultimedia>
#include "qmediaplayer.h"
#include <iostream>
#include <stdio.h> 
#include <QToolButton>
#include "squareswidget.h"
#include <tbytevector.h>
#include <id3v2tag.h>
#include <mpegfile.h>
#include <id3v2frame.h>
#include <id3v2header.h>
#include <attachedpictureframe.h>
#include "visualizer.h"

using namespace std;
using namespace TagLib;

enum {SELECT, TITLE, TRACK, TIME, ARTIST, ALBUM, GENRE, PATH};
const int COLS = PATH;

bool caseInsensitive(const QString &s1, const QString &s2)
{
	return s1.toLower() < s2.toLower();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::MainWindow:
//
// Constructor. Initialize user-interface elements and sets up signal-
// slot connections.
//
MainWindow::MainWindow	(QString program)
	   : m_directory(".")
{
	// setup GUI with actions, menus, widgets, and layouts
	createActions();	// create actions for each menu item
	createMenus  ();	// create menus and associate actions
	createWidgets();	// create window widgets
    createButtons();    // create buttons
	createLayouts();	// create widget layouts
	m_mediaplayer = new QMediaPlayer;
	// populate the list widgets with music library data
    initLists();		// init list widgets

	// set main window titlebar
    setWindowTitle("QTunes Music Player");
    setFocusPolicy(Qt::StrongFocus);
    isSearch = false;
    isDirectClicked = false;
    colorval = 0;

    ts_styleSheet = "QSlider::add-page:horizontal { \
                        background: #444; \
                        border: 1px solid #777; \
                        height: 10px; \
                        border-radius: 4px;} \
                        QSlider::handle:horizontal { \
                        background: qlineargradient(x1:0, y1:0, x2:1, y2:1, \
                        stop:0 #666, stop:1 #888); \
                        border: 1px solid #777; \
                        width: 13px; \
                        margin-top: -2px; \
                        margin-bottom: -2px; \
                        border-radius: 4px; \
                        } \
                        QSlider::handle:horizontal:hover { \
                        background: qlineargradient(x1:0, y1:0, x2:1, y2:1, \
                        stop:0 #999, stop:1 #BBB); \
                        border: 1px solid #444; \
                        border-radius: 4px; \
                        }";

    m_normalAction->setChecked(true);
    m_timeSlider->installEventFilter(this);

	// set central widget and default size
	setCentralWidget(m_mainWidget);
    setMinimumSize(830, 500);
    resize(830, 820);
    signalSlots();      // create signal-slot connections

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::~MainWindow:
//
MainWindow::~MainWindow() {}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::signalSlots:
//
// Creates the signal-slot connections for many of the widgets, buttons,
// etc
//
void
MainWindow::signalSlots(){
    // Connects player control buttons
    connect(m_stop, SIGNAL(clicked()), m_mediaplayer, SLOT(stop()));
    connect(m_play, SIGNAL(clicked()), this, SLOT(s_playButton()));
    connect(m_pause, SIGNAL(clicked()), this, SLOT(s_pauseButton()));
    connect(m_nextSong, SIGNAL(clicked()), this, SLOT(s_nextSong()));
    connect(m_prevSong, SIGNAL(clicked()), this, SLOT(s_prevSong()));
    connect(m_repeat,SIGNAL(toggled(bool)),this,SLOT(s_repeat()));
    connect(m_shuffle,SIGNAL(toggled(bool)),this,SLOT(s_shuffle()));
    connect(m_volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(s_setVolume(int)));
    connect(m_loadArt, SIGNAL(clicked()), this, SLOT(s_load()));
    connect(m_timeSlider, SIGNAL(sliderMoved(int)), this, SLOT(s_seek(int)));
    connect(m_searchbox, SIGNAL(returnPressed()), this, SLOT(s_searchSongs()));
    connect(m_search, SIGNAL(clicked()),this,SLOT(s_searchSongs()));
    connect(m_checkboxSelect,SIGNAL(toggled(bool)),this,SLOT(s_checkboxSelect()));

    // Connects mediaplayer signals to slot functions and widgets
    connect(m_mediaplayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));
    connect(m_mediaplayer, SIGNAL(positionChanged(qint64)), this, SLOT(s_setPosition(qint64)));
    connect(m_mediaplayer, SIGNAL(positionChanged(qint64)), this, SLOT(s_updateLabel(qint64)));
    connect(m_mediaplayer,SIGNAL(durationChanged(qint64)),this,SLOT(s_setDuration(qint64)));

    // Connects m_table and playlistTable signals to call functions that update the other table
    connect(m_table,SIGNAL(itemClicked(QTableWidgetItem*)),this,SLOT(s_playlistUpdate()));
    connect(m_playlistTable, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(s_tableUpdate()));
    connect (signalMapper, SIGNAL(mapped(int)), this, SLOT(s_directClicked(int)));

    // Connects a signal that sends a QList of paths and QList of album cover names to the
    // coverflow widget.
    connect(this, SIGNAL(s_artLoaded(QList<QString>*, QList<QString>*)),
            m_coverflow,SLOT(s_MP3Art(QList<QString>*,QList<QString>*)));

    // Connects signals from the coverflow widget to display the name of the center album in
    // the coverflow and to filter based on the album (if the center album art is double-
    // clicked).
    connect(m_coverflow,SIGNAL(s_albumSelected(QString)),this,SLOT(s_redrawAlbum(QString)));
    connect(m_coverflow,SIGNAL(s_currentAlbum(QString)),this,SLOT(s_albumLabel(QString)));

    // Connects a signal that changes the speed of the visualizer widget
    connect(this,SIGNAL(s_visualizerSpeed(signed int)),m_visualizer,
            SLOT(s_changeSpeed(signed int)));

    // Connects a signal that will set the visualizer on or off based on the state of the
    // mediaplayer.
    connect(m_mediaplayer,SIGNAL(stateChanged(QMediaPlayer::State)),m_visualizer,
            SLOT(s_toggle(QMediaPlayer::State)));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::eventFilter:
//
// Registers a left mouse click on the time slider.
// Used to jump the slider to a new value when clicked.
//

bool MainWindow::eventFilter(QObject *object, QEvent *event){
    //If the widget m_timeSlider is clicked, do something
    if(object == m_timeSlider && event->type() == QEvent::MouseButtonPress){

        //Creates a QMouseEvent in order to read the type of mouse click used
        QMouseEvent *mouseEvent1 = static_cast<QMouseEvent *>(event);

        //If the time slider is clicked with the left button
        if(mouseEvent1->button() == Qt::LeftButton){

            //newposition is mapped from the x-value of the mouse click within the time slider to a value from 0 to 400
            //It represents the destination (where we want the slider handle to jump to)
            int newposition = QStyle::sliderValueFromPosition(0,500,mouseEvent1->x(),500,0);

            //curposition is mapped from the current value of the time slider relative to its maximum value in the same range as above
            //It represents the slider handle's current position
            int curposition = QStyle::sliderValueFromPosition(0,500,m_timeSlider->value(),m_timeSlider->maximum(),0);

            //If these two values differ by a value greater than 10 (or 1/40 of the slider), the slider will jump
            //Without this, the slider will jump even when clicking the handle, and will therefore be undraggable
            if(abs(newposition - curposition) > 10){
                //The following two lines shift the new slider position more to the left or right depending on whether
                // it is clicked at the beginnning or end respectively. This allows the new position to be more accurate.
                int adjustTime = 10 - newposition/25;
                newposition = newposition - adjustTime;

                //Turns the new position into a fraction of the slider
                float percent = (float)newposition/500;

                //Multiplies the fraction of the bar by its total duration to get the exact time (may be off by a fraction of a millisecond)
                int newTime = percent*m_mediaplayer->duration();

                //Move both the m_mediaplayer position as well as the slider position to the new position
                s_seek(newTime);
                m_timeSlider->setValue(newTime);
                return true;
            }
        }
    }
    return false;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::keyPressEvent:
//
// Registers a certain keypress within the player and performs an action.
//
void MainWindow::keyPressEvent(QKeyEvent *event){
    switch(event->key()){

    // Space can be used to toggle the mediaplayer to play or pause
    case Qt::Key_Space:
            if(m_mediaplayer->state() == 2){
                m_mediaplayer->play();
                return;
            }
            else{
                m_mediaplayer->pause();
                return;
            }

    // Left and right arrow keys can be used to move the coverflow left and right
    case Qt::Key_Left:
        m_coverflow->shiftLeft();
        return;
    case Qt::Key_Right:
        m_coverflow->shiftRight();
        return;

    // Up and down arrow keys can be used to change the volume of the mediaplayer
    case Qt::Key_Up:
        m_volumeSlider->setValue(m_volumeSlider->value() + m_volumeSlider->pageStep());
        return;
    case Qt::Key_Down:
        m_volumeSlider->setValue(m_volumeSlider->value() - m_volumeSlider->pageStep());
        return;
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::createActions:
//
// Create actions to associate with menu and toolbar selection.
//
void
MainWindow::createActions()
{
    // Create action to load music folder
	m_loadAction = new QAction("&Load Music Folder", this);
	m_loadAction->setShortcut(tr("Ctrl+L"));
	connect(m_loadAction, SIGNAL(triggered()), this, SLOT(s_load()));

    // Create action to quit
	m_quitAction = new QAction("&Quit", this);
	m_quitAction->setShortcut(tr("Ctrl+Q"));
	connect(m_quitAction, SIGNAL(triggered()), this, SLOT(close()));

    // Create action to read about QTunes
	m_aboutAction = new QAction("&About", this);
	m_aboutAction->setShortcut(tr("Ctrl+A"));
	connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(s_about()));

    // The following five QActions set the different playback speeds of the player
    m_slowestAction = new QAction("&Slowest (0.50x)", this);
    m_slowestAction->setCheckable(true);
    connect(m_slowestAction, SIGNAL(triggered()), this, SLOT(s_changeSpeed()));

    m_slowerAction = new QAction("Slower (0.75x)", this);
    m_slowerAction->setCheckable(true);
    connect(m_slowerAction, SIGNAL(triggered()), this, SLOT(s_changeSpeed()));

    m_normalAction = new QAction("&Normal (1.00x)", this);
    m_normalAction->setCheckable(true);
    connect(m_normalAction, SIGNAL(triggered()), this, SLOT(s_changeSpeed()));

    m_fasterAction = new QAction("Fast (1.25x)", this);
    m_fasterAction->setCheckable(true);
    connect(m_fasterAction, SIGNAL(triggered()), this, SLOT(s_changeSpeed()));

    m_fastestAction = new QAction("&Fastest (1.50x)", this);
    m_fastestAction->setCheckable(true);
    connect(m_fastestAction, SIGNAL(triggered()), this, SLOT(s_changeSpeed()));

    // Adds all five actions into one QActionGroup, m_playbackAction
    // By default, QActionGroup has an exclusivity property, allowing only one
    // action to be checked at a time
    m_playbackAction = new QActionGroup(this);
    m_playbackAction->addAction(m_slowestAction);
    m_playbackAction->addAction(m_slowerAction);
    m_playbackAction->addAction(m_normalAction);
    m_playbackAction->addAction(m_fasterAction);
    m_playbackAction->addAction(m_fastestAction);

    // Create action to toggle night mode
    m_nightmodeAction = new QAction("&Night Mode", this);
    m_nightmodeAction->setShortcut(tr("Ctrl+N"));
    m_nightmodeAction->setCheckable(true);
    connect(m_nightmodeAction, SIGNAL(triggered()), this, SLOT(s_toggleNightMode()));

    // Create action to change slider color
    m_slidercolorAction = new QAction("&Cycle slider color",this);
    m_slidercolorAction->setShortcut(tr("Ctrl+C"));
    connect(m_slidercolorAction,SIGNAL(triggered()),this,SLOT(s_cycleSliderColor()));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::createMenus:
//
// Create menus and install in menubar.
//
void
MainWindow::createMenus()
{
    // Adds the load and quit action to the File menu
	m_fileMenu = menuBar()->addMenu("&File");
	m_fileMenu->addAction(m_loadAction);
	m_fileMenu->addAction(m_quitAction);

    // Adds the about action to the Help menu
	m_helpMenu = menuBar()->addMenu("&Help");
	m_helpMenu->addAction(m_aboutAction);

    // Adds the multiple playback speed options to the Playback Speed menu
    m_playbackMenu = menuBar()->addMenu("&Playback Speed");
    m_playbackMenu->addAction(m_slowestAction);
    m_playbackMenu->addAction(m_slowerAction);
    m_playbackMenu->addAction(m_normalAction);
    m_playbackMenu->addAction(m_fasterAction);
    m_playbackMenu->addAction(m_fastestAction);

    // Adds the nightmode action and the cycle slider color action to the Preferences menu
    m_prefMenu = menuBar()->addMenu("&Preferences");
    m_prefMenu->addAction(m_nightmodeAction);
    m_prefMenu->addAction(m_slidercolorAction);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::createWidgets:
//
// Create widgets to put in the layout.
//
void
MainWindow::createWidgets()
{
    m_mainWidget = new QWidget;             // the central widget of QTunes
    m_coverflow = new SquaresWidget;        // the coverflow widget
    m_visualizer = new VisualizerWidget;    // the visualzer widget

    // checkbox used to deselect or select loaded songs
    m_checkbox = new QCheckBox;

    // signalMapper used to connect checkboxes to the playlist if directly clicked
    signalMapper = new QSignalMapper;

    // Two splitters:
    // m_split1 is used to separate static album art and buttons/slider
    // m_split2 is used to separate the playlist table and the song table
    m_split1 = new QSplitter(Qt::Horizontal);
    m_split1->setMaximumWidth(830);
    m_split2 = new QSplitter(Qt::Horizontal);
    m_split2->setMinimumWidth(800);

    // m_staticImage will contain m_imageLabel, which will display the
    // static album art
    m_staticImage = new QWidget;
    m_staticImage->setMaximumHeight(100);
    m_staticImage->setMaximumWidth(100);
    m_staticImage->resize(100,100);

    // m_musicWidgets will contain the buttons and sliders
    m_musicWidgets = new QWidget;
    m_musicWidgets->setMaximumHeight(100);
    m_musicWidgets->setMinimumWidth(620);
    m_musicWidgets->setMaximumWidth(700);

    // m_albumLabel displays the name of the center album art
    m_albumLabel = new QLabel;
    m_albumLabel->setText("");
    m_albumLabel->setFrameStyle(QFrame::Box | QFrame::Sunken);
    m_albumLabel->setMaximumHeight(20);
    m_albumLabel->setAlignment(Qt::AlignCenter);
    m_albumLabel->setMinimumWidth(m_coverflow->width());
    m_albumLabel->setMaximumWidth(m_coverflow->width());
    m_albumLabel->setStyleSheet("QLabel {"
                                "background-color : transparent;"
                                "color : white;}");

    // initialize table widget and playlist table widget
    m_playlistTable = new QTableWidget(0, 1);
    QHeaderView *playlist_header = new QHeaderView(Qt::Horizontal,m_playlistTable);
    playlist_header->setSectionResizeMode(QHeaderView::Stretch);
    m_playlistTable->setHorizontalHeader(playlist_header);
    m_playlistTable->setHorizontalHeaderLabels(QStringList() << "Playlist");
    m_playlistTable->setAlternatingRowColors(true);
    m_playlistTable->setShowGrid(true);
    m_playlistTable->setEditTriggers (QAbstractItemView::NoEditTriggers);
    m_playlistTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_playlistTable->setMaximumWidth(175);
    m_playlistTable->resize(175,450);

    // Label widget with centered text and sunken panels
    for(int i=0; i<3; i++) {
        m_label[i] = new QLabel;
        m_label[i]->setAlignment(Qt::AlignCenter);
        m_label[i]->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    }

    // Initializing label text
    m_label[0]->setText("<b>Genre<\b>" );
    m_label[1]->setText("<b>Artist<\b>");
    m_label[2]->setText("<b>Album<\b>" );

    // Initializing list widgets: genre, artist, album
    for(int i=0; i<3; i++)
        m_panel[i] = new QListWidget;

    // Initializing m_table, the song table, with appropriate columns
	m_table = new QTableWidget(0, COLS);
	QHeaderView *header = new QHeaderView(Qt::Horizontal,m_table);
	header->setSectionResizeMode(QHeaderView::Stretch);
	m_table->setHorizontalHeader(header);
    m_table->setHorizontalHeaderLabels(QStringList() << "Select" <<
		"Name" << "Track" << "Time" << "Artist" << "Album" << "Genre");
    m_table->setAlternatingRowColors(true);
    m_table->setShowGrid(true);
    m_table->setEditTriggers (QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setMinimumWidth(600);
    m_table->resize(600,450);

    // init more signal/slot connections
	connect(m_panel[0],	SIGNAL(itemClicked(QListWidgetItem*)),
		this,		  SLOT(s_panel1   (QListWidgetItem*)));
    connect(m_panel[1],	SIGNAL(itemClicked(QListWidgetItem*)),
		this,		  SLOT(s_panel2   (QListWidgetItem*)));
    connect(m_panel[2],	SIGNAL(itemClicked(QListWidgetItem*)),
		this,		  SLOT(s_panel3   (QListWidgetItem*)));
    connect(m_table,	SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
		this,		  SLOT(s_play	  (QTableWidgetItem*)));

    // m_albumList and m_pathList will be sent to the coverflow widget
    m_albumList = new QList<QString>;
    m_pathList = new QList<QString>;

    // m_fullPathList is used to store the paths of all the songs loaded
    m_fullPathList = new QList<QString>;

    // m_fullAlbumList is used to store the albums of all the songs loaded
    m_fullAlbumlist = new QList<QString>;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::createButtons:
//
// Create buttons to control QTunes
//
void
MainWindow::createButtons(){
    // Initialization of the buttons used in QTunes
    m_play = new QToolButton;
    m_play->setDisabled(true);
    m_stop = new QToolButton;
    m_stop->setDisabled(true);
    m_prevSong = new QToolButton;
    m_prevSong->setDisabled(true);
    m_nextSong = new QToolButton;
    m_nextSong->setDisabled(true);
    m_pause = new QToolButton;
    m_pause->setDisabled(true);
    m_repeat = new QToolButton;
    m_repeat->setCheckable(true);
    m_repeat->setChecked(false);
    m_repeat->setDisabled(true);
    m_shuffle = new QToolButton;
    m_shuffle->setCheckable(true);
    m_shuffle->setChecked(false);
    m_shuffle->setDisabled(true);
    m_checkboxSelect = new QToolButton;
    m_checkboxSelect->setCheckable(true);
    m_checkboxSelect->setChecked(true);
    m_checkboxSelect->setDisabled(true);
    m_searchbox = new QLineEdit;
    m_searchbox->setPlaceholderText("Search (3 char min)");
    m_searchbox->setEnabled(false);
    m_search = new QToolButton;
    m_search->setEnabled(false);
    m_loadArt = new QToolButton;

    // Setting icons of buttons (SP means Standard Pixmap)
    m_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    m_stop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    m_prevSong->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    m_nextSong->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    m_pause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    m_loadArt->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
    m_search->setIcon(QIcon(":/Resources/Search.png"));
    m_repeat->setIcon(QIcon(":/Resources/Repeat.png"));
    m_shuffle->setIcon(QIcon(":/Resources/Shuffle.png"));

    // Setting sizes of some of the buttons and the text of m_checboxSelect
    m_search->setIconSize(QSize(16.5,16.5));
    m_repeat->setIconSize(QSize(16.5,16.5));
    m_shuffle->setIconSize(QSize(16.5,16.5));
    m_checkboxSelect->setText("Deselect All");
    m_checkboxSelect->resize(16.5,16.5);
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::createLayouts:
//
// Create layouts for widgets.
//
void
MainWindow::createLayouts()
{
    // Creating a 2-row, 3-column grid layout, where row0 and
	// row1 consist of labels and list widgets, respectively.
    QWidget	    *widget = new QWidget(this);
	QGridLayout *grid   = new QGridLayout(widget);
	grid->addWidget(m_label[0], 0, 0);
	grid->addWidget(m_label[1], 0, 1);
	grid->addWidget(m_label[2], 0, 2);
	grid->addWidget(m_panel[0], 1, 0);
	grid->addWidget(m_panel[1], 1, 1);
	grid->addWidget(m_panel[2], 1, 2);
    widget->setMaximumHeight(m_coverflow->height()*1.25);

    // buttonwidget will contain the row of buttons, volume slider, etc
    QWidget *buttonwidget = new QWidget;

    // sliderwidget will contain the row of the time slider and the time label
    QWidget *sliderwidget = new QWidget;

    // imageLabel will display the album art of the song being played
    m_imageLabel = new QLabel;
    m_imageLabel ->setText("No Art");
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setFixedSize(90,90);
    m_imageLabel->setStyleSheet("QLabel{border: 1px solid #BBB}");
    m_imageLabel ->setMaximumWidth(100);
    m_imageLabel ->setMaximumHeight(100);

    // m_volumeSlider is a slider that will be used to control the volume of QTunes
    m_volumeSlider = new QSlider(Qt::Horizontal, buttonwidget);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(80);
    m_volumeSlider->setDisabled(true);
    m_volumeSlider->setFixedWidth(120);

    // m_timeLabel will be used to display the current position of the song
	m_timeLabel = new QLabel;
    m_timeLabel->setText("<b>0:00 / 0:00</b>");
    m_timeSlider = new QSlider(Qt::Horizontal);
    m_timeSlider ->setParent(this);
    m_timeSlider ->setDisabled(true);
    m_timeSlider ->setFixedWidth(500);

    // m_buttonLayout will contain the row of buttons, volume slider, etc
    // this will be the layout of buttonwidget
    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout ->addWidget(m_volumeSlider);
    m_buttonLayout ->addWidget(m_loadArt);
    m_buttonLayout ->addWidget(m_prevSong);
    m_buttonLayout ->addWidget(m_stop);
    m_buttonLayout ->addWidget(m_play);
    m_buttonLayout ->addWidget(m_pause);
    m_buttonLayout ->addWidget(m_nextSong);
    m_buttonLayout ->addWidget(m_repeat);
    m_buttonLayout ->addWidget(m_shuffle);
    m_buttonLayout ->addWidget(m_checkboxSelect);
    m_buttonLayout ->addWidget(m_searchbox);
    m_buttonLayout ->addWidget(m_search);
    m_buttonLayout->setMargin(0);
    buttonwidget ->setLayout(m_buttonLayout);
    buttonwidget ->setMaximumHeight(50);
    buttonwidget ->setMaximumWidth(600);

    // m_sliderLayout will contain the row of the time slider and the time label
    // this will be the layut of sliderwidget
    m_sliderLayout = new QHBoxLayout;
    m_sliderLayout ->addWidget(m_timeSlider);
    m_sliderLayout ->addWidget(m_timeLabel);
    m_sliderLayout->setMargin(0);
    sliderwidget ->setLayout(m_sliderLayout);
    sliderwidget ->setMaximumHeight(50);
    sliderwidget ->setMinimumWidth(600);
    s_setSliderColor("#009900","#33CC33","#00FF00");

    // staticImageLayout will be the layout of m_staticImage and will contain the
    // static album art of the song playing
    QHBoxLayout* staticImageLayout = new QHBoxLayout;
    staticImageLayout->addWidget(m_imageLabel);
    m_imageLabel->resize(100,100);
    m_staticImage->setLayout(staticImageLayout);
    m_staticImage->setMaximumSize(100,100);

    // musicWidgetLayout will be the layout of m_musicWidgets and contains buttonwidget
    // and sliderwidget
    QVBoxLayout* musicWidgetLayout = new QVBoxLayout;
    musicWidgetLayout ->addWidget(buttonwidget,Qt::AlignCenter);
    musicWidgetLayout ->addWidget(sliderwidget,Qt::AlignCenter);
    m_musicWidgets->setLayout(musicWidgetLayout);

    // m_tabs is a QTabWidget that will allow the user to switch between coverflow, list
    // widgets, and the visualizer widget
    m_tabs = new QTabWidget;
    m_tabs->setMaximumHeight(m_coverflow->height()*1.25);
    m_tabs->setMovable(true);

    // coverflowTab will contain m_coverflow
    // this is necessary to ensure that the coverflow widget is centered in m_tabs
    QWidget *coverflowTab = new QWidget();
    QVBoxLayout *tabs_layout = new QVBoxLayout(coverflowTab);
    tabs_layout->addWidget(m_coverflow);
    tabs_layout->setAlignment(m_coverflow,Qt::AlignHCenter);
    tabs_layout->addWidget(m_albumLabel);
    tabs_layout->setAlignment(m_albumLabel,Qt::AlignHCenter);

    // visualizerTab will contain m_visualizer
    // this is necessary to ensure that the visualizer widget is centered in m_tabs
    QWidget *visualizerTab = new QWidget();
    QVBoxLayout *vis_layout = new QVBoxLayout(visualizerTab);
    vis_layout->addWidget(m_visualizer);
    vis_layout->setAlignment(m_visualizer,Qt::AlignHCenter);

    // Adds the widgets to m_tabs
    m_tabs->insertTab(0,coverflowTab,"COVERFLOW");
    m_tabs->insertTab(1,widget,"FILTERS");
    m_tabs->insertTab(2,visualizerTab,"VISUALIZER");

    m_split1->addWidget(m_staticImage);     // static album art to left side of m_split1
    m_split1->addWidget(m_musicWidgets);    // buttons, etc to right side of m_split1

    m_split2->addWidget(m_playlistTable);   // playlist table to left side of m_split2
    m_split2->addWidget(m_table);           // song table to right side of m_split2

    // m_mainBox is a layout that will contain everything in MainWindow
    m_mainBox = new QVBoxLayout;
    m_mainBox-> addWidget(m_tabs);
    m_mainBox->addWidget(m_split1);
    m_mainBox->setAlignment(m_split1,Qt::AlignHCenter);
    m_mainBox->addWidget(m_split2);
    m_mainBox->setAlignment(m_split2,Qt::AlignHCenter);
    m_mainBox->setStretch(3,2);
    m_mainWidget ->setLayout(m_mainBox);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::initLists:
//
// Populate lists with data (first time).
//
void
MainWindow::initLists()
{
	// error checking
	if(m_listSongs.isEmpty()) return;

    // Creating separate lists for genres, artists, and albums
	for(int i=0; i<m_listSongs.size(); i++) {
		m_listGenre  << m_listSongs[i][GENRE ];
		m_listArtist << m_listSongs[i][ARTIST ];
		m_listAlbum << m_listSongs[i][ALBUM ];
	}

    // Sorting each list
	qStableSort(m_listGenre .begin(), m_listGenre .end(), caseInsensitive);
	qStableSort(m_listArtist.begin(), m_listArtist.end(), caseInsensitive);
	qStableSort(m_listAlbum .begin(), m_listAlbum .end(), caseInsensitive);

    // Adds each list to m_panel
    m_panel[0]->addItem("All");
	for(int i=0; i<m_listGenre.size(); i+=m_listGenre.count(m_listGenre[i]))
		m_panel[0]->addItem(m_listGenre [i]);
    for(int i=0; i<m_listArtist.size(); i+=m_listArtist.count(m_listArtist[i]))
		m_panel[1]->addItem(m_listArtist [i]);
	for(int i=0; i<m_listAlbum.size(); i+=m_listAlbum.count(m_listAlbum[i]))
		m_panel[2]->addItem(m_listAlbum [i]);

    // All the data is copied into m_table
	QTableWidgetItem *item[COLS];
	for(int i=0; i<m_listSongs.size(); i++) {
		m_table->insertRow(i);
		for(int j=0; j<COLS; j++) {
			item[j] = new QTableWidgetItem;
			item[j]->setText(m_listSongs[i][j]);
			item[j]->setTextAlignment(Qt::AlignCenter);
			m_table->setItem(i, j, item[j]);
		}
	}

    // Generates checkbox column in m_table and connects the checkbox to the signalMapper
    for(int i = 0; i < m_table->rowCount(); i++) {
          QCheckBox* p_checkbox = new QCheckBox;
          QWidget* p_widget = new QWidget;

          QHBoxLayout* p_layout = new QHBoxLayout(p_widget);
          p_layout->addWidget(p_checkbox);
          p_layout->setAlignment(Qt::AlignCenter);
          p_widget->setLayout(p_layout);

          m_table->item(i,0)->setText("");
          m_table->setCellWidget(i,0,p_widget);

          connect (p_checkbox, SIGNAL(clicked()), signalMapper, SLOT(map()));
          signalMapper->setMapping(p_checkbox,i);
    }

    // Initalizes playlist with all songs and enables buttons
    if(m_playlistTable->rowCount() == 0)
    {
       for(int i = 0; i < m_table->rowCount(); i++) {
              QTableWidgetItem *playlistItem = new QTableWidgetItem;

              m_playlistTable->insertRow(i);
              m_playlistTable->setItem(i,0,playlistItem);
              playlistItem->setText(m_table->item(i,1)->text());
        }
        m_repeat->setEnabled(true);
        m_shuffle->setEnabled(true);
        m_checkboxSelect->setEnabled(true);
        m_searchbox->setEnabled(true);
        m_search->setEnabled(true);
    }

    // Selects checkbox whose song is in playlist
    for(int i = 0; i < m_playlistTable->rowCount(); i++)
    {
        for(int j = 0; j < m_table->rowCount(); j++)
        {
            m_checkbox = m_table->cellWidget(j,0)->findChild<QCheckBox *>();
            if(m_playlistTable->item(i,0)->text() == m_table->item(j,1)->text())
            {
                m_checkbox->setChecked(true);
            }
        }
    }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::redrawLists:
//
// Re-populate lists with data matching item's text in field x.
//
void
MainWindow::redrawLists(QListWidgetItem *listItem, int x)
{
    // Clears old data in the playlist and m_table unless function is called from search
    if(isSearch == false)
        m_table->setRowCount(0);
    m_playlistTable->setRowCount(0);

    // Copying data to table widget
	for(int i=0,row=0; i<m_listSongs.size(); i++) {
		// skip rows whose field doesn't match text
		if(m_listSongs[i][x] != listItem->text()) continue;

        // Will insert a row into m_table with the proper data from m_listSongs
		m_table->insertRow(row);
		QTableWidgetItem *item[COLS];
		for(int j=0; j<COLS; j++) {
			item[j] = new QTableWidgetItem;
			item[j]->setText(m_listSongs[i][j]);
			item[j]->setTextAlignment(Qt::AlignCenter);
			m_table->setItem(row,j,item[j]);
		}

        // Incrementing table row index (row <= i)
		row++;
	}

        //Initializes playlist with songs in m_table
        for(int i = 0; i < m_table->rowCount(); i++)
        {
            QTableWidgetItem *playlistItem = new QTableWidgetItem;
            playlistItem->setText(m_table->item(i,1)->text());
            m_playlistTable->insertRow(m_playlistTable->rowCount());
            m_playlistTable->setItem(i,0,playlistItem);
        }

        // Generates checkbox column in m_table and connects the checkbox to the signalMapper
        for(int i = 0; i < m_table->rowCount(); i++) {
              QCheckBox* p_checkbox = new QCheckBox;
              QWidget* p_widget = new QWidget;
              p_checkbox->setChecked(true);

              QHBoxLayout* p_layout = new QHBoxLayout(p_widget);
              p_layout->addWidget(p_checkbox);
              p_layout->setAlignment(Qt::AlignCenter);
              p_widget->setLayout(p_layout);

              m_table->item(i,0)->setText("");
              m_table->setCellWidget(i,0,p_widget);

              connect (p_checkbox, SIGNAL(clicked()), signalMapper, SLOT(map()));
              signalMapper->setMapping(p_checkbox,i);
        }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_redrawAlbum:
//
// Slot function that is connected to a signal emitted from the coverflow widget
// the signal will contain a QString that has the name of the album that was
// double-clicked for filtering
//
void
MainWindow::s_redrawAlbum(QString albumname){
    // Sets albumItem to be a QListWidgetItem with its text set to the
    // string passed
    QListWidgetItem *albumItem = new QListWidgetItem();
    albumItem->setText(albumname);

    // calls redrawLists so that m_table will be filtered appropriately
    redrawLists(albumItem,ALBUM);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::traverseDirs:
//
// Traverse all subdirectories and collect filenames into m_listSongs.
//
void
MainWindow::traverseDirs(QString path)
{
	QStringList	list;

	// init listDirs with subdirectories of path
	QDir dir(path);
	dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
	QFileInfoList listDirs = dir.entryInfoList();
    
	// init listFiles with all *.mp3 files in path
	QDir files(path);
	files.setFilter(QDir::Files);
	files.setNameFilters(QStringList("*.mp3"));
	QFileInfoList listFiles = files.entryInfoList();

	for(int i=0; i < listFiles.size(); i++) {
		// init list with default values: ""
		for(int j=0; j<=COLS+1; j++)
            list.insert(j, "");

		// store file pathname into 0th position in list
		QFileInfo fileInfo = listFiles.at(i);
		list.replace(PATH, fileInfo.filePath());
        if(m_fullPathList->contains(fileInfo.filePath())) continue;
        m_fullPathList->append(fileInfo.filePath());
		
		// creates variable source of FileRef class
		TagLib::FileRef source(QFile::encodeName(fileInfo.filePath()).constData());
		if(!source.isNull() && source.tag()) {
			// creates a Tag variable in order to read the tags of source
			TagLib::Tag *tag = source.tag();

            // if the field of tag is not an empty string, then it replaces it in list
            // appropriately
            if(tag->genre() != "")
                list.replace(GENRE, TStringToQString(tag->genre()));
            if(tag->artist() != "")
                list.replace(ARTIST, TStringToQString(tag->artist()));
			if(tag->album() != ""){
				list.replace(ALBUM, TStringToQString(tag->album()));

                // This if statement checks to see if the album has already been loaded
                // in (this may be a problem if there are two albums with the same name,
                // but different album art)
                if(!m_fullAlbumlist->contains(TStringToQString(tag->album()))){
                    // Appends the album to the QLists
                    // m_albumList and m_pathList will be sent in a signal to the coverflow
                    // widget
                    m_albumList->append(TStringToQString(tag->album()));
                    m_pathList->append(fileInfo.filePath());
                    m_fullAlbumlist->append(TStringToQString(tag->album()));
				}
			}
            if(tag->title() != "")
                list.replace(TITLE, TStringToQString(tag->title()));

            // the track tag must be converted to a QString
			QString temp_track = QString("%1").arg(tag->track());
			if(temp_track != "0") list.replace(TRACK, temp_track);

			// In order to get the length of the music file, we must get the
			// audio properties of the file using source.audioProperties()
			if(source.audioProperties()){
				// prop is a variable that represents these audio properties
				TagLib::AudioProperties *prop = source.audioProperties();

				// prop->length() will return the length of the audio file
				// in seconds
				int seconds = prop->length()%60;
				int minutes = prop->length()/60;
				QString temp_time;

                // the following if statement is necessary to properly
                //display the time if the seconds is a single digit
				if(seconds < 10)
					temp_time = QString("%1:0%2").arg(minutes).arg(seconds);
				else temp_time = QString("%1:%2").arg(minutes).arg(seconds);
				list.replace(TIME, temp_time);
            }
		}
		
		// append list (song data) into songlist m_listSongs;
		// uninitialized fields are empty strings
        m_listSongs << list;
	}
	
	// recursively descend through all subdirectories
	for(int i=0; i<listDirs.size(); i++) {
		QFileInfo fileInfo = listDirs.at(i);
        traverseDirs(fileInfo.filePath());
	}
	return;
}		

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_load:
//
// Slot function for File|Load and for the load music folder button
//
void
MainWindow::s_load()
{
	// open a file dialog box
	QFileDialog *fd = new QFileDialog;

    // Sets the file dialog box so that it will only show directories
	fd->setFileMode(QFileDialog::Directory);
	QString s = fd->getExistingDirectory(0, "Select Folder", m_directory,
             QFileDialog::ShowDirsOnly |
			 QFileDialog::DontResolveSymlinks);

	// check if cancel was selected
	if(s == NULL) return;

	// copy full pathname of selected directory into m_directory
	m_directory = s;

    // Resets many of the widgets that contain song metadata so songs
    // can be loaded in properly
    m_table->setRowCount(0);
    m_playlistTable->setRowCount(0);
    m_panel[0]->clear();
    m_panel[1]->clear();
    m_panel[2]->clear();
    m_pathList->clear();
    m_albumList->clear();
    m_mediaplayer->stop();

    // Traverses m_directory and its subdirectories for mp3 files
    traverseDirs(m_directory);
	initLists();

    // Emits a signal to the coverflow widget with a QList of paths and
    // a QList of album names
    if(m_pathList->size() != 0 || m_albumList->size() != 0)
        emit s_artLoaded(m_pathList,m_albumList);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_panel1:
//
// Slot function to adjust data if an item in panel1 (genre) is selected.
//
void
MainWindow::s_panel1(QListWidgetItem *item)
{
    // "All" is used to remove all filters placed on the song table
    if(item->text() == "All") {
        m_panel[0]->clear();
        m_panel[1]->clear();
        m_panel[2]->clear();
        m_table->setRowCount(0);
		initLists();
		return;
	}

    // Clearing lists
	m_panel[1] ->clear();
	m_panel[2] ->clear();
	m_listArtist.clear();
	m_listAlbum .clear();
	
    // Collecting list of artists and albums
    // These artist and albums are only collected if the song they are from
    // has the selected genre
	for(int i=0; i<m_listSongs.size(); i++) {
        if(m_listSongs[i][GENRE] == item->text()){
			m_listArtist << m_listSongs[i][ARTIST];
			m_listAlbum << m_listSongs[i][ALBUM];
        }
	}

    // Sorting remaining two panels for artists and albums
	qStableSort(m_listArtist.begin(), m_listArtist.end(), caseInsensitive);
	qStableSort(m_listAlbum .begin(), m_listAlbum .end(), caseInsensitive);

    // Adding items to panels; skip over non-unique entries
	for(int i=0; i<m_listArtist.size(); i+=m_listArtist.count(m_listArtist[i]))
		m_panel[1]->addItem(m_listArtist [i]);
	for(int i=0; i<m_listAlbum.size(); i+=m_listAlbum.count(m_listAlbum[i]))
		m_panel[2]->addItem(m_listAlbum [i]);

	redrawLists(item, GENRE);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_panel2:
//
// Slot function to adjust data if an item in panel2 (artist) is selected.
//
void
MainWindow::s_panel2(QListWidgetItem *item)
{
	// clear lists
	m_panel[2]->clear();
	m_listAlbum.clear();
	
	// collect list of albums
    // albums are only collected from songs with the selected artist
	for(int i=0; i<m_listSongs.size(); i++) {
		if(m_listSongs[i][ARTIST] == item->text())
			m_listAlbum << m_listSongs[i][ALBUM];
	}

	// sort remaining panel for albums
	qStableSort(m_listAlbum.begin(), m_listAlbum.end(), caseInsensitive);
	
	// add items to panel; skip over non-unique entries
	for(int i=0; i<m_listAlbum.size(); i+=m_listAlbum.count(m_listAlbum[i]))
		m_panel[2]->addItem(m_listAlbum [i]);

	redrawLists(item, ARTIST);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_panel3:
//
// Slot function to adjust data if an item in panel3 (album) is selected.
//
void
MainWindow::s_panel3(QListWidgetItem *item)
{
	redrawLists(item, ALBUM);
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_about:
//
// Slot function for Help|About
//
void
MainWindow::s_about()
{
    QMessageBox::about(this, "About QTunes",
                "<center><font size = 18><font color = red><b> QTunes Music Player - Spring 2015 </b></font></center> \
                 <center> Skeleton code provided by George Wolberg </center> \
                 <center> Coverflow, populating song tables and lists, </center> \
                 <center>  general QTunes layout, and general TagLib code by Matthew Liu </center> \
                 <center> Playlist, filters, and toolbar buttons by William Gao </center> \
                 <center> Visualizer, time/volume slider, playback speed, and tag to qimage code by Kenichi Yamamoto </center>");
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_playButton:
//
// Slot function for when the play button is pressed
//
void MainWindow::s_playButton(){
    // If there is no selected item in the song table, nothing happens
    if(m_table->currentItem() == NULL)
        return;

    // If there is a selected song in the playlist, set m_table to highlight the same song
    if(m_playlistTable->currentItem() != NULL){
        int i = 0;
        while(m_table->item(i,1)->text() != m_playlistTable->currentItem()->text() && i < m_table->rowCount())
            i++;
        m_table->setCurrentItem(m_table->item(i,1));
    }

    // Call to funtion s_play that will play the selected song
    s_play(m_table->currentItem());
    return;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_prevSong:
//
// Slot function for when the previous song button is pressed
//
void MainWindow::s_prevSong()
{
    // If there is no selected item in the song table, nothing happens
    if(m_table->currentItem() == NULL)
            return;

    // If playlist is empty or not being used, backtrack from current song and loop to last song if at beginning
    if(m_playlistTable->rowCount() == 0) {
        QTableWidgetItem *temp = m_table->currentItem();
        if(temp->row() == 0)
            temp = m_table->item(m_table->rowCount()-1,1);
        else temp = m_table->item(temp->row()-1,1);
        m_table->setCurrentItem(temp);
        s_play(m_table->currentItem());
    }
    else // Songs backtrack from the playlist until first song and the current song in m_table is adjusted to highlight the same song
    {
         int i = 0;
         if(m_playlistTable->currentRow() == 0)
         {
             while(m_table->item(i,1)->text() != m_playlistTable->currentItem()->text() && i < m_table->rowCount())
                 i++;
             m_table->setCurrentItem(m_table->item(i,1));
             s_play(m_table->currentItem());
             return;
         }
        if(m_playlistTable->currentItem() == NULL)
         {
             m_playlistTable->setCurrentItem(m_playlistTable->item(0,0));
             while(m_table->item(i,1)->text() != m_playlistTable->currentItem()->text() && i < m_table->rowCount())
                 i++;
             m_table->setCurrentItem(m_table->item(i,1));
             s_play(m_table->currentItem());
             return;
          }
          QTableWidgetItem *temp = m_playlistTable->item(m_playlistTable->currentRow()-1,0);
          m_playlistTable->setCurrentItem(temp);
          while(m_table->item(i,1)->text() != m_playlistTable->currentItem()->text() && i < m_table->rowCount())
              i++;
          m_table->setCurrentItem(m_table->item(i,1));
          s_play(m_table->currentItem());
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_nextSong:
//
// Slot function for when the next song button is pressed
//
void MainWindow::s_nextSong(){
    // If there is no selected item in the song table, nothing happens
    if(m_table->currentItem() == NULL)
            return;

    // If playlist is empty or not being used, advance from the current song and loops back to first song if at the last song
    if(m_playlistTable->rowCount() == 0)
    {
        QTableWidgetItem *temp = m_table->currentItem();
        if(temp->row() == m_table->rowCount()-1)
            temp = m_table->item(0,1);
        else temp = m_table->item(temp->row()+1,1);
        m_table->setCurrentItem(temp);
        s_play(m_table->currentItem());
    }
    else // Songs advance from the playlist until last song and updates m_table to highlight the same song
    {
        int i = 0;
        if(m_playlistTable->currentRow() == m_playlistTable->rowCount()-1)
        {
            while(m_table->item(i,1)->text() != m_playlistTable->currentItem()->text() && i < m_table->rowCount())
                i++;
            m_table->setCurrentItem(m_table->item(i,1));
            s_play(m_table->currentItem());
            return;
        }
        if(m_playlistTable->currentItem() == NULL)
        {
            m_playlistTable->setCurrentItem(m_playlistTable->item(0,0));
            while(m_table->item(i,1)->text() != m_playlistTable->currentItem()->text() && i < m_table->rowCount())
                    i++;
            m_table->setCurrentItem(m_table->item(i,1));
            s_play(m_table->currentItem());
            return;
        }
        QTableWidgetItem *temp = m_playlistTable->item(m_playlistTable->currentRow()+1,0);
        m_playlistTable->setCurrentItem(temp);
        while(m_table->item(i,1)->text() != m_playlistTable->currentItem()->text() && i < m_table->rowCount())
            i++;
        m_table->setCurrentItem(m_table->item(i,1));
        s_play(m_table->currentItem());
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_pauseButton:
//
// Slot function for when the pause button is pressed
//
void MainWindow::s_pauseButton(){
    //Pauses the song
    m_mediaplayer->pause();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_setVolume:
//
// Slot function for when the value of the volume slider is changed
//
void MainWindow::s_setVolume(int Volume){
    //Sets the volume
    m_mediaplayer->setVolume(Volume);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_setPosition:
//
// Slot function for when the position of the currently playing song is changed
// This will change the value of the time slider to match the new position
//
void MainWindow::s_setPosition(qint64 Position){
    //Sets the value of the slider to match the position of the currently playing song
    m_timeSlider->setValue(Position);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_seek:
//
// Slot function for when the value of the time slider is changed
//
void MainWindow::s_seek(int newPosition){
    //Seeks to a new position in the song specified by newPosition
    qint64 position = (qint64)newPosition;
    m_mediaplayer->setPosition(position);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_updateLabel:
//
// Slot function for when the position of the currently playing song is changed
// This will change the value of the time label to match the new position
//
void MainWindow::s_updateLabel(qint64 Time){
    //Get the end time of the currently playing song in minutes and seconds
    int endSecond = ((int)m_mediaplayer->duration() / 1000)%60;
    int endMinute = ((int)m_mediaplayer->duration() / 1000)/60;
    QString endTime;

    //If the seconds are less than ten, puts a zero in the tens place
    if(endSecond < 10)
        endTime = QString("%1:0%2").arg(endMinute).arg(endSecond);
    else endTime = QString("%1:%2").arg(endMinute).arg(endSecond);

    //Get the current time of the currently playing song in minutes and seconds
    int currentSecond = ((int)Time / 1000)%60;
    int currentMinute = ((int)Time / 1000)/60;
    QString currentTime;

    //Same as above
    if(currentSecond < 10)
        currentTime = QString("%1:0%2").arg(currentMinute).arg(currentSecond);
    else currentTime = QString("%1:%2").arg(currentMinute).arg(currentSecond);

    //Creates a label to display the current time of the song.
    QString timeLabel = " / ";

    //Adds the current time (as a QString) to the position before the "/", and bolds it
    timeLabel.prepend(currentTime);
    timeLabel.prepend("<b>");

    //Adds the end time (as a QString) to the position after the "/" and closes the bold tag
    timeLabel.append(endTime);
    timeLabel.append("</b>");

    //Sets the time label
    m_timeLabel->setText(timeLabel);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_changeSpeed:
//
// Slot function for actions that change the speed of the song playback
//
void MainWindow::s_changeSpeed(){
    //Checks to see which of the playback speed actions is checked and sets the speed.
    //Since they all belong to QActionGroup, only one option can be selected at a time.
    if(m_slowestAction->isChecked()){
        m_mediaplayer->setPlaybackRate(0.50);
        emit s_visualizerSpeed(-2);
    }
    if(m_slowerAction->isChecked()){
        m_mediaplayer->setPlaybackRate(0.75);
        emit s_visualizerSpeed(-1);
    }
    if(m_normalAction->isChecked()){
        m_mediaplayer->setPlaybackRate(1.00);
        emit s_visualizerSpeed(0);
    }
    if(m_fasterAction->isChecked()){
        m_mediaplayer->setPlaybackRate(1.25);
        emit s_visualizerSpeed(1);
    }
    if(m_fastestAction->isChecked()){
        m_mediaplayer->setPlaybackRate(1.50);
        emit s_visualizerSpeed(2);
    }
    return;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_setDuration:
//
// Slot function for when the duration of the song loaded into m_mediaplayer
// changes (will change when a new song is loaded)
//
void MainWindow::s_setDuration(qint64 Duration){
    //Sets the duration of the time slider
    m_timeSlider->setRange(0,Duration);
    return;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_searchSongs:
//
// Slot function when the search button is pressed or when return is pressed
// when typing things into the search bar
//
void MainWindow::s_searchSongs(){
    //Intialize variables
    QList<QString> m_listFound;
    m_listFound.clear();
    QString inputstr = m_searchbox->text();

    //If there is no text in the searchbox, simply return the unfiltered m_table
    if(inputstr == ""){
        m_table->setRowCount(0);
        initLists();
        return;
    }

    //If the length of input is less than 3, return. This is to prevent the program
    //from stalling on very large directories.
    else if(inputstr.size() < 3){
        return;
    }

    //If there is a title in m_listSongs that contains the input str, append it to the list
    //of found items
    for(int i=0; i<m_listSongs.size(); i++){
        if(m_listSongs[i][TITLE].contains(inputstr, Qt::CaseInsensitive)){
            m_listFound.append(m_listSongs[i][TITLE]);
        }
    }

    //Turn the QList<QString> of found titles to a QListWidget for redrawLists()
    //The bool allows for a certain lines of code within redrawLists() to be
    //temporarily disabled
    QListWidget foundsongs;
    foundsongs.addItems(m_listFound);
    isSearch = true;

    //We first clear the table, then we add each of found songs to m_table
    m_table->setRowCount(0);
    for(int i = foundsongs.count() - 1; i >= 0; i--){
        //qDebug() << m_listFound[i];
        redrawLists(foundsongs.item(i), TITLE);
    }
    isSearch = false;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_toggleNightMode:
//
// Slot function for when the night mode action is triggered
// Will toggle between night mode and normal mode for QTunes
//
void MainWindow::s_toggleNightMode(){
    // If the nightmode action is checked, set the stylesheet of the widgets
    // in MainWindow to a black or dark gray color.
    if(m_nightmodeAction->isChecked()){
        m_tabs->setStyleSheet("background: black; \
                          ");
        m_volumeSlider->setStyleSheet("QSlider::groove:horizonal{background-color: #444; \
            height: 5px; \
            border: 1px solid grey;} \
            QSlider::handle:horizonal{background: qlineargradient(x1:0, y1:0, x2:1, y2:1, \
            stop:0 #555, stop:1 #777); \
            border: 1px solid grey; \
            width: 5px; \
            margin: -5px;} \
            QSlider::handle:horizontal:disabled { \
            background-color: #444; \
            border: 1px solid grey; \
            width: 5px; \
            border-radius: 4px;} \
            ");
        this->setStyleSheet("MainWindow{background-color: black; \
            color: white;} \
            QMenu{background: #444; \
            color: white;} \
            QMenuBar{background: #444; \
            color: white;} \
            QMenuBar::item{background-color: #444;} \
            QMenuBar::item:selected{background-color: #666;} \
            QScrollBar:vertical{background-color: black;} \
            QTabBar::tab{background-color: #444; \
            color: white;} \
            QTabBar::tab:selected{background-color: #666;} \
            QTabWidget::pane{border: 2px solid #444} \
            QSplitter::handle{background-color: black;} \
            QLineEdit{background-color: black; \
            color: white; \
            border: 2px solid #444; \
            border-radius: 2px;} \
            QTableView{background-color: #333; \
            alternate-background-color: #222; \
            color: white;} \
            QTableCornerButton::section{background-color: #444; \
            border: 2px;} \
            QHeaderView::section{background-color: #444; \
            color: white;} \
            QLabel{color: white;} \
            QListWidget{color: white} \
            QToolButton{background: qlineargradient(x1:0, y1:0, x2:1, y2:1, \
            stop:0 #444, stop:1 #666); \
            color: white; \
            border: 2px solid #444; \
            border-radius: 2px;} \
            QToolButton::pressed{background: qlineargradient(x1:0, y1:0, x2:1, y2:1, \
            stop:0 #AAA, stop:1 #CCC); \
            color: white; \
            border: 2px solid white; \
            border-radius: 2px;} \
            QToolButton::hover{background: qlineargradient(x1:0, y1:0, x2:1, y2:1, \
            stop:0 #999, stop:1 #BBB); \
            color: white; \
            border: 2px solid #444; \
            border-radius: 2px;} \
            QCheckBox{background: black;} \
            ");
        m_timeSlider->setStyleSheet(m_timeSlider->styleSheet() + ts_styleSheet);
        m_imageLabel->setStyleSheet("QLabel{border: 1px solid #444;\
                                    color: white;}");
    }
    // If nightmode is untoggled, this resets the stylesheet to the default by replacing the
    // stylesheet QString with nothing (""). The time slider, however, is changed to white so
    // that it retains its appearance (and the slider color).
    else{
    m_tabs->setStyleSheet("");
    m_volumeSlider->setStyleSheet("");
    this->setStyleSheet("");
    m_imageLabel->setStyleSheet("QLabel{border: 1px solid #BBB}");
    m_timeSlider->setStyleSheet(m_timeSlider->styleSheet() + "QSlider::add-page:horizontal { \
        background: #fff; \
        border: 1px solid #777; \
        height: 10px; \
        border-radius: 4px;} \
        QSlider::handle:horizontal { \
        background: qlineargradient(x1:0, y1:0, x2:1, y2:1, \
        stop:0 #eee, stop:1 #ccc); \
        border: 1px solid #777; \
        width: 13px; \
        margin-top: -2px; \
        margin-bottom: -2px; \
        border-radius: 4px; \
        } \
        QSlider::handle:horizontal:hover { \
        background: qlineargradient(x1:0, y1:0, x2:1, y2:1, \
        stop:0 #fff, stop:1 #ddd); \
        border: 1px solid #444; \
        border-radius: 4px; \
        } \
        ");
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_cycleSliderColor:
//
// Slot function for when the cycle slider action is triggered
// This will change the time slider color
//
void MainWindow::s_cycleSliderColor(){
    // Cycles through the time slider's color in the following order:
    // Red->Orange->Yellow->Green->Blue->Indigo->Pink then back to Red
    if(colorval > 6)
        colorval = 0;

     switch(colorval){
     case 0:
         s_setSliderColor("#990000", "#CC0000", "#FF0000");
         break;
     case 1:
         s_setSliderColor("#E68A00", "#FF9900", "#FFA319");
         break;
     case 2:
         s_setSliderColor("#CCCC00", "#E6E600", "#FFFF00");
         break;
     case 3:
         s_setSliderColor("#009900","#33CC33","#00FF00");
         break;
     case 4:
         s_setSliderColor("#0099FF","#33CCFF","#0000FF");
         break;
     case 5:
         s_setSliderColor("#4B0082", "#6800B5", "#7700FF");
         break;
     case 6:
         s_setSliderColor("#990099","#CC00CC","#FF00FF");
         break;
     }
     colorval++;

     if(m_nightmodeAction->isChecked())
         m_timeSlider->setStyleSheet(m_timeSlider->styleSheet() + ts_styleSheet);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_setSliderColor:
//
// This function sets both the appearance of the time slider as well as the color of the slider's
// progress. It replaces QString arguments %1, %2, and %3 with hex code color arguments given by
// QString a, b, and c, respectively.
//
void MainWindow::s_setSliderColor(QString a, QString b, QString c){
    // Sets the stylesheet appearance as well as the color of the slider's progress via
    // the color code values passed to it.
    m_timeSlider->setStyleSheet(QString("QSlider::groove:horizontal { \
        border: 1px solid #bbb; \
        background: white; \
        height: 10px; \
        border-radius: 4px;\
        }\
        QSlider::sub-page:horizontal { \
        background: qlineargradient(x1: 0, y1: 0,    x2: 0, y2: 1, \
            stop: 0 %1, stop: 1 %2); \
        background: qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1, \
            stop: 0 %2, stop: 1 %3); \
        border: 1px solid #777; \
        height: 10px; \
        border-radius: 4px; \
        }\
        QSlider::add-page:horizontal { \
        background: #fff; \
        border: 1px solid #777; \
        height: 10px; \
        border-radius: 4px; \
        }\
        QSlider::handle:horizontal { \
        background: qlineargradient(x1:0, y1:0, x2:1, y2:1, \
            stop:0 #eee, stop:1 #ccc); \
        border: 1px solid #777; \
        width: 13px; \
        margin-top: -2px; \
        margin-bottom: -2px; \
        border-radius: 4px; \
        } \
        QSlider::handle:horizontal:hover { \
        background: qlineargradient(x1:0, y1:0, x2:1, y2:1, \
            stop:0 #fff, stop:1 #ddd); \
        border: 1px solid #444; \
        border-radius: 4px; \
        } \
        QSlider::sub-page:horizontal:disabled { \
        background: #bbb; \
        border-color: #999; \
        } \
        QSlider::add-page:horizontal:disabled { \
        background: #eee; \
        border-color: #999; \
        } \
        QSlider::handle:horizontal:disabled { \
        background: #eee; \
        border: 1px solid #aaa; \
        border-radius: 4px; \
        }").arg(a,b,c));
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::imageForTag
//
// Obtain the cover art for a given track using taglib.
//
QImage MainWindow::imageForTag(TagLib::ID3v2::Tag *tag)
{
    QImage image;
    //Creates a framelist from the given tag using "APIC",
    //Which stands for "attached picture"
    const TagLib::ID3v2::FrameList list = tag->frameList("APIC");

    // Returns a default album art if list is empty
    if(list.isEmpty()){
        image.load(":/Resources/Default.png");
        return image;
    }

    //front() specifies the cover art of the song
    TagLib::ID3v2::AttachedPictureFrame *frame =
        static_cast<TagLib::ID3v2::AttachedPictureFrame *>(list.front());
    //Load the picture from the song's frame into the Qimage, and return it
    image.loadFromData((const uchar *) frame->picture().data(), frame->picture().size());
    //If nothing is loaded and the image is Null, we return the default album art
    //since loading a null image can cause QTunes to crash.
    if(image.isNull()){
        image.load(":/Resources/Default.png");
        return image;
    }
    return image;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_play:
//
// Slot function to play an mp3 song.
// This uses TagLib as well as Qt5's Multimedia module.
//
void
MainWindow::s_play(QTableWidgetItem *item)
{
    // Does nothing if the item is null
    if(item == NULL)
        return;

    // Plays a new song from pause state if the current song selected isn't the same as before pausing
    if(m_mediaplayer->state() == 2 && item->row() == m_prevItem->row()){
        m_mediaplayer->play();
        return;
    }
    item = m_table->item(item->row(),1);

    // Traverses through m_listSongs looking for the song that was selected
	for(int i=0; i<m_listSongs.size(); i++) {
		// skip over songs whose title does not match
		if(m_listSongs[i][TITLE] != item->text()) continue;
		QString temp_title = QString("%1").arg(m_listSongs[i][PATH]);
		m_mediaplayer->setMedia(QUrl::fromLocalFile(temp_title));
		m_mediaplayer->play();

        // This creates an ID3v2Tag from which a QImage can be created usng
        // imageForTag
        QByteArray ba_temp = temp_title.toLocal8Bit();
        const char* filepath = ba_temp.data();
        TagLib::MPEG::File audioFile(filepath);
        TagLib::ID3v2::Tag *tag = audioFile.ID3v2Tag(true);
        QImage coverArt = imageForTag(tag);
		
        //Resizes coverArt to a fixed size (250x250) while keeping its aspect ratio
        m_resizedArt = coverArt.scaled(250,250,Qt::KeepAspectRatio);
        m_imageLabel->setScaledContents(true);

        //Puts the resized image into the image label
        m_imageLabel->setPixmap(QPixmap::fromImage(m_resizedArt));
        m_prevItem = item;
		return;
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::statusChanged:
//
// Slot function that is called when the status of m_mediaplayer is changed
//
void MainWindow::statusChanged(QMediaPlayer::MediaStatus status)
{
    // If the status of m_mediaplayer is BufferedMedia, then the media is buffered
    // This will enable the buttons and play the song loaded
    if(status == QMediaPlayer::BufferedMedia){
            qDebug("Media is buffered");
            m_timeSlider->setEnabled(true);
            m_volumeSlider->setEnabled(true);
            m_play->setEnabled(true);
            m_stop->setEnabled(true);
            m_prevSong->setEnabled(true);
            m_nextSong->setEnabled(true);
            m_pause->setEnabled(true);

            //Sets the window title to be the in the following format:
            //<Artist> - <Title>
            QString QTunes = " - ";
            QTunes.prepend(m_table->item(m_table->currentRow(),4)->text());
            QTunes.append(m_table->item(m_table->currentRow(),1)->text());
            setWindowTitle(QTunes);
            m_mediaplayer->play();
    }

    // Songs automatically advance to the next song in m_table after finishing if playlist is not being used
    if(status == QMediaPlayer::EndOfMedia && m_playlistTable->rowCount() == 0)
    {
        QTableWidgetItem* nextItem = new QTableWidgetItem;
        if(m_table->currentRow() + 1 > m_table->rowCount()-1)
        {
            m_table->setCurrentCell(0,1);
            s_play(m_table->currentItem());
            return;
        }

        nextItem = m_table->item(m_table->currentRow()+1,0);
        m_table->setCurrentItem(nextItem);
        s_play(m_table->currentItem());
    }

    // Songs automatically advance to the next song in the playlist and the m_table is adjusted to highlight the same song
    if(status == QMediaPlayer::EndOfMedia && m_playlistTable->currentRow() < m_playlistTable->rowCount())
    {
        if(m_repeat->isChecked()) s_play(m_table->currentItem());
        if(m_playlistTable->currentRow()+1 >= m_playlistTable->rowCount())
            return;

        int i = 0;
        QTableWidgetItem* nextItem = new QTableWidgetItem;
        nextItem = m_playlistTable->item(m_playlistTable->currentRow()+1,0);
        while(m_table->item(i,1)->text() != nextItem->text() && i < m_table->rowCount())
            i++;
        m_table->setCurrentItem(m_table->item(i,1));
        m_playlistTable->setCurrentItem(nextItem);
        s_play(m_table->currentItem());
    }
    // The following code creates a singleshot qtimer within a loop that will cause the loop to exit
    // after 1 ms. This is used in order to fix a specific problem with Direct Show Player that causes
    // certain songs to be unplayable in Windows.
    QEventLoop loop;
    QTimer::singleShot(1, &loop, SLOT(quit()));
    loop.exec();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_repeat:
//
// Slot function for when the repeat button is toggled
//
void MainWindow::s_repeat()
{
    // Turns off shuffle and adjusts the playlist to only the current selected song
    if(m_repeat->isChecked() == true)
    {
        m_shuffle->setChecked(false);
        QTableWidgetItem *playlistItem = new QTableWidgetItem;
        m_playlistTable->setRowCount(0);
        m_playlistTable->insertRow(0);
        if(m_table->currentItem() == NULL)
        {
            playlistItem->setText(m_table->item(0,1)->text());
            m_playlistTable->setItem(0,0,playlistItem);
        }
        else
        {
            playlistItem->setText(m_table->item(m_table->currentRow(),1)->text());
            m_playlistTable->setItem(0,0,playlistItem);
        }
    }

    // Repopulates the playlist with the selected songs
    else
    {
        m_playlistTable->setRowCount(0);
        int playlistRowCount = 0;
        for(int i = 0; i < m_table->rowCount(); i++)
        {
            m_checkbox = m_table->cellWidget(i,0)->findChild<QCheckBox *>();
            if(m_checkbox->isChecked() == true)
            {
                QTableWidgetItem* playlistItem = new QTableWidgetItem;
                playlistItem->setText(m_table->item(i,1)->text());
                m_playlistTable->insertRow(playlistRowCount);
                m_playlistTable->setItem(playlistRowCount,0,playlistItem);
                playlistRowCount++;
            }
        }
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_shuffle:
//
// Slot function for when the shuffle button is toggled
//
void MainWindow::s_shuffle()
{
    // Turns off repeat and shuffles the songs in the playlist
    if(m_shuffle->isChecked() == true)
    {
        m_repeat->setChecked(false);
        QTableWidgetItem* playlistItem1 = new QTableWidgetItem;
        QTableWidgetItem* playlistItem2 = new QTableWidgetItem;
        int randnum1 = 0;
        int randnum2 = 0;
        for(int i = 0; i < m_playlistTable->rowCount(); i++)
        {
            randnum1 = rand() % m_playlistTable->rowCount();
            randnum2 = rand() % m_playlistTable->rowCount();
            while(randnum1 == randnum2)
            {
                randnum1 = rand() % m_playlistTable->rowCount();
                randnum2 = rand() % m_playlistTable->rowCount();
            }
            playlistItem1 = m_playlistTable->takeItem(randnum1,0);
            playlistItem2 = m_playlistTable->takeItem(randnum2,0);
            m_playlistTable->setItem(randnum1,0,playlistItem2);
            m_playlistTable->setItem(randnum2,0,playlistItem1);
        }
    }

    // Unshuffles the songs back to original form
    else
    {
        m_playlistTable->setRowCount(0);
        int playlistRowCount = 0;
        for(int i = 0; i < m_table->rowCount(); i++)
        {
            m_checkbox = m_table->cellWidget(i,0)->findChild<QCheckBox *>();
            if(m_checkbox->isChecked() == true)
            {
                QTableWidgetItem* playlistItem = new QTableWidgetItem;
                playlistItem->setText(m_table->item(i,1)->text());
                m_playlistTable->insertRow(playlistRowCount);
                m_playlistTable->setItem(playlistRowCount,0,playlistItem);
                playlistRowCount++;
            }
        }
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_checkboxSelect:
//
// Slot function for when the checkbox that selects or deselects the songs if
// the song table is toggled
//
void MainWindow::s_checkboxSelect()
{
    //User toggled button to select all songs
    //Change button text to "Deselect" if user decides to deselect later
    //Set all checkboxes to checked and add songs to playlist as well as enable the repeat and shuffle button if disabled
    if(m_checkboxSelect->isChecked())
    {
        m_playlistTable->setRowCount(0);
        m_checkboxSelect->setText("Deselect All");
        int currentPosition = m_table->currentRow();
        for(int i = 0; i < m_table->rowCount(); i++)
        {
            QTableWidgetItem *playlistItem = new QTableWidgetItem;
            m_table->setCurrentCell(i,0);
            m_checkbox = m_table->cellWidget(i,0)->findChild<QCheckBox *>();
            m_checkbox->setChecked(true);
            playlistItem->setText(m_table->item(m_table->currentRow(),1)->text());
            m_playlistTable->insertRow(i);
            m_playlistTable->setItem(i,0,playlistItem);
        }
        m_table->setCurrentCell(currentPosition,1);
        m_repeat->setEnabled(true);
        m_shuffle->setEnabled(true);
    }

    // User toggled button to deselect all songs
    //Change button text to "Select" if user decides to select later
    //Set all checkboxes to unchecked and clears the playlist as well as disables the repeat and shuffle button
    else
    {
        m_checkboxSelect->setText("Select All");
        for(int i = 0; i < m_table->rowCount(); i++)
        {
            m_checkbox = m_table->cellWidget(i,0)->findChild<QCheckBox *>();
            m_checkbox->setChecked(false);
            m_playlistTable->setRowCount(0);
        }
        m_repeat->setChecked(false);
        m_repeat->setDisabled(true);
        m_shuffle->setChecked(false);
        m_shuffle->setDisabled(true);
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_playlistUpdate:
//
// Updates playlist with current status of selected or deselected songs
//
void MainWindow::s_playlistUpdate()
{
    //Sets text and checkbox to the current song in m_table
    QTableWidgetItem *playlistItem = new QTableWidgetItem;
    playlistItem->setText(m_table->item(m_table->currentRow(),1)->text());
    m_checkbox = m_table->cellWidget(m_table->currentRow(),0)->findChild<QCheckBox *>();

    //Sets checkbox to update only if its column is clicked, otherwise just update the current selected song in m_table
    if(m_table->currentColumn() != 0)
    {
        if(m_checkbox->isChecked() == false)
            return;

        if(m_playlistTable->currentItem() == NULL)
            m_playlistTable->setCurrentCell(0,0);

        if(m_repeat->isChecked())
        {
            m_playlistTable->currentItem()->setText(m_table->item(m_table->currentRow(),1)->text());
            return;
        }

        int i = 0;
        while(m_playlistTable->item(i,0)->text() != m_table->item(m_table->currentRow(),1)->text() && i < m_playlistTable->rowCount())
            i++;
        m_playlistTable->setCurrentCell(i,0);
    }
    else
    {
        // Does not toggle button if directly clicked
        if(isDirectClicked == false)
            m_checkbox->toggle();

         // Adds song to playlist if the checkbox is checked
         if(m_checkbox->isChecked() == true)
         {
            m_playlistTable->insertRow(0);
            m_playlistTable->setItem(0,0,playlistItem);

            if(m_repeat->isEnabled() == false || m_shuffle->isEnabled() == false)
            {
                m_repeat->setEnabled(true);
                m_shuffle->setEnabled(true);
            }

            if(m_playlistTable->rowCount() == m_table->rowCount())
                m_checkboxSelect->setChecked(true);
         }
         else //Removes song from playlist if unchecked
         {
             for(int i = 0; i < m_playlistTable->rowCount(); i++)
             {
                 if(m_table->item(m_table->currentRow(),1)->text() == m_playlistTable->item(i,0)->text())
                 {
                     m_playlistTable->removeRow(i);
                 }

                 if(m_playlistTable->rowCount() == 0)
                 {
                     m_checkboxSelect->setChecked(false);
                 }
            }
         }
    }
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_directClicked:
//
// Updates playlist with current status of selected or deselected songs if checkbox is directly clicked
//

void MainWindow::s_directClicked(int row)
{
    m_table->setCurrentCell(row,0);
    isDirectClicked = true;
    s_playlistUpdate();
    isDirectClicked = false;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_tableUpdate:
//
// Syncs current song in the playlist with the song in m_table
//
void MainWindow::s_tableUpdate()
{
    QTableWidgetItem *m_tableItem = new QTableWidgetItem;
    m_tableItem->setText(m_playlistTable->item(m_playlistTable->currentRow(),0)->text());
    int i = 0;
    while(m_table->item(i,1)->text() != m_playlistTable->item(m_playlistTable->currentRow(),0)->text()
          && i < m_table->rowCount())
        i++;
    m_table->setCurrentCell(i,0);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_albumLabel:
//
// Slot function that will set m_albumLabel to the album name passed by
// the coverflow widget
//
void MainWindow::s_albumLabel(QString newAlbum){
    m_albumLabel->setText(newAlbum);
}
