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
#include "clearPlaylist.xpm"
#include "pause.xpm"
#include "in.xpm"
#include "notin.xpm"
#include "filesave.xpm"
#include "skipforward.xpm"
#include "skipbackward.xpm"
#include "defaultDoubleClick.xpm"
#include "defaultMiddleClick.xpm"
#include "prelisten.xpm"
#include "playnow.xpm"
#include "enqueue.xpm"
#include "enqueueasnext.xpm"
#include "dequeueSong.xpm"
#include "songinfo.xpm"
#include "stopPrelisten.xpm"

extern YammiGui* gYammiGui;


/**
 * constructor, sets up the whole application.
 * (maybe not nice??)
 */
YammiGui::YammiGui( QWidget *parent, const char *name )
    : QMainWindow( parent, name )
{
	gYammiGui=this;
	// check whether xmms is running, if not: start it!
	if(!xmms_remote_is_running(0)) {
		cout << "xmms not running, trying to start it...\n";
		system("xmms > /dev/null &");
	}
	
	// set up model
	model=new YammiModel();
	model->readPreferences();						// read preferences if available (on first start: set to defaults)
	model->readSongDatabase();					// read song database
	model->readCategories();						// read categories
	model->readHistory();								// read history
	
	// set up the gui
	//****************************
	cout << "setting up gui...\n";
	
	// set up menu
	mainMenu = new QMenuBar(this);
	
	// file menu
	fileMenu = new QPopupMenu;
	fileMenu->insertItem( "&Update Database..",  this, SLOT(updateSongDatabase()), CTRL+Key_U );
	// after update: 	folderAll->update(&allSongs);
	fileMenu->insertItem( "&Check Consistency..",  this, SLOT(forAllCheckConsistency()), CTRL+Key_C );
	//after update:	folderAll->update(&allSongs);
	//							slotFolderChanged();

	fileMenu->insertItem( "&New Category..",  this, SLOT(newCategory()), CTRL+Key_N );
	fileMenu->insertItem( "&Preferences..",  this, SLOT(setPreferences()), CTRL+Key_P );
	fileMenu->insertItem( "Grab CD-&Track..",  this, SLOT(enqueueCdTrack()), CTRL+Key_T );
	fileMenu->insertItem( "&Quit", this, SLOT(endProgram()), CTRL+Key_Q );
	mainMenu->insertItem( "&File", fileMenu );
	
	// view menu: sorting options
	viewMenu = new QPopupMenu;
	viewMenu->insertItem( "Update View",  this, SLOT(updateView()));
	mainMenu->insertItem( "&View", viewMenu );
	
	// help menu	
	helpMenu = new QPopupMenu;
	helpMenu->insertItem( "&Handbook..",  this, SLOT(openHelp()), CTRL+Key_H );
	helpMenu->insertItem( "&About..", this, SLOT(aboutDialog()), CTRL+Key_A );
	mainMenu->insertItem( "&Help", helpMenu );
	
	// toolbar
	toolBar = new QToolBar ( this, "toolbar label");
  toolBar->setLabel( "Operations" );
	tbSaveDatabase = new QToolButton( QPixmap(filesave_xpm), "Save database (Ctrl-S)", QString::null,
														model, SLOT(save()), toolBar);
	tbPause = new QToolButton( QPixmap(pause_xpm), "Play/Pause (F1)", QString::null,
                           this, SLOT(xmms_playPause()), toolBar);
	tbBackward = new QToolButton( QPixmap(skipbackward_xpm), "Skip backward (F2)", QString::null,
 														this, SLOT(xmms_skipBackward()), toolBar);
	tbForward = new QToolButton( QPixmap(skipforward_xpm), "Skip forward (F3)", QString::null,
                           this, SLOT(xmms_skipForward()), toolBar);
	tbClearPlaylist = new QToolButton (QPixmap(clearPlaylist_xpm), "Clear playlist (F4)", QString::null,
                           this, SLOT(xmms_clearPlaylist()), toolBar);
		
	QLabel *searchLabel = new QLabel(toolBar);
	searchLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	searchLabel->setText( "  Search: " );
	searchLabel->setFrameStyle( QFrame::NoFrame );
	searchField = new QLineEdit ( toolBar );
	connect( searchField, SIGNAL(textChanged(const QString&)), SLOT(userTyped(const QString&)) );
	searchField->setFocus();
	searchField->setFixedWidth(210);
	QToolTip::add( searchField, "fuzzy search (Ctrl-F)");
	
	// button "add to wishlist"	
	QPushButton* addToWishListButton=new QPushButton("add to wishlist", toolBar);
	// current song label
	QPushButton* currentSongLabel=new QPushButton("current song...", toolBar);	
	
	QToolBar* toolBar2 = new QToolBar ( this, "toolbar2");
  toolBar2->setLabel( "Operations2" );

	// now all the buttons that correspond to context menu entries
	tbEnqueue = new QToolButton (QPixmap(enqueue_xpm), "Enqueue at end (F5)", QString::null,
                           this, SLOT(forAllSelectedEnqueue()), toolBar2);
	tbEnqueueAsNext = new QToolButton (QPixmap(enqueueasnext_xpm), "Enqueue as next (F6)", QString::null,
                           this, SLOT(forAllSelectedEnqueueAsNext()), toolBar2);
	tbPlayNow = new QToolButton (QPixmap(playnow_xpm), "Play now (F7)", QString::null,
                           this, SLOT(forAllSelectedPlayNow()), toolBar2);
	tbDequeueSong = new QToolButton (QPixmap(dequeueSong_xpm), "Dequeue Song (F8)", QString::null,
                           this, SLOT(forAllSelectedDequeue()), toolBar2);
	tbPrelistenStart = new QToolButton (QPixmap(prelisten_xpm), "Prelisten (start) (F9)", QString::null,
                           this, SLOT(forAllSelectedPrelistenStart()), toolBar2);
	tbPrelistenMiddle = new QToolButton (QPixmap(prelisten_xpm), "Prelisten (middle) (F10)", QString::null,
                           this, SLOT(forAllSelectedPrelistenMiddle()), toolBar2);
	tbPrelistenEnd = new QToolButton (QPixmap(prelisten_xpm), "Prelisten (end) (F11)", QString::null,
                           this, SLOT(forAllSelectedPrelistenEnd()), toolBar2);
	tbStopPrelisten = new QToolButton (QPixmap(stopPrelisten_xpm), "Stop prelisten (F12)", QString::null,
                           this, SLOT(stopPrelisten()), toolBar2);
	tbSongInfo = new QToolButton (QPixmap(songinfo_xpm), "Info...", QString::null,
                           this, SLOT(forAllSelectedSongInfo()), toolBar2);

	
	
	// status bar
	mainStatusBar=statusBar();
	mainStatusBar->message("Everything loaded", 10000);
	
	// now setup main area
	QSplitter* centralWidget=new QSplitter(Qt::Horizontal, this);
	
	// set up the quick browser on the left
	folderListView = new QListView( centralWidget );
	folderListView->header()->setClickEnabled( FALSE );
	folderListView->addColumn( "Quick Browser" );
	folderListView->setRootIsDecorated( TRUE );
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
	folderAll=new Folder( folderListView, QString("- All Music -"));
	folderAll->update(&(model->allSongs));
	
	// folder containing all artists with more than <n> songs	
	folderArtists = new FolderGroups( folderListView, QString( "Artists" ));
	folderArtists->update(&(model->allSongs), MyList::ByArtist);
	
	// folder containing all albums with more than <n> songs	
	folderAlbums = new FolderGroups( folderListView, QString( "Albums" ));
	folderAlbums->update(&(model->allSongs), MyList::ByAlbum);

	// folder containing all categories
	folderCategories = new FolderCategories( folderListView, QString("Categories"));
	folderCategories->update(model->allCategories, model->categoryNames);
	updateSongPopup();
	
	// folder containing media
	folderMedia = new FolderMedia( folderListView, QString("Media"));
	folderMedia->update(&(model->allSongs));

	// folder containing currently played song
	folderActual = new Folder(folderListView, QString("- Playlist"));
	folderActual->update(&(model->songsToPlay));

	// folder containing history
	folderHistory = new Folder(folderListView, QString("History"));
	folderHistory->update(&(model->songHistory));

	// folder containing songs played in this session
	folderSongsPlayed = new Folder(folderListView, QString("Songs Played"));
	folderSongsPlayed->update(&(model->songsPlayed));

	// folder containing unclassified songs
	folderUnclassified = new Folder(folderListView, QString("Unclassified"));
	for(SongEntry* entry=model->allSongs.first(); entry; entry=model->allSongs.next()) {
		if(!entry->song()->classified)
			model->unclassifiedSongs.append(entry);
	}
	folderUnclassified->update(&(model->unclassifiedSongs));
		
	folderSearchResults = new Folder( folderListView, QString("Search Results") );
	folderSearchResults->update(&searchResults);
	
	folderProblematic = new Folder( folderListView, QString("Problematic Songs") );
	cout << "..done\n";

	// connect all things...
	//**********************
	connect( addToWishListButton, SIGNAL( clicked() ), this, SLOT( addToWishList() ) );
	connect( currentSongLabel, SIGNAL( clicked() ), this, SLOT( currentlyPlayedSongPopup() ) );

	// signals of folderListView
  connect( folderListView, SIGNAL( selectionChanged( QListViewItem* ) ),
	     this, SLOT( slotFolderChanged() ) );
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

	// signals of folders
	connect(folderCategories, SIGNAL( CategoryRemoved() ), this, SLOT(removeCategory()));
	connect(folderCategories, SIGNAL( EnqueueFolder() ), this, SLOT(enqueueFolder()));
	connect(folderCategories, SIGNAL( BurnFolder() ), this, SLOT(burnFolder()));

	connect(folderArtists, SIGNAL( EnqueueFolder() ), this, SLOT(enqueueFolder()));
	connect(folderArtists, SIGNAL( BurnFolder() ), this, SLOT(burnFolder()));
	
	connect(folderAlbums, SIGNAL( EnqueueFolder() ), this, SLOT(enqueueFolder()));
	connect(folderAlbums, SIGNAL( BurnFolder() ), this, SLOT(burnFolder()));
	
	connect(folderAll, SIGNAL( BurnFolder() ), this, SLOT(burnFolder()));
	
	connect(folderMedia, SIGNAL( RemoveMedia() ), this, SLOT(removeMedia()));

	// some preparations for startup
	//******************************
	folderListView->firstChild()->setOpen( TRUE );
	folderListView->setCurrentItem( folderListView->firstChild()->firstChild() );
	folderListView->setSelected( folderListView->firstChild(), TRUE );

	songListView->setSelected( songListView->firstChild(), TRUE );
	songListView->setCurrentItem( songListView->firstChild() );
	updateSongPopup();
//	updateListViewColumns();

	model->allSongsChanged(false);
  model->categoriesChanged(false);
	shuttingDown=0;
	
	
	// check whether xmms is in shuffle mode: if yes, set it to normal
	// (confuses Yammi's playlistmanagement)
	if(xmms_remote_is_shuffle(0)) {
		xmms_remote_toggle_shuffle(0);
		cout << "switching off xmms shuffle mode (does confuse my playlist management otherwise)\n";
	}

	// check whether xmms is playing, if not: start playing!
	if(!xmms_remote_is_playing(0)) {
		xmms_remote_play(0);
		cout << "yammi is not playing, starting it...\n";
	}
	syncXmms2Yammi();

	// connect all timers
  connect( &regularTimer, SIGNAL(timeout()), SLOT(onTimer()) );
  regularTimer.start( 1000, FALSE );	// call onTimer once a second
	connect( &typeTimer, SIGNAL(timeout()), this, SLOT(searchFieldChanged()) );
	songsUntilShutdown=-1;
//	connect( &exitTimer, SIGNAL(timeout()), this, SLOT(shutDown()) );

	// finish!
  cout << "initialisation successfully completed!\n";
	mainStatusBar->message("Welcome to Yammi", 10000);
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
	syncYammi2Xmms(true);
	cout << "trying to exit gracefully...\n";
	if(model->allSongsChanged() || model->categoriesChanged()) {
		if( QMessageBox::warning( this, "Yammi", "Save changes?", "Yes", "No")==0)
			model->save();
		else
			cout << "not saving any changes (data may be lost)\n";
	}
	cout << "goodbye!\n";
}


