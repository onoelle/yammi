/***************************************************************************
                          yammigui.cpp  -  description
                             -------------------
    begin                : Tue Feb 27 2001
    copyright            : (C) 2001 by Brian O.Nölle
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

// include pixmaps

// general
#include "icons/yammiicon.xpm"
#include "icons/in.xpm"
#include "icons/notin.xpm"
//#include "icons/filesave.xpm"

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


// dialog includes
#include "preferencesdialog.h"
#include "DeleteDialog.h"
#include "WorkDialogBase.h"
#include "updatedatabasedialog.h"
#include "updatedatabasemediadialog.h"

#include "mp3info/CMP3Info.h"


extern YammiGui* gYammiGui;


/**
 * constructor, sets up the whole application.
 * (maybe not nice??)
 */
YammiGui::YammiGui( QWidget *parent, const char *name )
    : QMainWindow( parent, name )
{
	gYammiGui=this;
  this->setIcon(QPixmap(yammiicon_xpm));
	
//  this->tbSaveDatabase=0;
  
  // set up model
	model=new YammiModel();
	cout << "starting Yammi, version " << model->config.yammiVersion << "\n";

  ensureXmmsIsRunning();
  
	model->readPreferences();						// read preferences
	model->readSongDatabase();					// read song database
	model->readCategories();						// read categories
	model->readHistory();								// read history
	

  // restore session settings
  readSettings();
  
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
	databaseMenu->insertItem( "Update Database...",  this, SLOT(updateSongDatabaseHarddisk()));
	databaseMenu->insertItem( "Scan Removable Media...",  this, SLOT(updateSongDatabaseMedia()));
	databaseMenu->insertItem( "Check Consistency...",  this, SLOT(forAllCheckConsistency()));
	databaseMenu->insertItem( "Grab and encode CD-Track...",  this, SLOT(grabAndEncode()));
	mainMenu->insertItem( "&Database", databaseMenu );
		
  // settings menu
  QPopupMenu* settingsMenu = new QPopupMenu;
  settingsMenu->insertItem( "Configure Yammi...",  this, SLOT(setPreferences()));
	mainMenu->insertItem( "&Settings", settingsMenu );

  // help menu	
	QPopupMenu* helpMenu = new QPopupMenu;
	helpMenu->insertItem( "&Handbook...",  this, SLOT(openHelp()));
	helpMenu->insertItem( "&About...", this, SLOT(aboutDialog()));
	mainMenu->insertItem( "&Help", helpMenu );
	
	
	
  // media player toolbar
  QToolBar* mediaPlayerToolBar = new QToolBar (this, "mediaPlayerToolbar");
  mediaPlayerToolBar->setLabel( "Media Player Controls" );
  tbPlayPause = new QToolButton( QPixmap((const char**)pause_xpm), "Play/Pause (F1)", QString::null,
                           this, SLOT(xmms_playPause()), mediaPlayerToolBar);
	new QToolButton( QPixmap((const char**)stop_xpm), "Stop", QString::null,
                           this, SLOT(xmms_stop()), mediaPlayerToolBar);
	new QToolButton( QPixmap((const char**)skipbackward_xpm), "Skip backward (F2 / SHIFT-F2)", QString::null,
 														this, SLOT(xmms_skipBackward()), mediaPlayerToolBar);
	new QToolButton( QPixmap((const char**)skipforward_xpm), "Skip forward (F3 / SHIFT-F3)", QString::null,
                           this, SLOT(xmms_skipForward()), mediaPlayerToolBar);
	songSlider = new QSlider( QSlider::Horizontal, mediaPlayerToolBar, "songLength" );
	songSlider->setTickmarks(QSlider::Below);
	songSlider->setFixedWidth(180);
  songSlider->setPaletteBackgroundColor(QColor(179, 218, 226));
	isSongSliderGrabbed=false;
	connect( songSlider, SIGNAL(sliderReleased()), SLOT(songSliderMoved()) );
	connect( songSlider, SIGNAL(sliderPressed()), SLOT(songSliderGrabbed()) );
		
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
	new QToolButton (QPixmap(enqueue_xpm), "Enqueue at end (F5)", QString::null,
                           this, SLOT(forAllSelectedEnqueue()), toolBar2);
	new QToolButton (QPixmap(enqueueasnext_xpm), "Enqueue as next (F6)", QString::null,
                           this, SLOT(forAllSelectedEnqueueAsNext()), toolBar2);
	new QToolButton (QPixmap(playnow_xpm), "Play now (F7 / SHIFT-F7)", QString::null,
                           this, SLOT(forAllSelectedPlayNow()), toolBar2);
	new QToolButton (QPixmap(dequeueSong_xpm), "Dequeue Song (F8)", QString::null,
                           this, SLOT(forAllSelectedDequeue()), toolBar2);
	new QToolButton (QPixmap(dequeueAll_xpm), "Clear playlist (CTRL-F8)", QString::null,
                           this, SLOT(xmms_clearPlaylist()), toolBar2);
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
	folderActual->moveItem(folderAll);
	folderActual->update(&(model->songsToPlay));

	// folder containing songs played in this session
	folderSongsPlayed = new Folder(folderListView, QString("Songs Played"), &(model->songsPlayed));
	folderSongsPlayed->moveItem(folderActual);

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
	folderSearchResults->moveItem(folderAll);
	
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
//  connect( folderListView, SIGNAL( selectionChanged( QListViewItem* ) ),
//	     this, SLOT( slotFolderChanged() ) );
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
	folderListView->firstChild()->setOpen( TRUE );
	folderListView->setCurrentItem( folderListView->firstChild()->firstChild() );
	folderListView->setSelected( folderListView->firstChild(), TRUE );

	songListView->setSelected( songListView->firstChild(), TRUE );
	songListView->setCurrentItem( songListView->firstChild() );
	updateSongPopup();
	
//	updateListViewColumns();

	shuttingDown=0;
	controlPressed=false;
	shiftPressed=false;
	currentSong=0;
	
	// check whether xmms is in shuffle mode: if yes, set it to normal
	// (confuses Yammi's playlistmanagement)
	if(xmms_remote_is_shuffle(0)) {
		xmms_remote_toggle_shuffle(0);
		xmmsShuffleWasActivated=true;
		cout << "switching off xmms shuffle mode (does confuse my playlist management otherwise)\n";
	}
	else
		xmmsShuffleWasActivated=false;

	// check whether xmms is playing, if not: start playing!
	if(!xmms_remote_is_playing(0)) {
		if(xmms_remote_get_playlist_length(0)>0) {
			xmms_remote_play(0);
			cout << "xmms is not playing, starting it...\n";
		}
	}
	syncXmms2Yammi();
	checkPlaylistAvailability();

	// connect all timers
  connect( &regularTimer, SIGNAL(timeout()), SLOT(onTimer()) );
  regularTimer.start( 1000, FALSE );	// call onTimer once a second
	connect( &typeTimer, SIGNAL(timeout()), this, SLOT(searchFieldChanged()) );

	// finish!
  cout << "initialisation successfully completed!\n";

  mainStatusBar->message("Welcome to Yammi "+model->config.yammiVersion, 20000);
	slotFolderChanged();
	if(model->noPrefsFound && model->noDatabaseFound) {
		QMessageBox::information( this, "Yammi",	QString("Yammi - Yet Another Music Manager I...\n\n\n")+
																					"It looks like you are starting Yammi the first time...\n\n"+
																					"Please edit the preferences (File Menu -> Preferences)\n"+
																					"to adjust the path configuration and all other options,\n"+
																					"then perform a database update (File Menu -> Update Database)\n"+
																					"to scan your harddisk for mp3 files.\n\n"+
																					"have fun...\n\n"+
																					"Check out Yammi's website for new versions and other info:\n"+
																					"http://yammi.sourceforge.net");																					
	}
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
  updateGeometrySettings();
  writeSettings();
	syncYammi2Xmms(true);
	if(xmmsShuffleWasActivated)
		xmms_remote_toggle_shuffle(0);
	if(model->allSongsChanged() || model->categoriesChanged()) {
		QString msg="Save changes?\n\n";
		msg+="If you don't save, all changes will be lost\n";
		msg+="and the database might become inconsistent...\n";
		if( QMessageBox::warning( this, "Yammi", msg, "Yes", "No")==0)
			model->save();
	}
	else {
		// we only save history if there are more than 2 songs to add
		if(model->config.logging && model->songsPlayed.count()>2)
			model->saveHistory();
	}
	cout << "goodbye!\n";
}




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
}

