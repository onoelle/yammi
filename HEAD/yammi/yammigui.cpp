/***************************************************************************
                          yammigui.cpp  -  description
                             -------------------
    begin                : Tue Feb 27 2001
    copyright            : (C) 2001 by Brian O.N�lle
    email                : oli.noelle@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "yammigui.h"

using namespace std;

// include pixmaps

// general
#include "icons/yammiicon.xpm"
#include "icons/in.xpm"
#include "icons/notin.xpm"
#include "icons/some_in.xpm"
#include "icons/newCategory.xpm"

// media player actions
// now using the icons from multimedia chrome from mediabuilder.com:
// http://mediabuilder.com/ds_icons_multimedia_chrome_page_aa.html
#include "icons/play.xpm"
#include "icons/pause.xpm"
#include "icons/stop.xpm"
#include "icons/skipforward.xpm"
#include "icons/skipbackward.xpm"

// song actions
#include "icons/defaultDoubleClick.xpm"
#include "icons/defaultMiddleClick.xpm"
#include "icons/defaultControlClick.xpm"
#include "icons/defaultShiftClick.xpm"
#include "icons/prelisten.xpm"
#include "icons/playnow.xpm"
#include "icons/enqueue.xpm"
#include "icons/enqueueasnext.xpm"
#include "icons/dequeueSong.xpm"
#include "icons/dequeueAll.xpm"
#include "icons/songinfo.xpm"
#include "icons/stopPrelisten.xpm"
#include "icons/toFromPlaylist.xpm"


// dialog includes
#include "preferencesdialog.h"
#include "DeleteDialog.h"
#include "updatedatabasedialog.h"
#include "updatedatabasemediadialog.h"
#include "ConsistencyCheckDialog.h"
#include "ApplyToAllBase.h"

#include "mp3info/CMP3Info.h"
#include "ConsistencyCheckParameter.h"

// kde includes
//#ifdef ENABLE_NOATUN
//#include "kfiledialog.h"
//#endif

extern YammiGui* gYammiGui;


/**
 * Constructor, sets up the whole application.
 * (maybe not nice??)
 */
YammiGui::YammiGui(QString baseDir)
    : QMainWindow()
{
	gYammiGui=this;
  this->setIcon(QPixmap(yammiicon_xpm));
	
  // set up model
	model=new YammiModel();
	cout << "starting Yammi, version " << model->config.yammiVersion << "\n";

  model->readPreferences(baseDir);						// read preferences

  // set up media player
  //********************
  player=0;
  currentSong=0;
  chosenFolder=0;
#ifdef ENABLE_XMMS
  if(model->config.player==0) {
    cout << "media player: XMMS\n";
    cout << "     (if nothing happens after this line, you probably have to remove the xmms lock file (/tmp/xmms_<user>.0) and try again.\n";
    player = new XmmsPlayer(0, model);         // use xmms as media player (session 0)
  }
#endif

#ifdef ENABLE_NOATUN
  if(model->config.player==1) {
    cout << "media player: Noatun\n";
    player = new NoatunPlayer(model);         // use noatun as media player    
  }
#endif
  if(player==0) {
    cout << "WARNING: no media player support compiled in, only limited functionality available!\n";
    player = new DummyPlayer();
  }
  else {
    // connect player and yammi via signals
    connect( player, SIGNAL(playlistChanged()), this, SLOT(updatePlaylist()) );
    connect( player, SIGNAL(statusChanged()), this, SLOT(updatePlayerStatus()) );
  }


  model->readSongDatabase();					// read song database
	model->readCategories();						// read categories
	model->readHistory();								// read history
  isScanning=false;

	// set up gui
	//****************************
	cout << "setting up gui...\n";
	
	// set up menu
	QMenuBar* mainMenu = new QMenuBar(this);
	
	// file menu
	QPopupMenu* fileMenu = new QPopupMenu;
	fileMenu->insertItem( "Quit", this, SLOT(endProgram()), CTRL+Key_Q );
	mainMenu->insertItem( "&File", fileMenu );
  
	// edit menu
	QPopupMenu* editMenu = new QPopupMenu;
	editMenu->insertItem( "Select all",  this, SLOT(selectAll()));
	editMenu->insertItem( "Invert selection",  this, SLOT(invertSelection()));
	mainMenu->insertItem( "&Edit", editMenu );

  // view menu
	QPopupMenu* viewMenu = new QPopupMenu;
	viewMenu->insertItem( "Update Automatic Folder Structure",  this, SLOT(updateView()));
	mainMenu->insertItem( "&View", viewMenu );

  // database menu
  QPopupMenu* databaseMenu = new QPopupMenu;
  databaseMenu->insertItem( "Save Database...",  model, SLOT(save()));
	databaseMenu->insertItem( "Scan Harddisk...",  this, SLOT(updateSongDatabaseHarddisk()));
	databaseMenu->insertItem( "Scan Removable Media...",  this, SLOT(updateSongDatabaseMedia()));
	databaseMenu->insertItem( "Check Consistency...",  this, SLOT(forAllCheckConsistency()));
	databaseMenu->insertItem( "Grab And Encode CD-Track...",  this, SLOT(grabAndEncode()));
	mainMenu->insertItem( "&Database", databaseMenu );
		
  // settings menu
  QPopupMenu* settingsMenu = new QPopupMenu;
  settingsMenu->insertItem( "Configure Yammi...",  this, SLOT(setPreferences()));
	mainMenu->insertItem( "&Settings", settingsMenu );

  // playlist menu
  QPopupMenu* playlistMenu = new QPopupMenu;
  playlistMenu->insertItem( "Switch to/from Playlist...",  this, SLOT(toFromPlaylist()));
  playlistMenu->insertItem( "Clear Playlist...",  this, SLOT(clearPlaylist()));
  playlistMenu->insertItem( "Shuffle Playlist...",  this, SLOT(shufflePlaylist()));
	mainMenu->insertItem( "&Playlist", playlistMenu );

  // autoplay menu
  autoplayMenu = new QPopupMenu;
  autoplayMenu->insertItem( "Off",  this, SLOT(autoplayChanged(int)), 0, AUTOPLAY_OFF);
	autoplayMenu->insertItem( "Longest Not Played",  this, SLOT(autoplayChanged(int)), 0, AUTOPLAY_LNP);
	autoplayMenu->insertItem( "Random",  this, SLOT(autoplayChanged(int)), 0, AUTOPLAY_RANDOM);
  autoplayMenu->insertSeparator();
  autoplayMenu->insertItem( "", AUTOPLAY_FOLDER);
  autoplayMode=AUTOPLAY_OFF;
  autoplayMenu->setItemChecked(autoplayMode, true);

  mainMenu->insertItem( "&Autoplay", autoplayMenu );
  
  // help menu	
	QPopupMenu* helpMenu = new QPopupMenu;
	helpMenu->insertItem( "&Handbook...",  this, SLOT(openHelp()));
	helpMenu->insertItem( "&About...", this, SLOT(aboutDialog()));
	mainMenu->insertItem( "&Help", helpMenu );
	
	
	
  // media player toolbar
  QToolBar* mediaPlayerToolBar = new QToolBar (this, "mediaPlayerToolbar");
  mediaPlayerToolBar->setLabel( "Media Player Controls" );
  tbPlayPause = new QToolButton( QPixmap((const char**)pause_xpm), "Play/Pause (F1)", QString::null,
                           player, SLOT(playPause()), mediaPlayerToolBar);
  tbStop = new QToolButton( QPixmap((const char**)stop_xpm), "Stop", QString::null,
                           player, SLOT(stop()), mediaPlayerToolBar);
  tbSkipBackward = new QToolButton( QPixmap((const char**)skipbackward_xpm), "Skip backward (F2 / SHIFT-F2)", QString::null,
 														this, SLOT(skipBackward()), mediaPlayerToolBar);
  tbSkipForward = new QToolButton( QPixmap((const char**)skipforward_xpm), "Skip forward (F3 / SHIFT-F3)", QString::null,
                           this, SLOT(skipForward()), mediaPlayerToolBar);
  songSlider = new QSlider( QSlider::Horizontal, mediaPlayerToolBar, "songLength" );
  songSlider->setTickmarks(QSlider::Below);
  songSlider->setTickInterval(1000*60);
  songSlider->setFixedWidth(180);
  songSlider->setPaletteBackgroundColor(QColor(179, 218, 226));
  isSongSliderGrabbed=false;
  connect( songSlider, SIGNAL(sliderReleased()), SLOT(songSliderMoved()) );
  connect( songSlider, SIGNAL(sliderPressed()), SLOT(songSliderGrabbed()) );
  if(player->getName()=="dummy") {
    // disable the buttons if no media player available
    tbPlayPause->setEnabled(false);
    tbStop->setEnabled(false);
    tbSkipBackward->setEnabled(false);
    tbSkipForward->setEnabled(false);
    songSlider->setEnabled(false);
  }
		
	// main toolbar
	QToolBar* toolBar = new QToolBar ( this, "toolbar");
  toolBar->setLabel( "Main Toolbar" );
//	/ = new QToolButton( QPixmap(filesave_xpm), "Save database (Ctrl-S)", QString::null,
//														model, SLOT(save()), toolBar);
//  tbSaveDatabase->setEnabled(model->allSongsChanged());
	QLabel *searchLabel = new QLabel(toolBar);
	searchLabel->setText( "Search:" );
	searchLabel->setFrameStyle( QFrame::NoFrame );
	searchField = new LineEditShift ( toolBar );
	connect( searchField, SIGNAL(textChanged(const QString&)), SLOT(userTyped(const QString&)) );
	searchField->setFocus();
	searchField->setFixedWidth(175);
	QToolTip::add( searchField, "Fuzzy search (Ctrl-F)");
	
	// button "add to wishlist"	
	QPushButton* addToWishListButton=new QPushButton("to wishlist", toolBar);
	QToolTip::add( addToWishListButton, "Add this entry to the database as a \"wish\"");
	
	// current song label
	QPushButton* currentSongLabel=new QPushButton("current", toolBar);
	
	// song actions	toolbar
	QToolBar* toolBar2 = new QToolBar ( this, "toolbar2");
  toolBar2->setLabel( "Song Actions" );

	// now all the buttons that correspond to context menu entries
  new QToolButton (QPixmap(enqueue_xpm), "Enqueue at end (F5, SHIFT-F5 for random order)", QString::null,
                           this, SLOT(forAllSelectedEnqueue()), toolBar2);
	new QToolButton (QPixmap(enqueueasnext_xpm), "Enqueue as next (F6, SHIFT-F6 for random order)", QString::null,
                           this, SLOT(forAllSelectedEnqueueAsNext()), toolBar2);
	new QToolButton (QPixmap(playnow_xpm), "Play now (F7 / SHIFT-F7)", QString::null,
                           this, SLOT(forAllSelectedPlayNow()), toolBar2);
	new QToolButton (QPixmap(dequeueSong_xpm), "Dequeue Song (F8)", QString::null,
                           this, SLOT(forAllSelectedDequeue()), toolBar2);
	new QToolButton (QPixmap(dequeueAll_xpm), "Clear playlist (SHIFT-F8)", QString::null,
                           this, SLOT(clearPlaylist()), toolBar2);
	new QToolButton (QPixmap(prelisten_xpm), "Prelisten (start) (F9)", QString::null,
                           this, SLOT(forAllSelectedPrelistenStart()), toolBar2);
	new QToolButton (QPixmap(prelisten_xpm), "Prelisten (middle) (F10)", QString::null,
                           this, SLOT(forAllSelectedPrelistenMiddle()), toolBar2);
	new QToolButton (QPixmap(prelisten_xpm), "Prelisten (end) (F11)", QString::null,
                           this, SLOT(forAllSelectedPrelistenEnd()), toolBar2);
	new QToolButton (QPixmap(stopPrelisten_xpm), "Stop prelisten (F12)", QString::null,
                           this, SLOT(stopPrelisten()), toolBar2);
	new QToolButton (QPixmap(songinfo_xpm), "Info...", QString::null,
                           this, SLOT(forAllSelectedSongInfo()), toolBar2);
	new QToolButton (QPixmap(toFromPlaylist_xpm), "Switch to/from Playlist (CTRL-P)", QString::null,
                           this, SLOT(toFromPlaylist()), toolBar2);

	// removable media management
	QToolBar* toolBarRemovableMedia = new QToolBar ( this, "Removable Media Toolbar");
  toolBarRemovableMedia->setLabel( "Jukebox Functions" );
	QLabel *neededMediaLabel = new QLabel(toolBarRemovableMedia);
	neededMediaLabel->setText( "Needed media:" );
	mediaListCombo = new QComboBox( FALSE, toolBarRemovableMedia, "mediaList Combo" );
	mediaListCombo->setFixedWidth(150);
	loadFromMediaButton=new QPushButton("load", toolBarRemovableMedia);
	
	// Sleep mode
	QToolBar* toolBarSleepMode = new QToolBar ( this, "Sleep Mode Toolbar");
  toolBarSleepMode->setLabel( "Sleep Mode" );
	songsUntilShutdown=-3;
	sleepModeLabel = new QLabel(toolBarSleepMode);
	sleepModeLabel->setText( "Sleep mode:" );
	sleepModeLabel->setFrameStyle( QFrame::NoFrame );
	sleepModeButton=new QPushButton("(disabled)", toolBarSleepMode);
	connect( sleepModeButton, SIGNAL( clicked() ), this, SLOT( changeSleepMode() ) );
	QToolTip::add( sleepModeButton, "change sleep mode");
	sleepModeSpinBox=new QSpinBox(1, 99, 1, toolBarSleepMode);
	sleepModeSpinBox->setValue(songsUntilShutdown);
	sleepModeSpinBox->setEnabled(false);
	QToolTip::add( sleepModeSpinBox, "number songs until shutdown");
  connect( sleepModeSpinBox, SIGNAL( valueChanged(int) ), this, SLOT( changeShutdownValue(int) ) );

	// now setup main area
	QSplitter* centralWidget=new QSplitter(Qt::Horizontal, this);
	
  // statusbar
  mainStatusBar=statusBar();

  // set up the quick browser on the left
	folderListView = new QListView( centralWidget );
	folderListView->header()->setClickEnabled( FALSE );
	folderListView->addColumn( "Quick Browser" );
	folderListView->setRootIsDecorated( TRUE );
	folderListView->setSorting(-1);
	centralWidget->setResizeMode( folderListView, QSplitter::KeepSize );

  // set up the songlist on the right
	songListView = new MyListView( centralWidget );
	
	QValueList<int> lst;
	lst.append( 150 );
	centralWidget->setSizes( lst );

	setCentralWidget(centralWidget);
	cout << "..done\n";

	// now init all Folders
	//*********************
	cout << "setting up folders...\n";
	
	// folder containing all music
	folderAll=new Folder( folderListView, QString("All Music"), &(model->allSongs));
	
	// folder containing all artists with more than <n> songs	
	folderArtists = new FolderGroups( folderListView, QString( "Artists" ));
	folderArtists->moveItem(folderAll);
	folderArtists->update(&(model->allSongs), MyList::ByArtist);
	
	// folder containing all albums with more than <n> songs	
	folderAlbums = new FolderGroups( folderListView, QString( "Albums" ));
	folderAlbums->moveItem(folderArtists);
	folderAlbums->update(&(model->allSongs), MyList::ByAlbum);

	// folder containing all genres with more than <n> songs	
	folderGenres = new FolderGroups( folderListView, QString( "Genre" ));
	folderGenres->moveItem(folderAlbums);
	folderGenres->update(&(model->allSongs), MyList::ByGenre);

	// folder containing all categories
	folderCategories = new FolderCategories( folderListView, QString("Categories"));
	folderCategories->moveItem(folderGenres);
	folderCategories->update(model->allCategories, model->categoryNames);
	updateSongPopup();
	
	// folder containing media
	folderMedia = new FolderMedia( folderListView, QString("Media"));
	folderMedia->moveItem(folderCategories);
	folderMedia->update(&(model->allSongs));

	// folder containing currently played song
	folderActual = new FolderSorted(folderListView, QString("Playlist"));
//	folderActual->moveItem(folderAll);
	folderActual->update(&(model->songsToPlay));

	// folder containing songs played in this session
	folderSongsPlayed = new Folder(folderListView, QString("Songs Played"), &(model->songsPlayed));
	folderSongsPlayed->moveItem(folderCategories);

	// folder containing history
	folderHistory = new Folder(folderListView, QString("History"), &(model->songHistory));
	folderHistory->moveItem(folderSongsPlayed);

	// folder containing unclassified songs
	for(SongEntry* entry=model->allSongs.first(); entry; entry=model->allSongs.next()) {
		if(!entry->song()->classified)
			model->unclassifiedSongs.append(entry);
	}
	folderUnclassified = new Folder(folderListView, QString("Unclassified"), &(model->unclassifiedSongs));
	folderUnclassified->moveItem(folderMedia);
		
	folderSearchResults = new Folder( folderListView, QString("Search Results"), &searchResults );
	folderSearchResults->moveItem(folderActual);
	
	folderProblematic = new Folder( folderListView, QString("Problematic Songs") );
	folderProblematic->moveItem(folderUnclassified);
	cout << "..done\n";

	// connect all things...
	//**********************
	
	// signals of toolbar
	connect( addToWishListButton, SIGNAL( clicked() ), this, SLOT( addToWishList() ) );
	connect( currentSongLabel, SIGNAL( clicked() ), this, SLOT( currentlyPlayedSongPopup() ) );
	connect( loadFromMediaButton, SIGNAL( clicked() ), this, SLOT( loadMedia() ) );

	// signals of folderListView
  connect( folderListView, SIGNAL( currentChanged( QListViewItem* ) ),
	     this, SLOT( slotFolderChanged() ) );
	connect(folderListView, SIGNAL( rightButtonPressed( QListViewItem *, const QPoint& , int ) ),
	    this, SLOT( slotFolderPopup( QListViewItem *, const QPoint &, int ) ) );
	
	// signals of songListView
  connect(songListView, SIGNAL( rightButtonPressed( QListViewItem *, const QPoint& , int ) ),
	    this, SLOT( songListPopup( QListViewItem *, const QPoint &, int ) ) );
  connect(songListView, SIGNAL( doubleClicked( QListViewItem * ) ),
	    this, SLOT( doubleClick() ) );
  connect(songListView, SIGNAL( mouseButtonClicked( int, QListViewItem *, const QPoint&, int ) ),
	    this, SLOT( middleClick(int) ) );
  // for saving column settings
  connect(songListView->header(), SIGNAL( sizeChange(int, int, int) ),
	    this, SLOT( saveColumnSettings() ) );
  connect(songListView->header(), SIGNAL( indexChange(int, int, int) ),
	    this, SLOT( saveColumnSettings() ) );

	// signals of folders
	connect(folderCategories, SIGNAL( CategoryNew() ), this, SLOT(newCategory()));
	connect(folderCategories, SIGNAL( CategoryRemoved() ), this, SLOT(removeCategory()));
	connect(folderCategories, SIGNAL( CategoryRenamed() ), this, SLOT(renameCategory()));
	connect(folderMedia, SIGNAL( RemoveMedia() ), this, SLOT(removeMedia()));
	connect(folderMedia, SIGNAL( RenameMedia() ), this, SLOT(renameMedia()));

	// some preparations for startup
	//******************************
// TODO: remove?
//	folderListView->firstChild()->setOpen( TRUE );
	
	shuttingDown=0;
	controlPressed=false;
	shiftPressed=false;
  toFromRememberFolder=folderAll;
	
  // finish initialization of player
  player->syncPlayer2Yammi(&(model->songsToPlay));
  player->syncYammi2Player(false);
  checkPlaylistAvailability();
  
  if(folderActual->songList->count()>0) {
    // check whether player is playing, if not: start playing!
    if(player->getStatus()!=PLAYING) {
      cout << "media player is not playing, starting it...\n";
      player->play();
    }
  }
  // TODO: this should be probably done by a thread owned by the media player
  connect( &checkTimer, SIGNAL(timeout()), player, SLOT(check()) );
  checkTimer.start( 200, FALSE );

	// connect all timers
  connect( &regularTimer, SIGNAL(timeout()), SLOT(onTimer()) );
  regularTimer.start( 500, FALSE );	// call onTimer twice a second
	connect( &typeTimer, SIGNAL(timeout()), SLOT(searchFieldChanged()) );


  readSettings();

  QTimer* timer=new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(finishInitialization()) );
	timer->start(0, TRUE);
}