/// updates the view to reflect any changes in the model
void YammiGui::updateView()
{
	for(Song* s=model->allSongs.firstSong(); s; s=model->allSongs.nextSong())
		s->classified=false;	
	folderAll->update(&(model->allSongs));
	folderArtists->update(&(model->allSongs), MyList::ByArtist);
	folderAlbums->update(&(model->allSongs), MyList::ByAlbum);
	folderCategories->update(model->allCategories, model->categoryNames);
	folderMedia->update(&(model->allSongs));
	
	model->unclassifiedSongs.clear();
	for(Song* s=model->allSongs.firstSong(); s; s=model->allSongs.nextSong()) {
		if(!s->classified)
			model->unclassifiedSongs.appendSong(s);
	}

	folderUnclassified->update(&(model->unclassifiedSongs));
	folderProblematic->update(&(model->problematicSongs));
	folderSongsPlayed->update(&(model->songsPlayed));
	folderActual->update(&(model->songsToPlay));
	slotFolderChanged();
}


void YammiGui::updateListViewColumns()
{
	int toDel=songListView->columns();
	for(int i=0; i<toDel; i++)
		songListView->removeColumn(0);
	if(chosenFolder==folderHistory || chosenFolder==folderSongsPlayed)
		songListView->addColumn( "Played on", 135);
	if(chosenFolder==folderSearchResults)
		songListView->addColumn( "Match", 45);
	if(chosenFolder==folderProblematic)
		songListView->addColumn( "Reason", 120);
		
	songListView->addColumn( "Artist", 200);
	songListView->addColumn( "Title", 200);
	songListView->addColumn( "Album", 150);
	songListView->addColumn( "Length", 50);
	songListView->setColumnAlignment( 3, Qt::AlignRight );
	songListView->addColumn( "Year", 50);
	songListView->setColumnAlignment( 4, Qt::AlignRight );
	songListView->addColumn( "Track", 40);
	songListView->setColumnAlignment( 5, Qt::AlignRight );
	songListView->addColumn( "Added to", 60);
	songListView->setColumnAlignment( 6, Qt::AlignRight );
	songListView->addColumn( "Bitrate", 40);
	songListView->setColumnAlignment( 7, Qt::AlignRight );
	songListView->addColumn( "Filename", 80);
	songListView->addColumn( "Path", 80);
	songListView->addColumn( "Comment", 100);
	songListView->addColumn( "Last played", 100);
	songListView->setAllColumnsShowFocus( TRUE );
	songListView->setShowSortIndicator( TRUE );
	songListView->setSelectionMode( QListView::Extended );
	songListView->setAllColumnsShowFocus( TRUE );
}

