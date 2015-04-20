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
#include <QIcon>
#include <Qsize>
#include <QMediaPlaylist>
#include <QCheckBox>
using namespace std;
using namespace TagLib; 

enum { PLAYLIST, TITLE, TRACK, TIME, ARTIST, ALBUM, GENRE, PATH};
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

	// set main window titlebar
	QString copyright = "Copyright (C) 2015 by George Wolberg";
	QString version	  = "Version 1.0";
	QString title	  =  QString("%1   (%2)         %3")
			    .arg(program).arg(version).arg(copyright);
	setWindowTitle(title);

	// set central widget and default size
	setCentralWidget(m_mainWidget);
	setMinimumSize(400, 300);
    resize(1100, 830);
	m_artlist = new QList<QImage>;
	connect(m_stop, SIGNAL(clicked()), m_mediaplayer, SLOT(stop()));
	connect(m_play, SIGNAL(clicked()), this, SLOT(s_playbutton()));	
	connect(m_pause, SIGNAL(clicked()), this, SLOT(s_pausebutton()));
	connect(m_nextsong, SIGNAL(clicked()), this, SLOT(s_nextsong()));
	connect(m_prevsong, SIGNAL(clicked()), this, SLOT(s_prevsong()));
    connect(m_repeat, SIGNAL(toggled(bool)), this, SLOT(shuffle_off()));
    connect(m_shuffle, SIGNAL(toggled(bool)), this, SLOT(repeat_off()));
	connect(m_mediaplayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));
    connect(m_volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(s_setVolume(int)));
	connect(m_albumleft, SIGNAL(clicked()), m_squares, SLOT(s_shiftleft()));
	connect(m_albumright, SIGNAL(clicked()), m_squares, SLOT(s_shiftright()));
	connect(m_loadart, SIGNAL(clicked()), m_squares, SLOT(s_loadart()));
	connect(m_mediaplayer, SIGNAL(positionChanged(qint64)), this, SLOT(s_setPosition(qint64))); 
	connect(m_mediaplayer, SIGNAL(positionChanged(qint64)), this, SLOT(s_updateLabel(qint64))); 
	connect(m_timeSlider, SIGNAL(sliderMoved(int)), this, SLOT(s_seek(int))); 
	connect(this, SIGNAL(s_artLoaded(QList<QImage>*)), m_squares,SLOT(s_mp3art(QList<QImage>*)));
}



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::~MainWindow:
//
// Destructor. Save settings.
//
MainWindow::~MainWindow() {}



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

    m_playlistSplitter = new QWidget;
    m_tableSplitter = new QWidget;
    m_mergeSplitter = new QWidget;
	
	m_squares = new SquaresWidget;
	// initialize splitters

    m_leftSplit= new QSplitter(Qt::Vertical, m_playlistSplitter);
    m_playlistSplitter->setMinimumSize(220,500);

    m_rightSplit = new QSplitter(Qt::Vertical, m_tableSplitter);
    m_tableSplitter->setMinimumSize(810,10);

    m_mergeSplit = new QSplitter(Qt::Horizontal,m_mergeSplitter);
    m_mergeSplitter->setMinimumSize(1100,800);

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

	// initialize table widget: complete song data
    m_playlistTable = new QTableWidget(0, 1);
    QHeaderView *playlist_header = new QHeaderView(Qt::Horizontal,m_playlistTable);
    playlist_header->setSectionResizeMode(QHeaderView::Stretch);
    m_playlistTable->setHorizontalHeader(playlist_header);
    m_playlistTable->setHorizontalHeaderLabels(QStringList() << "Playlist");
    m_playlistTable->setAlternatingRowColors(true);
    m_playlistTable->setShowGrid(true);
    m_playlistTable->setEditTriggers (QAbstractItemView::NoEditTriggers);
    m_playlistTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_table = new QTableWidget(0, COLS);
    QHeaderView *table_header= new QHeaderView(Qt::Horizontal,m_table);
    table_header->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setHorizontalHeader(table_header);
    m_table->setHorizontalHeaderLabels(QStringList() << "Playlist" <<
		"Name" << "Track" << "Time" << "Artist" << "Album" << "Genre");

    m_table->setAlternatingRowColors(1);
    m_table->setShowGrid(1);
    m_table->setEditTriggers (QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);

	// init signal/slot connections
	connect(m_panel[0],	SIGNAL(itemClicked(QListWidgetItem*)),
		this,		  SLOT(s_panel1   (QListWidgetItem*)));
        connect(m_panel[1],	SIGNAL(itemClicked(QListWidgetItem*)),
		this,		  SLOT(s_panel2   (QListWidgetItem*)));
        connect(m_panel[2],	SIGNAL(itemClicked(QListWidgetItem*)),
		this,		  SLOT(s_panel3   (QListWidgetItem*)));
        connect(m_table,	SIGNAL(itemDoubleClicked(QTableWidgetItem*)),
		this,		  SLOT(s_play	  (QTableWidgetItem*)));
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
	m_buttonlayout = new QHBoxLayout;
	//m_play = new QPushButton("Play");
	m_play = new QToolButton;
	m_stop = new QToolButton;
	m_prevsong = new QToolButton;
	m_nextsong = new QToolButton;
	m_pause = new QToolButton;
    m_repeat = new QToolButton;
    m_repeat->setCheckable(true);
    m_repeat->setChecked(false);
    m_shuffle = new QToolButton;
    m_shuffle->setCheckable(true);
    m_shuffle->setChecked(false);
    m_play->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
	m_stop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
	m_prevsong->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
	m_nextsong->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
	m_pause->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    m_repeat->setIcon(QIcon(":/Resources/Repeat.png"));
    m_repeat->setIconSize(QSize(16.5,16.5));
    m_shuffle->setIcon(QIcon(":/Resources/Shuffle.png"));
    m_shuffle->setIconSize(QSize(16.5,16.5));

	m_albumleft = new QToolButton;
	m_albumright = new QToolButton;
	m_albumleft->setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
	m_albumright->setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
	
    m_playlist = new QMediaPlaylist;

	m_loadart = new QToolButton;
	m_loadart->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
	
	//m_resizedArt = new QImage;
	m_imagelabel = new QLabel; 
	m_imagelabel ->setText("No Art");
	//m_pause = new QPushButton("Pause");
	//m_stop = new QPushButton("Stop");
	//m_nextsong = new QPushButton("Next");
    m_volumeSlider = new QSlider(Qt::Horizontal, buttonwidget);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setSliderPosition(80);
	//m_volumeSlider->setMaximumWidth(100);
	m_timeLabel = new QLabel;
    m_timeLabel ->setText("<b>00:00 / 00:00</b>");
    m_timeSlider = new QSlider(Qt::Horizontal);
    m_timeSlider ->setParent(this);
    m_timeSlider ->setRange(0, 0);
	//m_volumeSlider->setMaximumWidth(50);
	m_buttonlayout ->addWidget(m_albumleft);
	m_buttonlayout ->addWidget(m_volumeSlider);
	m_buttonlayout ->addWidget(m_loadart);
	m_buttonlayout ->addWidget(m_prevsong);
	m_buttonlayout ->addWidget(m_stop);
	m_buttonlayout ->addWidget(m_play);
	m_buttonlayout ->addWidget(m_pause);
	m_buttonlayout ->addWidget(m_nextsong);
    m_buttonlayout ->addWidget(m_repeat);
    m_buttonlayout ->addWidget(m_shuffle);
	m_buttonlayout ->addWidget(m_timeLabel); 
	m_buttonlayout ->addWidget(m_albumright);

    buttonwidget ->setLayout(m_buttonlayout);
    buttonwidget ->setMaximumHeight(50);
	buttonwidget ->setMaximumWidth(500);
    m_timeSlider->setMinimumWidth(380);
    m_timeSlider->setStyleSheet("QSlider::groove:horizontal  {border: 4px solid #999999;"
                                "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #BABABA, stop:1 #FFFFFF);}"
                                "QSlider::handle:horizontal  {background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #999999, stop:1 #CCCCCC);"
                                "width: 10px;"
                                "height: 20px;"
                                "margin-top: -6px;"
                                "margin-bottom: -6px;}"
                                "QSlider::add-page:horizontal {background: white;"
                                "border: 1px solid #999999}"
                                "QSlider::sub-page:horizontal  {background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #66FF33, stop:1 #009933);}");

    m_playlistTable->setMaximumWidth(220);
    m_leftSplit->addWidget(m_playlistTable);
    m_leftSplit->resize(200,420);

    widget->setMaximumHeight(180);
    m_rightSplit->addWidget(widget);

    widget->setMaximumHeight(210);
    m_rightSplit->addWidget(m_table);

    m_rightSplit->resize(800,420);

    m_imagelabel ->setMaximumWidth(500);
    m_imagelabel ->setMaximumHeight(500);

	m_mainBox-> addWidget(m_squares);
    m_mainBox-> setAlignment(m_squares, Qt::AlignHCenter);
	m_mainBox-> addWidget(buttonwidget);
    m_mainBox-> setAlignment(buttonwidget, Qt::AlignHCenter);
    m_mainBox-> addWidget(m_timeSlider);
    m_mainBox-> setAlignment(m_timeSlider, Qt::AlignHCenter);
    /*m_mainBox-> addWidget(m_imagelabel);
    m_mainBox-> setAlignment(m_imagelabel, Qt::AlignHCenter);*/

    m_mergeSplit-> addWidget(m_playlistSplitter);
    m_mergeSplit-> addWidget(m_tableSplitter);
    m_mainBox-> addWidget(m_mergeSplit);
    m_mainBox-> setAlignment(m_mergeSplit, Qt::AlignHCenter);
    m_mainWidget ->setLayout(m_mainBox);
	
	//setSizes(m_rightSplit,(int)(300*.30), (int)(300*.70));
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
        for(int j=1; j<COLS; j++) {
			item[j] = new QTableWidgetItem;
			item[j]->setText(m_listSongs[i][j]);
			item[j]->setTextAlignment(Qt::AlignCenter);
            m_table->setItem(i, j, item[j]);
		}
    }

    m_checkboxList = new QListWidget;
    m_checkbox = new QCheckBox;

    for(int i = 0; i < m_table->rowCount(); i++) {
        QCheckBox* p_checkbox = new QCheckBox;
        QListWidgetItem* temp = new QListWidgetItem;
        m_checkboxList->addItem(temp);

        QWidget* p_widget = new QWidget;
        QHBoxLayout* p_layout = new QHBoxLayout(p_widget);
        p_layout->addWidget(p_checkbox);
        p_layout->setAlignment(Qt::AlignCenter);
        p_widget->setLayout(p_layout);

        m_checkboxList->setItemWidget(temp,p_widget);
        //m_checkboxList->setCurrentItem(temp);
        m_table->setCellWidget(i,0,m_checkboxList->itemWidget(temp));

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
            m_table->setItem(row,j+1,item[j]);
		}

		// increment table row index (row <= i)
		row++;
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

    //Initialize Playlist Column
    list.insert(0,"");

	for(int i=0; i < listFiles.size(); i++) {
		// adjust progress dialog to current settings (optional for now)
        //....
		// init list with default values: ""
        for(int j=1; j<=COLS; j++)
            list.insert(j, "N/A");

		// store file pathname into 0th position in list
		QFileInfo fileInfo = listFiles.at(i);
		list.replace(PATH, fileInfo.filePath());

		// convert it from QString to Ascii and store in source
		//set everything to unknown first
		
		// creates variable source of FileRef class
		TagLib::FileRef source(QFile::encodeName(fileInfo.filePath()).constData());
		
		if(!source.isNull() && source.tag()) {
			// creates a Tag variable in order to read the tags of source
			TagLib::Tag *tag = source.tag();
			// if the field of tag is not an empty string, then it replaces it appropriately
			if(tag->genre() != "") list.replace(GENRE, TStringToQString(tag->genre()));
			if(tag->artist() != "") list.replace(ARTIST, TStringToQString(tag->artist()));
			if(tag->album() != "") list.replace(ALBUM, TStringToQString(tag->album()));
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

                QString temp_path = QString("%1").arg(m_listSongs[i][PATH]);

                QByteArray ba_temp = temp_path.toLocal8Bit();
                const char* filepath = ba_temp.data();

                qDebug("Trying to load audiofile");
                TagLib::MPEG::File audioFile(QFile::encodeName(fileInfo.filePath()).constData());
                //qDebug() << fileInfo.filePath();
                //TagLib::MPEG::File audioFile(filepath);
                qDebug("audiofile loaded");
                TagLib::ID3v2::Tag *tag = new TagLib::ID3v2::Tag();
                qDebug("Testing of tag:");
                //QString test = QString("title:%1\nartist:%2").arg(tag->title()).arg(tag->artist());
                tag = audioFile.ID3v2Tag(true);
                qDebug() << "title:" << TStringToQString(tag->title());
                qDebug("tag converted");
                QImage coverArt = imageForTag(tag);
                qDebug("imageForTag executed");
                m_tdResizedArt = coverArt.scaled(250,250,Qt::KeepAspectRatio);
                m_artlist->append(m_tdResizedArt);

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
        s_artLoaded(m_artlist);
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
void
MainWindow::setSizes(QSplitter *splitter, int size1, int size2)
{
	QList<int> sizes;
	sizes.append(size1);
	sizes.append(size2);
	splitter->setSizes(sizes);
}


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
	if(item->text() == "ALL") {
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
	QMessageBox::about(this, "About qTunes",
			"<center> qTunes 1.0 </center> \
			 <center> by George Wolberg, 2015 </center>");
}

void MainWindow::s_playbutton(){
	s_play(m_table->currentItem());
}
void MainWindow::s_prevsong()
{
    if(m_table->currentItem() == NULL)
        return;
    QTableWidgetItem *temp = m_table->currentItem();
    if(temp->row() == 0)
        temp = m_table->item(m_table->rowCount()-1,0);
    else temp = m_table->item(temp->row()-1,0);
    m_table->setCurrentItem(temp);
    s_play(m_table->currentItem());
}
void MainWindow::s_nextsong(){
    if(m_table->currentItem() == NULL)
        return;
	QTableWidgetItem *temp = m_table->currentItem();
	if(temp->row() == m_table->rowCount()-1)
		temp = m_table->item(0,0);
	else temp = m_table->item(temp->row()+1,0);
	m_table->setCurrentItem(temp);
	s_play(m_table->currentItem());
}

void MainWindow::s_pausebutton(){
    m_mediaplayer->pause();
}

void MainWindow::s_setVolume(int Volume){
    m_mediaplayer->setVolume(Volume);
}

void MainWindow::s_setPosition(qint64 Position){
    m_timeSlider->setValue(Position);
}

void MainWindow::s_seek(int newPosition){
    qint64 position = (qint64)newPosition;
    m_mediaplayer->setPosition(position);
}

void MainWindow::s_updateLabel(qint64 Time){
    int endSecond = ((int)m_mediaplayer->duration() / 1000)%60;
    int endMinute = ((int)m_mediaplayer->duration() / 1000)/60;
    QString endTime;
    if(endSecond < 10)
        endTime = QString("%1:0%2").arg(endMinute).arg(endSecond);
    else endTime = QString("%1:%2").arg(endMinute).arg(endSecond);

    int currentSecond = ((int)m_mediaplayer->position() / 1000)%60;
    int currentMinute = ((int)m_mediaplayer->position() / 1000)/60;
    QString currentTime;
    if(currentSecond < 10)
        currentTime = QString("%1:0%2").arg(currentMinute).arg(currentSecond);
    else currentTime = QString("%1:%2").arg(currentMinute).arg(currentSecond);
    QString timeLabel = " / ";
    timeLabel.prepend(currentTime);
    timeLabel.prepend("<b>");
    timeLabel.append(endTime);
    timeLabel.append("</b>");
    m_timeLabel->setText(timeLabel);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::taglibAlbumArt
//
// Obtain the cover art for a given track using taglib.
//

QImage MainWindow::imageForTag(TagLib::ID3v2::Tag *tag)
{
    TagLib::ID3v2::FrameList list = tag->frameList("APIC");

    QImage image;
	//if(true){
    if(list.isEmpty()){
        //qDebug("No image found");
        image.load(":/Resources/Default_Music.ico");
        return image;
	}
    TagLib::ID3v2::AttachedPictureFrame *frame =
        static_cast<TagLib::ID3v2::AttachedPictureFrame *>(list.front());

    image.loadFromData((const uchar *) frame->picture().data(), frame->picture().size());

    return image;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_play:
//
// Slot function to play an mp3 song.
// This uses audiere. Replace with functions from Qt5 multimedia module
//

void
MainWindow::s_play(QTableWidgetItem *item)
{
    if(item == NULL)
        return;
    if(m_mediaplayer->state() == 2){
        qDebug("Resuming from paused state \n");
        m_mediaplayer->play();
        return;
    }
    item = m_table->item(item->row(),0);

    m_playlistTable->setRowCount(0);

    uint playlist_capacity;
    if(m_table->rowCount() < 5)
        playlist_capacity = m_table->rowCount();
    else
        playlist_capacity = 5;

    QTableWidgetItem* playlist_item[playlist_capacity]; // Playlist shows at most 5 songs at a time

    while(m_playlistTable->rowCount() + m_table->currentRow() < playlist_capacity) {
        playlist_item[m_playlistTable->rowCount()] = new QTableWidgetItem;
        playlist_item[m_playlistTable->rowCount()]->setText(m_table->item(m_playlistTable->rowCount() + m_table->currentRow(),1)->text());
        m_playlistTable->insertRow(m_playlistTable->rowCount());
        m_playlistTable->setItem(m_playlistTable->rowCount()-1,0,playlist_item[m_playlistTable->rowCount()-1]);
    }

    /*QTextStream out(stdout);
    out << QString("s_play1\n");
    for(int i=0; i<m_listSongs.size(); i++) {
        //skip over songs whose title does not match
       if(m_listSongs[i][TITLE] != item->text()) continue;
       QString temp_title = QString("%1").arg(m_listSongs[i][PATH]);
       m_mediaplayer->setMedia(QUrl::fromLocalFile(temp_title));
    }*/

     qDebug() << m_checkboxList->item(m_checkboxList->currentRow()+1)->isSelected();
     qDebug() << m_checkboxList->count();
        for(int i=0; i<m_listSongs.size(); i++) {

        // Setup Playlist with links
        QString temp_title = QString("%1").arg(m_listSongs[i][PATH]);
        m_playlist->addMedia(QUrl::fromLocalFile(temp_title));

        // The following two lines turns temp_title from a QString to a const char* in order
                // to input the path into TagLib;:MPEG::File

        QByteArray ba_temp = temp_title.toLocal8Bit();
        const char* filepath = ba_temp.data();

        TagLib::MPEG::File audioFile(filepath); //Creates a MPEG file from filepath
        TagLib::ID3v2::Tag *tag = audioFile.ID3v2Tag(true); //Creates ID3v2 *Tag to be used in following function
        QImage coverArt = imageForTag(tag);
		
        m_resizedArt = coverArt.scaled(250,250,Qt::KeepAspectRatio);
        m_imagelabel->setScaledContents(true);
        m_imagelabel->setPixmap(QPixmap::fromImage(m_resizedArt));
        }

        m_mediaplayer->setPlaylist(m_playlist);
        m_playlist->setCurrentIndex(m_table->currentRow());

        m_mediaplayer->play();
		qDebug("Trying to play \n");
		if(m_stop->isDown()){
            qDebug("Trying to stop");
			m_mediaplayer->stop();
		}
		else qDebug("Not stopped");
		return;
	}
//}
void MainWindow::statusChanged(QMediaPlayer::MediaStatus status)
{
	if(status == QMediaPlayer::BufferedMedia){
        m_timeSlider->setRange(0,m_mediaplayer->duration());
        qDebug("Media is buffered");
    }

    if(status == QMediaPlayer::EndOfMedia && m_repeat->isChecked() == true)
        m_mediaplayer->play();

    if(status == QMediaPlayer::EndOfMedia && m_shuffle->isChecked() == true){
        QTableWidgetItem *currentsong = m_table->currentItem();
        QTableWidgetItem *nextsong = m_table->currentItem();
        int list_length = m_table->rowCount();
        while(nextsong == currentsong)
            nextsong = m_table->item(rand() % list_length,0);
        m_table->setCurrentItem(nextsong);
        s_play(m_table->currentItem());
    }
}

void MainWindow::repeat_off()
{
    if(m_shuffle->isChecked() == true)
        m_repeat->setChecked(false);
}

void MainWindow::shuffle_off()
{
    if(m_repeat->isChecked() == true)
        m_shuffle->setChecked(false);
}
