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
		system("xmms > /tmp/xmms_output.delme &");
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
	tbSaveDatabase = new QToolButton( QPixmap(filesave_xpm), "Save database", QString::null,
														model, SLOT(save()), toolBar, "save2" );
	tbBackward = new QToolButton( QPixmap(skipbackward_xpm), "Skip backward", QString::null,
 														this, SLOT(xmms_skipBackward()), toolBar);
	tbForward = new QToolButton( QPixmap(skipforward_xpm), "Skip forward", QString::null,
                           this, SLOT(xmms_skipForward()), toolBar, "skip forward2" );
	tbPause = new QToolButton( QPixmap(pause_xpm), "Play/Pause", QString::null,
                           this, SLOT(xmms_playPause()), toolBar, "pause2" );
	tbClearPlaylist = new QToolButton (QPixmap(clearPlaylist_xpm), "Clear playlist", QString::null,
                           this, SLOT(xmms_clearPlaylist()), toolBar, "clear2" );
		
	QLabel *searchLabel = new QLabel(toolBar);
	searchLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	searchLabel->setText( "  Search: " );
	searchLabel->setFrameStyle( QFrame::NoFrame );
	searchField = new QLineEdit ( toolBar );
	connect( searchField, SIGNAL(textChanged(const QString&)), SLOT(userTyped(const QString&)) );
	searchField->setFocus();
	searchField->setFixedWidth(210);
	// button "add to wishlist"	
	QPushButton* addToWishListButton=new QPushButton("add to wishlist", toolBar);
	// current song label
	QPushButton* currentSongLabel=new QPushButton("current song...", toolBar);	
	
	// now all the buttons that correspond to context menu entries
	tbEnqueue = new QToolButton (QPixmap(enqueue_xpm), "Enqueue at end", QString::null,
                           this, SLOT(forAllSelectedEnqueue()), toolBar, "songEnqueue2" );
	tbEnqueueAsNext = new QToolButton (QPixmap(enqueueasnext_xpm), "Enqueue as next", QString::null,
                           this, SLOT(forAllSelectedEnqueueAsNext()), toolBar, "songEnqueue2" );
	tbPlayNow = new QToolButton (QPixmap(playnow_xpm), "Play now", QString::null,
                           this, SLOT(forAllSelectedPlayNow()), toolBar, "songEnqueue2" );
	tbDequeueSong = new QToolButton (QPixmap(dequeueSong_xpm), "Dequeue Song", QString::null,
                           this, SLOT(forAllSelectedDequeue()), toolBar, "dequeueSong2" );
	tbPrelistenStart = new QToolButton (QPixmap(prelisten_xpm), "Prelisten (start)", QString::null,
                           this, SLOT(forAllSelectedPrelistenStart()), toolBar, "songEnqueue2" );
	tbPrelistenMiddle = new QToolButton (QPixmap(prelisten_xpm), "Prelisten (middle)", QString::null,
                           this, SLOT(forAllSelectedPrelistenMiddle()), toolBar, "songEnqueue2" );
	tbPrelistenEnd = new QToolButton (QPixmap(prelisten_xpm), "Prelisten (end)", QString::null,
                           this, SLOT(forAllSelectedPrelistenEnd()), toolBar, "songEnqueue2" );
	tbStopPrelisten = new QToolButton (QPixmap(stopPrelisten_xpm), "Stop prelisten", QString::null,
                           this, SLOT(stopPrelisten()), toolBar, "stopPrelisten2" );
	tbSongInfo = new QToolButton (QPixmap(songinfo_xpm), "Info...", QString::null,
                           this, SLOT(forAllSelectedSongInfo()), toolBar, "songInfo2" );

	
	
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
//	songListView = new QListView( centralWidget ); dragggg
	songListView = new MyListView( centralWidget );
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
	songListView->setAllColumnsShowFocus( TRUE );
	songListView->setShowSortIndicator( TRUE );
	songListView->setSelectionMode( QListView::Extended );
	songListView->setAllColumnsShowFocus( TRUE );
	
	QValueList<int> lst;
	lst.append( 150 );
	centralWidget->setSizes( lst );

	setCentralWidget(centralWidget);
	cout << "..done\n";

	// now init all Folders
	//*********************
	cout << "setting up folders...\n";
	
	// folder containing all music
	folderAll=new FolderAll( folderListView, QString("- All Music -"));
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
	folderActual = new FolderActual(folderListView, QString("- Playlist -"));
	folderActual->update(songsToPlay);

	// folder containing history
	folderHistory = new FolderHistory(folderListView, QString("History"));
	folderHistory->update(model->songHistory);

	// folder containing unclassified songs
	folderUnclassified = new FolderUnclassified(folderListView, QString("Unclassified"));
	folderUnclassified->update(&(model->allSongs));
		
	folderSearchResults = new Folder( folderListView, QString("Search Results") );
	
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
	
	model->allSongsChanged(false);
  model->categoriesChanged(false);
	shuttingDown=0;
	
	// check whether xmms is playing, if not: start playing!
	if(!xmms_remote_is_playing(0))
		xmms_remote_play(0);
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
	for(Song* s=model->allSongs.next(); s; s=model->allSongs.next())
		s->classified=false;	
	folderAll->update(&(model->allSongs));
	folderArtists->update(&(model->allSongs), MyList::ByArtist);
	folderAlbums->update(&(model->allSongs), MyList::ByAlbum);
	folderCategories->update(model->allCategories, model->categoryNames);
	folderMedia->update(&(model->allSongs));
	folderUnclassified->update(&(model->allSongs));
	folderProblematic->update(&(model->problematicSongs));
	folderActual->update(songsToPlay);
	slotFolderChanged();
}