/// opens the preferences dialogue
void YammiGui::setPreferences()
{
	PreferencesDialog d(this, "preferencesDialog", true);
	d.LineEditTrashDir->setText(model->config.trashDir);
	d.LineEditScanDir->setText(model->config.scanDir);
	d.CheckBoxCutPlaylist->setChecked(model->config.cutShort);
	d.CheckBoxLogging->setChecked(model->config.logging);
	d.CheckBoxChildSafe->setChecked(model->config.childSafe);
	d.CheckBoxTagsConsistent->setChecked(model->config.tagsConsistent);
	d.CheckBoxFilenamesConsistent->setChecked(model->config.filenamesConsistent);
	d.LineEditCriticalSize->setText(QString("%1").arg(model->config.criticalSize));
	d.LineEditSecondSoundDevice->setText(model->config.secondSoundDevice);
	d.LineEditSearchThreshold->setText(QString("%1").arg(model->config.searchThreshold));
	d.LineEditSearchMaximumNoResults->setText(QString("%1").arg(model->config.searchMaximumNoResults));
	d.LineEditGrabAndEncodeCmd->setText(QString("%1").arg(model->config.grabAndEncodeCmd));
	
	
	
	d.ComboBoxDoubleClickAction->insertItem("None");
	d.ComboBoxDoubleClickAction->insertItem("Enqueue");
	d.ComboBoxDoubleClickAction->insertItem("EnqueueAsNext");
	d.ComboBoxDoubleClickAction->insertItem("PlayNow");
	d.ComboBoxDoubleClickAction->insertItem("SongInfo");
	d.ComboBoxDoubleClickAction->insertItem("PrelistenStart");
	d.ComboBoxDoubleClickAction->insertItem("PrelistenMiddle");
	d.ComboBoxDoubleClickAction->insertItem("PrelistenEnd");

	d.ComboBoxDoubleClickAction->setCurrentItem(model->config.doubleClickAction);
	
	d.ComboBoxMiddleClickAction->insertItem("None");
	d.ComboBoxMiddleClickAction->insertItem("Enqueue");
	d.ComboBoxMiddleClickAction->insertItem("EnqueueAsNext");
	d.ComboBoxMiddleClickAction->insertItem("PlayNow");
	d.ComboBoxMiddleClickAction->insertItem("SongInfo");
	d.ComboBoxMiddleClickAction->insertItem("PrelistenStart");
	d.ComboBoxMiddleClickAction->insertItem("PrelistenMiddle");
	d.ComboBoxMiddleClickAction->insertItem("PrelistenEnd");
	d.ComboBoxMiddleClickAction->setCurrentItem(model->config.middleClickAction);
	
	// show dialog
	int result=d.exec();

	if(result==QDialog::Accepted) {
		model->config.trashDir=d.LineEditTrashDir->text();
		model->config.scanDir=d.LineEditScanDir->text();
		if(model->config.trashDir.right(1)!="/")
			model->config.trashDir+="/";
		if(model->config.scanDir.right(1)!="/")
			model->config.scanDir+="/";
		model->config.doubleClickAction=(action)d.ComboBoxDoubleClickAction->currentItem();
		model->config.middleClickAction=(action)d.ComboBoxMiddleClickAction->currentItem();
		model->config.cutShort=d.CheckBoxCutPlaylist->isChecked();
		model->config.logging=d.CheckBoxLogging->isChecked();
		model->config.tagsConsistent=d.CheckBoxTagsConsistent->isChecked();
		model->config.filenamesConsistent=d.CheckBoxFilenamesConsistent->isChecked();
		if(model->config.childSafe && !d.CheckBoxChildSafe->isChecked()) {
			bool ok;
			QString passwd=QString(QInputDialog::getText( "password", "enter password", QLineEdit::Password, QString(""), &ok, this ));
			if(passwd=="funny")
			model->config.childSafe=false;
		}
		else {
			model->config.childSafe=d.CheckBoxChildSafe->isChecked();
		}
		model->config.criticalSize=atoi(d.LineEditCriticalSize->text());
		model->config.secondSoundDevice=d.LineEditSecondSoundDevice->text();
		model->config.searchThreshold=atoi(d.LineEditSearchThreshold->text());
		model->config.searchMaximumNoResults=atoi(d.LineEditSearchMaximumNoResults->text());
		model->config.grabAndEncodeCmd=d.LineEditGrabAndEncodeCmd->text();
		updateSongPopup();
		model->savePreferences();
	}
}