void YammiGui::finishInitialization()
{
  // restore session settings
  mainStatusBar->message("Welcome to Yammi "+model->config.yammiVersion, 10000);
  changeToFolder(chosenFolder, true);
	songListView->setSelected( songListView->firstChild(), TRUE );
	songListView->setCurrentItem( songListView->firstChild() );
	updateSongPopup();
  
	if(model->noPrefsFound && model->noDatabaseFound) {
		QMessageBox::information( this, "Yammi",	QString("Yammi - Yet Another Music Manager I...\n\n\n")+
																					"It looks like you are starting Yammi the first time...\n\n"+
                                          " Welcome to convenient song lookups and organization!\n\n"+
																					"Please edit the settings (Settings -> Configure Yammi...)\n"+
																					"to adjust your personal configuration and options"+
                                          "(especially the path to your media files).\n"+
																					"Then perform a database update (Database -> Scan Harddisk...)\n"+
																					"to scan your harddisk for media files.\n\n"+
																					"I hope you have fun using Yammi...\n\n"+
																					"Please check out Yammi's website for new versions and other info:\n"+
																					"http://yammi.sourceforge.net");
	}

  // finish!
  cout << "initialisation successfully completed!\n";
}

/// exit program
void YammiGui::endProgram()
{
	cout << "endProgram\n";
	close(false);
}


/// destructor
YammiGui::~YammiGui()
{
	cout << "trying to exit gracefully...\n";
	if(model->allSongsChanged() || model->categoriesChanged()) {
		QString msg="Save changes?\n\n";
		msg+="If you don't save, all changes will be lost\n";
		msg+="and the database might become inconsistent...\n";
		if(QMessageBox::warning( this, "Yammi", msg, "Yes", "No")==0) {
			model->save();
    }
	}
	else {
		// we only save history if there are more than 2 songs to add
		if(model->config.logging && model->songsPlayed.count()>2) {
			model->saveHistory();
    }
	}
  updateGeometrySettings();
  writeSettings();
  player->syncYammi2Player(true);    
  delete(player);
	cout << "goodbye!\n";
}


/**
 * This slot is called on changes in the playlist (model->songsToPlay),
 * signalled by the mediaplayer or on changes from within yammigui (enqueing, dequeing songs, ...)
 */
void YammiGui::updatePlaylist()
{
  // prepare: stop user dragging action if necessary
  if(songListView->dragging) {
    stopDragging();
  }

  updateCurrentSongStatus();
  
  // update gui
  folderActual->correctOrder();
  player->syncYammi2Player(false);
  folderContentChanged(folderActual);
}

void YammiGui::updateCurrentSongStatus()
{
  Song* firstInPlaylist=model->songsToPlay.count()>0 ? model->songsToPlay.firstSong() : 0;
  if(firstInPlaylist!=currentSong) {
    // a change in the first (=currently played) song!

    // handle last song
    handleLastSong(currentSong);
    // handle new song
    handleNewSong(model->songsToPlay.firstSong());
  }
}

/**
 * Puts the song that was just played into folder songsPlayed.
 */
void YammiGui::handleLastSong(Song* lastSong)
{
  if(lastSong==0) {
    return;
  }
  // we put last song in folder songsPlayed
  // but first check, whether already in there (due to xmms status change bug)
  if(model->songsPlayed.count()>=1) {
    Song* lastLogged=((SongEntry*)model->songsPlayed.getLast())->song();
    if(lastLogged==lastSong) {
      return;
    }
  }
  MyDateTime now;
	SongEntryTimestamp* entry=new SongEntryTimestamp(lastSong, &currentSongStarted);
	lastSong->lastPlayed=entry->timestamp;
	folderSongsPlayed->addEntry(entry);		// append to songsPlayed
}

/**
 * Called when a new song is played: updates title bar, songSlider, checks sleepMode
 */
void YammiGui::handleNewSong(Song* newSong)
{
  currentSong=newSong;
  if(newSong==0) {
    setCaption("Yammi - not playing");
    currentFile="";
    songSlider->setValue(0);
    songSlider->setRange(0, 0);
    songSlider->setTickInterval(1000*60);
    return;
  }
  // TODO: take swapped file?
  currentFile=newSong->location();
  currentSongStarted=currentSongStarted.currentDateTime();

  if(songsUntilShutdown>0) {
    songsUntilShutdown--;
    sleepModeSpinBox->setValue(songsUntilShutdown);
    if(songsUntilShutdown==0) {
      cout << "sleep mode: shutting down now...\n";
      shutDown();
    }
  }

  // set title to currently played song
  setCaption("Yammi: "+currentSong->displayName());

  // setup songSlider
//  cout << "calling setRange, length: " << currentSong->length*1000 << "\n";
  songSlider->setRange(0, currentSong->length*1000);
  songSlider->setTickInterval(1000*60);
}

/**
 * This slot should be called on changes in the player status.
 * eg. signalled by the mediaplayer
 */
void YammiGui::updatePlayerStatus()
{
  if(player==0) {
    return;
  }
  if(player->getStatus()==PLAYING) {
    tbPlayPause->setIconSet(QIconSet(QPixmap((const char**)pause_xpm)));
    if(currentSong==0 || currentFile!=player->getCurrentFile()) {
      handleLastSong(currentSong);
      (model->getSongFromFilename(player->getCurrentFile()));
    }
  }
  else {
    tbPlayPause->setIconSet(QIconSet(QPixmap((const char**)play_xpm)));
  }

  // check, if we stopped because playlist empty and sleep mode activated
  if(songsUntilShutdown>0) {
    if(player->getStatus()==STOPPED && model->songsToPlay.count()==0) {
      cout << "shutting down now...\n";
      shutDown();
    }
  }
}


/**
 * Writes the current session settings of GUI to a settings file.
 */
void YammiGui::writeSettings()
{
  QSettings settings;
  settings.insertSearchPath( QSettings::Unix, model->config.yammiBaseDir );
  settings.writeEntry( "/Yammi/geometry/posx", geometryX );
  settings.writeEntry( "/Yammi/geometry/posy", geometryY );
  settings.writeEntry( "/Yammi/geometry/width",  geometryWidth);
  settings.writeEntry( "/Yammi/geometry/height", geometryHeight);
  settings.writeEntry( "/Yammi/songlistview/columnOrder" , columnOrder);
  for(int i=0; i<MAX_COLUMN_NO; i++) {
    settings.writeEntry( "/Yammi/songlistview/column"+QString("%1").arg(i)+"Width" , columnWidth[i]);
  }
  settings.writeEntry( "/Yammi/folder/current", chosenFolder->folderName());
  settings.writeEntry( "/Yammi/folder/autoplay", autoplayFoldername);
}

/**
 * Restores session settings.
 * Initializes chosenFolder to the last open folder.
 */
void YammiGui::readSettings()
{
  QSettings settings;
  settings.insertSearchPath( QSettings::Unix, model->config.yammiBaseDir );

  // geometry
  int posx = settings.readNumEntry( "/Yammi/geometry/posx", 0 );
  int posy = settings.readNumEntry( "/Yammi/geometry/posy", 0 );
  int width = settings.readNumEntry( "/Yammi/geometry/width", 1024 );
  int height = settings.readNumEntry( "/Yammi/geometry/height", 468 );
  setGeometry(QRect(posx, posy, width, height));

  // column order
  columnOrder=settings.readListEntry("/Yammi/songlistview/columnOrder");
  if(columnOrder.count()==0) {
    cout << "no column order found\n";
  }
  for(int i=0; i<MAX_COLUMN_NO; i++) {
    columnWidth[i]=settings.readNumEntry("/Yammi/songlistview/column"+QString("%1").arg(i)+"Width");
  }

  // chosen folder
  chosenFolder=getFolderByName(settings.readEntry("/Yammi/folder/current", "All Music"));
  if(chosenFolder==0) {
    chosenFolder=folderAll;
  }  

  // autoplay folder
  autoplayFoldername=settings.readEntry("/Yammi/folder/autoplay", "All Music");
  autoplayMenu->changeItem(AUTOPLAY_FOLDER, "Folder: "+autoplayFoldername);
}


Folder* YammiGui::getFolderByName(QString folderName)
{
  for(QListViewItem* i=folderListView->firstChild(); i; i=i->itemBelow()) {
    Folder* f=(Folder*)i;
    if(f->folderName()==folderName) {
      return f;
    }
    for(QListViewItem* i2=i->firstChild(); i2; i2=i2->itemBelow()) {
      Folder* f2=(Folder*)i2;
      if(f2->folderName()==folderName) {
        return f2;
      }
    }
  }
  return 0;
}









