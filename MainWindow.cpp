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
#include "MainWindow.h"
#include <fileref.h>
#include <tag.h>
#include <tpropertymap.h>
#include <QMediaPlayer>
#include <QtMultimedia>
#include "qmediaplayer.h"
#include <iostream>
using namespace std;

enum {TITLE, TRACK, TIME, ARTIST, ALBUM, GENRE, PATH};
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
	createButtons(); 
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
	setCentralWidget(m_mainSplit);
	setMinimumSize(400, 300);
	resize(900, 600);
	connect(m_stop, SIGNAL(clicked()),
		m_mediaplayer, SLOT(stop()));
	/*connect(m_play, SIGNAL(clicked()),
		this, SLOT(s_play(m_table->currentItem())));*/
	connect(m_play, SIGNAL(clicked()),
		this, SLOT(s_playbutton()));	
    connect(m_pause, SIGNAL(clicked()),
        this, SLOT(s_pausebutton()));
	connect(m_nextsong, SIGNAL(clicked()),
		this, SLOT(s_nextsong()));
	connect(m_mediaplayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
            this, SLOT(statusChanged(QMediaPlayer::MediaStatus)));
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
	// initialize splitters
	m_mainSplit  = new QSplitter(this);
	m_leftSplit  = new QSplitter(Qt::Vertical, m_mainSplit);
	m_rightSplit = new QSplitter(Qt::Vertical, m_mainSplit);

	// init labels on left side of main splitter
	for(int i=0; i<2; i++) {
		m_labelSide[i] = new QLabel(QString("Label%1").arg(i));
		m_labelSide[i]->setAlignment(Qt::AlignCenter);
	}

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
	m_table = new QTableWidget(0, COLS);
	QHeaderView *header = new QHeaderView(Qt::Horizontal,m_table);
	//header->setResizeMode(QHeaderView::Stretch);
	m_table->setHorizontalHeader(header);
	m_table->setHorizontalHeaderLabels(QStringList() <<
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

void MainWindow::createButtons(){
	/*m_stop = new QPushButton("Stop");
	m_play = new QPushButton("Play");
	/*connect(m_play, SIGNAL(clicked()),
		this, SLOT(s_play(m_table->currentItem())));
	connect(m_stop, SIGNAL(clicked()),
		m_mediaplayer, SLOT(stop()));
	m_buttonlayout = new QHBoxLayout;
	m_buttonlayout->addWidget(m_play);
	m_buttonlayout->addWidget(m_stop);*/
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
	QWidget *buttonwidget = new QWidget(m_labelSide[0]);
	m_buttonlayout = new QHBoxLayout;
	m_play = new QPushButton("Play");
    m_pause = new QPushButton("Pause");
	m_stop = new QPushButton("Stop");
	m_nextsong = new QPushButton("Next");
	m_buttonlayout ->addWidget(m_play);
    m_buttonlayout ->addWidget(m_pause);
	m_buttonlayout ->addWidget(m_stop);
	m_buttonlayout ->addWidget(m_nextsong);
	buttonwidget ->setLayout(m_buttonlayout);
	//m_play = new QPushButton("Play", m_stop);
	//QWidget *buttonwidget = new QWidget;
	//buttonwidget->setLayout(m_buttonlayout);
	//m_leftSplit ->addWidget(buttonwidget);
	m_leftSplit ->addWidget(m_labelSide[0]);
	m_leftSplit ->addWidget(m_labelSide[1]);
	m_rightSplit->addWidget(widget );
	m_rightSplit->addWidget(m_table);

	// set main splitter sizes
	setSizes(m_mainSplit, (int)(width ()*.25), (int)(width ()*.75));
	setSizes(m_leftSplit, (int)(height()*.5), (int)(height()*.5));
	setSizes(m_rightSplit,(int)(height()*.4), (int)(height()*.6));
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
		for(int j=0; j<COLS; j++) {
			item[j] = new QTableWidgetItem;
			item[j]->setText(m_listSongs[i][j]);
			item[j]->setTextAlignment(Qt::AlignCenter);
			m_table->setItem(i, j, item[j]);
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
	
	for(int i=0; i < listFiles.size(); i++) {
		// adjust progress dialog to current settings (optional for now)
		//....

		// init list with default values: ""
		for(int j=0; j<=COLS; j++)
			list.insert(j, "");

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
	}

	// base case: no more subdirectories
	if(listDirs.size() == 0) return;
	
	// recursively descend through all subdirectories
	for(int i=0; i<listDirs.size(); i++) {
		QFileInfo fileInfo = listDirs.at(i);
		traverseDirs( fileInfo.filePath() );
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
void MainWindow::s_nextsong(){
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

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// MainWindow::s_play:
//
// Slot function to play an mp3 song.
// This uses audiere. Replace with functions from Qt5 multimedia module
//
void
MainWindow::s_play(QTableWidgetItem *item)
{
    if(m_mediaplayer->state() == 2){
        qDebug("Resuming from paused state \n");
        m_mediaplayer->play();
        return;
    }
	item = m_table->item(item->row(),0);
	QTextStream out(stdout);
	out << QString("s_play1\n");
	//if (mediaplayer->error() != 0) return;
	//QString temp_title = QString("%1").arg(m_listSongs[i][PATH]);
	for(int i=0; i<m_listSongs.size(); i++) {
		// skip over songs whose title does not match
		if(m_listSongs[i][TITLE] != item->text()) continue;
		QString temp_title = QString("%1").arg(m_listSongs[i][PATH]);
		m_mediaplayer->setMedia(QUrl::fromLocalFile(temp_title));
		m_mediaplayer->setVolume(100);
		m_mediaplayer->play();
		qDebug("Trying to play \n");
		if(m_stop->isDown()){
            qDebug("Trying to stop");
			m_mediaplayer->stop();
		}
		else qDebug("Not stopped");
		return;
	}
}
void MainWindow::statusChanged(QMediaPlayer::MediaStatus status)
{
	if(status == QMediaPlayer::LoadedMedia){
		qDebug("Media is loaded");
		/*mediaplayer->setVolume(100);
		mediaplayer->play();
		cout << "status changed \n" ;*/
	}
}