/// opens the preferences dialogue
void YammiGui::setPreferences()
{
	PreferencesDialog d(this, "testiiiii", true);
	d.LineEditBaseDir->setText(model->config.baseDir);
	d.LineEditScanDir->setText(model->config.scanDir);
	d.CheckBoxCutPlaylist->setChecked(model->config.cutShort);
	d.CheckBoxLogging->setChecked(model->config.logging);
	d.CheckBoxChildSafe->setChecked(model->config.childSafe);
	d.CheckBoxTagsConsistent->setChecked(model->config.tagsConsistent);
	d.CheckBoxFilenamesConsistent->setChecked(model->config.filenamesConsistent);
	d.LineEditCriticalSize->setText(QString("%1").arg(model->config.criticalSize));
	d.LineEditSecondSoundDevice->setText(model->config.secondSoundDevice);
	
	
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
		model->config.baseDir=d.LineEditBaseDir->text();
		model->config.scanDir=d.LineEditScanDir->text();
		if(model->config.baseDir.right(1)!="/")
			model->config.baseDir+="/";
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
	
	songPlayPopup->insertItem(getPopupIcon(Enqueue), "...Enqueue", this, SLOT(forSelection(int)), CTRL+Key_E, Enqueue);		
	songPlayPopup->insertItem(getPopupIcon(EnqueueAsNext), "...Play as next", this, SLOT(forSelection(int)), CTRL+Key_N, EnqueueAsNext);
	songPlayPopup->insertItem(getPopupIcon(PlayNow), "...Play now!", this, SLOT(forSelection(int)), CTRL+Key_I, PlayNow);
	songPopup->insertItem( "Play/Enqueue...", songPlayPopup);
	
	if(model->config.secondSoundDevice!="") {
		songPrelistenPopup = new QPopupMenu(songPopup);
		songPrelistenPopup->insertItem(getPopupIcon(PrelistenStart), "...start", this, SLOT(forSelection(int)), 0, PrelistenStart);
		songPrelistenPopup->insertItem(getPopupIcon(PrelistenMiddle), "...middle", this, SLOT(forSelection(int)), 0, PrelistenMiddle);
		songPrelistenPopup->insertItem(getPopupIcon(PrelistenEnd), "...end", this, SLOT(forSelection(int)), 0, PrelistenEnd);
		songPopup->insertItem( "Prelisten to...", songPrelistenPopup);
	}
	songPopup->insertItem(getPopupIcon(SongInfo), "Info...", this, SLOT(forSelection(int)), 0, SongInfo);
	songPopup->insertItem( "Insert Into...", playListPopup);
	songPopup->insertItem( "Dequeue from playlist", this, SLOT(forSelection(int)), 0, Dequeue);
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
	model->allSongs.append( newSong );
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
	Song* s=selectedSongs.first();
	// for all songs contained in that category...
	for(Song* tmp=category->first(); tmp; tmp=category->next()) {
		if(s==tmp) {
			remove=true;
			break;
		}
	}
	
	// go through list of songs
	for(Song* s=selectedSongs.first(); s; s=selectedSongs.next()) {
		if(!remove)
			category->append(s);
		else
			category->remove(s);
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
		model->allSongs.removeRef(s2);
	}
	if(what==2) {				// okay, delete s1
		cout << "deleting s1\n";
		forSong(s1, DeleteFile);				// move it to trash...
		model->allSongs.removeRef(s1);
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
	QString searchStr=searchField->text();
	if(searchStr.length()<1) return;
	
	FuzzySearch fs;
	fs.initialize(searchStr.lower(), 2, 4);			// STEP 1
	
	// search through all songs
	Song* s=model->allSongs.first();
	QString composed;
	for(; s; s=model->allSongs.next()) {
		composed=s->artist + " - " + s->title + "  " + s->album;
		if(s->artist=="" || s->title=="") {							// if tags incomplete use filename for search
			composed+=s->filename;
		}
		fs.checkNext(composed.lower(), (void*)s);				// STEP 2
	}

	fs.newOrder();											// STEP 3
	BestMatchEntry** bme;
	bme=fs.getBestMatchesList();				// STEP 4
	
	// insert n best matches into search result list
	folderSearchResults->clearSongs();
	int noResults=60;
	for(int j=0; j<noResults && bme[j]; j++) {
		folderSearchResults->addSong( (Song*) bme[j]->objPtr);
//		cout << j << ".Position, value: " << bme[j]->sim << "\n";
	}
	folderListView->setCurrentItem( folderSearchResults );
	folderListView->setSelected( folderSearchResults, TRUE );
//	if(chosenFolder!=folderSearchResults)
	slotFolderChanged();
	songListView->setContentsPos( 0, 0);			// set scroll position to top
	QListViewItem* x=songListView->firstChild();
	int threshold=700;
	int selected=0;
	x->setSelected(true);											// select first anyway
	for(int j=0; j<noResults && bme[j]; j++) {
		if(bme[j]->sim>threshold) {
			x->setSelected(true);
			selected++;
		}
		x=x->nextSibling();
	}
	
	/*
	if(selected==1)
		qApp->beep();
	if(selected>1) {
//	 keyboard led stuff
	#include "Xlib.h"
	#include "XKBlib.h"
	
	disp=this->x11Display();
	unsigned int states;
	XkbGetIndicatorState(disp,XkbUseCoreKbd,&states);
	for (i=0,bit=1;i<XkbNumIndicators;i++,bit<<=1)
    {
        int flag;
        flag=states&bit;
        switch (i)
        {
         case 1: // Numlock
           if (flag)
           break;
         case 0: // CapsLock
           if (flag)
           break;
         case 2: //ScrollLock
           if (flag)
           break;
        }
    }
 extern	Bool	 XkbSetIndicatorMap(Display *, unsigned long,	XkbDescPtr);

		qApp->beep();			// different beep/feedback here for multiple matches
	}
	*/
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
	if(newFolder==folderSearchResults || newFolder==folderActual || newFolder==folderHistory) {
		songListView->setSorting(-1);
	}
	// switch from unsorted to sorted folder => switch sorting back on
	if( (chosenFolder==folderSearchResults || chosenFolder==folderActual || chosenFolder==folderHistory) &&
			(newFolder!=folderSearchResults && newFolder!=folderActual && newFolder!=folderHistory) ) {
		songListView->setSorting(0, TRUE);
	}
	
	// only allow dragging of songs in folderActual
	if(chosenFolder==folderActual)
		songListView->draggable=true; 	// disabled so far, until stable???
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
	addFolderContent(chosenFolder);
//	QApplication::restoreOverrideCursor();
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
	/*
	if(i==0) {
		for ( QListViewItem* f=folder->firstChild(); f; f=f->nextSibling() )
			addFolderContent((Folder*)f);	
	}
	*/
}

void YammiGui::addFolderContentSnappy()
{	
	int i=0;
	Song* s;
	for (s = folderToAdd->firstSong(); s && i<alreadyAdded; s = folderToAdd->nextSong(), i++ )
	{}
	
	SongListItem* lastOne=0;
	for (; s && i<=alreadyAdded+200; s = folderToAdd->nextSong(), i++ ) {
		lastOne=new SongListItem( songListView, s, lastOne);
	}
	alreadyAdded=i;
	if(s) {		// any songs left to add?
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
	Song* first=selectedSongs.first();
	if (selected>1) 							// more than one song selected
		label=QString("%1 songs selected").arg(selected);
	else
		label=first->displayName();
	songPopup->changeItem ( 113, label);
		
	// for each category: determine whether first song contained or not
	// we don't check whether all selected songs are contained, just first(old: current)
	int k=0;
	for(MyList* category=model->allCategories.first(); category; category=model->allCategories.next(), k++) {
		if(category->containsRef(first))
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
		selectedSongs.append(s);
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
	
	for(Song* s=selectedSongs.first(); s; s=selectedSongs.next()) {
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
		for(Song* s=selectedSongs.first(); s; s=selectedSongs.next()) {
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
	selectedSongs.append(s);
}

/// makes a list containing only the currently played song
void YammiGui::getCurrentlyPlayedSong()
{
	selectedSongs.clear();
	selectedSongs.append(lastPlayed.at(lastPlayed.count()-1));
}
			
/// makes a list of the currently selected songs
void YammiGui::getSelectedSongs()
{
	selectedSongs.clear();
	QListViewItem* i=songListView->firstChild();
	for(; i; i=i->itemBelow()) {						// go through list of songs
		if(i->isSelected()) {
			Song* s=((SongListItem*) i)->song();
			selectedSongs.append(s);
		}
	}
}

/// makes a list of all songs in database
void YammiGui::getAllSongs()
{
	selectedSongs.clear();
	for(Song* s=model->allSongs.first(); s; s=model->allSongs.next()) {
		selectedSongs.append(s);
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
		// let user choose directory
		dir=QFileDialog::getExistingDirectory(QString(model->config.baseDir), this, QString("hallo"), QString("caption"), true);
		if(dir.isNull())
			return;
		if(dir.right(1)=="/")						// strip trailing slash
			dir=dir.left(dir.length()-1);
	}
		
	// 2. dequeue + putback
	/*
  int pos=xmms_remote_get_playlist_pos(0);
  int length=xmms_remote_get_playlist_length(0);
	// remove all enqueued songs, but remember file names
	int toRemember=length-pos-1;
	QArray<gchar*> rem(toRemember);
	bool putBack=false;
	if(act==EnqueueAsNext || act==PlayNow) {
		// enqueue as next song
		// (we need to delete all following and reinsert them after inserting desired song)
		// (xmmsctrl does not provide any commands for a more convenient method)
		for(int i=0; i<toRemember; i++) {
			putBack=true;
			rem[i]=new gchar[200];
			strcpy(rem[i], xmms_remote_get_playlist_file(0, pos+1));
		    xmms_remote_playlist_delete(0, pos+1);
	  }
	  if(act==EnqueueAsNext)
	  	act=Enqueue;
	}
	*/
	
	// 3. determine delete mode
	bool deleteEntry=false;
	bool deleteFile=false;
	if(act==Delete) {
		DeleteDialog dd( this,  "testiii", true);
		int result=dd.exec();
		if(result==QDialog::Accepted) {
			deleteFile=dd.CheckBoxDeleteFile->isChecked();
			deleteEntry=dd.CheckBoxDeleteDbEntry->isChecked();
		}
		else
			return;
	}

	
	
	// OKAY: go through list of songs
	for(Song* s=selectedSongs.first(); s; s=selectedSongs.next()) {

		if(act==Delete) {
			if(deleteFile)	forSong(s, DeleteFile);
			if(deleteEntry)	forSong(s, DeleteEntry);
		}
		else
			forSong(s, act, dir);
//		if(act==PlayNow)									// we play first selected song...
//			act=Enqueue;										// ...and enqueue all others
		if(act==PrelistenStart || act==PrelistenMiddle || act==PrelistenEnd)
			break;
	
	}
	
	
	// afterwork...
	/*
	if(putBack) {
		// enqueue all remembered songs
		for(int i=0; i<toRemember; i++) {
			xmms_remote_playlist_add_url_string(0, rem[i]);
			delete(rem[i]);
	  }
	}
	*/

	// some OPs need view update
	if(deleteEntry) {
		updateView();
	}
	if(act==Enqueue || act==EnqueueAsNext || act==Dequeue) {
		folderActual->update(songsToPlay);
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
	songsToPlay.clear();
	for(int i=0; i<xmms_remote_get_playlist_length(0); i++) {
 		char buf[200];
		strcpy(buf, xmms_remote_get_playlist_file(0, i));
		Song* check=getSongEntryFromFilename(QString(buf));
		if(!check)	// song not found in database
			continue;
		songsToPlay.append(check);
	}
	// 3. delete all but the keepInXmms first songs
	for(int i=xmms_remote_get_playlist_length(0)-1; i>(model->config.keepInXmms); i--) {
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
		for(int i=0; i<(int)songsToPlay.count() && (i<model->config.keepInXmms || syncAll); i++) {
				gchar url[300];
				strcpy(url, songsToPlay.at(i)->location());
				xmms_remote_playlist_add_url_string(0, url);
		}
		return;
	}
	
	// okay, at least one song in xmms playlist
		
			
	// iterate through first keepInXmms songs in Xmms playlist
	// if different than corresponding yammi entry => delete
	// if playlist too short => insert yammi entries
	for(int i=1; i<model->config.keepInXmms || ( syncAll && i<(int)songsToPlay.count() ); i++) {
		
		// check whether playlist entry existing
		if(i<(int)xmms_remote_get_playlist_length(0)) {
			// yes, existing!
	 		char buf[300];
			strcpy(buf, xmms_remote_get_playlist_file(0, i));
			Song* check=getSongEntryFromFilename(QString(buf));
			if(i<(int)songsToPlay.count()) {
				Song* s=songsToPlay.at(i);
				if(check==s)
					continue;		// okay, both are the same
				// ups, different!
				cout << "song entry " << i << " different => deleting and replacing all following!\n";
				for(int toDel=xmms_remote_get_playlist_length(0)-1; toDel>=i; toDel--) {
					// delete all following
					xmms_remote_playlist_delete(0, toDel);
				}
				for(int toInsert=i; toInsert<model->config.keepInXmms; toInsert++) {
					// reinsert
					gchar url[300];
					Song* s=songsToPlay.at(toInsert);
					strcpy(url, s->location());
					xmms_remote_playlist_add_url_string(0, url);
				}
				return;
			}
			else {
				cout << "xmms playlist longer than yammi playlist, deleting!\n";
				for(int toDel=xmms_remote_get_playlist_length(0)-1; toDel>=i; toDel--) {
					// delete all following
					xmms_remote_playlist_delete(0, toDel);
				}
			}
		}
		else {	
			// playlist too short => check whether songs in songsToPlay
			if(i<(int)songsToPlay.count()) {
				cout << "playlist was too short, filling up\n";
				Song* s=songsToPlay.at(i);
				gchar url[300];
				strcpy(url, s->location());
				xmms_remote_playlist_add_url_string(0, url);
			}
			else {
				cout << "yammi playlist too short to fill up!\n";
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
		folderActual->update(songsToPlay);
		syncYammi2Xmms();
		xmms_remote_playlist_next(0);
		if(!xmms_remote_is_playing(0))
			xmms_remote_play(0);
		mainStatusBar->message(QString("playing %1").arg(s->displayName()), 2000);
		break;
		
	case Enqueue:								// enqueue at end
		if(s->filename=="" || !s->checkReadability())
			return;
		if(model->config.managePlaylist) {
			songsToPlay.append(s);
			mainStatusBar->message(QString("%1 enqueued at end").arg(s->displayName()), 3000);
		}
		else {
			gchar url[300];
			strcpy(url, s->location());
			xmms_remote_playlist_add_url_string(0, url);
			mainStatusBar->message(QString("%1 enqueued at end").arg(s->displayName()), 3000);
		}
		break;
				
	case EnqueueAsNext: {				// enqueue as next
		if(s->filename=="" || !s->checkReadability())
			return;
		if(model->config.managePlaylist) {
			songsToPlay.insert(1, s);
		}
		else {
				// enqueue as next song
				// (we need to delete all following and reinsert them after inserting desired song)
				// (xmmsctrl does not provide any commands for a more convenient method)
  	  	int pos=xmms_remote_get_playlist_pos(0);
    		int length=xmms_remote_get_playlist_length(0);
		
			// remove all enqueued songs, but remember file names
			int toRemember=length-pos-1;
			if(toRemember>0) {
				QArray<gchar*> rem(toRemember);
				for(int i=0; i<toRemember; i++) {
					rem[i]=new gchar[200];
					strcpy(rem[i], xmms_remote_get_playlist_file(0, pos+1));
			    	xmms_remote_playlist_delete(0, pos+1);
			  }
				forSong(s, Enqueue);				// enqueue desired song
				mainStatusBar->message(QString("%1 enqueued as next").arg(s->displayName()), 2000);
				// enqueue all remembered songs
				for(int i=0; i<toRemember; i++) {
					xmms_remote_playlist_add_url_string(0, rem[i]);
					delete(rem[i]);
			  }
			}
			else {
				forSong(s, Enqueue);				// enqueue desired song
			}
		}
		mainStatusBar->message(QString("%1 enqueued as next").arg(s->displayName()), 2000);
	}
		break;
	
	case Dequeue:
		// search for selected song and dequeue
		for(int i=1; i<(int)songsToPlay.count(); i++) {
			Song* check=songsToPlay.at(i);
			if(check==s) {
				songsToPlay.remove(i);
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
		selectedSongs.append(s);
		forSelectionSongInfo();
	  break;
	}
	
	case CheckConsistency:
	 {
		if(s->filename=="")
			return;
		if(s->checkConsistency()==false) {
			if(model->problematicSongs.contains(s)==0)
				model->problematicSongs.append(s);
		}
	 }
		break;
		
	case Delete: {
		DeleteDialog dd( this,  "testiii", true);
		int result=dd.exec();
		if(result==QDialog::Accepted) {
			if(dd.CheckBoxDeleteFile->isChecked()) {
				forSong(s, DeleteFile);				// 1. move songfile to trash
				//mmm s->deleteFile();
				mainStatusBar->message(QString("%1 removed (file)").arg(s->displayName()), 2000);
			}
			if(dd.CheckBoxDeleteDbEntry->isChecked()) {
				forSong(s, DeleteEntry);			// 2. remove from database
				//mmm s->deleteFile();
				mainStatusBar->message(QString("%1 removed (db entry)").arg(s->displayName()), 2000);
			}
		}
	} break;
	
	case DeleteEntry:							// delete db entry
		model->allSongs.removeRef(s);
		model->allSongsChanged(true);
		break;
	
	case DeleteFile:						// move songfile to trash
		s->deleteFile(model->config.baseDir+"trash");
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
//		QMessageBox::information( this, "Yammi", "renaming from "+s->location()+" to "+newname+" failed!", "Fine." );
//			model->allSongsChanged(true);
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
																					"Version 0.1, 12-2001 by Oliver N�lle\n\n"+
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
		Song* save=songsToPlay.getFirst();
		songsToPlay.clear();
		songsToPlay.append(save);
		folderActual->update(songsToPlay);
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
 * - updating FolderActual
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
			
			Song* current=getSongEntryFromFilename(QString(file));

			// song entry found
			if(current!=0) {
				
				lastPlayed.append(current);			// append current...
				// shorten history?
				/*while(lastPlayed.count()>10) {		// model->config.noLastPlayed
					lastPlayed.remove((unsigned int)0);
				}*/
		  	
		  	if(model->config.managePlaylist) {
		  		// take over playlist management from xmms
		  		
		  		// remove played song(s) from xmms playlist
		  		int i;
		  		for(i=0; xmms_remote_get_playlist_pos(0)>0; i++) {
		  			xmms_remote_playlist_delete(0, 0);
		  			songsToPlay.removeFirst();
		  		}
		  		cout << "removed " << i << " entries from xmms/yammi\n";
		  		
		  		// should just check, not change anything:
		  		syncYammi2Xmms();
		  		
					folderActual->update(songsToPlay);
					if(chosenFolder==folderActual)
						slotFolderChanged();
					else
						songListView->triggerUpdate();					
				}
				// end of playlist management
				else {
		  	
			  	// get next 10 songs...
					toPlay.clear();
		  		for(int i=xmms_remote_get_playlist_pos(0)+1; i<xmms_remote_get_playlist_length(0); i++)
					{	  	
						strcpy(file, xmms_remote_get_playlist_file(0, i));
						Song* stillToPlay=getSongEntryFromFilename(QString(file));
						if(stillToPlay)
							toPlay.append(stillToPlay);
					}

					if(!songListView->dragging)
						folderActual->update(toPlay);
				
				}
				
				// set title to currently played song
				setCaption("Yammi: "+current->displayName());
				
				// logging
       	if(model->config.logging) {
					MyDateTime now = QDateTime::currentDateTime();
     			QString logentry=QString("<%1><%2><%3>\n").arg(current->artist).arg(current->title).arg(now.writeToString());

        		
					QFile logfile( model->config.yammiBaseDir+"/logfile.log" );
     			if ( !logfile.open( IO_ReadWrite  ) )
     				return;
     			logfile.at(logfile.size());
        		
   	  		if(logfile.writeBlock(logentry, logentry.length()) < 1) {
   					cout << "error writing to logfile\n";
   				}
   				logfile.close();
     		}

			}
			else {				// song not found in database
				setCaption("Yammi - song not in database");
			}	
			
			// update view, if folderActual is currently shown folder
//			QListViewItem *i = folderListView->currentItem();
//			Folder* chosenFolder = ( Folder* )i;
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
Song* YammiGui::getSongEntryFromFilename(QString filename)
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
	QString artist(QInputDialog::getText( caption, message, QString("Held"), &ok, this ));
	if(!ok)
		return;
	
	caption="Enter title";
	message="Please enter title";
	QString title(QInputDialog::getText( caption, message, QString("tollesLied"), &ok, this ));
	if(!ok)
		return;
	
	QString filename=QString("%1%2 - %3.mp3").arg(model->config.scanDir).arg(artist).arg(title);
	QString cmd=QString("grabAndEncode %1 \"%2\" \"%3\" \"%4\" &").arg(trackNr).arg(artist).arg(title).arg(filename);
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
//	cout << "key(): " << e->key() << "text(): " << e->text() << "ascii(): " << e->ascii() << "\n";
	int key=e->key();
	switch(key) {
		case Key_F1:
			xmms_skipBackward();
			break;
		case Key_F2:
			xmms_skipForward();
			break;
		case Key_F3:
			xmms_playPause();
			break;
		
		case Key_F5:
			forAllSelected(PlayNow);
			break;
		case Key_F6:
			forAllSelected(EnqueueAsNext);
			break;
		case Key_F7:
			forAllSelected(Enqueue);
			break;
		case Key_F8:
			forAllSelected(SongInfo);
			break;
			
		case Key_F12:	{									// exit program (press twice for shutting down computer)
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
		
/*
		case Key_1:	{					// determine how many songs to play before shutting down
		case Key_2:
		case Key_0:
		case Key_Plus:
			cout << "number pressed\n";
		 if(shuttingDown==0) {
		 	cout << "playing n songs before shutting down...\n";
		 }
		
		} break;
*/
		
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
		case Key_Escape:
			searchField->setText("");
			searchField->setFocus();
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
	QString cmd1b=QString("kill -9 `ps h -o \"%p\" -C aplay`");
	system(cmd1b);
	// now play song via mpg123 or aplay on second sound device /dev/dsp1
	if(s->filename.right(3)=="mp3") {
		QString skip=QString(" --skip %1").arg(seconds*skipTo*38/100);
		QString cmd2=QString("mpg123 -a /dev/dsp1%1 \"%2\" &").arg(skip).arg(s->location());
		cout << "length: " << seconds << ", " << cmd2 << "\n";
		system(cmd2);
	}
	if(s->filename.right(3)=="wav") {
		QString cmd2=QString("aplay -c1 \"%1\" &").arg(s->location());
		system(cmd2);
	}
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
	songsToPlay.clear();
	for ( QListViewItem* item=songListView->firstChild(); item; item=item->nextSibling() ) {
		songsToPlay.append(((SongListItem*)item)->song());
	}
	syncYammi2Xmms();
	songListView->dragging=false;
	songListView->setCursor(Qt::arrowCursor);
}