/// Update the internal geometry settings.
void YammiGui::updateGeometrySettings()
{
//  cout << "updating geometry: x(): " << x() << ", y(): " << y() << ", width(): " << width() << ", height(): " << height() << "\n";
  geometryX=x();
  geometryY=y();
  geometryWidth=width();
  geometryHeight=height();
}



/// updates the automatically calculated folders after changes to song database
void YammiGui::updateView()
{
	for(Song* s=model->allSongs.firstSong(); s; s=model->allSongs.nextSong())
		s->classified=false;	
	folderArtists->update(&(model->allSongs), MyList::ByArtist);
	folderAlbums->update(&(model->allSongs), MyList::ByAlbum);
	folderGenres->update(&(model->allSongs), MyList::ByGenre);
	folderMedia->update(&(model->allSongs));
	
	model->unclassifiedSongs.clear();
	for(Song* s=model->allSongs.firstSong(); s; s=model->allSongs.nextSong()) {
		if(!s->classified)
			model->unclassifiedSongs.appendSong(s);
	}
	folderUnclassified->update(&(model->unclassifiedSongs));
  folderContentChanged();
}


/** Updates listview columns.
 * Tries to maintain the size and position of columns (as good as possible, as columns are changing)
 */
void YammiGui::updateListViewColumns()
{
  int noOldColumns=songListView->columns();
  for(int i=0; i<noOldColumns; i++) {
		songListView->removeColumn(0);
  }

  if(chosenFolder==folderActual)
		songListView->addColumn( "Pos", 35);
	if(chosenFolder==folderHistory || chosenFolder==folderSongsPlayed)
		songListView->addColumn( "Played on", 135);
	if(chosenFolder==folderSearchResults)
		songListView->addColumn( "Match", 45);
	if(chosenFolder==folderProblematic)
		songListView->addColumn( "Reason", 120);
	if(((QListViewItem*)chosenFolder)->parent()==folderCategories)
		songListView->addColumn( "Pos", 35);
		
		
	int offset=songListView->columns();
	songListView->addColumn( "Artist", 200);
	songListView->addColumn( "Title", 200);
	songListView->addColumn( "Album", 150);
	songListView->addColumn( "Length", 50);
	songListView->setColumnAlignment( offset+3, Qt::AlignRight );
	songListView->addColumn( "Year", 50);
	songListView->setColumnAlignment( offset+4, Qt::AlignRight );
	songListView->addColumn( "Track", 40);
	songListView->setColumnAlignment( offset+5, Qt::AlignRight );
	songListView->addColumn( "Genre", 40);
//	songListView->setColumnAlignment( offset+6, Qt::AlignRight );
	songListView->addColumn( "Added to", 60);
	songListView->setColumnAlignment( offset+7, Qt::AlignRight );
	songListView->addColumn( "Bitrate", 40);
	songListView->setColumnAlignment( offset+8, Qt::AlignRight );
	songListView->addColumn( "Filename", 80);
	songListView->addColumn( "Path", 80);
	songListView->addColumn( "Comment", 100);
	songListView->addColumn( "Last played", 100);
	songListView->setAllColumnsShowFocus( TRUE );
	songListView->setShowSortIndicator( TRUE );
	songListView->setSelectionMode( QListView::Extended );
	songListView->setAllColumnsShowFocus( TRUE );


  int noNewColumns=songListView->columns();
  QHeader* header=songListView->header();

  // iterate through old columns, and if found in new, move to the target position
  bool exists[MAX_COLUMN_NO];
  for(int x=0; x<MAX_COLUMN_NO; x++) {
    exists[x]=false;
  }
  
  int target=0;
  for(int i=0; i<(int)columnOrder.count(); i++) {
    QString search=columnOrder[i];
    for(int j=0; j<noNewColumns; j++) {
      if(header->label(j)==search) {
        header->moveSection(j, target);
        header->resizeSection(j, columnWidth[i]);
        target++;
        exists[j]=true;
      }
    }
  }
  target=0;
  for(int j=0; j<noNewColumns; j++) {
    if(!exists[j]) {
      header->moveSection(j, target);
      target++;
    }
  }
  saveColumnSettings();
}

void YammiGui::saveColumnSettings()
{
  int noColumns=songListView->columns();
  QHeader* header=songListView->header();
  columnOrder.clear();
  for(int j=0; j<noColumns; j++) {
    int section=header->mapToSection(j);
    columnOrder.append(header->label(section));
    columnWidth[j]=header->sectionSize(section);
  }
}

/// opens the preferences dialogue
void YammiGui::setPreferences()
{
	PreferencesDialog d(this, "preferencesDialog", true, &model->config);

	// show dialog
	int result=d.exec();

	if(result==QDialog::Accepted) {
		updateSongPopup();
		model->savePreferences();
	}
}


/// Updates the popup-menu for songs, especially available categories
void YammiGui::updateSongPopup()
{
	// submenu containing all categories
	playListPopup = new QPopupMenu();
	playListPopup->insertItem(QIconSet( QPixmap(newCategory_xpm)), "New Category...", this, SLOT(toCategory(int)), 0, 9999);
	for(unsigned int i=0; i<model->categoryNames.count(); i++) {
		playListPopup->insertItem(QIconSet( QPixmap(in_xpm)), model->categoryNames[i], this, SLOT(toCategory(int)), 0, 10000+i);
	}
		
	// define popup menu for songs
	songPopup = new QPopupMenu( songListView );
	songPopup->insertItem( "Song Name", 113 );
	songPopup->insertSeparator();
	
	songPlayPopup = new QPopupMenu(songPopup);
	
	songPlayPopup->insertItem(getPopupIcon(Enqueue), "...Enqueue", this, SLOT(forSelection(int)), 0, Enqueue);
	songPlayPopup->insertItem(getPopupIcon(EnqueueRandom), "...Enqueue (random)", this, SLOT(forSelection(int)), 0, EnqueueRandom);
	songPlayPopup->insertItem(getPopupIcon(EnqueueAsNext), "...Enqueue as next", this, SLOT(forSelection(int)), 0, EnqueueAsNext);
	songPlayPopup->insertItem(getPopupIcon(EnqueueAsNextRandom), "...Enqueue as next (random)", this, SLOT(forSelection(int)), 0, EnqueueAsNextRandom);
	songPlayPopup->insertItem(getPopupIcon(PlayNow), "...Play now!", this, SLOT(forSelection(int)), 0, PlayNow);
	songPlayPopup->insertItem(getPopupIcon(Dequeue), "...Dequeue", this, SLOT(forSelection(int)), 0, Dequeue);
	songPopup->insertItem( "Play/Enqueue...", songPlayPopup);
	
	if(model->config.secondSoundDevice!="") {
		songPrelistenPopup = new QPopupMenu(songPopup);
		songPrelistenPopup->insertItem(getPopupIcon(PrelistenStart), "...start", this, SLOT(forSelection(int)), 0, PrelistenStart);
		songPrelistenPopup->insertItem(getPopupIcon(PrelistenMiddle), "...middle", this, SLOT(forSelection(int)), 0, PrelistenMiddle);
		songPrelistenPopup->insertItem(getPopupIcon(PrelistenEnd), "...end", this, SLOT(forSelection(int)), 0, PrelistenEnd);
		songPopup->insertItem( "Prelisten to...", songPrelistenPopup);
	}
	songPopup->insertItem(getPopupIcon(SongInfo), "Info...", this, SLOT(forSelection(int)), 0, SongInfo);
	songPopup->insertItem( "Insert Into/Remove From...", playListPopup);

	songSearchPopup = new QPopupMenu(songPopup);
	songSearchPopup->insertItem( "Entry", this, SLOT(searchSimilar(int)), 0, 1000);
	songSearchPopup->insertItem( "Artist", this, SLOT(searchSimilar(int)), 0, 1001);
	songSearchPopup->insertItem( "Title", this, SLOT(searchSimilar(int)), 0, 1002);
	songSearchPopup->insertItem( "Album", this, SLOT(searchSimilar(int)), 0, 1003);
	songPopup->insertItem( "Search for similar...", songSearchPopup);
	
	if(model->config.childSafe)
		return;
	
	songAdvancedPopup = new QPopupMenu(songPopup);
	songAdvancedPopup->insertItem(getPopupIcon(Delete), "Delete...", this, SLOT(forSelection(int)), 0, Delete);
	songAdvancedPopup->insertItem(getPopupIcon(MoveTo), "Move file to...", this, SLOT(forSelection(int)), 0, MoveTo);
	songAdvancedPopup->insertItem(getPopupIcon(CheckConsistency), "Check Consistency", this, SLOT(forSelection(int)), 0, CheckConsistency);
	songAdvancedPopup->insertItem(getPopupIcon(BurnToMedia), "Burn to Media...", this, SLOT(forSelection(int)), 0, BurnToMedia);
	songPopup->insertItem( "Advanced...", songAdvancedPopup);
	
	
	pluginPopup = new QPopupMenu(songPopup);
	for(unsigned int i=0; i<model->config.pluginMenuEntry.count(); i++) {
		pluginPopup->insertItem( model->config.pluginMenuEntry[i], this, SLOT(forSelectionPlugin(int)), 0, 2000+i);
	}
	songPopup->insertItem( "Plugins...", pluginPopup);
}


// returns icon for popup
QIconSet YammiGui::getPopupIcon(action whichAction)
{
	if(model->config.doubleClickAction==whichAction)
		return QIconSet(QPixmap(defaultDoubleClick_xpm));
	if((model->config.middleClickAction==whichAction))
		return QIconSet(QPixmap(defaultMiddleClick_xpm));
	if((model->config.controlClickAction==whichAction))
		return QIconSet(QPixmap(defaultControlClick_xpm));
	if((model->config.shiftClickAction==whichAction))
		return QIconSet(QPixmap(defaultShiftClick_xpm));
	else
		return (QIconSet) NULL;
}


/// adds the text in search field to the wishlist
void YammiGui::addToWishList()
{
	QString toAdd=searchField->text();
	MyDateTime wishDate=wishDate.currentDateTime();
	Song* newSong=new Song("{wish}", toAdd, "", "", "", 0, 0, wishDate, 0, "", 0, 0);
	folderAll->addSong(newSong);
  forSong(newSong, SongInfo, NULL);
	model->allSongsChanged(true);
	searchField->setText("{wish}");
  folderContentChanged(folderAll);
  searchField->setText("");	
}

/**
 * adds all selected songs to the category (specified by index)
 * if current song is already in category => removes all selected from that category (if they are in)
 */
void YammiGui::toCategory(int index)
{
	index-=10000;
  if(index==-1) {
    // create new category
    if(!newCategory()) {
      return;
    }
    index=model->allCategories.count()-1;
  }
	// choose the desired category
	MyList* category=model->allCategories.first();
	for(int i=0; i<index; i++) {
		category=model->allCategories.next();
	}
	QString chosen=model->categoryNames[index];
	
  // determine whether all/some/none of the selected songs are already in the chosen category
  int mode=category->containsSelection(&selectedSongs);

	// determine mode (add/remove): we only use remove mode if all selected songs are contained in category
  bool remove=(mode==2);

  
	// get pointer to the folder
	FolderSorted* categoryFolder=0;
	for( QListViewItem* f=folderCategories->firstChild(); f; f=f->nextSibling() ) {
		if( ((Folder*)f)->folderName()==chosen )
			categoryFolder=(FolderSorted*)f;
	}
	
	if(categoryFolder==0) {
		cout << "folder not found!\n";
		return;
	}
	// go through list of songs
	for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
		if(!remove) {
      if(!categoryFolder->songList->containsSong(s)) {
        categoryFolder->addSong(s);
      }
    }
		else {
			categoryFolder->removeSong(s);
    }
	}
	
	model->categoriesChanged(true);
  folderContentChanged(categoryFolder);
	
	updateSongPopup();
}


void YammiGui::decide(Song* s1, Song* s2)
{
	QString str1("s1: "+s1->artist+","+s1->title+","+s1->filename+","+QString("%1").arg(s1->bitrate)+","+s1->path);
	QString str2("s2: "+s2->artist+","+s2->title+","+s2->filename+","+QString("%1").arg(s2->bitrate)+","+s2->path);
	cout << str1 << "\n";
	cout << str2 << "\n";
				
	int def=0;									// 1=keep s1, 2=keep s2, 0=keep both
	if(s1->bitrate > s2->bitrate) {
		def=1;
	}
	if(s1->bitrate < s2->bitrate) {
		def=2;
	}
				
	int what=QMessageBox::critical( this, "Two identical songs", str1+"\n"+str2+"\n", "Keep both", "Keep s1", "Keep s2", def);
	if(what==0) {				// keep both => do nothing
	}
	if(what==1) {				// okay, delete s2
		cout << "deleting s2\n";
		forSong(s2, DeleteFile);				// move it to trash...
//hh		model->allSongs.removeSong(s2);
		folderAll->removeSong(s2);
	}
	if(what==2) {				// okay, delete s1
		cout << "deleting s1\n";
		forSong(s1, DeleteFile);				// move it to trash...
//hh		model->allSongs.removeSong(s1);
		folderAll->removeSong(s1);
	}
	model->allSongsChanged(true);
}



void YammiGui::userTyped( const QString& searchStr )
{
	if(searchStr.length()<1) return;
	typeTimer.stop();
	typeTimer.start( 300, TRUE );
//	searchFieldChanged();
}


/// searches for similar entries like the current song
void YammiGui::searchSimilar(int what)
{
	what-=1000;
	QString searchFor;
//	getCurrentSong();
	Song* refSong=selectedSongs.firstSong();
	switch(what)
	{
		case 1:
			searchFor=refSong->artist;
			break;
		case 2:
			searchFor=refSong->title;
			break;
		case 3:
			searchFor=refSong->album;
			break;
		default:
			searchFor=refSong->displayName();
	}		
	searchField->setText(searchFor);
	searchFieldChanged();
}