/// Updates the popup-menu for songs, especially available categories
void YammiGui::updateSongPopup()
{
	// submenu containing all categories
	playListPopup = new QPopupMenu();
	int i=0;
	for(QString name=model->categoryNames.first(); name; name=model->categoryNames.next(), i++) {
		playListPopup->insertItem(QIconSet( QPixmap(in_xpm)), name, this, SLOT(toPlayList(int)), 0, i);
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
	if(model->config.childSafe)
		return;
	
	songAdvancedPopup = new QPopupMenu(songPopup);
	songAdvancedPopup->insertItem( "Delete", this, SLOT(forSelection(int)), 0, Delete);
	songAdvancedPopup->insertItem( "Copy file to...", this, SLOT(forSelection(int)), 0, CopyTo);
	songAdvancedPopup->insertItem( "Copy file as WAV to...", this, SLOT(forSelection(int)), 0, CopyAsWavTo);
	songAdvancedPopup->insertItem( "Move file to...", this, SLOT(forSelection(int)), 0, MoveTo);
	songAdvancedPopup->insertItem( "Check Consistency", this, SLOT(forSelection(int)), 0, CheckConsistency);
	songPopup->insertItem( "Advanced...", songAdvancedPopup);
}


// returns icon for popup
QIconSet YammiGui::getPopupIcon(action whichAction)
{
	if(model->config.doubleClickAction==whichAction)
		return QIconSet(QPixmap(defaultDoubleClick_xpm));
	if((model->config.middleClickAction==whichAction))
		return QIconSet(QPixmap(defaultMiddleClick_xpm));
	else
		return (QIconSet) NULL;
}


/// adds the text in search field to the wishlist
void YammiGui::addToWishList()
{
	QString toAdd=searchField->text();
	MyDateTime wishDate=wishDate.currentDateTime();
	Song* newSong=new Song("{wish}", toAdd, "", "", "", 0, 0, wishDate, 0, "", 0);
	model->allSongs.appendSong( newSong );
	model->allSongsChanged(true);
	folderAll->update(&(model->allSongs));
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
	// choose the desired category
	MyList* category=model->allCategories.first();
	for(int i=0; i<index; i++) {
		category=model->allCategories.next();
	}
	QString chosen=model->categoryNames.at(index);
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
	
	// go through list of songs
	for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
		if(!remove)
			category->appendSong(s);
		else
			category->removeSong(s);
	}
	
	model->categoriesChanged(true);
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
		model->allSongs.removeSong(s2);
	}
	if(what==2) {				// okay, delete s1
		cout << "deleting s1\n";
		forSong(s1, DeleteFile);				// move it to trash...
		model->allSongs.removeSong(s1);
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
		searchResults.append( new SongEntryInt ((Song*)bme[noResults]->objPtr, bme[noResults]->sim) );
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
	QListViewItem *i = folderListView->currentItem();
	if ( !i )
		return;
	Folder* newFolder=(Folder*)i;
	
	int position=-1;
	int sorting=-1;
	if(chosenFolder==newFolder) {			// same folder as before
		position=1;
		sorting=1;
	}
	
	// switch to unsorted folder => switch sorting off
	if(newFolder==folderActual)
		songListView->setSorting(-1);
	else
		songListView->setSorting(0);		// sort by first column
		
	// switch from unsorted to sorted folder => switch sorting back on
	if( (chosenFolder==folderActual) &&	(newFolder!=folderActual) ) {
		songListView->setSorting(0, TRUE);
	}
	
	// only allow dragging of songs in folderActual
	if(newFolder==folderActual)
		songListView->draggable=true;
	else
		songListView->draggable=false;
	
	// perform new search (to reflect changes in between)
/*	
	if(newFolder==folderSearchResults && chosenFolder!=folderSearchResults) {
		chosenFolder=newFolder;
		searchFieldChanged();
	}
*/
	
	chosenFolder = newFolder;
	songListView->clear();
	updateListViewColumns();
	addFolderContent(chosenFolder);
}

/*
void YammiGui::slotSortOrderChanged(int sortOrder)
{
	cout << "test\n";
	QListViewItem *i = folderListView->currentItem();
	if ( !i )
		return;
	songListView->clear();
	Folder* chosenFolder = ( Folder* )i;
	
	// first sort the list
	chosenFolder->songList.setSortOrder(sortOrder);
	chosenFolder->songList.sort();
	songListView->setSorting(-1);
	addFolderContent(chosenFolder);
}
*/

/// recursively add the content of folder and all subfolders
/// for now: folder contains songs OR subfolders, but not both!
void YammiGui::addFolderContent(Folder* folder)
{	
	folderToAdd=folder;
	
	alreadyAdded=0;
	// should we first sort the entries???
	// we need to get the column to sort folder->songList.sort();
	addFolderContentSnappy();
	
	// if no songs in this folder => add subfolders?
	/* if(i==0) {
		for ( QListViewItem* f=folder->firstChild(); f; f=f->nextSibling() )
			addFolderContent((Folder*)f);	
	}	*/
	
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
		lastOne=new SongListItem( songListView, entry, lastOne);
	}
	alreadyAdded=i;
	if(entry) {		// any songs left to add?
		QTimer* timer=new QTimer(this);
		connect(timer, SIGNAL(timeout()), this, SLOT(addFolderContentSnappy()) );
		timer->start(0, TRUE);
	}
	else {		// no, we are finished
		QApplication::restoreOverrideCursor();
	}
}


/// user clicked on a song
void YammiGui::slotSongChanged()
{
/*
    QListViewItem *i = songListView->currentItem();
    if ( !i )
			return;
    if ( !i->isSelected() ) {
			return;
    }
    SongListItem *item = ( SongListItem* )i;
    Song *s = item->song();
*/
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
	// we don't check whether all selected songs are contained, just first(old: current)
	int k=0;
	for(MyList* category=model->allCategories.first(); category; category=model->allCategories.next(), k++) {
		if(category->containsSong(first)>0)
			playListPopup->changeItem(k, QIconSet( QPixmap(in_xpm)), playListPopup->text(k));
		else
			playListPopup->changeItem(k, QIconSet( QPixmap(notin_xpm)), playListPopup->text(k));
	}
		
 	// for wishes: disable menu entries?
 	// depending only on first?????
 	bool enable=true;
 	if(first->artist=="{wish}" || first->filename=="")
 		enable=false;
		
 	songPopup->setItemEnabled(Enqueue, enable);
 	songPopup->setItemEnabled(EnqueueAsNext, enable);
 	songPopup->setItemEnabled(PlayNow, enable);
 	songPopup->setItemEnabled(CheckConsistency, enable);
 	songPopup->setItemEnabled(CopyTo, enable);
 	songPopup->setItemEnabled(CopyAsWavTo, enable);
 	songPopup->setItemEnabled(MoveTo, enable);
}


