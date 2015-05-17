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
#include <QPalette>
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

enum { SELECT, TITLE, TRACK, TIME, ARTIST, ALBUM, GENRE, PATH, ALBUMID};
const int COLS = PATH;

bool caseInsensitive(const QString &s1, const QString &s2)
{
	return s1.toLower() < s2.toLower();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::MainWindow:
//
// Constructor. Initialize user-interface elements.
//
MainWindow::MainWindow	(QString program)
	   : m_directory(".")
{
	// setup GUI with actions, menus, widgets, and layouts
	createActions();	// create actions for each menu item
	createMenus  ();	// create menus and associate actions
	createWidgets();	// create window widgets
	createLayouts();	// create widget layouts
	m_mediaplayer = new QMediaPlayer;
	// populate the list widgets with music library data
	initLists();		// init list widgets

    setWindowTitle("QTunes Music Player");
    setFocusPolicy(Qt::StrongFocus);
    isSearch = false;
    colorval = 0;

    m_normalAction->setChecked(true);
    m_timeSlider->installEventFilter(this);
	// set central widget and default size
	setCentralWidget(m_mainWidget);
	setMinimumSize(400, 300);
        resize(830, 830);
	m_artlist = new QList<QImage>;
	connect(m_stop, SIGNAL(clicked()), m_mediaplayer, SLOT(stop()));
	connect(m_play, SIGNAL(clicked()), this, SLOT(s_playbutton()));	
	connect(m_pause, SIGNAL(clicked()), this, SLOT(s_pausebutton()));
	connect(m_nextsong, SIGNAL(clicked()), this, SLOT(s_nextsong()));
	connect(m_prevsong, SIGNAL(clicked()), this, SLOT(s_prevsong()));
    connect(m_repeat, SIGNAL(toggled(bool)), this, SLOT(s_repeat()));
    connect(m_shuffle, SIGNAL(toggled(bool)), this, SLOT(s_shuffle()));
    connect(m_checkboxSelect, SIGNAL(toggled(bool)), this, SLOT(s_checkboxSelect()));

	connect(m_mediaplayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));
    connect(m_volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(s_setVolume(int)));
    //connect(m_albumleft, SIGNAL(clicked()), m_squares, SLOT(s_shiftleft()));
    //connect(m_albumright, SIGNAL(clicked()), m_squares, SLOT(s_shiftright()));
    //connect(m_loadart, SIGNAL(clicked()), m_squares, SLOT(s_loadart()));
    connect(m_loadart, SIGNAL(clicked()), this, SLOT(s_load()));
	connect(m_mediaplayer, SIGNAL(positionChanged(qint64)), this, SLOT(s_setPosition(qint64))); 
	connect(m_mediaplayer, SIGNAL(positionChanged(qint64)), this, SLOT(s_updateLabel(qint64))); 
        connect(m_mediaplayer, SIGNAL(durationChanged(qint64)), this, SLOT(s_setDuration(qint64)));
    connect(m_timeSlider, SIGNAL(sliderMoved(int)), this, SLOT(s_seek(int)));
    connect(m_table, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(playlistUpdate()));
    connect(this, SIGNAL(s_artLoaded(QList<QImage>*)), m_squares,SLOT(s_mp3art(QList<QImage>*)));
    connect(m_mediaplayer, SIGNAL(stateChanged(QMediaPlayer::State)), m_visualizer, SLOT(s_toggle(QMediaPlayer::State)));
    connect(this, SIGNAL(s_visualizerSpeed(signed int)), m_visualizer, SLOT(s_changeSpeed(signed int)));
    connect(m_searchbox, SIGNAL(returnPressed()), this, SLOT(s_searchSongs()));
    connect(m_search, SIGNAL(clicked()), this, SLOT(s_searchSongs()));
}




// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::~MainWindow:
//
// Destructor. Save settings.
//
MainWindow::~MainWindow() {}

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
                //The following two if statements shift the new slider position more to the left or right
                // when clicked at the beginnning and end respectively. This allows the new position to be more accurate.
                if(newposition <= 250){
                    int adjustedlow = 10 - newposition/25;
                    newposition = newposition - adjustedlow;
                }
                if(newposition >= 250){
                    int adjustedhigh = -10 + newposition/25;
                    newposition = newposition + adjustedhigh;
                }
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
    if(object == m_volumeSlider && event->type() == QEvent::MouseButtonPress){
        QMouseEvent *mouseEvent2 = static_cast<QMouseEvent *>(event);
        if(mouseEvent2->button() == Qt::LeftButton){
            int newvolume = QStyle::sliderValueFromPosition(0,100,mouseEvent2->x(),100,0);
            int curvolume = QStyle::sliderValueFromPosition(0,100,m_volumeSlider->value(),m_volumeSlider->maximum(),0);
            qDebug() << newvolume << curvolume;
            if(abs(newvolume - curvolume) > 5)
                m_volumeSlider->setValue(newvolume);
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
    case Qt::Key_Space:
            if(m_mediaplayer->state() == 2){
                m_mediaplayer->play();
                return;
            }
            else{
                m_mediaplayer->pause();
                return;
            }
    case Qt::Key_Left:
        m_squares->s_shiftleft();
        return;
    case Qt::Key_Right:
        m_squares->s_shiftright();
        return;
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
	m_loadAction = new QAction("&Load Music Folder", this);
	m_loadAction->setShortcut(tr("Ctrl+L"));
	connect(m_loadAction, SIGNAL(triggered()), this, SLOT(s_load()));

	m_quitAction = new QAction("&Quit", this);
	m_quitAction->setShortcut(tr("Ctrl+Q"));
	connect(m_quitAction, SIGNAL(triggered()), this, SLOT(close()));

	m_aboutAction = new QAction("&About", this);
	m_aboutAction->setShortcut(tr("Ctrl+A"));
	connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(s_about()));

        // The following five QActions set the different playback speeds of the player

        m_slowestAction = new QAction("Slowest (0.50x)", this);
        m_slowestAction->setCheckable(true);
        connect(m_slowestAction, SIGNAL(triggered()), this, SLOT(s_changeSpeed()));

        m_slowerAction = new QAction("Slower (0.75x)", this);
        m_slowerAction->setCheckable(true);
        connect(m_slowerAction, SIGNAL(triggered()), this, SLOT(s_changeSpeed()));

        m_normalAction = new QAction("Normal (1.00x)", this);
        m_normalAction->setCheckable(true);
        connect(m_normalAction, SIGNAL(triggered()), this, SLOT(s_changeSpeed()));

        m_fasterAction = new QAction("Fast (1.25x)", this);
        m_fasterAction->setCheckable(true);
        connect(m_fasterAction, SIGNAL(triggered()), this, SLOT(s_changeSpeed()));

        m_fastestAction = new QAction("Fastest (1.50x)", this);
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

        m_nightmodeAction = new QAction("Night Mode", this);
        m_nightmodeAction->setShortcut(tr("Ctrl+N"));
        m_nightmodeAction->setCheckable(true);
        connect(m_nightmodeAction, SIGNAL(triggered()), this, SLOT(s_toggleNightMode()));

        m_slidercolorAction = new QAction("Cycle slider color", this);
        m_slidercolorAction->setShortcut(tr("Ctrl+C"));
        connect(m_slidercolorAction, SIGNAL(triggered()), this, SLOT(s_cycleSliderColor()));
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::createMenus:
//
// Create menus and install in menubar.
//
void
MainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu("&File");
    m_fileMenu->addAction(m_loadAction);
    m_fileMenu->addAction(m_quitAction);

    m_helpMenu = menuBar()->addMenu("&Help");
    m_helpMenu->addAction(m_aboutAction);

    m_playbackMenu = menuBar()->addMenu("&Playback Speed");
    m_playbackMenu->addAction(m_slowestAction);
    m_playbackMenu->addAction(m_slowerAction);
    m_playbackMenu->addAction(m_normalAction);
    m_playbackMenu->addAction(m_fasterAction);
    m_playbackMenu->addAction(m_fastestAction);

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
        m_mainWidget = new QWidget;
        m_mainBox = new QVBoxLayout;
        
        m_split1 = new QSplitter(Qt::Horizontal);
        m_split1->setMaximumWidth(830);
        
        m_split2 = new QSplitter(Qt::Horizontal);
        m_split2->setMinimumWidth(800);

        m_staticImage = new QWidget;
        m_staticImage->setMaximumHeight(100);
        m_staticImage->setMaximumWidth(100);
        m_staticImage->resize(100,100);
	
        m_musicWidgets = new QWidget;
        m_musicWidgets->setMaximumHeight(100);
        m_musicWidgets->setMinimumWidth(620);
        m_musicWidgets->setMaximumWidth(700);

        //m_musicWidgets->resize(600,100);

        m_squares = new SquaresWidget;

	// initialize label on right side of main splitter
	for(int i=0; i<3; i++) {
		// make label widget with centered text and sunken panels
		m_label[i] = new QLabel;
		m_label[i]->setAlignment(Qt::AlignCenter);
		m_label[i]->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	}

	// initialize label text
	m_label[0]->setText("<b>Genre<\b>" );
	m_label[1]->setText("<b>Artist<\b>");
	m_label[2]->setText("<b>Album<\b>" );

	// initialize list widgets: genre, artist, album
	for(int i=0; i<3; i++)
		m_panel[i] = new QListWidget;

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
        m_table->setMaximumHeight(325);
        m_table->resize(600,360);
        //m_table->setRowCount(10);

	// init signal/slot connections
	connect(m_panel[0],	SIGNAL(itemClicked(QListWidgetItem*)),
		this,		  SLOT(s_panel1   (QListWidgetItem*)));
        connect(m_panel[1],	SIGNAL(itemClicked(QListWidgetItem*)),
		this,		  SLOT(s_panel2   (QListWidgetItem*)));
        connect(m_panel[2],	SIGNAL(itemClicked(QListWidgetItem*)),
		this,		  SLOT(s_panel3   (QListWidgetItem*)));
        connect(m_table,	SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
		this,		  SLOT(s_play	  (QTableWidgetItem*)));

    m_visualizer = new VisualizerWidget;
    m_checkboxTable = new QTableWidget;
    m_checkbox = new QCheckBox;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::createLayouts:
//
// Create layouts for widgets.
//
void
MainWindow::createLayouts()
{
	// create a 2-row, 3-column grid layout, where row0 and
	// row1 consist of labels and list widgets, respectively.
	QWidget	    *widget = new QWidget(this);
	QGridLayout *grid   = new QGridLayout(widget);
	grid->addWidget(m_label[0], 0, 0);
	grid->addWidget(m_label[1], 0, 1);
	grid->addWidget(m_label[2], 0, 2);
	grid->addWidget(m_panel[0], 1, 0);
	grid->addWidget(m_panel[1], 1, 1);
	grid->addWidget(m_panel[2], 1, 2);

	// add widgets to the splitters
	QWidget *buttonwidget = new QWidget;
    QWidget *sliderwidget = new QWidget;
	m_buttonlayout = new QHBoxLayout;
    m_sliderlayout = new QHBoxLayout;
	m_play = new QToolButton;
    m_play->setEnabled(false);
	m_stop = new QToolButton;
    m_stop->setEnabled(false);
	m_prevsong = new QToolButton;
    m_prevsong->setEnabled(false);
	m_nextsong = new QToolButton;
    m_nextsong->setEnabled(false);
	m_pause = new QToolButton;
    m_pause->setEnabled(false);

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
    //m_searchbox->setMaximumWidth(200);
    m_searchbox->setPlaceholderText("Search");
    m_searchbox->setEnabled(false);

    m_search = new QToolButton;
    m_search->setEnabled(false);

    m_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	m_stop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
	m_prevsong->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	m_nextsong->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	m_pause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
	
    m_repeat->setIcon(QIcon(":/Resources/Repeat.png"));
    m_repeat->setIconSize(QSize(16.5,16.5));
    m_shuffle->setIcon(QIcon(":/Resources/Shuffle.png"));
    m_shuffle->setIconSize(QSize(16.5,16.5));
    m_search->setIcon(QIcon(":/Resources/Search.png"));
    m_search->setIconSize(QSize(16.5,16.5));

    m_checkboxSelect->setText("Deselect All");
    m_checkboxSelect->resize(16.5,16.5);

	m_loadart = new QToolButton;
	m_loadart->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
	
	m_imagelabel = new QLabel; 
	m_imagelabel ->setText("No Art");
    m_volumeSlider = new QSlider(Qt::Horizontal, buttonwidget);
    m_volumeSlider->setFixedWidth(100);
    m_volumeSlider->setEnabled(false);
	m_timeLabel = new QLabel;
    m_timeLabel ->setText("<b>0:00 / 0:00</b>");
    m_timeSlider = new QSlider(Qt::Horizontal);
    m_timeSlider ->setParent(this);
    //m_timeSlider ->setRange(0, 0);
    m_timeSlider ->setEnabled(false);
    m_timeSlider ->setFixedWidth(500);

	//m_volumeSlider->setMaximumWidth(50);
    //m_buttonlayout ->addWidget(m_albumleft);
	m_buttonlayout ->addWidget(m_volumeSlider);
	m_buttonlayout ->addWidget(m_loadart);
	m_buttonlayout ->addWidget(m_prevsong);
	m_buttonlayout ->addWidget(m_stop);
	m_buttonlayout ->addWidget(m_play);
	m_buttonlayout ->addWidget(m_pause);
    m_buttonlayout ->addWidget(m_nextsong);
    m_buttonlayout ->addWidget(m_repeat);
    m_buttonlayout ->addWidget(m_shuffle);
    m_buttonlayout ->addWidget(m_checkboxSelect);
    m_buttonlayout ->addWidget(m_searchbox);
    m_buttonlayout ->addWidget(m_search);
    //m_buttonlayout ->addWidget(m_albumright);

    m_sliderlayout ->addWidget(m_timeSlider);
    m_sliderlayout ->addWidget(m_timeLabel);

	buttonwidget ->setLayout(m_buttonlayout);
    buttonwidget ->setMaximumHeight(50);
    buttonwidget ->setMaximumWidth(600);
    sliderwidget ->setLayout(m_sliderlayout);
    sliderwidget ->setMaximumHeight(50);
    sliderwidget ->setMinimumWidth(600);

    s_setSliderColor("#009900","#33CC33","#00FF00");
	
    widget->setMaximumHeight(m_squares->height()*1.25);
    m_imagelabel->setMaximumWidth(100);
    m_imagelabel->setMaximumHeight(100);

    QHBoxLayout* staticImageLayout = new QHBoxLayout;
    staticImageLayout->addWidget(m_imagelabel);
    m_imagelabel->resize(100,100);
    m_staticImage->setLayout(staticImageLayout);
    m_staticImage->setMaximumSize(100,100);


    QVBoxLayout* musicWidgetLayout = new QVBoxLayout;
    musicWidgetLayout ->addWidget(buttonwidget,Qt::AlignCenter);
    musicWidgetLayout ->addWidget(sliderwidget,Qt::AlignCenter);
    m_musicWidgets->setLayout(musicWidgetLayout);

    m_tabs = new QTabWidget;
    m_tabs->setMaximumHeight(m_squares->height()*1.25);
    m_tabs->setMovable(true);
    //const QString temp_label = QString("COVERFLOW");
    //m_squares->setLayout()
    QWidget *squares_widget = new QWidget();
    QVBoxLayout *tabs_layout = new QVBoxLayout(squares_widget);
    tabs_layout->addWidget(m_squares);
    tabs_layout->setAlignment(m_squares,Qt::AlignHCenter);

    QWidget *vis_widget = new QWidget();
    QVBoxLayout *vis_layout = new QVBoxLayout(vis_widget);
    vis_layout->addWidget(m_visualizer);
    vis_layout->setAlignment(m_visualizer,Qt::AlignHCenter);


    m_tabs->insertTab(0,squares_widget,"COVERFLOW");
    m_tabs->insertTab(1,widget,"FILTERS");
    m_tabs->insertTab(2,vis_widget,"VISUALIZER");

    m_split1->addWidget(m_staticImage);
    m_split1->addWidget(m_musicWidgets);

    m_split2->addWidget(m_playlistTable);
    m_split2->addWidget(m_table);

    m_mainBox-> addWidget(m_tabs);
    //m_mainBox-> addWidget(buttonwidget);
    //m_mainBox-> setAlignment(buttonwidget, Qt::AlignHCenter);
    //m_mainBox-> addWidget(m_timeSlider);
    //m_mainBox-> setAlignment(m_timeSlider, Qt::AlignHCenter);
	
    m_mainBox->addWidget(m_split1);
    m_mainBox->setAlignment(m_split1, Qt::AlignHCenter);

    m_mainBox-> addWidget(m_split2);
    m_mainBox-> setAlignment(m_split2, Qt::AlignHCenter);

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

	// create separate lists for genres, artists, and albums
	for(int i=0; i<m_listSongs.size(); i++) {
		m_listGenre  << m_listSongs[i][GENRE ];
		// do same for m_listArtist and m_listAlbum using ARTIST and ALBUM constants
		m_listArtist << m_listSongs[i][ARTIST ];
		m_listAlbum << m_listSongs[i][ALBUM ];
	}

	// sort each list
	qStableSort(m_listGenre .begin(), m_listGenre .end(), caseInsensitive);
	qStableSort(m_listArtist.begin(), m_listArtist.end(), caseInsensitive);
	qStableSort(m_listAlbum .begin(), m_listAlbum .end(), caseInsensitive);

	// add each list to list widgets, filtering out repeated strings
        m_panel[0]->addItem("All");
	for(int i=0; i<m_listGenre.size(); i+=m_listGenre.count(m_listGenre[i]))
		m_panel[0]->addItem(m_listGenre [i]);
	// do same for m_listArtist and m_listAlbum to populate m_panel[1] and m_panel[2]
	for(int i=0; i<m_listArtist.size(); i+=m_listArtist.count(m_listArtist[i]))
		m_panel[1]->addItem(m_listArtist [i]);
	for(int i=0; i<m_listAlbum.size(); i+=m_listAlbum.count(m_listAlbum[i]))
		m_panel[2]->addItem(m_listAlbum [i]);

	// copy data to table widget
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

        for(int i = 0; i < m_table->rowCount(); i++) {
              QTableWidgetItem *playlistItem = new QTableWidgetItem;
              QCheckBox* p_checkbox = new QCheckBox;
              QWidget* p_widget = new QWidget;
              p_checkbox->setChecked(true);

              QHBoxLayout* p_layout = new QHBoxLayout(p_widget);
              p_layout->addWidget(p_checkbox);
              p_layout->setAlignment(Qt::AlignCenter);
              p_widget->setLayout(p_layout);

              m_checkboxTable->setCellWidget(i,0,p_widget);

              m_table->item(i,0)->setText("");
              m_table->setCellWidget(i,0,p_widget);

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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::redrawLists:
//
// Re-populate lists with data matching item's text in field x.
//
void
MainWindow::redrawLists(QListWidgetItem *listItem, int x)
{
    if(isSearch == false)
        m_table->setRowCount(0);

	// copy data to table widget
	for(int i=0,row=0; i<m_listSongs.size(); i++) {
		// skip rows whose field doesn't match text
		if(m_listSongs[i][x] != listItem->text()) continue;

		m_table->insertRow(row);
		QTableWidgetItem *item[COLS];
		for(int j=0; j<COLS; j++) {
			item[j] = new QTableWidgetItem;
			item[j]->setText(m_listSongs[i][j]);
			item[j]->setTextAlignment(Qt::AlignCenter);
			// put item[j] into m_table in proper row and column j
			m_table->setItem(row,j,item[j]);
		}

		// increment table row index (row <= i)
		row++;
	}

        for(int i = 0; i < m_table->rowCount(); i++) {

              QTableWidgetItem *playlistItem = new QTableWidgetItem;
              QCheckBox *p_checkbox = new QCheckBox;
              QWidget* p_widget = new QWidget;
              p_checkbox->setChecked(true);

              QHBoxLayout* p_layout = new QHBoxLayout(p_widget);
              p_layout->addWidget(p_checkbox);
              p_layout->setAlignment(Qt::AlignCenter);
              p_widget->setLayout(p_layout);

              m_table->item(i,0)->setText("");
              m_table->setCellWidget(i,0,p_widget);


              m_playlistTable->insertRow(i);
              playlistItem->setText(m_table->item(i,1)->text());
              m_playlistTable->setItem(i,0,playlistItem);

        }
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::traverseDirs:
//
// Traverse all subdirectories and collect filenames into m_listSongs.
//
void
MainWindow::traverseDirs(QString path)
{
	QString		key, val;
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

	m_progressBar->setMaximum(listFiles.size());
	
	QString prev_album = "", current_album = "";
	int albumcount = 0;
	bool newart;
	for(int i=0; i < listFiles.size(); i++) {
		// adjust progress dialog to current settings (optional for now)
		//....

		// init list with default values: ""
		for(int j=0; j<=COLS+1; j++)
			list.insert(j, "N/A");

		// store file pathname into 0th position in list
		QFileInfo fileInfo = listFiles.at(i);
		list.replace(PATH, fileInfo.filePath());

		// convert it from QString to Ascii and store in source
		//set everything to unknown first
		
		// creates variable source of FileRef class
		TagLib::FileRef source(QFile::encodeName(fileInfo.filePath()).constData());
		
		newart = false;
		if(!source.isNull() && source.tag()) {
			// creates a Tag variable in order to read the tags of source
			TagLib::Tag *tag = source.tag();
			// if the field of tag is not an empty string, then it replaces it appropriately
			if(tag->genre() != "") list.replace(GENRE, TStringToQString(tag->genre()));
			if(tag->artist() != "") list.replace(ARTIST, TStringToQString(tag->artist()));
			if(tag->album() != ""){
				list.replace(ALBUM, TStringToQString(tag->album()));
				current_album = TStringToQString(tag->album());
				if(prev_album != current_album){
					albumcount+=1;
					newart = true;
				}
				else newart = false;
				QString tempalbum = QString("%1").arg(albumcount);
				list.replace(ALBUMID,tempalbum);
			}
			prev_album = TStringToQString(tag->album());
			if(tag->title() != "") list.replace(TITLE, TStringToQString(tag->title()));
			// track is not stored as a string, so I convert it to a QString
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
				// the following if statement is necessary to avoid
				// representing the time wrong
				if(seconds < 10)
					temp_time = QString("%1:0%2").arg(minutes).arg(seconds);
				else temp_time = QString("%1:%2").arg(minutes).arg(seconds);
				list.replace(TIME, temp_time);
			}			
		}
		

		// append list (song data) into songlist m_listSongs;
		// uninitialized fields are empty strings
		m_listSongs << list;
		
		/*QByteArray ba_temp = (fileInfo.filePath()).toLocal8Bit();
		const char* t_filepath = ba_temp.constData();*/
		if(newart){
            //TagLib::MPEG::File audioFile(QFile::encodeName(fileInfo.filePath()).constData());
            QByteArray ba_temp = fileInfo.filePath().toLocal8Bit();
            const char* filepath = ba_temp.data();
            TagLib::MPEG::File audioFile(filepath); //Creates a MPEG file from filepath
            TagLib::ID3v2::Tag *tag = new TagLib::ID3v2::Tag();
            tag = audioFile.ID3v2Tag(true); //Creates ID3v2 *Tag to be used in following function
            //TagLib::ID3v2::Tag *tag = audioFile.ID3v2Tag();
			QImage coverArt = imageForTag(tag);
			m_tdResizedArt = coverArt.scaled(250,250,Qt::KeepAspectRatio);
			m_artlist->append(m_tdResizedArt);
		}
	}

	// base case: no more subdirectories
	//if(listDirs.size() == 0) return;
	
	// recursively descend through all subdirectories
	for(int i=0; i<listDirs.size(); i++) {
		QFileInfo fileInfo = listDirs.at(i);
		traverseDirs( fileInfo.filePath() );
	}
	//qDebug("Trying to emit");
	if(listDirs.size() == 0){
		emit s_artLoaded(m_artlist);
		//qDebug("Emitted \n size: %d",m_artlist->size());
		return;
	}
	return;
}		



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::setSizes:
//
// Set splitter sizes.
//
/*void
MainWindow::setSizes(QSplitter *splitter, int size1, int size2)
{
	QList<int> sizes;
	sizes.append(size1);
	sizes.append(size2);
	splitter->setSizes(sizes);
}*/


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_load:
//
// Slot function for File|Load
//
void
MainWindow::s_load()
{
	// open a file dialog box
	QFileDialog *fd = new QFileDialog;

	fd->setFileMode(QFileDialog::Directory);
	QString s = fd->getExistingDirectory(0, "Select Folder", m_directory,
			 QFileDialog::ShowDirsOnly |
			 QFileDialog::DontResolveSymlinks);

	// check if cancel was selected
	if(s == NULL) return;

	// copy full pathname of selected directory into m_directory
	m_directory = s;

	// init progress bar
	m_progressBar = new QProgressDialog(this);
	m_progressBar->setWindowTitle("Updating");
	m_progressBar->setFixedSize(300,100);
	m_progressBar->setCancelButtonText("Cancel");

        m_listSongs.clear();
        m_table->setRowCount(0);
        m_playlistTable->setRowCount(0);
        m_panel[0]->clear();
        m_panel[1]->clear();
        m_panel[2]->clear();
        m_artlist->clear();
        m_mediaplayer->stop();
        traverseDirs(m_directory);
	initLists();
	m_progressBar->close(); 
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_panel1:
//
// Slot function to adjust data if an item in panel1 (genre) is selected.
//
void
MainWindow::s_panel1(QListWidgetItem *item)
{
     if(item->text() == "All") {
        m_panel[0]->clear();
        m_panel[1]->clear();
        m_panel[2]->clear();
        m_table->setRowCount(0);
        initLists();
                return;
        }

	// clear lists
	m_panel[1] ->clear();
	m_panel[2] ->clear();
	m_listArtist.clear();
	m_listAlbum .clear();
	
	// collect list of artists and albums
	for(int i=0; i<m_listSongs.size(); i++) {
		if(m_listSongs[i][GENRE] == item->text())
			m_listArtist << m_listSongs[i][ARTIST];
			m_listAlbum << m_listSongs[i][ALBUM];
	}

	// sort remaining two panels for artists and albums
	qStableSort(m_listArtist.begin(), m_listArtist.end(), caseInsensitive);
	qStableSort(m_listAlbum .begin(), m_listAlbum .end(), caseInsensitive);

	// add items to panels; skip over non-unique entries
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
             <center> Coverflow, populating song table, and general TagLib code by Matthew Liu </center> \
             <center> Playlist, filters, and toolbar buttons by William Gao </center> \
             <center> Visualizer, time/volume slider, playback speed, and tag to qimage code by Kenichi Yamamoto </center>");
}

void MainWindow::s_playbutton(){
    if(m_table->currentItem() == NULL)
        return;

    if(m_playlistTable->currentItem() != NULL)
    {
        int i = 0;
        while(m_table->item(i,1)->text() != m_playlistTable->currentItem()->text() && i < m_table->rowCount())
            i++;
        m_table->setCurrentItem(m_table->item(i,1));
    }

	s_play(m_table->currentItem());
    return;
}

void MainWindow::s_prevsong()
{
    if(m_table->currentItem() == NULL)
        return;

    if(m_playlistTable->rowCount() == 0) {
        QTableWidgetItem *temp = m_table->currentItem();
        if(temp->row() == 0)
            temp = m_table->item(m_table->rowCount()-1,1);
        else temp = m_table->item(temp->row()-1,1);
        m_table->setCurrentItem(temp);
        s_play(m_table->currentItem());
    }
    else
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

void MainWindow::s_nextsong()
{
    if(m_table->currentItem() == NULL)
        return;

    if(m_playlistTable->rowCount() == 0)
    {
        QTableWidgetItem *temp = m_table->currentItem();
        if(temp->row() == m_table->rowCount()-1)
            temp = m_table->item(0,1);
        else temp = m_table->item(temp->row()+1,1);
        m_table->setCurrentItem(temp);
        s_play(m_table->currentItem());
    }
    else
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

void MainWindow::s_pausebutton(){
     //Pauses the song
    m_mediaplayer->pause();
}

void MainWindow::s_setVolume(int Volume){
    //Sets the volume
    m_mediaplayer->setVolume(Volume);
}

void MainWindow::s_setPosition(qint64 Position){
    //Sets the value of the slider to match the position of the currently playing song
    m_timeSlider->setValue(Position);
}

void MainWindow::s_seek(int newPosition){
     //Seeks to a new position in the song specified by newPosition
    qint64 position = (qint64)newPosition;
    m_mediaplayer->setPosition(position);
}

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
    int currentSecond = ((int)m_mediaplayer->position() / 1000)%60;
    int currentMinute = ((int)m_mediaplayer->position() / 1000)/60;
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

void MainWindow::s_setDuration(qint64 Duration){
    //Sets the duration of the time slider
    m_timeSlider->setRange(0,Duration);
    return;
}

void MainWindow::s_searchSongs(){
    QList<QString> m_listFound;
    m_listFound.clear();
    QString inputstr = m_searchbox->text();

    for(int i=0; i<m_listSongs.size(); i++){
        if(m_listSongs[i][TITLE].contains(inputstr, Qt::CaseInsensitive)){
            m_listFound.append(m_listSongs[i][TITLE]);
        }
    }

    QListWidget foundsongs;
    foundsongs.addItems(m_listFound);
    isSearch = true;

    //redrawSearched(foundsongs, TITLE);
    m_table->setRowCount(0);
    for(int i = foundsongs.count() - 1; i >= 0; i--){
        qDebug() << m_listFound[i];
        redrawLists(foundsongs.item(i), TITLE);
    }
    isSearch = false;
}

void MainWindow::s_toggleNightMode(){

if(m_nightmodeAction->isChecked()){
m_tabs->setStyleSheet("background: black; \
                      ");
this->setStyleSheet("MainWindow{background-color: black; \
                    color: white;} \
                    QMenu{background: #444444; \
                    color: white;} \
                    QMenuBar{background: #444444; \
                    color: white;} \
                    QMenuBar::item{background-color: #444444;} \
                    QMenuBar::item:selected{background-color: #666666;} \
                    QScrollBar:vertical{background-color: black;} \
                    QTabBar::tab{background-color: #444444; \
                    color: white;} \
                    QTabBar::tab:selected{background-color: #666666;} \
                    QTabWidget::pane{border: 2px solid #444444} \
                    QSplitter::handle{background-color: black;} \
                    QLineEdit{background-color: black; \
                    color: white; \
                    border: 2px solid #444444;} \
                    QTableView{background-color: #333333; \
                    alternate-background-color: #555555; \
                    color: white;} \
                    QTableCornerButton::section{background-color: #444444; \
                    border: 2px;} \
                    QHeaderView::section{background-color: #444444; \
                    color: white;} \
                    ");
}
else{
m_tabs->setStyleSheet("");
this->setStyleSheet("");
}
}

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
}

void MainWindow::s_setSliderColor(QString a, QString b, QString c){

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
// MainWindow::taglibAlbumArt
//
// Obtain the cover art for a given track using taglib.
//

QImage MainWindow::imageForTag(TagLib::ID3v2::Tag *tag)
{
    QImage image;
    //Creates a framelist from the given tag using "APIC",
    //Which stands for "attached picture"
    const TagLib::ID3v2::FrameList list = tag->frameList("APIC");


     //If the list is empty, then load a default art image from QResource
    if(list.isEmpty()) {
        image.load(":/Resources/Default.png");
        return image;
    }

    //It isn't empty, so get the attached picture frame from the framelist
    //front() specifies the cover art of the song
    TagLib::ID3v2::AttachedPictureFrame *frame =
        static_cast<TagLib::ID3v2::AttachedPictureFrame *>(list.front());

        //Load the picture from the song's frame into the Qimage, and return it
    image.loadFromData((const uchar *) frame->picture().data(), frame->picture().size());

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
    if(item == NULL)
        return;
    if(m_mediaplayer->state() == 2){
        m_mediaplayer->play();
        return;
    }
    item = m_table->item(item->row(),1);

    for(int i=0; i<m_listSongs.size(); i++) {
        // skip over songs whose title does not match
        if(m_listSongs[i][TITLE] != item->text()) continue;
        QString temp_title = QString("%1").arg(m_listSongs[i][PATH]);
        m_mediaplayer->setMedia(QUrl::fromLocalFile(temp_title));
        //Play the song
        m_mediaplayer->play();

        // The following two lines turns temp_title from a QString to a const char* in order
                // to input the path into TagLib;:MPEG::File

        QByteArray ba_temp = temp_title.toLocal8Bit();
        const char* filepath = ba_temp.data();
        TagLib::MPEG::File audioFile(filepath); //Creates a MPEG file from filepath
        TagLib::ID3v2::Tag *tag = audioFile.ID3v2Tag(true); //Creates ID3v2 *Tag to be used in following function
        QImage coverArt = imageForTag(tag);
		
        //Resizes coverArt to a fixed size (250x250) while keeping its aspect ratio
        m_resizedArt = coverArt.scaled(250,250,Qt::KeepAspectRatio);
        m_imagelabel->setScaledContents(true);
        //Puts the resizd image into the image label
        m_imagelabel->setPixmap(QPixmap::fromImage(m_resizedArt));
    }

    if(m_stop->isDown()){
			m_mediaplayer->stop();
        }
        return;
}
void MainWindow::statusChanged(QMediaPlayer::MediaStatus status)
{
    if(status == QMediaPlayer::BufferedMedia){
    qDebug("Media is buffered");
    m_timeSlider->setEnabled(true);
    m_volumeSlider->setEnabled(true);
    m_volumeSlider->setValue(80);
    m_play->setEnabled(true);
    m_stop->setEnabled(true);
    m_prevsong->setEnabled(true);
    m_nextsong->setEnabled(true);
    m_pause->setEnabled(true);
    //autocomplete(m_listSongs);
    //Sets the window title to be the in the following format:
    //<Artist> - <Title>
    QString QTunes = " - ";
    QTunes.prepend(m_table->item(m_table->currentRow(),4)->text());
    QTunes.append(m_table->item(m_table->currentRow(),1)->text());
    setWindowTitle(QTunes);
    }

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

    if(status == QMediaPlayer::EndOfMedia && m_playlistTable->currentRow() < m_playlistTable->rowCount()-1)
    {
        int i = 0;

        if(m_playlistTable->currentItem() == NULL)
            m_playlistTable->setCurrentCell(0,0);

        QTableWidgetItem* nextItem = new QTableWidgetItem;
        nextItem = m_playlistTable->item(m_playlistTable->currentRow()+1,0);
        while(m_table->item(i,1)->text() != nextItem->text() && i < m_table->rowCount())
            i++;
        m_table->setCurrentItem(m_table->item(i,1));
        m_playlistTable->setCurrentItem(nextItem);
        s_play(m_table->currentItem());
    }
}

void MainWindow::s_repeat()
{
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

void MainWindow::s_shuffle()
{
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

void MainWindow::s_checkboxSelect()
{
    if(m_checkboxSelect->isChecked())
    {
        m_checkboxSelect->setText("Deselect All");
        int currentPosition = m_table->currentRow();
        for(int i = 0; i < m_table->rowCount(); i++)
        {
            m_table->setCurrentCell(i,0);
            playlistUpdate();
        }
        m_table->setCurrentCell(currentPosition,1);
        m_repeat->setEnabled(true);
        m_shuffle->setEnabled(true);
    }
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

void MainWindow::playlistUpdate()
{
    QTableWidgetItem *playlistItem = new QTableWidgetItem;

    if(m_table->currentColumn() != 0) return;

    m_checkbox = m_table->cellWidget(m_table->currentRow(),0)->findChild<QCheckBox *>();
    m_checkbox->toggle();

     if(m_checkbox->isChecked() == true)
     {
             m_playlistTable->insertRow(m_playlistTable->rowCount());
             m_playlistTable->setItem(m_playlistTable->rowCount()-1,0,playlistItem);
             playlistItem->setText(m_table->item(m_table->currentRow(),1)->text());
     }
     else
     {
         for(int i = 0; i < m_playlistTable->rowCount(); i++)
         {
             if(m_table->item(m_table->currentRow(),1)->text() == m_playlistTable->item(i,0)->text())
             {
                 m_playlistTable->removeRow(i);
             }
         }
     }
}