/// search field changed => update search results
void YammiGui::searchFieldChanged()
{
	QString searchStr=" "+searchField->text()+" ";
	if(searchStr.length()<3) return;
	
	FuzzySearch fs;
	fs.initialize(searchStr.lower(), 2, 4);			// STEP 1
	
	// search through all songs
	Song* s=model->allSongs.firstSong();
	QString composed;
	for(; s; s=model->allSongs.nextSong()) {
		composed=" " + s->artist + " - " + s->title + " - " + s->album + " - " + s->comment + " ";
		if(s->artist=="" || s->title=="") {							// if tags incomplete use filename for search
			composed=s->filename+"- "+composed;
		}
		fs.checkNext(composed.lower(), (void*)s);				// STEP 2
	}

	fs.newOrder();											// STEP 3
	BestMatchEntry** bme;
	bme=fs.getBestMatchesList();				// STEP 4
	
	// insert n best matches into search result list
	searchResults.clear();
	int noResults=0;
	for(; noResults<model->config.searchMaximumNoResults && bme[noResults] && bme[noResults]->sim>(model->config.searchThreshold*10); noResults++) {
		searchResults.append( new SongEntryInt2 ((Song*)bme[noResults]->objPtr, bme[noResults]->sim) );
	}
	folderSearchResults->update(&searchResults);
  if(chosenFolder==folderSearchResults) {
    folderContentChanged(folderSearchResults);
  }
  else {
    changeToFolder(folderSearchResults);
  }
	songListView->setContentsPos( 0, 0);			// set scroll position to top
	QListViewItem* item=songListView->firstChild();
	if(item)
		item->setSelected(true);											// select first anyway
	int threshold=700;
	for(int j=0; j<noResults && bme[j] && bme[j]->sim>threshold; j++, item=item->nextSibling()) {
		item->setSelected(true);
	}
}


/**
 * user clicked on a folder
 */
void YammiGui::slotFolderChanged()
{
	QListViewItem *i = folderListView->currentItem();
	if ( !i )
		return;
	changeToFolder((Folder*)i);
}

/**
 * Change to the specified folder and do necessary view updates.
 */
void YammiGui::changeToFolder(Folder* newFolder, bool changeAnyway)
{
  if(!changeAnyway && newFolder==chosenFolder) {
    // don't do anything if current folder is already the specified one
    return;
  }
  
  QApplication::setOverrideCursor( Qt::waitCursor );
  // TODO: history of visited folders, something like:
  //visitedFoldersHistory->add(chosenFolder);
  // TODO: remember sort order when changing folders?
  //int oldSortOrder=chosenFolder->songList->getSortOrder();

  chosenFolder = newFolder;
  
  if(chosenFolder==folderActual)
		songListView->dontTouchFirst=true;				// don't allow moving the first
	else
		songListView->dontTouchFirst=false;
	
  updateListViewColumns();
  songListView->sortedBy=1;

  folderListView->setCurrentItem( (QListViewItem*)chosenFolder );
  folderListView->setSelected( (QListViewItem*)chosenFolder , TRUE );
  folderListView->ensureItemVisible((QListViewItem*)chosenFolder);

  folderContentChanged();
}

void YammiGui::folderContentChanged()
{
	songListView->clear();
	addFolderContent(chosenFolder);
  if(chosenFolder==folderActual) {
    updateCurrentSongStatus();
  }
}

void YammiGui::folderContentChanged(Folder* folder)
{
	if(folder==chosenFolder) {
		folderContentChanged();
  }
	else {
		songListView->triggerUpdate();
    if(folder==folderActual) {
      updateCurrentSongStatus();
    }
  }
}


/// recursively add the content of folder and all subfolders
/// for now: folder contains songs OR subfolders, but not both!
void YammiGui::addFolderContent(Folder* folder)
{	
  folderToAdd=folder;
	alreadyAdded=0;
	
	// filling the listview is much slower than with qt2.3...
	// what do we do about it?
	// maybe disable sorting while filling, and then enable when we are ready???
	if(folder->songList!=0) {
		songListView->setSorting(-1);
		songListView->setUpdatesEnabled(false);
		addFolderContentSnappy();
	}
	else		// no songList in that folder
		QApplication::restoreOverrideCursor();
}

void YammiGui::addFolderContentSnappy()
{	
	int i=0;
	SongEntry* entry;
	SongListItem* lastOne=(SongListItem*)songListView->firstChild();
	for (entry = folderToAdd->firstEntry(); entry && i<alreadyAdded; entry = folderToAdd->nextEntry(), i++ ) {
		SongListItem* check=(SongListItem*)lastOne->itemBelow();
		if(check!=0)
			lastOne=check;
	}
	
	for (; entry && i<=alreadyAdded+200; entry = folderToAdd->nextEntry(), i++ ) {
		lastOne=new SongListItem( songListView, entry); //, lastOne);
	}
	alreadyAdded=i;
	if(entry) {		// any songs left to add?
		QTimer* timer=new QTimer(this);
		connect(timer, SIGNAL(timeout()), this, SLOT(addFolderContentSnappy()) );
		timer->start(0, TRUE);
	}
	else {		// no, we are finished
		QApplication::restoreOverrideCursor();
		songListView->setUpdatesEnabled(true);
    if(((QListViewItem*)chosenFolder)->parent()==folderAlbums) {
      songListView->setSorting(5);
    }
    else {
      songListView->setSorting(0);      
    }
	}
}


/// user clicked on a song
void YammiGui::slotSongChanged()
{
}


/// pushbutton on current song
void YammiGui::currentlyPlayedSongPopup()
{
	// get currently played song
	getCurrentlyPlayedSong();
	// we should get mouse position, but how???
	doSongPopup(QPoint(500, 50));
}

/// rmb on songlist: song popup for selection
void YammiGui::songListPopup( QListViewItem* Item, const QPoint & point, int )
{
	// get selection
	getSelectedSongs();
	doSongPopup(point);
}

void YammiGui::doSongPopup(QPoint point)
{	
	int selected=selectedSongs.count();
	if( selected<=0 )
		return;										// only if at least one song selected
	adjustSongPopup();
 	songPopup->popup( point );
}	


/// adjust SongPopup corresponding to <selectedSongs>
void YammiGui::adjustSongPopup()
{
	int selected=selectedSongs.count();	
	QString label;
	Song* first=selectedSongs.firstSong();
	if (selected>1) 							// more than one song selected
		label=QString("%1 songs selected").arg(selected);
	else
		label=first->displayName();
	songPopup->changeItem ( 113, label);
		
	// for each category: determine whether first song contained or not
	// we don't check whether all selected songs are contained, just first
	int k=0;
	for(MyList* category=model->allCategories.first(); category; category=model->allCategories.next(), k++) {
    int mode=category->containsSelection(&selectedSongs);
    switch(mode) {
      case 0:
        playListPopup->changeItem(10000+k, QIconSet( QPixmap(notin_xpm)), playListPopup->text(10000+k));
        break;
      case 1:
			  playListPopup->changeItem(10000+k, QIconSet( QPixmap(some_in_xpm)), playListPopup->text(10000+k));
        break;
      case 2:
			  playListPopup->changeItem(10000+k, QIconSet( QPixmap(in_xpm)), playListPopup->text(10000+k));
        break;
    }
	}
		
 	// for songs not on local harddisk: disable certain menu entries
 	// only if exactly one song selected!
 	bool enable=true;
 	if(selected==1 && first->filename=="") {
 		enable=false;
  }
		
 	songPopup->setItemEnabled(PlayNow, enable);
 	songPopup->setItemEnabled(Dequeue, enable);
 	songPopup->setItemEnabled(PrelistenStart, enable);
 	songPopup->setItemEnabled(PrelistenMiddle, enable);
 	songPopup->setItemEnabled(PrelistenEnd, enable);
 	songPopup->setItemEnabled(CheckConsistency, enable);
 	songPopup->setItemEnabled(MoveTo, enable);
 	songPopup->setItemEnabled(BurnToMedia, enable);
}


/// folder popup menu
void YammiGui::slotFolderPopup( QListViewItem* Item, const QPoint & point, int )
{
	QListViewItem *i = folderListView->currentItem();	
	Folder* chosenFolder = ( Folder* )i;
	
	// get selection: complete folder content (shown in songlist)
	// (take the order as shown in the songlist)
	selectedSongs.clear();
	for(QListViewItem* i=songListView->firstChild(); i; i=i->itemBelow())						// go through list of songs
		selectedSongs.appendSong(((SongListItem*) i)->song());

	if(selectedSongs.count()==0) {
		// no songs in this folder
		chosenFolder->popup( point, 0);
		return;
	}
	adjustSongPopup();
	chosenFolder->popup( point, songPopup);
}



// executes a plugin on a selection of songs
void YammiGui::forSelectionPlugin(int pluginIndex)
{
	pluginIndex-=2000;

  bool confirm=model->config.pluginConfirm[pluginIndex]=="true";
  QString mode=model->config.pluginMode[pluginIndex];
  QString cmd=model->config.pluginCommand[pluginIndex];

  if(cmd.contains("%X")>0) {
    QString dir=QFileDialog::getExistingDirectory(QString(""), this, QString("yammi"), QString("choose directory for plugin"), true);
    if(dir.isNull())
      return;
    cmd.replace(QRegExp("%X"), dir);
  }
  if(cmd.contains("%Y")>0) {
    QString file=QFileDialog::getSaveFileName(QString(""), 0, this, QString("yammi"), QString("choose file for plugin"));
    if(file.isNull())
      return;
    cmd.replace(QRegExp("%Y"), file);
  }
  if(cmd.contains("%Z")>0) {
    bool ok;
    QString inputString=QString(QInputDialog::getText( "Get input parameter", "Type in argument for plugin:", QLineEdit::Normal, QString(""), &ok, this ));
    if(!ok) {
      return;
    }
    cmd.replace(QRegExp("%Z"), inputString);
  }


  if(mode=="single") {
    QProgressDialog progress( "Executing song plugin cmd...", "Cancel", 100, this, "progress", TRUE );
    progress.setTotalSteps(selectedSongs.count());
    qApp->processEvents();

    int index=1;
    for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong(), index++) {
      QString cmd2=makeReplacements(cmd, s, index);
      
      if(index==1 && confirm) {
        // before  executing cmd on first song, ask user
        QString msg="Execute the following command on each selected song?\n";
        msg+="(here shown: values for first song)\n\n";
        for(unsigned int i=0; i<cmd2.length(); i+=80) {
          msg+=cmd2.mid(i, 80)+"\n";
        }
        if( QMessageBox::warning( this, "Yammi", msg, "Yes", "No")!=0) {
          return;
        }
      }
      progress.setProgress(index);
      qApp->processEvents();
      if(progress.wasCancelled())
        return;
      system(cmd2);
    }
  }

  if(mode=="group") {
    int index=1;
    QString customList="";
    for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong(), index++) {
      QString entry = model->config.pluginCustomList[pluginIndex];
      customList+=makeReplacements(entry, s, index);
    }

    // custom list can be long => better put it into a file...
    QFile customListFile(model->config.yammiBaseDir+"/customlist.txt");
    customListFile.open(IO_WriteOnly);
    customListFile.writeBlock( customList, qstrlen(customList) );
    customListFile.close();
    cmd.replace(QRegExp("%l"), "`cat "+model->config.yammiBaseDir+"/customlist.txt`");
    cmd.replace(QRegExp("%L"), customList);

    if(confirm) {
      cout << "plugin command: " << cmd << "\n";
      cout << "custom list: " << customList << "\n";
      QString msg="Execute the following command:\n";
      for(unsigned int i=0; i<cmd.length(); i+=80) {
        msg+=cmd.mid(i, 80)+"\n";
        if(i>1200) {
          msg+="\n...\n(command truncated)";
          break;
        }
      }
      if( QMessageBox::warning( this, "Yammi", msg, "Yes", "No")!=0) {
        return;
      }
    }
    system(cmd);
  }
}


QString YammiGui::makeReplacements(QString input, Song* s, int index)
{
  // 1. prepare strings
  // length
  QString lengthStr=QString("%1").arg(s->length % 60);
	if (lengthStr.length()==1)
	 	lengthStr="0"+lengthStr;
  // medialist
  QString mediaList="";
	for(unsigned int i=0; i<s->mediaName.count(); i++) {
		if(i!=0)
			mediaList+=", ";
		mediaList+=s->mediaName[i];
	}
  // filename without suffix
  int suffixPos = s->filename.findRev('.');
  QString filenameWithoutSuffix=s->filename.left(suffixPos);
  // trackNr
  QString trackNrStr;
  if(s->trackNr==0)
    trackNrStr="";
  else
    trackNrStr=QString("%1").arg(s->trackNr);

  // replace
  input.replace(QRegExp("%f"), s->location());
	input.replace(QRegExp("%F"), s->filename);
	input.replace(QRegExp("%W"), filenameWithoutSuffix);  
	input.replace(QRegExp("%p"), s->path);
	input.replace(QRegExp("%a"), s->artist);
	input.replace(QRegExp("%t"), s->title);
	input.replace(QRegExp("%u"), s->album);
	input.replace(QRegExp("%b"), QString("%1").arg(s->bitrate));
	input.replace(QRegExp("%i"), QString("%1").arg(index));
	input.replace(QRegExp("%l"), QString("%1:%2").arg((s->length) / 60).arg(lengthStr));
	input.replace(QRegExp("%s"), QString("%1").arg(s->length));
  input.replace(QRegExp("%n"), "\n");
	input.replace(QRegExp("%m"), mediaList);
  input.replace(QRegExp("%r"), trackNrStr);
  input.replace(QRegExp("%0r"), trackNrStr.rightJustify(2,'0'));
  return input;  
}


/**
 * edit song info
 * (mass editing)
 */