/// folder popup menu
void YammiGui::slotFolderPopup( QListViewItem* Item, const QPoint & point, int )
{
	// get selection: folder content
	selectedSongs.clear();
	QListViewItem *i = folderListView->currentItem();	
	Folder* chosenFolder = ( Folder* )i;
	for(Song* s=chosenFolder->firstSong(); s; s=chosenFolder->nextSong()) {
		selectedSongs.appendSong(s);
	}
	if(selectedSongs.count()==0) {
		cout << "no songs\n";
		chosenFolder->popup( point, 0);
		return;
	}
	adjustSongPopup();
//	Folder* f=(Folder*) Item;
	chosenFolder->popup( point, songPopup);
}




/**
 * edit song info
 * (mass editing)
 */
void YammiGui::forSelectionSongInfo()
{
	QString _artist, _title, _album, _comment, _path, _filename, _year, _trackNr;
	MyDateTime _addedTo;
	int _length=0;
	long double _size=0;
		
	int selected=0;
	SongInfoDialog si(this, "test", true);
	
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
		
	
	// show dialog
	int result=si.exec();
	
	if(result==QDialog::Accepted) {
		// now set the edited info for all selected songs
		for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
			bool change=false;
			if(si.LineEditArtist->text()!="!" && si.LineEditArtist->text()!=s->artist)	{ s->artist=si.LineEditArtist->text(); change=true; }
			if(si.LineEditTitle->text()!="!" && si.LineEditTitle->text()!=s->title)			{ s->title=si.LineEditTitle->text(); change=true; }
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
			if(change) {
				model->allSongsChanged(true);
				s->tagsDirty=true;						// mark song as dirty(tags)
				s->filenameDirty=(s->checkFilename()==false);
			}				
		}	
		slotFolderChanged();
	}
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
	model->checkConsistency();
	folderProblematic->update(&model->problematicSongs);
}

/**
 * for the songs in <selectedSongs> do <action>
 */
void YammiGui::forSelection(action act)
{
	// special treatment
	if(act==SongInfo) {
		forSelectionSongInfo();
		return;
	}
	
	// 1. destination directory
	QString dir;
	if(act==MoveTo || act==CopyTo || act==CopyAsWavTo) {
		// let user choose directory (we should provide a starting directory???)
		dir=QFileDialog::getExistingDirectory(QString(""), this, QString("yammi"), QString("choose directory"), true);
		if(dir.isNull())
			return;
		if(dir.right(1)=="/")						// strip trailing slash
			dir=dir.left(dir.length()-1);
	}
			
	// 3. determine delete mode
	bool deleteFile=false;
	bool deleteEntry=false;
	if(act==Delete) {
		DeleteDialog dd( this,  "deleteDialog", true);
		cout << "asking in forSelection\n";
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
		cout << "for loop: " << s->displayName() << "\n";
		
		if(act==Delete) {
			if(deleteFile)	forSong(s, DeleteFile);
			if(deleteEntry)	forSong(s, DeleteEntry);
			if(s==0)
				cout << "seems to be zero\n";
			else
				cout << s->displayName() << "\n";
		}
		else
			forSong(s, act, dir);
//		if(act==PlayNow)									// we play first selected song...
//			act=Enqueue;										// ...and enqueue all others
		if(act==PrelistenStart || act==PrelistenMiddle || act==PrelistenEnd)
			break;
	
	}
	
	
	// some OPs need view update
	if(deleteEntry) {
		updateView();
	}
	if(act==Enqueue || act==EnqueueAsNext || act==Dequeue) {
		folderActual->update(&(model->songsToPlay));
		syncYammi2Xmms();
		if(chosenFolder==folderActual)
			slotFolderChanged();
		else
			songListView->triggerUpdate();
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
		xmms_remote_playlist_delete(0, i);
	}
	// 2. insert all (including currently played) songs into yammi playlist
	model->songsToPlay.clear();
	for(int i=0; i<xmms_remote_get_playlist_length(0); i++) {
 		char buf[200];
		strcpy(buf, xmms_remote_get_playlist_file(0, i));
		Song* check=getSongFromFilename(QString(buf));
		if(!check)	// song not found in database
			continue;
		model->songsToPlay.appendSong(check);
	}
	// 3. delete all but the keepInXmms first songs
	for(int i=xmms_remote_get_playlist_length(0)-1; i>=model->config.keepInXmms; i--) {
		xmms_remote_playlist_delete(0, i);
	}
}

/**
 * tries to sync the xmms playlist with the yammi playlist
 * ie. writes all entries from yammi playlist into xmms playlist
 * howmany==-1 => sync all
 */
void YammiGui::syncYammi2Xmms(bool syncAll)
{
	// check whether xmms playlist is empty
	if(xmms_remote_get_playlist_length(0)==0) {
		// xmms playlist empty
		for(int i=0; i<(int)model->songsToPlay.count() && (i<model->config.keepInXmms || syncAll); i++) {
				gchar url[300];
				strcpy(url, model->songsToPlay.at(i)->song()->location());
				xmms_remote_playlist_add_url_string(0, url);
		}
		return;
	}
	
	// okay, at least one song in xmms playlist
		
			
	// iterate through first <keepInXmms> songs in Xmms playlist
	// if different than corresponding yammi entry => delete
	// if playlist too short => insert yammi entries
	for(int i=1; i<model->config.keepInXmms || ( syncAll && i<(int)model->songsToPlay.count() ); i++) {
		
		// check whether playlist entry existing
		if(i<(int)xmms_remote_get_playlist_length(0)) {
			// yes, existing!
	 		char buf[300];
			strcpy(buf, xmms_remote_get_playlist_file(0, i));
			Song* check=getSongFromFilename(QString(buf));
			if(i<(int)model->songsToPlay.count()) {
				Song* s=model->songsToPlay.at(i)->song();
				if(check==s)
					continue;		// okay, both are the same
				// ups, different!
				cout << "song entry " << i << " different => deleting and replacing all following!\n";
				for(int toDel=xmms_remote_get_playlist_length(0)-1; toDel>=i; toDel--) {
					// delete all following
					xmms_remote_playlist_delete(0, toDel);
					myWait(100);
				}
				for(int toInsert=i; toInsert<model->config.keepInXmms; toInsert++) {
					// reinsert
					gchar url[300];
					Song* s=model->songsToPlay.at(toInsert)->song();
					strcpy(url, s->location());
					xmms_remote_playlist_add_url_string(0, url);
					myWait(100);
				}
				return;
			}
			else {
				cout << "xmms playlist longer than yammi playlist, deleting!\n";
				for(int toDel=xmms_remote_get_playlist_length(0)-1; toDel>=i; toDel--) {
					// delete all following
					xmms_remote_playlist_delete(0, toDel);
					myWait(100);
				}
			}
		}
		else {	
			// playlist too short => check whether songs in songsToPlay
			if(i<(int)model->songsToPlay.count()) {
				Song* s=model->songsToPlay.at(i)->song();
				gchar url[300];
				strcpy(url, s->location());
				xmms_remote_playlist_add_url_string(0, url);
				myWait(100);
			}
		}
	} // end of for
	
	// if playlist too long => shorten?
	
}