/// restores session settings
void YammiGui::readSettings()
{
  QSettings settings;
  settings.insertSearchPath( QSettings::Unix, model->config.yammiBaseDir );
  int posx = settings.readNumEntry( "/Yammi/geometry/posx", 0 );
  int posy = settings.readNumEntry( "/Yammi/geometry/posy", 0 );
  int width = settings.readNumEntry( "/Yammi/geometry/width", 1024 );
  int height = settings.readNumEntry( "/Yammi/geometry/height", 468 );
  setGeometry(QRect(posx, posy, width, height));
  columnOrder=settings.readListEntry("/Yammi/songlistview/columnOrder");
  if(columnOrder.count()==0)
    cout << "no column order found\n";
  for(int i=0; i<MAX_COLUMN_NO; i++) {
    columnWidth[i]=settings.readNumEntry("/Yammi/songlistview/column"+QString("%1").arg(i)+"Width");
  }
  // todo: anything else we want to restore?
  // (currently opened folder, ...)
}












/// Update the internal geometry settings.
void YammiGui::updateGeometrySettings()
{
  cout << "updating geomertry: x(): " << x() << ", y(): " << y() << ", width(): " << width() << ", height(): " << height() << "\n";
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
	folderMedia->update(&(model->allSongs));
	
	model->unclassifiedSongs.clear();
	for(Song* s=model->allSongs.firstSong(); s; s=model->allSongs.nextSong()) {
		if(!s->classified)
			model->unclassifiedSongs.appendSong(s);
	}
	folderUnclassified->update(&(model->unclassifiedSongs));
	slotFolderChanged();
}


/** Updates listview columns if necessary.
 * Tries to maintain the size and position of columns (as good as possible, as columns are changing)
 */