void YammiGui::forSelectionSongInfo()
{
	QString _artist, _title, _album, _comment, _path, _filename, _year, _trackNr, _bitrate, _proposedFilename;
	MyDateTime _addedTo, _lastTimePlayed;
	int _length=0;
	long double _size=0;
	int _genreNr=0;
		
	SongInfoDialog si(this, "test", true);
	
	// fill combobox with genres, but sort them first
	QStringList genreList;
	genreList.append("");
	for(int genreNr=0; genreNr<=CMP3Info::getMaxGenreNr(); genreNr++) {
		genreList.append(CMP3Info::getGenre(genreNr));
	}
	genreList.sort();
	for ( QStringList::Iterator it = genreList.begin(); it != genreList.end(); ++it ) {
		si.ComboBoxGenre->insertItem((*it).latin1());
	}

	int selected=0;
  QDateTime invalid;
	for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
		selected++;
		if(selected==10)			// set wait cursor (summing size of 2000 files may take a while...)
			QApplication::setOverrideCursor( Qt::waitCursor );
			
		// get filesize
		QFile file(s->location());
		if(file.exists())
			_size+=file.size();
		_length+=s->length;
	
		// insert all media, over that songs are distributed
		for(unsigned int m=0; m<s->mediaName.count(); m++) {
			bool found=false;
			for(int n=0; n<si.ComboBoxMedia->count(); n++) {
				if(si.ComboBoxMedia->text(n)==s->mediaName[m])
					found=true;
			}
			if(!found)
				si.ComboBoxMedia->insertItem(s->mediaName[m]);
		}

		if(selected==1) {
			_addedTo=s->addedTo;
			_album=s->album;
			_artist=s->artist;
			_comment=s->comment;
			_title=s->title;
			_trackNr=QString("%1").arg(s->trackNr);
			_year=QString("%1").arg(s->year);
			_path=s->path;
			_filename=s->filename;
			_bitrate=QString("%1 kb/s").arg(s->bitrate);
			_genreNr=s->genreNr;
      _proposedFilename=s->constructFilename();
      _lastTimePlayed=s->lastPlayed;
		}
		else {
			if(_addedTo!=s->addedTo)          _addedTo=invalid; //.setDate(QDate::fromString(""));
			if(_album!=s->album)							_album="!";
			if(_artist!=s->artist)						_artist="!";
			if(_comment!=s->comment)					_comment="!";
			if(_title!=s->title)							_title="!";
			if(_trackNr!=QString("%1").arg(s->trackNr))		_trackNr="!";
			if(_year!=QString("%1").arg(s->year))					_year="!";
			if(_path!=s->path)								_path="!";
			if(_filename!=s->filename)				_filename="!";
			if(_bitrate!=QString("%1").arg(s->bitrate))					_bitrate="!";
			if(_genreNr!=s->genreNr)					_genreNr=-1;
			if(_proposedFilename!=s->constructFilename())	_proposedFilename="!";
			if(_lastTimePlayed!=s->lastPlayed) _lastTimePlayed=invalid; //.setDate(QDate::fromString(""));
		}
	}

	if(selected>=10)
		QApplication::restoreOverrideCursor();
	
	// now edit the (common) info
	si.LineEditArtist->setText(_artist);
	si.LineEditTitle->setText(_title);
	si.LineEditAlbum->setText(_album);
	si.LineEditComment->setText(_comment);
	if(_year!="0")			si.LineEditYear->setText(_year);
	if(_trackNr!="0")		si.LineEditTrack->setText(_trackNr);
//	MyDateTime d=_addedTo;
//	if(_addedTo.isValid())
	if(_addedTo.isValid())
		si.LineEditAddedTo->setText(_addedTo.writeToString());
	else
		si.LineEditAddedTo->setText("!");
	if(_lastTimePlayed.isValid()) {
    MyDateTime never;
    never.setDate(QDate(1900,1,1));
    never.setTime(QTime(0,0,0));
    if(_lastTimePlayed!=never)
      si.LineEditLastPlayed->setText(_lastTimePlayed.writeToString());
    else
      si.LineEditLastPlayed->setText("never");
  }
	else {
		si.LineEditLastPlayed->setText("!");
  }
  
	si.ReadOnlyPath->setText(_path);
	si.ReadOnlyFilename->setText(_filename);
	si.ReadOnlyProposedFilename->setText(_proposedFilename);
	if(selected>1) {
		si.LabelHeading->setText(QString("Mass editing: %1 songs").arg(selected));
		si.LabelSize->setText("Size (total)");
		si.LabelLength->setText("Length (total)");
		QString x;		
		si.ReadOnlyLength->setText(x.sprintf("%d:%02d:%02d (hh:mm:ss)", _length/(60*60), (_length % (60*60))/60, _length % 60));
	}
	else {
		si.LabelHeading->setText(_artist+" - "+_title);
		QString x;		
		si.ReadOnlyLength->setText(x.sprintf("%2d:%02d (mm:ss)", _length/60, _length % 60));
	}
	si.ReadOnlySize->setText( QString("%1 MB (%2 Bytes)")
				.arg( (float)_size/(float)(1024*1024) , 4,'f', 2 )
				.arg( (float)_size                    ,10,'f', 0 )
				);
	si.ReadOnlyBitrate->setText(_bitrate);
	
	if(_genreNr==-1)
		si.ComboBoxGenre->setCurrentItem(0);
	else {
		int found=genreList.findIndex(CMP3Info::getGenre(_genreNr));
		if(found!=-1)
			si.ComboBoxGenre->setCurrentItem(found);
	}
	
	// show dialog
	int result=si.exec();
	
	if(result==QDialog::Accepted) {
		// get genreNr
		int sortedGenreNr=si.ComboBoxGenre->currentItem();
		int tryGenreNr=-1;
		if(sortedGenreNr!=0)
      tryGenreNr=CMP3Info::getGenreIndex(genreList[sortedGenreNr]);

		
		// now set the edited info for all selected songs
		for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
			bool change=false;
			if(si.LineEditArtist->text()!="!" && si.LineEditArtist->text()!=s->artist)	{ s->artist=si.LineEditArtist->text(); change=true; }
			if(si.LineEditTitle->text()!="!" && si.LineEditTitle->text()!=s->title)			{ s->title=si.LineEditTitle->text(); change=true; }
			if(change) 		// for artist and title: mark categories as dirty on change!
				model->markPlaylists(s);
			if(si.LineEditAlbum->text()!="!" && si.LineEditAlbum->text()!=s->album) 		{ s->album=si.LineEditAlbum->text(); change=true; }
			if(si.LineEditComment->text()!="!" && si.LineEditComment->text()!=s->comment)	{s->comment=si.LineEditComment->text(); change=true; }
			if(si.LineEditYear->text()!="!") {
				int tryYear=atoi(si.LineEditYear->text());
				if(tryYear!=s->year) {s->year=tryYear; change=true; }
			}
			if(si.LineEditTrack->text()!="!") {
				int tryTrackNr=atoi(si.LineEditTrack->text());
				if(tryTrackNr!=s->trackNr) {s->trackNr=tryTrackNr; change=true; }
			}
			MyDateTime newAddedTo;
			newAddedTo.readFromString(si.LineEditAddedTo->text());
			if(newAddedTo.isValid())
				if(newAddedTo!=s->addedTo) { s->addedTo=newAddedTo; change=true; }
			
			if(tryGenreNr!=-1)
				if(tryGenreNr!=s->genreNr) { s->genreNr=tryGenreNr; change=true; }
			
			if(change) {
				model->allSongsChanged(true);
				s->tagsDirty=true;						// mark song as dirty(tags)
				s->filenameDirty=(s->checkFilename()==false);
				// manual update: go through list of songs and correct, if necessary
				for(SongListItem* i=(SongListItem*)songListView->firstChild(); i; i=(SongListItem*)i->itemBelow()) {
					if(i->song()!=s)
						continue;
					i->setColumns(i->songEntry);
				}
			}				
		}	
	}
}


/// prepare burning selection to media
/// (burning order will be the order of the selected songs)
void YammiGui::forSelectionBurnToMedia()
{
	long double totalSize=0;
	long double size=-1;

	bool ok;
	QString collName=QString(QInputDialog::getText( "collection name", "Please enter collection name:", QLineEdit::Normal, QString("my mp3 collection"), &ok, this ));
	if(!ok)
		return;

	QString startIndexStr=QString(QInputDialog::getText( "collection start number", "Please enter start index:", QLineEdit::Normal, QString("1"), &ok, this ));
	if(!ok)
		return;

	QProgressDialog progress( "Preparing media...", "Cancel", 100, this, "progress", TRUE );
	progress.setMinimumDuration(0);
	progress.setAutoReset(false);
  progress.setProgress(0);
	progress.setTotalSteps(selectedSongs.count());
	qApp->processEvents();

	int startIndex=atoi(startIndexStr);
  int mediaNo=startIndex-1;
	QString mediaName=QString("%1_%2").arg(collName).arg(mediaNo);
	QString mediaDir=QString(model->config.yammiBaseDir+"/media/"+mediaName);
	long double sizeLimit=(long double)model->config.criticalSize*1024.0*1024.0;
	int count=0;
	for(Song* s=selectedSongs.firstSong(); s; ) {
	  progress.setProgress(count);
		if(progress.wasCancelled())
			break;

		QFileInfo fi(s->location());
		if(size==-1 || size+fi.size()>sizeLimit) {
      // medium is full, prepare new one
			mediaNo++;
			mediaName=QString("%1_%2").arg(collName).arg(mediaNo);
			mediaDir=QString(model->config.yammiBaseDir+"/media/"+mediaName);
			progress.setLabelText("Preparing media "+mediaName);
      cout << "Preparing media " << mediaName << " (" << count << " files processed so far)...\n";
			QDir dir(mediaDir);
      if(dir.exists()) {
        cout << "directory \"" << mediaDir << "\" already exists, calculating available space...\n";
        size=diskUsage(mediaDir, sizeLimit);
        if(size==-1 || size+fi.size()>sizeLimit) {
          cout << "directory already too full, skipping...\n";
          continue;
        }
        cout << ((int)size/1024.0/1024.0) << " MB already used\n";
      }
      else {
        dir.mkdir(mediaDir);
        size=0;
      }
		}

    // okay, we can really add the song to the current media
		size+=fi.size();
    totalSize+=fi.size();
		// linux specific
		QString cmd=QString("ln -s \"%1\" \"%3/%4\"").arg(s->location()).arg(mediaDir).arg(s->filename);
		system(cmd);
		s->addMediaLocation(mediaName, s->filename);
    s=selectedSongs.nextSong();
    count++;
	}
	
	cout << "no of media: " << mediaNo+1-startIndex << " (size limit: " << model->config.criticalSize << " MB, ";
  cout << "index " << startIndex << " to " << mediaNo << ")\n";
	cout << "no of files: " << count << "\n";
	cout << "size of last media: " << (int)(size/1024.0/1024.0) << " MB\n";
	cout << "size in total: " << (int)(totalSize/1024.0/1024.0) << " MB\n";
	folderMedia->update(&(model->allSongs));
	model->allSongsChanged(true);
	QString msg=QString("Result of \"Burn to media\" process:\n\n\
  no of media: %1, (size limit: %2 MB)\n\
  first media index: %3\n\
  last media index: %4\n\
  no of files: %5\n\
  size of last media: %6 MB\n\
  size in total: %7 MB\n\n\n\
  You have now for each medium a directory in\n\
  %8,\n\
  containing symbolic links to all contained songs.\n\n\
  For burning these files to a CD, you can use a\n\
  burning program of your choice and burn\n\
  each directory to a seperate CD.\n\
  (Depending on your burning program, you might have\n\
  to check an option \"follow symlinks\" or similar).")
  .arg(mediaNo+1-startIndex).arg(model->config.criticalSize).arg(startIndex).arg(mediaNo)
  .arg(count).arg((int)(size/1024.0/1024.0)).arg((int)(totalSize/1024.0/1024.0))
  .arg(model->config.yammiBaseDir+"/media/");
	QMessageBox::information( this, "Yammi", msg, "Fine." );
}



/** calculate disk usage for a directory (including all subdirectories)
 * returns -1 if too full
 */
long double YammiGui::diskUsage(QString path, long double sizeLimit)
{
  QDir d(path);

  d.setFilter(QDir::Files);
  const QFileInfoList* list = d.entryInfoList();
  QFileInfoListIterator it( *list );								      // create list iterator

  long double size=0;

  // step 1: sum up all files
  for(QFileInfo *fi; (fi=it.current()); ++it ) {						// for each file/dir
    size+=fi->size();
    if(size>sizeLimit) {
      return -1;
    }
  }

  // step 2: recursively sum up subdirectories
	QDir d2(path);
  d2.setFilter(QDir::Dirs);
	const QFileInfoList* list2 = d2.entryInfoList();
	QFileInfoListIterator it2( *list2 );								      // create list iterator

	for(QFileInfo *fi2; (fi2=it2.current()); ++it2 ) {						// for each file/dir
		if(fi2->fileName()=="." || fi2->fileName()=="..")
      continue;
		double long toAdd=diskUsage(fi2->filePath(), sizeLimit);
    if(toAdd==-1) {
      return -1;
    }
    size+=toAdd;
    if(size>sizeLimit) {
      return -1;
    }
	}

  cout << "disk usage in directory " << path << ": " << ((int)size/1024.0/1024.0) << " MB\n";
  return size;
}






/// makes a list containing only the current song
void YammiGui::getCurrentSong()
{
	selectedSongs.clear();
	QListViewItem* i=songListView->currentItem();
	Song* s=((SongListItem*) i)->song();
	selectedSongs.appendSong(s);
}

/// makes a list containing only the currently played song
void YammiGui::getCurrentlyPlayedSong()
{
	selectedSongs.clear();
	selectedSongs.append(model->songsToPlay.at(0));
}
			
/// makes a list of the currently selected songs
void YammiGui::getSelectedSongs()
{
	selectedSongs.clear();
	QListViewItem* i=songListView->firstChild();
	for(; i; i=i->itemBelow()) {						// go through list of songs
		if(i->isSelected()) {
			Song* s=((SongListItem*) i)->song();
			selectedSongs.appendSong(s);
		}
	}
}

/// makes a list of all songs in database
void YammiGui::getAllSongs()
{
	selectedSongs.clear();
	for(Song* s=model->allSongs.firstSong(); s; s=model->allSongs.nextSong()) {
		selectedSongs.appendSong(s);
	}
}

/// for current song do <action>
void YammiGui::forCurrent(action act)
{
	getCurrentSong();
	forSelection(act);
}

/// for selected songs do <action>
void YammiGui::forAllSelected(action act)
{
	getSelectedSongs();
	forSelection(act);
}
	
/// for all songs do <action>
void YammiGui::forAll(action act)
{
	getAllSongs();
	forSelection(act);
}

/// check consistency, fill up list of problematic songs
void YammiGui::forAllCheckConsistency()
{
  getAllSongs();
  forSelectionCheckConsistency();
}

void YammiGui::forSelectionCheckConsistency()
{
  cout << this->selectedSongs.count() << " songs selected to check\n";
  
	CheckConsistencyDialog d(this, "Check consistency - settings");
  d.TextLabel1->setText(QString("checking %1 songs").arg(selectedSongs.count()));
  
	// show dialog
	int result=d.exec();
	if(result!=QDialog::Accepted)
    return;

  ConsistencyCheckParameter p;  
  p.checkForExistence=d.CheckBoxCheckForExistence->isChecked();
  p.updateNonExisting=d.CheckBoxUpdateNonExisting->isChecked();
  p.checkTags=d.CheckBoxCheckTags->isChecked();
  p.correctTags=d.CheckBoxCorrectTags->isChecked();
  p.checkFilenames=d.CheckBoxCheckFilenames->isChecked();
  p.correctFilenames=d.CheckBoxCorrectFilenames->isChecked();
  p.checkDoubles=d.CheckBoxCheckDoubles->isChecked();

  QProgressDialog progress( "Checking consistency...", "Cancel", 100, this, "progress", TRUE );
	progress.setMinimumDuration(0);
	progress.setAutoReset(false);
  progress.setProgress(0);
	qApp->processEvents();
  
	model->checkConsistency(&progress, &selectedSongs, &p);

  folderProblematic->update(&model->problematicSongs);
	folderContentChanged(folderProblematic);

	QString msg=QString(
  "Result of consistency check:\n\n\
  %1 problematic issues identified (check folder \"Problematic Songs\")\n\
      (that folder won't be saved)\n\
  %2 songs not existing, of these:\n\
      %3 entries updated (filename cleared)\n\
      %4 entries deleted (because not existing on any media)\n\
  %5 songs with inconsistent tags, of these:\n\
      %6 tags corrected\n\
  %7 songs with inconsistent filename, of these:\n\
      %8 filenames corrected\n\
  %9 double entries found\
  ").arg(model->problematicSongs.count()).arg(p.nonExisting).arg(p.nonExistingUpdated)
  .arg(p.nonExistingDeleted).arg(p.dirtyTags).arg(p.tagsCorrected)
  .arg(p.dirtyFilenames).arg(p.filenamesCorrected).arg(p.doublesFound);  
	QMessageBox::information( this, "Yammi", msg, "Fine." );
}