/**
 * performs some action for a song
 */
void YammiGui::forSong(Song* s, action act, QString dir=0)
{		
	switch (act) {
	case None:									// no action
		return;
	case PlayNow:								// enqueue at front and immediately skip to it
		if(s->filename=="" || !s->checkReadability())
			return;
		forSong(s, EnqueueAsNext);
		folderActual->update(&(model->songsToPlay));
		syncYammi2Xmms();
		xmms_remote_playlist_next(0);
		if(!xmms_remote_is_playing(0))
			xmms_remote_play(0);
		mainStatusBar->message(QString("playing %1").arg(s->displayName()), 2000);
		break;
		
	case Enqueue:								// enqueue at end
		if(s->filename=="" || !s->checkReadability())
			return;
		model->songsToPlay.appendSong(s);
		mainStatusBar->message(QString("%1 enqueued at end").arg(s->displayName()), 3000);
		break;
				
	case EnqueueAsNext: {				// enqueue as next
		if(s->filename=="" || !s->checkReadability())
			return;
		model->songsToPlay.insert(1, new SongEntry(s));
		mainStatusBar->message(QString("%1 enqueued as next").arg(s->displayName()), 2000);
	}
		break;
	
	case Dequeue:
		// search for selected song and dequeue
		for(int i=1; i<(int)model->songsToPlay.count(); i++) {
			Song* check=model->songsToPlay.at(i)->song();
			if(check==s) {
				model->songsToPlay.remove(i);
				cout << "song removed\n";
				mainStatusBar->message(QString("song %1 dequeued").arg(s->displayName()), 3000);
				i--;
			}
		}
		break;
	
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
		if(s->checkConsistency()==false) {
			if(model->problematicSongs.containsSong(s)==0)
				model->problematicSongs.appendSong(s);
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
		model->allSongs.removeSong(s);
		model->allSongsChanged(true);
		break;
	
	case DeleteFile:						// move songfile to trash
		s->deleteFile(model->config.trashDir);
		break;
		
	case CopyTo:						// copy songfile to other location
		mainStatusBar->message(QString("copying song %1").arg(s->displayName()), 2000);
		s->copyTo(dir);
		mainStatusBar->message(QString("song %1 copied").arg(s->displayName()), 2000);
		break;

	case CopyAsWavTo:
		s->copyAsWavTo(dir);
		break;
	
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
	if(button!=4)	return;		// only interested in middle button
	forCurrent(model->config.middleClickAction);
}
	

/// open konqueror with help
void YammiGui::openHelp()
{
	// should we undo the hardcoded path ???
	system("/usr/share/doc/packages/yammi/index.html &");
}

/// display about dialog
void YammiGui::aboutDialog()
{
	QMessageBox::information( this, "Yammi",	QString("Yammi - Yet Another Music Manager\n\n\n")+
																					"Version 0.1, 12-2001 by Oliver Nölle\n\n"+
																					"Contact: oli.noelle@web.de\n\n"+
																					"Project home page: yammi.sourceforge.net\n\n\n"+
																					"have fun...\n");
}


/** creates a new category */
void YammiGui::newCategory(){
	bool ok = false;
	QString caption("Enter name for category");
	QString message("Please enter name of category");
	QString newName=QString(QInputDialog::getText( caption, message, QString("new category"), &ok, this ));
	if(ok) {
		model->newCategory(newName);
		folderCategories->update(model->allCategories, model->categoryNames);
		updateSongPopup();		
	}
}


void YammiGui::removeCategory()
{
	QListView* lv=folderListView;
	QListViewItem* i = lv->currentItem();
	QString name=i->text(0);
	model->removeCategory(name);
	folderCategories->update(model->allCategories, model->categoryNames);
	updateSongPopup();
}

/// enqueue content of folder to xmms
void YammiGui::enqueueFolder()
{
	QListViewItem *i = folderListView->currentItem();
	Folder* chosenFolder = ( Folder* )i;
	chosenFolder->songList.setSortOrder(MyList::ByTrack);
	chosenFolder->songList.sort();
	
	for(Song* s=chosenFolder->firstSong(); s; s=chosenFolder->nextSong()) {
		forSong(s, Enqueue);
	}
	folderActual->update(&(model->songsToPlay));
}


/// remove media
// uahhaa... ugly! make mediaName + mediaLocation a struct/class, oli!
void YammiGui::removeMedia()
{
	QListViewItem *i = folderListView->currentItem();
	Folder* chosenFolder = ( Folder* )i;
	QString mediaName=chosenFolder->folderName();
	model->removeMedia(mediaName);
}


/// prepare burning of an arbitrary folder
void YammiGui::burnFolder()
{
	QListViewItem *i = folderListView->currentItem();
	Folder* chosenFolder = ( Folder* )i;
	// set the sort order (=burning order)
	chosenFolder->songList.setSortOrder(MyList::ByArtist);
	chosenFolder->songList.sort();
	
	long double totalSize=-model->config.criticalSize*1024*1024+1;
	long double size=model->config.criticalSize*1024*1024+1;								// start with a full medium

	bool ok;
	QString collName=QString(QInputDialog::getText( "collection name", "Please enter collection name:", QString("collection name2"), &ok, this ));
	if(!ok)
		return;

	int mediaNo=0;
	QString mediaName=QString("%1_%2").arg(collName).arg(mediaNo);
	QString mediaDir=QString(model->config.yammiBaseDir+"/media/"+mediaName);
	
	for(Song* s=chosenFolder->firstSong(); s; s=chosenFolder->nextSong()) {
		QFileInfo fi(s->location());
		if(size+fi.size()>(long double)model->config.criticalSize*1024.0*1024.0) {
			// medium is full, prepare new one
			mediaNo++;
			mediaName=QString("%1_%2").arg(collName).arg(mediaNo);
			mediaDir=QString(model->config.yammiBaseDir+"/media/"+mediaName);
			QString cmd=QString("mkdir \"%1\"").arg(mediaDir);
			system(cmd);
			totalSize+=size;
			size=0;
		}
		size+=fi.size();
		QString cmd=QString("ln -s \"%1\" \"%3/%4\"").arg(s->location()).arg(mediaDir).arg(s->filename);
		system(cmd);
		s->addMediaLocation(mediaName, s->filename);
	}
	totalSize+=size;				// add last (half full) media
	
	cout << "number media: " << mediaNo << " (critical size: " << model->config.criticalSize << " MB)\n";
	cout << "size of last media: " << size << " (=" << size/(1024*1024) << " MB)\n";
	cout << "size in total: " << totalSize << " (=" << totalSize/(1024*1024) << " MB)\n";
	model->allSongsChanged(true);
}


/// toggle between play and pause
void YammiGui::xmms_playPause()
{
	gboolean playing=xmms_remote_is_playing(0);
	if(playing)
		xmms_remote_pause(0);
	else
		xmms_remote_play(0);
}


/// skip forward in playlist
void YammiGui::xmms_skipForward()
{
	int x= xmms_remote_get_playlist_pos(0);
	xmms_remote_set_playlist_pos(0, x+1);
}

/// skip backward in playlist
void YammiGui::xmms_skipBackward()
{
	int x= xmms_remote_get_playlist_pos(0);
	xmms_remote_set_playlist_pos(0, x-1);
}

/// clear all playlist items except currently played song
void YammiGui::xmms_clearPlaylist()
{
	if(model->config.childSafe)
		return;
	if( QMessageBox::warning( this, "Yammi", "Clear complete playlist?", "Yes", "No")==0) {
		Song* save=model->songsToPlay.firstSong();
		model->songsToPlay.clear();
		model->songsToPlay.appendSong(save);
		folderActual->update(&(model->songsToPlay));
		syncYammi2Xmms();
		if(chosenFolder==folderActual)
			slotFolderChanged();
		else
			songListView->triggerUpdate();
	}
	
/*
  int pos=xmms_remote_get_playlist_pos(0);
	int length=xmms_remote_get_playlist_length(0);
	for(int i=length-1; i>=0; i--) {
		if(i!=pos || !xmms_remote_is_playing(0))
	  	xmms_remote_playlist_delete(0, i);
	}
*/
}

/**
 * onTimer is called periodically to do some things independently of any user action
 * - cutShort
 * - logging
 * - updating folderActual
 */
void YammiGui::onTimer()
{	
	// perform these actions only if xmms is playing
	if(xmms_remote_is_playing(0)) {
  	
		// check whether currently played song has changed
 		char file[200];
  	int pos=xmms_remote_get_playlist_pos(0);
		strcpy(file, xmms_remote_get_playlist_file(0, pos));
		
		if(currentFile!=file) {
		// *** song change detected ***
		// ****************************
			cout << "song change detected\n";
			if(songListView->dragging)
				stopDragging();
			
			currentFile=QString(file);
			if(songsUntilShutdown!=-1) {
				songsUntilShutdown--;
				if(songsUntilShutdown<=0) {
					cout << "shutting down now...\n";
					shutDown();
				}
				cout << songsUntilShutdown << " songs left before shutdown...\n";
			}
			
			currentSong=getSongFromFilename(QString(file));

			// song entry found
			if(currentSong!=0) {
				
				SongEntryTimestamp* entry=new SongEntryTimestamp(currentSong);
				model->songsPlayed.append(entry);		// append to songsPlayed
				folderSongsPlayed->update(&(model->songsPlayed));
				if(model->config.logging)
					model->logSong(currentSong);
				if(chosenFolder==folderSongsPlayed)
					slotFolderChanged();
		  	
	  		// take over playlist management from xmms...
		  	// remove played song(s) from xmms playlist
		  	int i;
		  	for(i=0; xmms_remote_get_playlist_pos(0)>0; i++) {
		  		xmms_remote_playlist_delete(0, 0);
		  		model->songsToPlay.removeFirst();
		  	}
		  	cout << "removed " << i << " entries from xmms/yammi\n";
		  	
		  	// should just check, not change anything:
		  	syncYammi2Xmms();
		  		
				folderActual->update(&(model->songsToPlay));
				if(chosenFolder==folderActual)
					slotFolderChanged();
				// ...end of playlist management
				
				// set title to currently played song
				setCaption("Yammi: "+currentSong->displayName());
			}
			else {				// song not found in database
				setCaption("Yammi - song not in database");
			}	
			
			// update view, if folderActual is currently shown folder
			songListView->triggerUpdate();					
			if(chosenFolder==folderActual)
				slotFolderChanged();		

		}
		// *** end of song change ***
		// **************************
		
		// shorten the list of played songs to maximum 5
		// only check this when songChange??
  	if(model->config.cutShort) {
  		while(xmms_remote_get_playlist_pos(0)>5)		// model->config.cutShortNumber
  			xmms_remote_playlist_delete(0, 0);
  	}
  	
	}
	else {				// xmms not playing (after stop, NOT after pause)
		setCaption("Yammi - XMMS not playing");
		currentFile="";
		if(songsUntilShutdown!=-1) {
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
// returns 0 if no song entry found
Song* YammiGui::getSongFromFilename(QString filename)
{
	// strip filename to relative name
	int pos=filename.findRev('/', -1);
	QString lookFor=filename.right(filename.length()-pos-1);
	for(Song* s=folderAll->firstSong(); s; s=folderAll->nextSong()) {
		if(s->filename==lookFor)
			return s;
	}
	return 0;
}				


// just beta:
// copy a track from audio-cd to harddisk and add as wav to database
void YammiGui::enqueueCdTrack()
{
	bool ok = false;
	QString caption("Enter track number");
	QString message("Please enter track number");
	QString trackNrStr(QInputDialog::getText( caption, message, QString("1"), &ok, this ));
	int trackNr=atoi(trackNrStr);
	if((!ok) || trackNr<1)
		return;
	
	caption="Enter artist";
	message="Please enter artist";
	QString artist(QInputDialog::getText( caption, message, QString("MyArtist"), &ok, this ));
	if(!ok)
		return;
	
	caption="Enter title";
	message="Please enter title";
	QString title(QInputDialog::getText( caption, message, QString("Fantastic Song"), &ok, this ));
	if(!ok)
		return;
	
	QString filename=QString("%1%2 - %3.mp3").arg(model->config.scanDir).arg(artist).arg(title);
	QString cmd=QString("%1 %2 \"%3\" \"%4\" \"%5\" &").arg(model->config.grabAndEncodeCmd).arg(trackNr).arg(artist).arg(title).arg(filename);
	system(cmd);
	grabbedTrackFilename=filename;
	mainStatusBar->message("grabbing track, will be available shortly...", 10000);
	// now we start a timer to check for availability of new track every 5 seconds
	QTimer* timer=new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(checkForGrabbedTrack()) );
	timer->start(10000, TRUE);
}

/** checks for availability of a song that is currently being grabbed and encoded
 */
void YammiGui::checkForGrabbedTrack(){
	QFileInfo fileInfo(grabbedTrackFilename);
 	if(!fileInfo.exists()) {
		QTimer* timer=new QTimer(this);
		connect(timer, SIGNAL(timeout()), this, SLOT(checkForGrabbedTrack()) );
		timer->start(3000, TRUE);
		return;
	}
	if(!fileInfo.isReadable()) {
		cout << "new grabbed track " << grabbedTrackFilename << " is unreadable\n";
 		return;
 	}
	mainStatusBar->message("new song available in inbox!!! (perform \"update database\" to make it available)", 5000); 	
}

void YammiGui::shutDown()
{
	if(shuttingDown==0)				// canceled
		return;
	if(shuttingDown==2) {
		xmms_remote_quit(0);						// properly close xmms => xmms will remember last played song + playlist
		system("shutdownscript &");			// shutdown computer
	}
	endProgram();
}


void YammiGui::keyPressEvent(QKeyEvent* e)
{
	cout << "key(): " << e->key() << "text(): " << e->text() << "ascii(): " << e->ascii() << "\n";
	int key=e->key();
	switch(key) {
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
			xmms_clearPlaylist();
			break;
		
		case Key_F5:
			forAllSelected(Enqueue);
			break;
		case Key_F6:
			forAllSelected(EnqueueAsNext);
			break;
		case Key_F7:
			forAllSelected(PlayNow);
			break;
		case Key_F8:
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
			
		case Key_Pause:	{									// exit program (press twice for shutting down computer)
		 if(shuttingDown==0) {
				qApp->beep();
				shuttingDown=1;
				songsUntilShutdown=2;
				// exitTimer.start( 5000, TRUE );
				// play("bye.wav")
				cout << "shutting down (normal)...\n";
				
			}	
			else if(shuttingDown==1 && !model->config.childSafe) {							// shutdown computer !!!
				qApp->beep();
				shuttingDown=2;
				cout << "shutting down (computer!), press F12 again to cancel...\n";
			}
			else {
				qApp->beep();
				songsUntilShutdown=-1;
				shuttingDown=0;
				cout << "shutting down cancelled!\n";
			}
		}	break;
				
		case Key_PageUp:
			if(songsUntilShutdown!=-1) {
				songsUntilShutdown++;
				cout << "songs until shutdown: " << songsUntilShutdown << "\n";
			}
			break;
		case Key_PageDown:
			if(songsUntilShutdown!=-1) {
				songsUntilShutdown--;
				cout << "songs until shutdown: " << songsUntilShutdown << "\n";
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
		
		case Key_F:		// F (how about Ctrl?)
			if(e->state()!=ControlButton)
				break;
			
		case Key_Escape:
			searchField->setText("");
			searchField->setFocus();
			break;
		
		case Key_S:		// F (how about Ctrl?)
			if(e->state()!=ControlButton)
				break;
			model->save();
			break;
		
		default:
			e->ignore();
	}
}

// stops playback on headphone
void YammiGui::stopPrelisten()
{
	// kill any previous mpg123 prelisten process
	QString cmd1=QString("kill -9 `ps h -o \"%p\" -C mpg123`");
	system(cmd1);
	QString cmd1b=QString("kill -9 `ps h -o \"%p\" -C aplay`");
	system(cmd1b);
}

/* sends the song to headphones
 * skipTo: 0 = beginning of song, 100 = end
 * old version: QString cmd=QString("mpg123 -s \"%1\" | aplay -c1 -m &").arg(s->location());
 */
void YammiGui::preListen(Song* s, int skipTo)
{
	int seconds=s->length;
	// first, kill any previous mpg123 prelisten process
	QString cmd1=QString("kill -9 `ps h -o \"%p\" -C mpg123`");
	system(cmd1);
//	QString cmd1b=QString("kill -9 `ps h -o \"%p\" -C aplay`");
//	system(cmd1b);
	// now play song via mpg123 or aplay on sound device configured in prefs
	if(s->filename.right(3)=="mp3") {
		QString skip=QString(" --skip %1").arg(seconds*skipTo*38/100);
		QString cmd2=QString("mpg123 -a %1 %2 \"%3\" &").arg(model->config.secondSoundDevice).arg(skip).arg(s->location());
		cout << "length: " << seconds << ", " << cmd2 << "\n";
		system(cmd2);
	}
/*	no support for prelistening to wavs right now...
	if(s->filename.right(3)=="wav") {
		QString cmd2=QString("aplay -c1 \"%1\" &").arg(s->location());
		system(cmd2);
	}
*/
}

void YammiGui::updateSongDatabase()
{
	model->updateSongDatabase();
	updateView();
	QMessageBox::information( this, "Yammi",
				QString("Some statistics: \n\n %1 songs added\n %2 songs corrupt, not added")
				.arg(model->songsAdded).arg(model->corruptSongs), "Fine." );
}

void YammiGui::stopDragging()
{
	cout << "stop dragging\n";
	// here we have to synchronize with xmms playlist
	model->songsToPlay.clear();
	for ( QListViewItem* item=songListView->firstChild(); item; item=item->nextSibling() ) {
		model->songsToPlay.appendSong(((SongListItem*)item)->song());
	}
	syncYammi2Xmms();
	songListView->dragging=false;
	songListView->setCursor(Qt::arrowCursor);
}


void YammiGui::myWait(int msecs)
{
	QTime t;
	t.start();
	while(t.elapsed()<msecs);
}