void YammiGui::updateListViewColumns(Folder* oldFolder, Folder* newFolder)
{
  if(oldFolder==newFolder)
    return;

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
	songListView->setColumnAlignment( offset+5, Qt::AlignRight );
	songListView->addColumn( "Added to", 60);
	songListView->setColumnAlignment( offset+6, Qt::AlignRight );
	songListView->addColumn( "Bitrate", 40);
	songListView->setColumnAlignment( offset+7, Qt::AlignRight );
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
  for(int x=0; x<MAX_COLUMN_NO; x++)
    exists[x]=false;
  
  int target=0;
  for(int i=0; i<(int)columnOrder.count(); i++) {
    QString search=columnOrder[i];
    for(int j=0; j<noNewColumns; j++) {
      if(header->label(j)==search) {
//        cout << "found: i: " << i << ", j: " << j << "label: " << header->label(j) << "\n";
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
//      cout << "not existing: " << header->label(j) << "\n";
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
//    cout << "j: " << j << ", columnWidth: " << columnWidth[j] << ", label: " << header->label(section) << "\n";
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
	for(unsigned int i=0; i<model->categoryNames.count(); i++) {
		playListPopup->insertItem(QIconSet( QPixmap(in_xpm)), model->categoryNames[i], this, SLOT(toPlayList(int)), 0, 10000+i);
	}
		
	// define popup menu for songs
	songPopup = new QPopupMenu( songListView );
	songPopup->insertItem( "Song Name", 113 );
	songPopup->insertSeparator();
	
	songPlayPopup = new QPopupMenu(songPopup);
	
	songPlayPopup->insertItem(getPopupIcon(Enqueue), "...Enqueue", this, SLOT(forSelection(int)), 0, Enqueue);		
	songPlayPopup->insertItem(getPopupIcon(EnqueueAsNext), "...Play as next", this, SLOT(forSelection(int)), 0, EnqueueAsNext);
	songPlayPopup->insertItem(getPopupIcon(PlayNow), "...Play now!", this, SLOT(forSelection(int)), 0, PlayNow);
	songPlayPopup->insertItem(getPopupIcon(Dequeue), "...Dequeue", this, SLOT(forSelection(int)), 0, Dequeue);
	songPopup->insertItem( "Play...", songPlayPopup);
	
	if(model->config.secondSoundDevice!="") {
		songPrelistenPopup = new QPopupMenu(songPopup);
		songPrelistenPopup->insertItem(getPopupIcon(PrelistenStart), "...start", this, SLOT(forSelection(int)), 0, PrelistenStart);
		songPrelistenPopup->insertItem(getPopupIcon(PrelistenMiddle), "...middle", this, SLOT(forSelection(int)), 0, PrelistenMiddle);
		songPrelistenPopup->insertItem(getPopupIcon(PrelistenEnd), "...end", this, SLOT(forSelection(int)), 0, PrelistenEnd);
		songPopup->insertItem( "Prelisten to...", songPrelistenPopup);
	}
	songPopup->insertItem(getPopupIcon(SongInfo), "Info...", this, SLOT(forSelection(int)), 0, SongInfo);
	songPopup->insertItem( "Insert Into...", playListPopup);

	songSearchPopup = new QPopupMenu(songPopup);
	songSearchPopup->insertItem( "Entry", this, SLOT(searchSimilar(int)), 0, 1000);
	songSearchPopup->insertItem( "Artist", this, SLOT(searchSimilar(int)), 0, 1001);
	songSearchPopup->insertItem( "Title", this, SLOT(searchSimilar(int)), 0, 1002);
	songSearchPopup->insertItem( "Album", this, SLOT(searchSimilar(int)), 0, 1003);
	songPopup->insertItem( "Search for similar...", songSearchPopup);
	
	if(model->config.childSafe)
		return;
	
	songAdvancedPopup = new QPopupMenu(songPopup);
	songAdvancedPopup->insertItem( "Delete...", this, SLOT(forSelection(int)), 0, Delete);
	songAdvancedPopup->insertItem( "Move file to...", this, SLOT(forSelection(int)), 0, MoveTo);
	songAdvancedPopup->insertItem( "Check Consistency", this, SLOT(forSelection(int)), 0, CheckConsistency);
	songAdvancedPopup->insertItem( "Burn to Media...", this, SLOT(forSelection(int)), 0, BurnToMedia);	
	songPopup->insertItem( "Advanced...", songAdvancedPopup);
	
	
	pluginPopup = new QPopupMenu(songPopup);
	for(unsigned int i=0; i<model->config.pluginMenuEntry->count(); i++) {
		pluginPopup->insertItem( (*(model->config.pluginMenuEntry))[i], this, SLOT(forSelectionPlugin(int)), 0, 2000+i);
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
	slotFolderChanged();
	searchField->setText("");	
}

/**
 * adds all selected songs to the category (specified by index)
 * if current song is already in category => removes all selected from that category (if they are in)
 */
void YammiGui::toPlayList(int index)
{
	index-=10000;
	// choose the desired category
	MyList* category=model->allCategories.first();
	for(int i=0; i<index; i++) {
		category=model->allCategories.next();
	}
	QString chosen=model->categoryNames[index];
	cout << "chosen1: " << chosen << "\n";
	
	// determine mode (add/remove)
	bool remove=false;
	Song* s=selectedSongs.firstSong();
	// for all songs contained in that category...
	for(Song* tmp=category->firstSong(); tmp; tmp=category->nextSong()) {
		if(s==tmp) {
			remove=true;
			break;
		}
	}

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
		if(!remove)
			categoryFolder->addSong(s);
		else
			categoryFolder->removeSong(s);
	}
	
	model->categoriesChanged(true);
	
//hh
/*		
	if( chosenFolder->folderName()==chosen ) {
		QString chosen2=chosenFolder->folderName();
		cout << "chosen2: " << chosen2 << "\n";
		// here we lose the chosenFolder (as it is deleted and re-inserted)
		folderCategories->update(model->allCategories, model->categoryNames);
		// so count until we have it again & select
		for( QListViewItem* f=folderCategories->firstChild(); f; f=f->nextSibling() ) {
			if( ((Folder*)f)->folderName()==chosen ) {
				folderListView->setCurrentItem(f);   //f->setSelected(true);
				chosenFolder=(Folder*)f;
				cout << "selected\n";
			}
		}
		slotFolderChanged();
	}
	else {
		folderCategories->update(model->allCategories, model->categoryNames);
	}
	*/
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
	
	folderListView->setCurrentItem( folderSearchResults );
	folderListView->setSelected( folderSearchResults, TRUE );
	slotFolderChanged();
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
 * eg. user clicked on a folder, or folder content changed
 */
void YammiGui::slotFolderChanged()
{
  QApplication::setOverrideCursor( Qt::waitCursor );
	Folder* oldFolder=chosenFolder;
	QListViewItem *i = folderListView->currentItem();
	if ( !i )
		return;
	Folder* newFolder=(Folder*)i;
	
	if(newFolder==folderActual)
		songListView->dontTouchFirst=true;				// don't allow moving the first
	else
		songListView->dontTouchFirst=false;
	
	chosenFolder = newFolder;
	songListView->clear();
	songListView->sortedBy=1;
//	songListView->setSorting(0);
	updateListViewColumns(oldFolder, newFolder);
	addFolderContent(chosenFolder);
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
		songListView->setSorting(0);
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
		if(category->containsSong(first)>0)
			playListPopup->changeItem(10000+k, QIconSet( QPixmap(in_xpm)), playListPopup->text(10000+k));
		else
			playListPopup->changeItem(10000+k, QIconSet( QPixmap(notin_xpm)), playListPopup->text(10000+k));
	}
		
 	// for songs not on local harddisk: disable certain menu entries
 	// only if exactly one song selected!
 	bool enable=true;
 	if(selected==1 && first->filename=="")
 		enable=false;
		
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
		cout << "no songs in this folder\n";
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

  bool confirm=((*model->config.pluginConfirm)[pluginIndex]=="true");
  QString mode=((*model->config.pluginMode)[pluginIndex]);
  QString cmd=(*model->config.pluginCommand)[pluginIndex];
  if(cmd.contains("%X")>0) {
    // let user choose directory (we should provide a starting directory???)
    QString dir=QFileDialog::getExistingDirectory(QString(""), this, QString("yammi"), QString("choose directory for plugin"), true);
    if(dir.isNull())
      return;
//      if(dir.right(1)=="/")						// strip trailing slash
//        dir=dir.left(dir.length()-1);
    cmd.replace(QRegExp("%X"), dir);
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
      QString entry=(*model->config.pluginCustomList)[pluginIndex];
      customList+=makeReplacements(entry, s, index);
    }

    // custom list can be long => better put it into a file...
    QFile customListFile(model->config.yammiBaseDir+"/customlist.txt");
    customListFile.open(IO_WriteOnly);
    customListFile.writeBlock( customList, qstrlen(customList) );
    customListFile.close();
    cmd.replace(QRegExp("%l"), "`cat "+model->config.yammiBaseDir+"/customlist.txt`");

    if(confirm) {
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
  input.replace(QRegExp("%f"), s->location());
	input.replace(QRegExp("%F"), s->filename);
	input.replace(QRegExp("%p"), s->path);
	input.replace(QRegExp("%a"), s->artist);
	input.replace(QRegExp("%t"), s->title);
	input.replace(QRegExp("%u"), s->album);
	input.replace(QRegExp("%b"), QString("%1").arg(s->bitrate));
	input.replace(QRegExp("%i"), QString("%1").arg(index));
	QString lengthStr=QString("%1").arg(s->length % 60);
	if (lengthStr.length()==1)
	 	lengthStr="0"+lengthStr;
	input.replace(QRegExp("%l"), QString("%1:%2").arg((s->length) / 60).arg(lengthStr));
  input.replace(QRegExp("%n"), "\n");
	QString mediaList="";
	for(unsigned int i=0; i<s->mediaName.count(); i++) {
		if(i!=0)
			mediaList+=", ";
		mediaList+=s->mediaName[i];
	}
	input.replace(QRegExp("%m"), mediaList);
  return input;  
}

/*
// creates a playlist (or a space-seperated list of songs) and executes
// a command on it (eg. mp3burn)
void YammiGui::forSelectionPluginPlaylist(int pluginIndex)
{
	pluginIndex-=3000;
	
}

*/


/**
 * edit song info
 * (mass editing)
 */
void YammiGui::forSelectionSongInfo()
{
	QString _artist, _title, _album, _comment, _path, _filename, _year, _trackNr, _bitrate;
	MyDateTime _addedTo;
	int _length=0;
	long double _size=0;
	int _genreNr=0;
		
	int selected=0;
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
		}
		else {
			if(_addedTo!=s->addedTo)					_addedTo=MyDateTime();
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
	MyDateTime d=_addedTo;
	if(d.isValid())
		si.LineEditAddedTo->setText(d.writeToString());
	else
		si.LineEditAddedTo->setText("!");	
	si.ReadOnlyPath->setText(_path);
	si.ReadOnlyFilename->setText(_filename);
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
	long double totalSize=-model->config.criticalSize*1024*1024+1;
	long double size=model->config.criticalSize*1024*1024+1;				// start with a full medium

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

	
	int mediaNo=atoi(startIndexStr)-1;
	QString mediaName=QString("%1_%2").arg(collName).arg(mediaNo);
	QString mediaDir=QString(model->config.yammiBaseDir+"/media/"+mediaName);
	long double sizeLimit=(long double)model->config.criticalSize*1024.0*1024.0;
	int count=0;
	for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong(), count++) {
	  progress.setProgress(count);
  	qApp->processEvents();
		if(progress.wasCancelled())
			break;

		QFileInfo fi(s->location());
		if(size+fi.size()>sizeLimit) {
			// medium is full, prepare new one
			mediaNo++;
			mediaName=QString("%1_%2").arg(collName).arg(mediaNo);
			mediaDir=QString(model->config.yammiBaseDir+"/media/"+mediaName);
			progress.setLabelText("Preparing media "+mediaName+"...");
			QDir dir;
			dir.mkdir(mediaDir);
			totalSize+=size;
			size=0;
		}
		size+=fi.size();
		// linux specific
		QString cmd=QString("ln -s \"%1\" \"%3/%4\"").arg(s->location()).arg(mediaDir).arg(s->filename);
		system(cmd);
		s->addMediaLocation(mediaName, s->filename);
	}
	totalSize+=size;				// add last (half full) media
	
	cout << "number media: " << mediaNo << " (critical size: " << model->config.criticalSize << " MB)\n";
	cout << "size of last media: " << size << " (=" << size/(1024*1024) << " MB)\n";
	cout << "size in total: " << totalSize << " (=" << totalSize/(1024*1024) << " MB)\n";
	folderMedia->update(&(model->allSongs));
	model->allSongsChanged(true);
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
	QProgressDialog progress( "Checking consistency...", "Cancel", 100, this, "progress", TRUE );
	progress.setMinimumDuration(0);
	progress.setAutoReset(false);
  progress.setProgress(0);
	qApp->processEvents();	
	model->checkConsistency(&progress);
	folderProblematic->update(&model->problematicSongs);
	QMessageBox::information( this, "Yammi",
	QString("Result of consistency check:\n\n %1 songs were found problematic (check in folder Problematic Songs)\n (Folder Problematic Songs won't be saved)")
		.arg(model->problematicSongs.count()), "Fine." );	
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
	// end of special treatment
	
	// 1. destination directory
	QString dir;
	if(act==MoveTo) {
		// let user choose directory (we should provide a starting directory???)
		dir=QFileDialog::getExistingDirectory(QString(""), this, QString("yammi"), QString("choose directory"), true);
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
	
	
	// some OPs need view update
	if(deleteEntry) {
		updateView();
	}
	if(deleteFile) {
		slotFolderChanged();
	}
	if(act==Enqueue || act==EnqueueAsNext || act==Dequeue) {
		syncYammi2Xmms();
		if(chosenFolder==folderActual)
			slotFolderChanged();
		else
			songListView->triggerUpdate();
		checkPlaylistAvailability();
	}

	if(act==SongInfo) {
		slotFolderChanged();
	}
	if(act==CheckConsistency) {
		folderProblematic->update(&(model->problematicSongs));
		slotFolderChanged();
	}
}

/**
 * writes all songs found in xmms playlist to yammi's playlist
 * also, clears xmms playlist except for the first config.keepInXmms
 */
void YammiGui::syncXmms2Yammi()
{
	// 1. delete all songs already played
	for(int i=xmms_remote_get_playlist_pos(0)-1; i>=0; i--) {
//		cout << QString("xmms_remote_playlist_delete %1").arg(i) << "\n";
		xmms_remote_playlist_delete(0, i);
//		cout << "..done\n";
	}
	// 2. insert all (including currently played) songs into yammi playlist
	folderActual->clearSongs();
	for(int i=0; i<xmms_remote_get_playlist_length(0); i++) {
 		char buf[200];
//		cout << QString("xmms_remote_get_playlist_file %1").arg(i) << "\n";
		strcpy(buf, xmms_remote_get_playlist_file(0, i));
//		cout << "..done\n";
		Song* check=getSongFromFilename(QString(buf));
		if(!check)	// song not found in database
			continue;
		folderActual->addSong(check);
	}
	// 3. delete all but the keepInXmms first songs
	for(int i=xmms_remote_get_playlist_length(0)-1; i>=model->config.keepInXmms; i--) {
//		cout << QString("xmms_remote_playlist_delete %1").arg(i) << "\n";
		xmms_remote_playlist_delete(0, i);
//		cout << "..done\n";
	}
}

/**
 * tries to sync the xmms playlist with the yammi playlist
 * ie. writes all entries from yammi playlist into xmms playlist
 */
void YammiGui::syncYammi2Xmms(bool syncAll)
{
	// check whether xmms playlist is empty
	if(xmms_remote_get_playlist_length(0)==0) {
		for(int i=0; i<(int)model->songsToPlay.count() && (i<model->config.keepInXmms || syncAll); i++) {
			gchar url[300];
			strcpy(url, model->songsToPlay.at(i)->song()->location());
//			cout << QString("xmms_remote_playlist_add_url_string %1").arg(url) << "\n";
			xmms_remote_playlist_add_url_string(0, url);
//			cout << "..done\n";
		}
		return;
	}
	
	// okay, one or more songs in xmms playlist
		
	// iterate through first <keepInXmms> or all (syncAll) songs in xmms playlist
	// if playlist too short => insert yammi entries
	int iXmms=0;
	int iYammi=0;
	for(; iXmms<model->config.keepInXmms || ( syncAll && iYammi<(int)model->songsToPlay.count() ); iXmms++, iYammi++) {
		
		// check whether xmms playlist entry existing
		if(iXmms<(int)xmms_remote_get_playlist_length(0)) {
			// yes, existing => compare yammi and xmms entry
	 		char buf[300];
//			cout << QString("xmms_remote_get_playlist_file %1").arg(iXmms) << "\n";
			strcpy(buf, xmms_remote_get_playlist_file(0, iXmms));
//			cout << "..done\n";
			Song* check=getSongFromFilename(QString(buf));
			
			// corresponding yammi entry existing?
			if(iYammi<(int)model->songsToPlay.count()) {
				Song* s=model->songsToPlay.at(iYammi)->song();
				if(check==s)
					continue;		// okay, both are the same
				
				// song mismatch between yammi and xmms
				
				// case 1: xmms song not in yammi database
				// => leave unknown song
				if(check==0) {
					iYammi--;
					continue;
				}
				
				// case 2: xmms song is yammi+1 (some song moved in front of it)
				// => insert the song that was inserted
				if(iYammi+1<(int)model->songsToPlay.count() && check==model->songsToPlay.at(iYammi+1)->song()) {
					// check if songfile is available...
					QString loc=checkAvailability(s);
					if(loc=="" || loc=="never") {
						iXmms--;
						continue;
					}
					gchar url[300];
					strcpy(url, loc);
//					cout << QString("xmms_remote_playlist_ins_url_string %1 %2").arg(url).arg(iXmms) << "\n";
					xmms_remote_playlist_ins_url_string(0, url, iXmms);
//					cout << "..done\n";
					continue;
				}
				
				// case 3: xmms+1 song is yammi (song removed from there) => delete
//				cout << QString("xmms_remote_playlist_delete %1").arg(iXmms) << "\n";
				xmms_remote_playlist_delete(0, iXmms);
//				cout << "..done\n";
				iXmms--;
				iYammi--;
				
			}
			else {		// yammi playlist too short
				if(check==0)
					cout << "xmms playlist longer than yammi playlist, but unknown song\n";
				else {
					cout << "xmms playlist longer than yammi playlist, deleting\n";
					xmms_remote_playlist_delete(0, iXmms);
					myWait(50);
					iXmms--;
					continue;
				}
			}
		}
		else {			// xmms playlist too short => check whether songs in songsToPlay
			if(iYammi<(int)model->songsToPlay.count()) {
//				cout << "trying to fill up xmms playlist with song from yammi\n";
				Song* s=model->songsToPlay.at(iYammi)->song();
				// check if songfile is available...
				QString loc=checkAvailability(s);
				if(loc=="" || loc=="never") {
					iXmms--;
					continue;
				}
				gchar url[300];
				strcpy(url, loc);
//				cout << QString("xmms_remote_playlist_add_url %1").arg(url) << "\n";
				xmms_remote_playlist_add_url_string(0, url);
//				cout << "..done\n";
				myWait(50);
			}
		}
	} // end of for
	
	// now process leftover songs in xmms playlist
	for(; iXmms<(int)xmms_remote_get_playlist_length(0); ) {
 		char buf[300];
//		cout << QString("xmms_remote_get_playlist_file %1").arg(iXmms) << "\n";
		strcpy(buf, xmms_remote_get_playlist_file(0, iXmms));
//		cout << "..done\n";
		Song* check=getSongFromFilename(QString(buf));
		if(check==0)
			continue;
//		cout << QString("xmms_remote_playlist_delete %1").arg(iXmms) << "\n";
		xmms_remote_playlist_delete(0, iXmms);
//		cout << "..done\n";
		myWait(50);
	}
	
	// if xmms is not playing, we might have inserted songs before the active one
	// => set active song to first
	// caution! xmms reports as not playing sometimes (immediately after skip forward?)
/*	
	if(!xmms_remote_is_playing(0)) {
		xmms_remote_set_playlist_pos(0, 0);
	}
*/
	
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
    ensureXmmsIsRunning();
		if(s->filename=="" || !s->checkReadability()) {
			cout << "song not available (try to first enqueue and load from a media)\n";
			return;
		}

		forSong(s, EnqueueAsNext);

    if(xmms_remote_is_playing(0)) {
      // xmms playing or playing but paused
      if(!xmms_remote_is_paused(0)) {
        // case 1: xmms is playing
        xmms_skipForward();
      }
      else {
        // case 2: xmms is paused => start playing
        xmms_skipForward();
        xmms_remote_play(0);
      }
    }
    else {
      // case 3: xmms is stopped (this is a bit dirty...)
      xmms_remote_set_playlist_pos(0, 0);
      xmms_remote_play(0);
    }
			
		mainStatusBar->message(QString("playing %1").arg(s->displayName()), 2000);
		break;
		
	case Enqueue:								// enqueue at end
    ensureXmmsIsRunning();
		folderActual->addSong(s);
		mainStatusBar->message(QString("%1 enqueued at end").arg(s->displayName()), 3000);
		break;
				
	case EnqueueAsNext: {				// enqueue as next
    ensureXmmsIsRunning();
		// songsToPlay is empty, or first song is still to play
		if(model->songsToPlay.count()==0 || currentSong!=model->songsToPlay.at(0)->song())
			model->songsToPlay.insert(0, new SongEntryInt(s, 13));
		else
			model->songsToPlay.insert(1, new SongEntryInt(s, 13));
		folderActual->correctOrder();
		syncYammi2Xmms();
		mainStatusBar->message(QString("%1 enqueued as next").arg(s->displayName()), 2000);
	}
		break;
	
	case Dequeue: {
		// search for selected song and dequeue
		int i=1;
		if(!xmms_remote_is_playing(0))
			i=0;
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
	
	case DeleteEntry:							// delete db entry
		folderAll->removeSong(s);
		model->allSongsChanged(true);
		break;
	
	case DeleteFile:						// move songfile to trash
		s->deleteFile(model->config.trashDir);
		model->allSongsChanged(true);
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
	system("konqueror /opt/kde3/share/doc/HTML/en/yammi/index.html &");
}

/// display about dialog
void YammiGui::aboutDialog()
{
	QMessageBox::information( this, "Yammi",	QString("Yammi - Yet Another Music Manager I...\n\n\n")+
																					"Version "+model->config.yammiVersion+", 12-2001 - 8-2002 by Oliver Nölle\n\n"+
																					"Project home page: yammi.sourceforge.net\n\n\n"+
																					"Contact: oli.noelle@web.de\n\n"+
																					"have fun...\n");
}


/** creates a new category */
void YammiGui::newCategory(){
	bool ok = false;
	QString caption("Enter name for category");
	QString message("Please enter name of category");
	QString newName=QString(QInputDialog::getText( caption, message, QLineEdit::Normal, QString("new category"), &ok, this ));
	if(ok) {
		model->newCategory(newName);
		folderCategories->update(model->allCategories, model->categoryNames);
		updateSongPopup();		
	}
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



// check whether xmms is running, if not: start it!
void YammiGui::ensureXmmsIsRunning()
{
	if(!xmms_remote_is_running(0)) {
		cout << "xmms not running, trying to start it...\n";
		system("xmms > /dev/null &");
	}
}



/// toggle between play and pause
void YammiGui::xmms_playPause()
{
  ensureXmmsIsRunning();
	xmms_remote_play_pause(0);
}


/// skip forward in playlist (if controlPressed without crossfading)
void YammiGui::xmms_skipForward()
{
  ensureXmmsIsRunning();
  if(shiftPressed) {
    xmms_remote_pause(0);
  }
  int x= xmms_remote_get_playlist_pos(0);
	xmms_remote_set_playlist_pos(0, x+1);
  if(shiftPressed) {
    xmms_remote_play(0);
  }
}


/// skip backward in playlist (if controlPressed without crossfading)
void YammiGui::xmms_skipBackward()
{
  ensureXmmsIsRunning();
  if(shiftPressed) {
    xmms_remote_pause(0);
  }
  int count=model->songsPlayed.count();
	if(count==0)			// empty folder songsPlayed => can's skip backwards
		return;
	
	// 1. get and remove last song from songsPlayed
	Song* last=model->songsPlayed.at(count-1)->song();
	model->songsPlayed.remove(count-1);
	folderSongsPlayed->updateTitle();
//	cout << "last: " << last->displayName() << "\n";
	
	int pos=xmms_remote_get_playlist_pos(0);
	gchar url[500];
	strcpy(url, last->location());
	xmms_remote_playlist_ins_url_string(0, url, pos);
	xmms_remote_set_playlist_pos(0, pos);
	currentSong=0;
	folderActual->insertSong(last, 0);
	
	// update necessary?
	if(chosenFolder==folderActual || chosenFolder==folderSongsPlayed)
		slotFolderChanged();
	else
		songListView->triggerUpdate();

  if(shiftPressed) {
    xmms_remote_play(0);
  }
}

/// stop playback
void YammiGui::xmms_stop()
{
  ensureXmmsIsRunning();
	xmms_remote_stop(0);
}

/// clear all playlist items except currently played song
void YammiGui::xmms_clearPlaylist()
{
  ensureXmmsIsRunning();
	if(model->config.childSafe)
		return;
	Song* save=0;
	if(currentSong!=0 && xmms_remote_is_playing(0) && model->songsToPlay.count()>1) {
    if( QMessageBox::warning( this, "Yammi", "Clear complete playlist?\n(except currently played song)", "Yes", "No")!=0)
      return;
    save=model->songsToPlay.firstSong();
  }
  else {
    if( QMessageBox::warning( this, "Yammi", "Clear complete playlist?\n(including currently played song)", "Yes", "No")!=0)
      return;
  }
	model->songsToPlay.clear();
	if(save!=0)
		folderActual->addSong(save);
	else
		folderActual->updateTitle();
	syncYammi2Xmms();
	if(chosenFolder==folderActual)
		slotFolderChanged();
	else
		songListView->triggerUpdate();
}

/// called whenever user grabs the songSlider
/// causes xmms to jump to the given song position
void YammiGui::songSliderGrabbed()
{
	isSongSliderGrabbed=true;
}

/// called whenever user released the songSlider
/// causes xmms to jump to the given song position
void YammiGui::songSliderMoved()
{
	isSongSliderGrabbed=false;
	xmms_remote_jump_to_time(0, songSlider->value());
}

/**
 * onTimer is called periodically to do some things independently of any user action
 * - logging
 * - updating folderActual
 */
void YammiGui::onTimer()
{	
//	cout << "calling onTimer\n";
	// perform these actions only if xmms is playing
	if(xmms_remote_is_playing(0)) {
  	
		// check whether currently played song has changed
 		char file[300];
  	int pos=xmms_remote_get_playlist_pos(0);
		strcpy(file, xmms_remote_get_playlist_file(0, pos));
				
    // adjust songSlider (if user is not currently dragging it around)
		gint outputTime=outputTime=xmms_remote_get_output_time(0);
		if(!isSongSliderGrabbed && !xmms_remote_is_paused(0))
			songSlider->setValue(outputTime);

    if(xmms_remote_is_paused(0))
      tbPlayPause->setIconSet(QIconSet(QPixmap((const char**)play_xpm)));
    else
      tbPlayPause->setIconSet(QIconSet(QPixmap((const char**)pause_xpm)));
    
		
		/*	some xmms statistics...
		gint len=xmms_remote_get_playlist_length(0);
		gint pTime=xmms_remote_get_playlist_time(0, 0);
		gint rate, freq, nch;
		xmms_remote_get_info(0, &rate, &freq, &nch);
		cout << "outputTime: " << outputTime << ", length: " << len << ", pTime: "
		<< pTime << ", rate: " << rate << ", freq: " << freq << ", nch " << nch << "\n";
		*/
		

		
		if(currentFile!=file) {
		// *** song change detected ***
		// ****************************
			if(songListView->dragging)
				stopDragging();
			
			currentFile=file;
			if(songsUntilShutdown>0) {
				songsUntilShutdown--;
				sleepModeSpinBox->setValue(songsUntilShutdown);
				if(songsUntilShutdown==0) {
					cout << "shutting down now...\n";
					shutDown();
				}
			}
			
			// check, whether we put last song in folder songsPlayed
			if(currentSong!=0) {
				MyDateTime now;
//				int playTime=currentSongStarted.secsTo(now);
//				cout << "song length: " << currentSong->length << "\n";
//				cout << "playTime: " << playTime << "\n";
//				int x=100*(playTime+10)/currentSong->length;
//				cout << "quotient x: " << x << "\n";
				if(true) {
					// eg. more than 60% of the song was played => put it into songsPlayed
					SongEntryTimestamp* entry=new SongEntryTimestamp(currentSong, &currentSongStarted);
					currentSong->lastPlayed=entry->timestamp;
					folderSongsPlayed->addEntry(entry);		// append to songsPlayed
				}
			}
			currentSong=getSongFromFilename(QString(file));
			currentSongStarted=currentSongStarted.currentDateTime();
			// setup songSlider
//			cout << QString("xmms_remote_get_output_time\n");
			gint outputTime=outputTime=xmms_remote_get_output_time(0);
//			cout << "..done\n";
			
//			cout << QString("xmms_remote_get_playlist_time %1\n").arg(pos);
			gint pTime=xmms_remote_get_playlist_time(0, pos);
//			cout << "..done\n";
			songSlider->setRange(0, pTime);
			songSlider->setTickInterval(1000*60);

			// song entry found
			if(currentSong!=0) {		  					
				// set title to currently played song
				setCaption("Yammi: "+currentSong->displayName());
			}
			else {				// song not found in database
				setCaption("Yammi - song not in database");
			}	
			
			// remove played song(s) from xmms and yammi playlist
		  for(int i=0; true ; i++) {
				int check=xmms_remote_get_playlist_pos(0);
//				cout << "check: " << check << "\n";
				if(!check>0)
				 	break;
//				cout << "removing played songs: " << i << "\n";
//				cout << QString("xmms_remote_get_playlist_file 0\n");
				strcpy(file, xmms_remote_get_playlist_file(0, 0));
//				cout << "..done, returned: " << file << "\n";
				Song* x=getSongFromFilename(QString(file));
//				cout << QString("__xmms_remote_playlist_delete 0\n");
		  	
		  	// the following call sometimes seems to crash xmms
		  	// (and does not return until xmms is killed => freezes yammi)
		  	//************************************************************
		  	xmms_remote_playlist_delete(0, 0);
				myWait(100);
//				cout << "..done\n";
		  	if(x==model->songsToPlay.at(0)->song())
			 		model->songsToPlay.removeFirst();
		  }
		  folderActual->correctOrder();
		  	
		  // should just check and fill up, not change anything:
		  syncYammi2Xmms();

			// update view, if folderActual is currently shown folder
			if(chosenFolder==folderActual)
				slotFolderChanged();
			else
				songListView->triggerUpdate();					

		}
		// *** end of song change ***
		// **************************
  	
	}
	else {				// xmms not playing (after stop, NOT after pause)
    tbPlayPause->setIconSet(QIconSet(QPixmap((const char**)play_xpm)));
		setCaption("Yammi - XMMS not playing");
		currentFile="";
		currentSong=0;
		if(songsUntilShutdown>0) {
			// change this!!! ???
			// we assume end of playlist and therefore shutdown (almost) immediately
			songsUntilShutdown--;
			if(songsUntilShutdown<=0) {
				cout << "shutting down now...\n";
				shutDown();
			}
		}
	}
}




// finds out the corresponding song entry given a filename
// (now also takes care of songs in swap dir)
// returns 0 if no song entry found
Song* YammiGui::getSongFromFilename(QString filename)
{
	// strip filename to relative name
	int pos=filename.findRev('/', -1);
	QString path=filename.left(pos+1);
	QString lookFor=filename.right(filename.length()-pos-1);
	
	if(path==model->config.swapDir) {
		for(SongEntry* entry=model->allSongs.first(); entry; entry=model->allSongs.next()) {
			if(entry->song()->filename=="" && entry->song()->constructFilename()==lookFor)
				return entry->song();
		}
	}
	else {
		for(SongEntry* entry=model->allSongs.first(); entry; entry=model->allSongs.next()) {
			if(entry->song()->filename==lookFor)
				return entry->song();
		}
	}
	return 0;
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
	
	// linux specific
	QString filename=QString("%1%2 - %3.mp3").arg(model->config.scanDir).arg(artist).arg(title);
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
	model->addSongToDatabase(grabbedTrackFilename, 0);
	updateView();
	folderProblematic->update(&model->problematicSongs);
	folderAll->updateTitle();
  QString msg="Yammi tried to add the grabbed song to the database.\n\nSome statistics: \n\n";
  msg+=QString("%1 songs added to database\n").arg(model->entriesAdded);
  msg+=QString("%1 songs corrupt (=not added)\n").arg(model->corruptSongs);
  msg+=QString("%1 songs problematic (check in folder Problematic Songs)").arg(model->problematicSongs.count());
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
    
		xmms_remote_quit(0);						// properly close xmms => xmms will remember playlist
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
//      cout << "control pressed\n";
			break;
		case Key_Shift:
			shiftPressed=true;
//      cout << "shift pressed\n";
			break;
		case Key_F1:
			xmms_playPause();
			break;
		case Key_F2:
      xmms_skipBackward();
			break;
		case Key_F3:
 			xmms_skipForward();
			break;
		case Key_F4:
			xmms_stop();
			break;
		case Key_F5:
			forAllSelected(Enqueue);
			break;
		case Key_F6:
			forAllSelected(EnqueueAsNext);
			break;
		case Key_F7:
//			if(shiftPressed && xmms_remote_is_playing(0)) {
//   			xmms_remote_pause(0);
//      }
 			forAllSelected(PlayNow);
			break;
		case Key_F8:
			if(shiftPressed)
				xmms_clearPlaylist();
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
		case Key_Up: {
			QListViewItem* i=songListView->firstChild();
			for(; i; i=i->itemBelow()) {
				if(i->isSelected()) {
					if(i->itemAbove()) {
						i=i->itemAbove();
						songListView->clearSelection();
						songListView->setSelected(i, true);
					}
					break;
				}
			}
			} break;
		case Key_Down: {
			QListViewItem* i=songListView->firstChild();
			for(; i; i=i->itemBelow()) {
				if(i->isSelected()) {
					if(i->itemBelow()) {
						i=i->itemBelow();
						songListView->clearSelection();
						songListView->setSelected(i, true);
					}
					break;
				}
			}
			} break;
		
		case Key_F:		// Ctrl-F
			if(e->state()!=ControlButton)
				break;
			
		case Key_Escape:
			searchField->setText("");
			searchField->setFocus();
			break;
		
		case Key_S:		// Ctrl-S
			if(controlPressed) //e->state()!=ControlButton)
				break;
			model->save();
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
		cout << "shutting down, press Pause again to cancel...\n";
 		if(model->allSongsChanged() || model->categoriesChanged()) {
 			if( QMessageBox::warning( this, "Yammi", "Save changes?\n(answering no will cancel sleep mode)", "Yes", "No")==0)
 				model->save();
 			else {
 				shuttingDown=0;
 				songsUntilShutdown=-3;
				sleepModeButton->setText("(disabled)");
        sleepModeSpinBox->setEnabled(false);
 				cout << "shutting down aborted!\n";
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
 		cout << "shutting down cancelled!\n";
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
  bool checkExistence=d.CheckBoxExistenceCheck->isChecked();
  updateSongDatabase(checkExistence, scanDir, filePattern, 0);
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
  updateSongDatabase(false, mediaDir, filePattern, mediaName);
}



void YammiGui::updateSongDatabase(bool checkExistence, QString scanDir, QString filePattern, QString media)	
{
	QProgressDialog progress( "Scanning...", "Cancel", 100, this, "progress", TRUE );
	progress.setMinimumDuration(0);
	progress.setAutoReset(false);
  progress.setProgress(0);
	qApp->processEvents();
	
	model->updateSongDatabase(checkExistence, scanDir, filePattern, media, &progress);
	updateView();
	folderProblematic->update(&model->problematicSongs);
	folderAll->updateTitle();
  QString msg="Updated your database.\n\nStatistics: \n\n";
  msg+=QString("%1 songs added to database\n").arg(model->entriesAdded);
  msg+=QString("%1 songs corrupt (=not added)\n").arg(model->corruptSongs);
  msg+=QString("%1 songs problematic (check in folder Problematic Songs)\n").arg(model->problematicSongs.count());
  msg+=QString("%1 entries updated\n").arg(model->entriesUpdated);
  msg+=QString("%1 entries deleted\n").arg(model->entriesDeleted);
	QMessageBox::information( this, "Yammi", msg, "Fine." );
}


void YammiGui::stopDragging()
{
	((FolderSorted*)chosenFolder)->syncWithListView(songListView);
	slotFolderChanged();

	if(chosenFolder==folderActual) {
		syncYammi2Xmms();
	}
	
	if(((QListViewItem*)chosenFolder)->parent()==folderCategories) {
		// we have to save the order
		model->categoriesChanged(true);
	}
}


void YammiGui::myWait(int msecs)
{
	QTime t;
	t.start();
	while(t.elapsed()<msecs);
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

/** checks whether a song is available on the local harddisk
 * or needs to be retrieved from a removable media
 * if song available, returns the complete path+filename to the songfile
 * (if in swap dir, the file will be touched to implement the LRU strategy)
 * if not yet available, returns ""
 * if never available, returns "never"
 */
QString YammiGui::checkAvailability(Song* s, bool touch)
{
	if(s->location()!="/") {
		QFileInfo fi(s->location());
		if(fi.exists() && fi.isReadable()) {
			return s->location();
		}
//		cout << "song " << s->displayName() << "has location given, but file does not exist or is not readable!\n";
	}
	// no location given, check whether already existing in swap dir
	QString dir=model->config.swapDir;
	QString filename=s->constructFilename();
	QFileInfo fi(dir+filename);
	if(fi.exists() && fi.isReadable()) {
		if(touch) {
			// linux specific
			QString cmd;
			cmd=QString("touch \"%1\"").arg(dir+filename);
			system(cmd);
/*		does not work: touching a file
			QFile touchFile(dir+filename);
			if(!touchFile.open(IO_ReadWrite))
				cout << "could not touch songfile (for LRU method)\n";
			else {
				touchFile.flush();
				touchFile.close();
			}
*/
		}
		return dir+filename;
	}
	
	// not available, need to load it from media
	if(s->mediaLocation.count()!=0)
		return "";
	else
		return "never";
}


// if known media inserted, loads all songs occurring in playlist into swap dir
void YammiGui::loadSongsFromMedia(QString mediaName)
{	
	int songsToLoad=0;
	for(unsigned int i=1; i<model->songsToPlay.count(); i++) {
		Song* s=model->songsToPlay.at(i)->song();
		if(checkAvailability(s)=="")
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
		if(checkAvailability(s)=="") {
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
//					cmd=QString("mv \"%1%2.part\" \"%3%4\"").arg(swapDir).arg(filename).arg(swapDir).arg(filename);
//					system(cmd);
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
	
	syncYammi2Xmms();
	checkPlaylistAvailability();
	if(chosenFolder==folderActual) {
		slotFolderChanged();
	}	
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
			if(checkAvailability(s, true)=="") {				// this needs harddisk (slow)
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