/**
 * for the songs in <selectedSongs> do <action>
 */
void YammiGui::forSelection(action act)
{
	// special treatments for the following actions
	if(act==SongInfo) {
		forSelectionSongInfo();
		return;
	}
	if(act==BurnToMedia) {
		forSelectionBurnToMedia();
		return;
	}
  if(act==CheckConsistency) {
    forSelectionCheckConsistency();
    return;
  }
	// end of special treatment
	
	// 1. destination directory
	QString dir;
	if(act==MoveTo) {
		// let user choose directory (we should provide a starting directory???)
//#ifdef ENABLE_NOATUN
//    cout << "trying...\n";
//    dir=KFileDialog::getOpenFileName(QString("/mm"), QString("*.mp3"), this, QString("yammi"));
//    if(dir!=0)
//      cout << "dir: " << dir << "\n";
//    return;
//#else
		dir=QFileDialog::getExistingDirectory(QString(""), this, QString("yammi"), QString("choose directory"), true);
//#endif    
		if(dir.isNull())
			return;
		if(dir.right(1)=="/")						// strip trailing slash
			dir=dir.left(dir.length()-1);
	}
			
	// 2. determine delete mode
	bool deleteFile=false;
	bool deleteEntry=false;
	if(act==Delete) {
		DeleteDialog dd( this,  "deleteDialog", true);
		if(selectedSongs.count()==1)
			dd.LabelSongname->setText(selectedSongs.firstSong()->displayName());
		else
			dd.LabelSongname->setText(QString("Delete %1 songs").arg(selectedSongs.count()));
		// fill dialog with onMedia info...(for all toDelete songs)
		for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
			for(unsigned int i=0; i<s->mediaName.count(); i++) {
				QString toInsert(s->mediaName[i]);
				bool exists=false;
				for(int j=0; j<dd.ComboBoxOnMedia->count(); j++) {
					if(dd.ComboBoxOnMedia->text(j)==toInsert)
						exists=true;
				}
				if(!exists)
					dd.ComboBoxOnMedia->insertItem(toInsert);
			}
		}
		int result=dd.exec();
		if(result==QDialog::Accepted) {
			deleteFile=dd.CheckBoxDeleteFile->isChecked();
			deleteEntry=dd.CheckBoxDeleteDbEntry->isChecked();
		}
		else
			return;
	}

	
	if(act==EnqueueAsNext) {
    // reverse the order, to get intended play order
    selectedSongs.reverse();
  }
  if(act==EnqueueRandom) {
    selectedSongs.shuffle();
    act=Enqueue;
  }
  if(act==EnqueueAsNextRandom) {
    selectedSongs.shuffle();
    act=EnqueueAsNext;
  }
  if(act==PlayNow) {
    // reverse the order, but keep first song
    SongEntry* first=selectedSongs.take(0);
    selectedSongs.reverse();
    selectedSongs.prepend(first);
  }
	
	// OKAY: go through list of songs
	for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
		if(act==Delete) {
			if(deleteFile)	forSong(s, DeleteFile);
			if(deleteEntry)	forSong(s, DeleteEntry);
		}
		else {
			forSong(s, act, dir);
			if(act==PlayNow)									// we play first selected song...
				act=EnqueueAsNext;							// ...and enqueue all others
			if(act==PrelistenStart || act==PrelistenMiddle || act==PrelistenEnd)
				break;
		}
	}
	
	
	// some operations need view update
	if(deleteEntry) {
		updateView();
		model->allSongsChanged(true);
    model->categoriesChanged(true);
		folderContentChanged(folderAll);
	}
	if(deleteFile) {
		model->allSongsChanged(true);
		folderContentChanged(folderAll);
	}
	if(act==Enqueue || act==EnqueueAsNext || act==Dequeue) {
    player->syncYammi2Player(false);
    folderContentChanged(folderActual);
		checkPlaylistAvailability();
	}
}

/**
 * performs some action for a song
 */
void YammiGui::forSong(Song* s, action act, QString dir)
{		
	switch (act) {
	case None:									// no action
		return;
	case PlayNow:								// enqueue at front and immediately skip to it
		if(s->filename=="" || !s->checkReadability()) {
			cout << "song not available (try to first enqueue and load from a media)\n";
			return;
		}

		forSong(s, EnqueueAsNext);

    if(player->getStatus()==STOPPED) {
      // case 1: player stopped (this is a bit dirty...)
      player->play();
    }
    else if(player->getStatus()==PAUSED) {
      // case 2: player paused => start playing
      player->skipForward(shiftPressed);
      player->play();      
    }
    else if(player->getStatus()==PLAYING) {
      // case 3: player is playing
      player->skipForward(shiftPressed);
    }
			
    mainStatusBar->message(QString("playing %1").arg(s->displayName()), 2000);
		break;
		
	case Enqueue:								// enqueue at end
		folderActual->addSong(s);
		mainStatusBar->message(QString("%1 enqueued at end").arg(s->displayName()), 3000);
		break;
				
	case EnqueueAsNext: {				// enqueue as next
		// songsToPlay is empty, or first song is still to play
		if(model->songsToPlay.count()==0 || currentSong!=model->songsToPlay.at(0)->song())
			model->songsToPlay.insert(0, new SongEntryInt(s, 13));
		else
			model->songsToPlay.insert(1, new SongEntryInt(s, 13));
		folderActual->correctOrder();
    player->syncYammi2Player(false);      
		mainStatusBar->message(QString("%1 enqueued as next").arg(s->displayName()), 2000);
	}
		break;
	
	case Dequeue: {
		// search for selected song and dequeue
		int i=1;
    if(player->getStatus()==STOPPED) {
      i=0;
    }
		for(; i<(int)model->songsToPlay.count(); i++) {
			Song* check=model->songsToPlay.at(i)->song();
			if(check==s) {
				model->songsToPlay.remove(i);
				cout << "song removed\n";
				mainStatusBar->message(QString("song %1 dequeued").arg(s->displayName()), 3000);
				i--;
			}
		}
		folderActual->correctOrder();
		break;
	}
	// these 3 cases send song to headphone, jumping to start/middle/end
	case PrelistenStart:
		preListen(s, 0);
		break;
	case PrelistenMiddle:
		preListen(s, 33);
		break;
	case PrelistenEnd:
		preListen(s, 95);
		break;
		
		
	case SongInfo:
	{
		selectedSongs.clear();
		selectedSongs.appendSong(s);
		forSelectionSongInfo();
	  break;
	}
	
	case CheckConsistency:
	 {
		if(s->filename=="")
			return;
		QString diagnosis=s->checkConsistency(model->config.tagsConsistent, model->config.filenamesConsistent);
		if(diagnosis!="") {
			model->problematicSongs.append(new SongEntryString(s, diagnosis));
		}
	 }
		break;
		
	case Delete: {
    cout << "debug info: creating delete dialog inside \"forSong\"\n";
		DeleteDialog dd( this,  "testiii", true);
		int result=dd.exec();
		if(result==QDialog::Accepted) {
			if(dd.CheckBoxDeleteFile->isChecked()) {
				forSong(s, DeleteFile);				// 1. move songfile to trash
				mainStatusBar->message(QString("%1 removed (file)").arg(s->displayName()), 2000);
			}
			if(dd.CheckBoxDeleteDbEntry->isChecked()) {
				forSong(s, DeleteEntry);			// 2. remove from database
				mainStatusBar->message(QString("%1 removed (db entry)").arg(s->displayName()), 2000);
			}
		}
	} break;
	
	case DeleteEntry:	{						// delete db entry
    // remove from database
    folderAll->removeSong(s);

    // ...and from categories
    for( QListViewItem* f=folderCategories->firstChild(); f; f=f->nextSibling() ) {
      FolderSorted* category=(FolderSorted*)f;
      category->removeSong(s);
    }
    // ...and from playlist
    folderActual->removeSong(s);

  } break;
	
	case DeleteFile:						// move songfile to trash
		s->deleteFile(model->config.trashDir);
		break;
		
/*
  // removed! both can be implemented as plugin!
	case CopyTo:						// copy songfile to other location
		mainStatusBar->message(QString("copying song %1").arg(s->displayName()), 2000);
		s->copyTo(dir);
		mainStatusBar->message(QString("song %1 copied").arg(s->displayName()), 2000);
		break;

	case CopyAsWavTo:
		s->copyAsWavTo(dir);
		break;

*/	
	case MoveTo: 										// move file to another directory
		s->moveTo(dir);
		break;
			
	default:
		cout << "undefined song action: " << act << "\n";
	}
}		


/// doubleClick on song
void YammiGui::doubleClick()
{
	forCurrent(model->config.doubleClickAction);
}

/// middleClick on song
void YammiGui::middleClick(int button)
{
//	cout << "button: " << button << "\n";
//	if(button==1) {			// left button
//	}
	if(button==4) {				// middle button
		if(!controlPressed && !shiftPressed)
			forCurrent(model->config.middleClickAction);
		if(controlPressed && !shiftPressed)
			forCurrent(model->config.controlClickAction);
		if(shiftPressed && !controlPressed)
			forCurrent(model->config.shiftClickAction);
//		if(shiftPressed && controlPressed)
//			cout << "both\n";
	}
}

//void YammiGui::leftClick()	

/// open konqueror with help
void YammiGui::openHelp()
{
	// linux specific
	// hardcoded path, we should probably read $KDEDIR or something instead...
  // (but I don't know how to do that...)
	system("konqueror $KDEDIR/share/doc/HTML/en/yammi/index.html &");
}

/// display about dialog
void YammiGui::aboutDialog()
{
  QString msg=QString("Yammi - Yet Another Music Manager I...\n\n\n");
  msg+="Version "+model->config.yammiVersion+", 12-2001 - 5-2003 by Oliver N�lle\n";
  msg+="Media player: "+player->getName()+"\n";
  #ifdef ENABLE_XMMS
  msg+="- XMMS support: yes\n";
  #else
  msg+="- XMMS support: no\n";
  #endif
  #ifdef ENABLE_NOATUN
  msg+="- Noatun support: yes\n";
  #else
  msg+="- Noatun support: no\n";
  #endif
  #ifdef ENABLE_OGGLIBS
  msg+="- ogglibs support: yes\n";
  #else
  msg+="- ogglibs support: no\n";
  #endif
  #ifdef ENABLE_ID3LIB
  msg+="- id3lib support: yes\n";
  #else
  msg+="- id3lib support: no\n";
  #endif
  msg+="\nProject home page: yammi.sourceforge.net\n\n\n";
  msg+="Contact: oli.noelle@web.de\n\n";
  msg+="have fun...\n";
  QMessageBox::information( this, "Yammi",msg);
}


/** creates a new category */
bool YammiGui::newCategory(){
	bool ok = false;
	QString caption("Enter name for category");
	QString message("Please enter name of category");
	QString newName=QString(QInputDialog::getText( caption, message, QLineEdit::Normal, QString("new category"), &ok, this ));
	if(!ok) {
    return false;
  }
	model->newCategory(newName);
	folderCategories->update(model->allCategories, model->categoryNames);
	updateSongPopup();		
  return true;
}


void YammiGui::removeCategory()
{
	QListViewItem* i = folderListView->currentItem();
	QString name=((Folder*)i)->folderName();
	if( QMessageBox::warning( this, "Yammi", "Delete category \""+name+"\"?\n (will be deleted immediately!)", "Yes", "No")==0) {
		model->removeCategory(name);
		folderCategories->update(model->allCategories, model->categoryNames);
		updateSongPopup();
	}
}

void YammiGui::renameCategory()
{
	QListViewItem* i = folderListView->currentItem();
	QString oldName=((Folder*)i)->folderName();
	bool ok;
	QString newName=QString(QInputDialog::getText( "collection name", "Please enter new name:", QLineEdit::Normal, oldName, &ok, this ));
	if(!ok)
		return;

	model->renameCategory(oldName, newName);
	folderCategories->update(model->allCategories, model->categoryNames);
	updateSongPopup();
}


/**
 * Sets the selected autoplay folder to the currently selected folder.
 * Updates the autoplayMenu.
 */
void YammiGui::autoplayFolder()
{
	Folder* f = (FolderSorted*)folderListView->currentItem();
  autoplayFoldername=f->folderName();
  autoplayMenu->changeItem(AUTOPLAY_FOLDER, "Folder: "+autoplayFoldername);    
}


/**
 * Called, when the autoplayMode changed.
 * Updates the autoplayMenu.
 */
void YammiGui::autoplayChanged(int mode)
{
  autoplayMenu->setItemChecked(autoplayMode, false);
  autoplayMenu->setItemChecked(mode, true);
  autoplayMode=mode;
}

/// remove media
// uahhaa... ugly! make mediaName + mediaLocation a struct/class, oli!
void YammiGui::removeMedia()
{
	QListViewItem *i = folderListView->currentItem();
	Folder* chosenFolder = ( Folder* )i;
	QString mediaName=chosenFolder->folderName();
	if( QMessageBox::warning( this, "Yammi", "Remove media "+mediaName+" and the corresponding directory?\n(which contains the symbolic links to the songs)", "Yes", "No")!=0)
		return;
	model->removeMedia(mediaName);
	folderMedia->update(&(model->allSongs));
}


void YammiGui::renameMedia()
{
	QListViewItem* i = folderListView->currentItem();
	QString oldName=((Folder*)i)->folderName();
	bool ok;
	QString newName=QString(QInputDialog::getText( "Rename Media", "Please enter new name:", QLineEdit::Normal, oldName, &ok, this ));
	if(!ok)
		return;
	model->renameMedia(oldName, newName);
	folderMedia->update(&(model->allSongs));
}
	

/// invoke an externally configured program/script on the content of a folder
void YammiGui::pluginOnFolder()
{
	QFile f(model->config.yammiBaseDir+"/plugin.temp" );
	if ( !f.open( IO_WriteOnly  ) )
		return;
	QTextStream str(&f);
	cout << " ...done\n";
	
	for(Song* s=chosenFolder->firstSong(); s; s=chosenFolder->nextSong()) {
		cout << s->path << "/" << s->filename << "\n";
		str << s->path <<"/" << s->filename << "\n";
	}
	f.close();
}



/// clear all playlist items except currently played song
void YammiGui::shufflePlaylist()
{
  if(model->songsToPlay.count()<=1) {
    // shuffling 0 or 1 song does not make too much sense...
    return;
  }
  Song* firstSong=0;
  if(player->getStatus()!=STOPPED) {
    firstSong=model->songsToPlay.take(0)->song();
  }
  model->songsToPlay.shuffle();
  if(firstSong!=0) {
    model->songsToPlay.prepend(new SongEntryInt(firstSong, 0));
  }
  folderActual->correctOrder();
  folderContentChanged(folderActual);
  player->syncYammi2Player(false);
}


/// clear all playlist items except currently played song
void YammiGui::clearPlaylist()
{
	if(model->config.childSafe)
		return;
	Song* save=0;

  bool saveCurrentSong=(currentSong!=0 && player->getStatus()!=STOPPED && model->songsToPlay.count()>1);
  ApplyToAllBase confirm(this, "confirmDialog", true);
  QString msg=QString("Clear complete playlist?\n(%1 entries)").arg(model->songsToPlay.count());
  confirm.TextLabel->setText(msg);
  confirm.CheckBoxApply->setText("including current song");
  confirm.CheckBoxApply->setChecked(!saveCurrentSong);
  // show dialog
  int result=confirm.exec();
  if(result!=QDialog::Accepted) {
    return;
  }
  if(!confirm.CheckBoxApply->isChecked()) {
    save=model->songsToPlay.firstSong();
  }
  model->songsToPlay.clear();
  if(!confirm.CheckBoxApply->isChecked())
    folderActual->addSong(save);
  folderActual->updateTitle();
  player->syncYammi2Player(false);
  folderContentChanged(folderActual);
}

/// called whenever user grabs the songSlider
void YammiGui::songSliderGrabbed()
{
	isSongSliderGrabbed=true;
}

/// called when user releases the songSlider
/// causes the player to jump to the given song position
void YammiGui::songSliderMoved()
{
	isSongSliderGrabbed=false;
  player->jumpTo(songSlider->value());
}




/**
 * onTimer is called periodically to do some things independently of any user action
 * - logging
 * - updating folderActual
 */
void YammiGui::onTimer()
{	
  if(isScanning) {
    // we don't want to perform any update actions while Yammi is scanning...
    return;
  }
  if(player==0) {
    return;
  }

  // check for autplay function: fill up playlist?
  if(folderActual->songList->count()<5 && autoplayMode!=AUTOPLAY_OFF && this->autoplayFoldername!="") {
    autoFillPlaylist();
  }
  
	// perform these actions only if player is playing or paused
	if(player->getStatus()!=STOPPED) {
  	
    // adjust songSlider (if user is not currently dragging it around)
		int outputTime=player->getCurrentTime();
		if(!isSongSliderGrabbed && player->getStatus() != PAUSED) {
			songSlider->setValue(outputTime);
    }

	}
}


void YammiGui::autoFillPlaylist()
{
  Folder* toAddFrom=getFolderByName(autoplayFoldername);
  if(toAddFrom!=0 && toAddFrom->songList->count()>0) {
    // fill up from chosen autoplay folder
    int total=toAddFrom->songList->count();
    Song* songToAdd=0;
    
    if(autoplayMode==AUTOPLAY_RANDOM) {
      // method 1: randomly pick a song, no further intelligence
      // create random number
      QDateTime dt = QDateTime::currentDateTime();
      QDateTime xmas( QDate(2050,12,24), QTime(17,00) );
      int chosen=(dt.secsTo(xmas) + dt.time().msec()) % total;
      if(chosen<0) {
        chosen=-chosen;
      }
      songToAdd=toAddFrom->songList->at(chosen)->song();
      if(folderActual->songList->containsSong(songToAdd)) {
        // don't add a song that is already in playlist
        songToAdd=0;
      }
    }

    if(autoplayMode==AUTOPLAY_LNP) {
      // method 2: try to pick the song we didn't play for longest time
      int rememberSortOrder=toAddFrom->songList->getSortOrder();
      toAddFrom->songList->setSortOrderAndSort(MyList::ByLastPlayed, true);

      // if more than one song have the same timestamp: choose randomly
      QDateTime lnpTimestamp;
      MyList candidateList;
      for(unsigned int i=0; i<toAddFrom->songList->count(); i++) {
        Song* check=toAddFrom->songList->at(i)->song();
        if(folderActual->songList->containsSong(check)) {
          continue;
        }
        if(lnpTimestamp.isNull()) {
          lnpTimestamp=check->lastPlayed;
        }
        else if(check->lastPlayed!=lnpTimestamp) {
          break;
        }
        candidateList.appendSong(check);
      }

      int candidates=candidateList.count();
      if(candidates>0) {
        if(candidates==1) {
          songToAdd=candidateList.firstSong();
        }
        else {
          cout << "candidate songs: " << candidates << "\n";
          QDateTime dt = QDateTime::currentDateTime();
          QDateTime xmas( QDate(2050,12,24), QTime(17,00) );
          int chosen=(dt.secsTo(xmas) + dt.time().msec()) % candidates;
          songToAdd=candidateList.at(chosen)->song();        
        }
      }
      toAddFrom->songList->setSortOrderAndSort(rememberSortOrder, true);
    }

    if(songToAdd!=0) {
      folderActual->addSong(songToAdd);
      player->syncYammi2Player(false);
      // update view, if folderActual is currently shown folder
      folderContentChanged(folderActual);
        // TODO: do we have to update the autoplayfolder???
//      if(chosenFolder->folderName()==autoplayFoldername) {
//        folderContentChanged();
//      }
    }
  }
}




// grab a track from audio-cd, encode, and add to database
void YammiGui::grabAndEncode()
{
	bool ok = false;
	QString caption("Enter track number");
	QString message("Please enter track number");
	QString trackNrStr(QInputDialog::getText( caption, message, QLineEdit::Normal, QString("1"), &ok, this ));
	if(!ok)
		return;
	int trackNr=atoi(trackNrStr);
	if(trackNr<1)
		return;
	
	caption="Enter artist";
	message="Please enter artist";
	QString artist(QInputDialog::getText( caption, message, QLineEdit::Normal, QString("MyArtist"), &ok, this ));
	if(!ok)
		return;
	
	caption="Enter title";
	message="Please enter title";
	QString title(QInputDialog::getText( caption, message, QLineEdit::Normal, QString("Fantastic Song"), &ok, this ));
	if(!ok)
		return;
	
	QString filename=QString("%1%2 - %3.mp3").arg(model->config.scanDir).arg(artist).arg(title);
  QFileInfo fi(filename);
  if(fi.exists()) {
    QMessageBox::information( this, "Yammi", "The file\n"+filename+"\nalready exists!\n\nPlease choose a different artist/title combination.", "Ok");
    return;
  }
	// linux specific
	QString cmd=QString("%1 %2 \"%3\" \"%4\" \"%5\" &").arg(model->config.grabAndEncodeCmd).arg(trackNr).arg(artist).arg(title).arg(filename);
	system(cmd);
	grabbedTrackFilename=filename;
	mainStatusBar->message("grabbing track, will be available shortly...", 30000);
	// now we start a timer to check for availability of new track every 5 seconds
	QTimer* timer=new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(checkForGrabbedTrack()) );
	timer->start(5000, TRUE);
}

/** checks for availability of a song that is currently being grabbed and encoded
 */
void YammiGui::checkForGrabbedTrack(){
	QFileInfo fileInfo(grabbedTrackFilename);
 	if(!fileInfo.exists()) {
		QTimer* timer=new QTimer(this);
		connect(timer, SIGNAL(timeout()), this, SLOT(checkForGrabbedTrack()) );
		timer->start(5000, TRUE);
		return;
	}
	if(!fileInfo.isReadable()) {
		cout << "new grabbed track " << grabbedTrackFilename << " is unreadable\n";
 		return;
 	}
	mainStatusBar->message("grabbed song available", 20000);
  model->entriesAdded=0;
	model->corruptSongs=0;
  model->problematicSongs.clear();

  model->addSongToDatabase(grabbedTrackFilename, 0);
	updateView();
	folderProblematic->update(&model->problematicSongs);
	folderAll->updateTitle();
  QString msg="Yammi tried to add the grabbed song to the database.\n\nSome statistics: \n\n";
  msg+=QString("%1 songs added to database\n").arg(model->entriesAdded);
  msg+=QString("%1 songs corrupt (=not added)\n").arg(model->corruptSongs);
  msg+=QString("%1 problematic issues(check in folder Problematic Songs)").arg(model->problematicSongs.count());
	QMessageBox::information( this, "Yammi", msg, "Fine." );
}

void YammiGui::shutDown()
{
  if(shuttingDown==0)				// canceled
		return;
	if(shuttingDown==1) {
    QProgressDialog progress( "shuttind down now...", "Cancel", 10, this, "progress", TRUE );
    progress.setMinimumDuration(0);
    int total=10;                 // number of seconds before shutdown...
    progress.setTotalSteps(total);
    progress.setProgress(0);
    for(int i=0; i<=total; i++) {
      progress.setProgress(i);
      QString msg=QString("shutting down in %1 seconds").arg(total-i);
      progress.setLabelText(msg);
      qApp->processEvents();
      QTimer* ttimer=new QTimer();
      qApp->processEvents();
      ttimer->start(1000, TRUE);
      while(ttimer->isActive()) {
        qApp->processEvents();
      }
      delete(ttimer);
      if(progress.wasCancelled()) {
        changeSleepMode();
        return;
      }
    }
    
    player->quit();                                 	// properly close player (for xmms: will save playlist) 
    if(model->config.shutdownScript!="")
      system(model->config.shutdownScript+" &");			// invoke shutdown script
    endProgram();
	}
}


void YammiGui::keyPressEvent(QKeyEvent* e)
{
//  cout << "x: " << this->x() << "pos.x: " << this->pos().x() << "\n";
//  cout << "geometry.x: " << this->geometry().left() << "frameGeometry.x: " << this->frameGeometry().left() << "\n";
//  cout << "key(): " << e->key() << "text(): " << e->text() << "ascii(): " << e->ascii() << "\n";
	int key=e->key();
	switch(key) {
		case Key_Control:
			controlPressed=true;
			break;
		case Key_Shift:
			shiftPressed=true;
			break;
		case Key_F1:
      player->playPause();
			break;
		case Key_F2:
      skipBackward();
			break;
		case Key_F3:
      skipForward();
			break;
		case Key_F4:
      player->stop();
			break;
		case Key_F5:
			if(shiftPressed) {
        forAllSelected(EnqueueRandom);
      }
      else {
        forAllSelected(Enqueue);
      }
			break;
		case Key_F6:
			if(shiftPressed) {
        forAllSelected(EnqueueAsNextRandom);
      }
      else {
        forAllSelected(EnqueueAsNext);
      }
			break;
		case Key_F7:
 			forAllSelected(PlayNow);
			break;
		case Key_F8:
			if(shiftPressed)
				clearPlaylist();
			else
				forAllSelected(Dequeue);
			break;			
		case Key_F9:
			forAllSelected(PrelistenStart);
			break;
			
		case Key_F10:
			forAllSelected(PrelistenMiddle);
			break;
			
		case Key_F11:
			forAllSelected(PrelistenEnd);
			break;
			
		case Key_F12:
			stopPrelisten();
			break;
			
		case Key_Pause:									// exit program (press twice for shutting down computer)
			changeSleepMode();
			break;
				
		case Key_PageUp:
			if(songsUntilShutdown>0) {
				songsUntilShutdown++;
				sleepModeSpinBox->setValue(songsUntilShutdown);
			}
			break;
		case Key_PageDown:
			if(songsUntilShutdown>1) {
				songsUntilShutdown--;
				sleepModeSpinBox->setValue(songsUntilShutdown);
			}
			break;
      
		case Key_Up:
			for(QListViewItem* i=songListView->firstChild(); i; i=i->itemBelow()) {
				if(i->isSelected()) {
					if(i->itemAbove()) {
						i=i->itemAbove();
						songListView->clearSelection();
						songListView->setSelected(i, true);
            songListView->ensureItemVisible(i);
					}
					break;
				}
			}
      break;
      
		case Key_Down:			
			for(QListViewItem* i=songListView->firstChild(); i; i=i->itemBelow()) {
				if(i->isSelected()) {
					if(i->itemBelow()) {
						i=i->itemBelow();
						songListView->clearSelection();
						songListView->setSelected(i, true);
            songListView->ensureItemVisible(i);
					}
					break;
				}
			}
      break;
		
		case Key_F:		// Ctrl-F
			if(e->state()!=ControlButton)
				break;
			searchField->setText("");
			searchField->setFocus();
			break;
			
		case Key_Escape:
			searchField->setText("");
			searchField->setFocus();
			break;
		
		case Key_S:		// Ctrl-S
			if(e->state()!=ControlButton)
				break;
			model->save();
			break;
		
		case Key_P:		// Ctrl-P
			if(e->state()!=ControlButton)
				break;
			toFromPlaylist();
			break;

    default:
			e->ignore();
	}
}

void YammiGui::keyReleaseEvent(QKeyEvent* e)
{
//	cout << "release key(): " << e->key() << "text(): " << e->text() << "ascii(): " << e->ascii() << "\n";
	int key=e->key();
	switch(key) {
		case Key_Control:
			controlPressed=false;
//      cout << "control released\n";
			break;
		case Key_Shift:
			shiftPressed=false;
//      cout << "shift released\n";
			break;
		default:
			e->ignore();
	}
}

		
void YammiGui::toFromPlaylist()
{
  if(chosenFolder!=folderActual) {
    // switch to playlist
    toFromRememberFolder=chosenFolder;
    changeToFolder(folderActual);
  }
  else {
    // switch back to last open folder
    changeToFolder(toFromRememberFolder);
  }
}

void YammiGui::changeSleepMode()
{
 	if(shuttingDown==0 && !model->config.childSafe) {							// shutdown computer !!!
 		// disabled -> shutdown
 		qApp->beep();
 		shuttingDown=1;
		songsUntilShutdown=3;
		sleepModeButton->setText("shutdown");
    sleepModeSpinBox->setValue(3);
 		sleepModeSpinBox->setEnabled(true);
		cout << "sleep mode, press again to cancel...\n";
 		if(model->allSongsChanged() || model->categoriesChanged()) {
 			if( QMessageBox::warning( this, "Yammi", "Save changes?\n(answering no will cancel sleep mode)", "Yes", "No")==0)
 				model->save();
 			else {
 				shuttingDown=0;
 				songsUntilShutdown=-3;
				sleepModeButton->setText("(disabled)");
        sleepModeSpinBox->setEnabled(false);
 				cout << "sleep mode cancelled!\n";
 			}
 		}
 	}
 	else {
 		// shutdown -> disabled
 		qApp->beep();
 		songsUntilShutdown=-3;
		sleepModeButton->setText("(disabled)");
		sleepModeSpinBox->setEnabled(false);
 		shuttingDown=0;
 		cout << "sleep mode cancelled!\n";
 	}
}

void YammiGui::changeShutdownValue(int value)
{
  songsUntilShutdown=value;
}



// stops playback on headphone
void YammiGui::stopPrelisten()
{
	// kill any previous prelisten process
  // todo: would be better to remember the pid and kill more selectively... for now this must do...
  if(lastPrelistened=="MP3") {
    QString cmd1=QString("kill `ps h -o \"%p\" -C mpg123` 2&> /dev/null");
    system(cmd1);
    lastPrelistened="";
  }
  if(lastPrelistened=="OGG") {
    QString cmd2=QString("kill `ps h -o \"%p\" -C ogg123` 2&> /dev/null");
    system(cmd2);
    lastPrelistened="";
  }
  if(lastPrelistened=="WAV") {
    // how do we stop "play"? killing all sox processes is probably not always desired...
    QString cmd3=QString("kill `ps h -o \"%p\" -C play` 2&> /dev/null");
    system(cmd3);
    QString cmd4=QString("kill `ps h -o \"%p\" -C sox` 2&> /dev/null");
    system(cmd4);
    lastPrelistened="";
  }
}

/* sends the song to headphones
 * skipTo: 0 = beginning of song, 100 = end
 * old version: QString cmd=QString("mpg123 -s \"%1\" | aplay -c1 -m &").arg(s->location());
 */
void YammiGui::preListen(Song* s, int skipTo)
{
	int seconds=s->length;

  // first, kill any previous mpg123 prelisten process
  stopPrelisten();

  // now play song via mpg123, ogg123 or aplay on sound device configured in prefs
	if(s->filename.right(3).upper()=="MP3") {
		QString skip=QString(" --skip %1").arg(seconds*skipTo*38/100);
		QString cmd=QString("mpg123 -a %1 %2 \"%3\" &").arg(model->config.secondSoundDevice).arg(skip).arg(s->location());
		system(cmd);
    lastPrelistened="MP3";
	}
  if(s->filename.right(3).upper()=="OGG") {
		QString skip=QString(" --skip %1").arg(seconds*skipTo/100);
		QString cmd=QString("ogg123 -d oss -odsp:%1 %2 \"%3\" &").arg(model->config.secondSoundDevice).arg(skip).arg(s->location());
		system(cmd);
    lastPrelistened="OGG";
  }
	if(s->filename.right(3).upper()=="WAV") {
		QString skip=QString(" trim %1s").arg(seconds*skipTo*441);
		QString cmd=QString("play -d %1 \"%2\" %3 &").arg(model->config.secondSoundDevice).arg(s->location()).arg(skip);
    cout << cmd << "\n";
		system(cmd);
    lastPrelistened="WAV";
	}

}

void YammiGui::updateSongDatabaseHarddisk()
{
	UpdateDatabaseDialog d(this, "Update Database (harddisk) Dialog");

  d.LineEditScanDir->setText(model->config.scanDir);
  d.LineEditFilePattern->setText("*.mp3 *.ogg *.wav");
	// show dialog
	int result=d.exec();
	if(result!=QDialog::Accepted)
    return;

  QString scanDir=d.LineEditScanDir->text();
  QString filePattern=d.LineEditFilePattern->text();
  updateSongDatabase(scanDir, filePattern, 0);
}


void YammiGui::updateSongDatabaseMedia()
{
	UpdateDatabaseMediaDialog d(this, "Update Database (media) Dialog");

  d.LineEditMediaDir->setText(model->config.mediaDir);
  d.LineEditFilePattern->setText("*.mp3 *.ogg *.wav");
  d.CheckBoxMountMediaDir->setChecked(model->config.mountMediaDir);
	// show dialog
	int result=d.exec();
	if(result!=QDialog::Accepted)
    return;
  if(d.LineEditMediaDir->text()=="") {
    QMessageBox::information( this, "Yammi", "You have to enter a name for the media!", "Okay" );
    return;
  }

  model->config.mountMediaDir=d.CheckBoxMountMediaDir->isChecked();
  QString mediaName=d.LineEditMediaName->text();
  QString mediaDir=d.LineEditMediaDir->text();
  QString filePattern=d.LineEditFilePattern->text();
  updateSongDatabase(mediaDir, filePattern, mediaName);
}



void YammiGui::updateSongDatabase(QString scanDir, QString filePattern, QString media)	
{
  if(model->config.childSafe) {
    return;
  }
  QProgressDialog progress( "Scanning...", "Cancel", 100, this, "progress", TRUE );
	progress.setMinimumDuration(0);
	progress.setAutoReset(false);
  progress.setProgress(0);
	qApp->processEvents();

  isScanning=true;
	model->updateSongDatabase(scanDir, filePattern, media, &progress);
  cout << "debug info: updating view...\n";
  
	updateView();
  cout << "debug info: updating folder problematic...\n";
	folderProblematic->update(&model->problematicSongs);
  cout << "debug info: updating title of folderAll...\n";
	folderAll->updateTitle();
  QString msg="Updated your database.\n\nStatistics: \n\n";
  msg+=QString("%1 songs added to database\n").arg(model->entriesAdded);
  msg+=QString("%1 songs corrupt (=not added)\n").arg(model->corruptSongs);
  msg+=QString("%1 songs problematic (check in folder Problematic Songs)\n").arg(model->problematicSongs.count());
	QMessageBox::information( this, "Yammi", msg, "Fine." );

  // TODO: check update actions after scanning...without danger? (=> don't need to stop xmms?)
//  player->syncPlayer2Yammi(folderActual);
  isScanning=false;

/*
TODO: show most recently added songs in songlist
  (does not work with the following lines)
  folderListView->setCurrentItem( folderListView->firstChild()->firstChild() );
	folderListView->setSelected( folderListView->firstChild(), TRUE );
  slotFolderChanged();
	songListView->sortedBy=7;
  songListView->setSorting(7);
*/
}


void YammiGui::stopDragging()
{
	((FolderSorted*)chosenFolder)->syncWithListView(songListView);
	folderContentChanged();

	if(chosenFolder==folderActual) {
		player->syncYammi2Player(false);
	}
	
	if(((QListViewItem*)chosenFolder)->parent()==folderCategories) {
		// we have to save the order
		model->categoriesChanged(true);
	}
}



/** selects all in songListView */
void YammiGui::selectAll(){
	for(QListViewItem* i=songListView->firstChild(); i; i=i->itemBelow()) {
		i->setSelected(true);
	}
	songListView->triggerUpdate();
}

/** inverts selection in songListView */
void YammiGui::invertSelection(){
	for(QListViewItem* i=songListView->firstChild(); i; i=i->itemBelow()) {
		i->setSelected(!i->isSelected());
	}
	songListView->triggerUpdate();
}


// if known media inserted, loads all songs occurring in playlist into swap dir
void YammiGui::loadSongsFromMedia(QString mediaName)
{	
	int songsToLoad=0;
	for(unsigned int i=1; i<model->songsToPlay.count(); i++) {
		Song* s=model->songsToPlay.at(i)->song();
		if(model->checkAvailability(s)=="")
			songsToLoad++;
	}
	
	QProgressDialog progress( "Loading song files...", "Cancel", songsToLoad, this, "progress", TRUE );
	progress.setMinimumDuration(0);
  progress.setProgress(0);
	qApp->processEvents();
	
	QString mediaDir=model->config.mediaDir;
	QString swapDir=model->config.swapDir;
	// mount swap dir
	if(model->config.mountMediaDir) {
		// linux specific
		QString cmd;
		cmd=QString("mount %1").arg(model->config.mediaDir);
		system(cmd);
	}
	// iterate through playlist and load all songs on that media into swap dir
	// (that are not available so far)
	int loaded=0;
	for(unsigned int i=1; i<model->songsToPlay.count(); i++) {
		if(progress.wasCancelled())
			break;
		Song* s=model->songsToPlay.at(i)->song();
		if(model->checkAvailability(s)=="") {
			for(unsigned int j=0; j<s->mediaLocation.count(); j++) {
				if(s->mediaName[j]==mediaName) {				
					cout << "loading song " << s->displayName() << "from " << mediaDir << s->mediaLocation[j] << "\n";
					progress.setLabelText("loading song: "+s->displayName()+" ("+QString("%1").arg(i+1)+". in playlist)");
			    progress.setProgress(loaded);
  			  qApp->processEvents();
					if(progress.wasCancelled())
						break;
					QString filename=s->constructFilename();
					QString cmd;
					cmd=QString("cp \"%1%2\" \"%3%4.part\"").arg(mediaDir).arg(s->mediaLocation[j]).arg(swapDir).arg(filename);
					system(cmd);
					QDir dir;
					dir.rename(swapDir+filename+".part", swapDir+filename);
					// check swap size (if necessary, delete LRU songs)
					checkSwapSize();
					loaded++;
				}
			}
		}
	}
	progress.setProgress(loaded);
	qApp->processEvents();
	
	// unmount swap dir
	if(model->config.mountMediaDir) {
		// linux specific
		QString cmd;
		cmd=QString("umount %1").arg(model->config.mediaDir);
		system(cmd);
	}

  player->syncYammi2Player(false);
	checkPlaylistAvailability();
//	folderContentChanged(folderActual);
  songListView->triggerUpdate();
}

// manages loading songfiles from removable media
void YammiGui::checkPlaylistAvailability()
{
//	cout << "checking availability...\n";
	// iterate through playlist & check whether we need to load songs to swap dir
	
	// collect all possibly required media into a listbox,
	// the most urgent media first
	mediaListCombo->clear();
	for(unsigned int i=1; i<model->songsToPlay.count(); i++) {
		Song* s=model->songsToPlay.at(i)->song();
		if(s->filename=="") {				// for performance, we first test this (fast)
			if(model->checkAvailability(s, true)=="") {				// this needs harddisk (slow)
				for(unsigned int j=0; j<s->mediaLocation.count(); j++) {
					bool exists=false;
					for(int k=0; k<mediaListCombo->count(); k++) {
						if(mediaListCombo->text(k)==s->mediaName[j])
							exists=true;
					}
					if(!exists)
						mediaListCombo->insertItem(s->mediaName[j]);
				}
			}
		}
	}
	if(mediaListCombo->count()==0) {
		mediaListCombo->insertItem("<none>");
		loadFromMediaButton->setEnabled(false);
	}
	else {
		loadFromMediaButton->setEnabled(true);
	}
}

/// loads the currently in the media list chosen media
void YammiGui::loadMedia()
{
	QString mediaName=mediaListCombo->currentText();
	loadSongsFromMedia(mediaName);
}

/**
 * checks whether the swapped songs take more space than the given limit
 * if they do, we delete the least recently used song files
 */
void YammiGui::checkSwapSize()
{
	long double sizeLimit=(long double)model->config.swapSize*1024.0*1024.0;
	long double size=0.0;
 	QString path=model->config.swapDir;
 	cout << "checking swap size in directory " << path << ", limit: " << model->config.swapSize << " MB\n";
	QDir d(path);
	
	d.setFilter(QDir::Files);
	d.setSorting(QDir::Time);			// most recent first
	const QFileInfoList *list = d.entryInfoList();
	QFileInfoListIterator it( *list );

	for(QFileInfo *fi; (fi=it.current()); ++it ) {
		if (fi->isDir())							// if directory...		=> skip
			continue;
		if ((fi->extension(FALSE)).upper()!="MP3")		// only count our swapped files
			continue;

		if(size+fi->size()>sizeLimit) {
			// swap dir too full, delete this entry
			cout << "removing from swap dir: " << fi->fileName() << "\n";
			QDir dir;
			if(!dir.remove(path+fi->fileName()))
				cout << "could not remove LRU song from swap dir\n";
		}
		else
			size+=fi->size();
	}
}


void YammiGui::skipForward()
{
  player->skipForward(shiftPressed);
}

void YammiGui::skipBackward()
{
  int count=model->songsPlayed.count();
	if(count==0)			// empty folder songsPlayed => can't skip backwards
		return;

	// 1. get and remove last song from songsPlayed
	Song* last=model->songsPlayed.at(count-1)->song();
	model->songsPlayed.remove(count-1);
//  model->songsToPlay->insert(0, new SongEntry(0, last);
//	currentSong=0;
	folderActual->insertSong(last, 0);

  player->skipBackward(shiftPressed);

  // skipping backward creates a new entry in songsPlayed that we don't want
  // => remove it again
	model->songsPlayed.remove(model->songsPlayed.count()-1);
  folderSongsPlayed->updateTitle();
	// update necessary?
  folderContentChanged(folderActual);
  folderContentChanged(folderSongsPlayed);
}



void YammiGui::saveState(QSessionManager& sm )
{
  cout << "saveState() called\n";
}

void YammiGui::commitData(QSessionManager& sm )
{
  cout << "commitData() called\n";
  if ( sm.allowsInteraction() ) {
    if(model->allSongsChanged() || model->categoriesChanged()) {
      QString msg="Save changes?\n\n";
      msg+="If you don't save, all changes will be lost\n";
      msg+="and the database might become inconsistent...\n";
      switch( QMessageBox::warning(this, "Yammi", msg, "&Yes", "&No", "Cancel", 0, 2) ) {
        case 0: // yes
          sm.release();
          // save document here; if saving fails, call sm.cancel()
          model->save();
          break;
        case 1: // continue without saving
          break;
        default: // cancel
          sm.cancel();
          break;
      }
    }
    else {
      // we only save history if there are more than 2 songs to add
      if(model->config.logging && model->songsPlayed.count()>2) {
        model->saveHistory();
      }
    }
  }
  else {
    cout << "no interaction allowed by session manager...\n";
    // we did not get permission to interact, then
    // do something reasonable instead.
    // we save our changes
    model->save();
  }
}

