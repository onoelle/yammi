/***************************************************************************
                          yammigui.cpp  -  description
                             -------------------
    begin                : Tue Feb 27 2001
    copyright            : (C) 2001 by Brian O.Nölle
    email                : yammi-developer@lists.sourceforge.net
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
#include "yammiicons.h"
#include "yammimodel.h"

// dialog includes
#include "preferencesdialog.h"
#include "DeleteDialog.h"
#include "updatedatabasedialog.h"
#include "updatedatabasemediadialog.h"
#include "ConsistencyCheckDialog.h"
#include "ApplyToAllBase.h"

#include "CMP3Info.h"
#include "ConsistencyCheckParameter.h"


#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kaccel.h>
#include <kfiledialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kurlrequesterdlg.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kmessagebox.h>
#include <kprogress.h>
#include <kdebug.h>

#include <qslider.h>


// qt includes (non gui)
// #include <qregexp.h>
// #include <qdir.h>
// #include <qtextstream.h>
// #include <qstring.h>
// #include <qdatetime.h>
// #include <qlist.h>
// #include <qevent.h>

// qt includes (gui-stuff)
// #include <qtooltip.h>
#include <qheader.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qhbox.h>
// #include <qlabel.h>
#include <qpushbutton.h>
// #include <qhbox.h>
// #include <qvbox.h>
// #include <qpainter.h>
// #include <qpalette.h>
#include <qpopupmenu.h>
// #include <qinputdialog.h>
// #include <qmenudata.h>
// #include <qmenubar.h>
#include <qlistview.h>
#include <qtooltip.h>
// #include <qlineedit.h>
// #include <qmultilineedit.h>
// #include <qlayout.h>
// #include <qsplitter.h>
// #include <qstatusbar.h>
// 

#include "yammimodel.h"
#include "song.h"
#include "songentry.h"
#include "songentryint.h"
#include "songentryint2.h"
#include "songentrystring.h"
#include "songentrytimestamp.h"
#include "songlistitem.h"
#include "songinfo.h"
#include "fuzzsrch.h"
#include "folder.h"
#include "foldergroups.h"
#include "foldercategories.h"
#include "foldermedia.h"
#include "foldersorted.h"
#include "mylistview.h"
#include "lineeditshift.h"
#include "mediaplayer.h"

#include "dummyplayer.h"
#ifdef ENABLE_XMMS
#include "xmmsplayer.h"
#else
#define XmmsPlayer DummyPlayer
#endif 

//Noatun and Arts player always enabled
#include "noatunplayer.h"
#include "artsplayer.h"


static QString columnName[] = { i18n("Artist"), i18n("Title"), i18n("Album"), i18n("Length"),
		i18n("Year"), i18n("TrackNr"), i18n("Genre"), i18n("AddedTo"), i18n("Bitrate"),
		i18n("Filename"), i18n("Path"), i18n("Comment"), i18n("Last Played") };


extern YammiGui* gYammiGui;

/////////////////////////////////////////////////////////////////////////////////////////////
YammiGui::YammiGui( ) : KMainWindow( )
{

//FIXME Warning!!!! gYammiGui is (should) be set in main, but there are calls in *this* constructor 
// that rely on the variable pointing to the yammi instance.. since the variable in main gets assigned
// after the constructor has finished, we need to do this here, until the code is cleaned up
	gYammiGui = this;
	
	m_config.loadConfig( );
	currentSong = 0;
	chosenFolder = 0;
  selectionMode = SELECTION_MODE_USER_SELECTED;
	
	// set up model
	model = new YammiModel( this );
	
	loadMediaPlayer( );
	// connect player and yammi via signals
	connect( player, SIGNAL(playlistChanged()), this, SLOT(updatePlaylist()) );
	connect( player, SIGNAL(statusChanged()), this, SLOT(updatePlayerStatus()) );
	
	setupActions( );
	createMenuBar( );
	createMainWidget( );
	createFolders( );
	
	// final touches before start up
	isScanning = false;
	controlPressed = false;
	shiftPressed = false;
	toFromRememberFolder=folderAll;
	
  // finish initialization of player
	player->syncPlayer2Yammi(&(model->songsToPlay));
	player->syncYammi2Player(false);
	checkPlaylistAvailability();
  
	// check whether player is playing, if not: start playing!
	if(folderActual->songlist().count() > 0 && player->getStatus()!=PLAYING ) 
		player->play();
    
	// TODO: this should be probably done by a thread owned by the media player
	connect( &checkTimer, SIGNAL(timeout()), player, SLOT(check()) );
	checkTimer.start( 100, FALSE );

	// connect all timers
	connect( &regularTimer, SIGNAL(timeout()), SLOT(onTimer()) );
	regularTimer.start( 500, FALSE );	// call onTimer twice a second
	
	//let KDE save and restore the main window settings (geometry, toolbar position, etc)
	//setAutoSaveSettings( );
	//FIXME - why setAutoSaveSettings is not working...it sounds like KDE should do this automatically
	// but it doesn't
	readOptions( );
}

void YammiGui::loadDatabase( const QString &db )
{
	if(!db.isEmpty())
		m_config.dbFile = db;
	model->readSongDatabase(  );
	model->readCategories();
	model->readHistory();
	//update dynamic folders based on database contents
	updateView(true);
}

void YammiGui::saveOptions()
{
	kdDebug()<<"saveOptions()"<<endl;
	KConfig *cfg = kapp->config();
	KActionCollection *ac = actionCollection( );
	
	cfg->setGroup("General Options");
	cfg->writeEntry("Geometry", size());
	//Toolbars
	cfg->writeEntry("Show MainToolbar", static_cast<KToggleAction*>(ac->action("MainToolbar"))->isChecked());
	cfg->writeEntry("MainToolbar Pos", (int) toolBar("MainToolbar")->barPos());
	cfg->writeEntry("Show MediaPlayerToolbar", static_cast<KToggleAction*>(ac->action("MediaPlayerToolbar"))->isChecked());
	cfg->writeEntry("MediaPlayerToolbar Pos", (int) toolBar("MediaPlayerToolbar")->barPos());
	cfg->writeEntry("Show SongActionsToolbar", static_cast<KToggleAction*>(ac->action("SongActionsToolbar"))->isChecked());
	cfg->writeEntry("SongActionsToolbar Pos", (int) toolBar("SongActionsToolbar")->barPos());
	cfg->writeEntry("Show PrelistenToolbar", static_cast<KToggleAction*>(ac->action("PrelistenToolbar"))->isChecked());
	cfg->writeEntry("PrelistenToolbar Pos", (int) toolBar("PrelistenToolbar")->barPos());
	cfg->writeEntry("Show RemovableMediaToolbar", static_cast<KToggleAction*>(ac->action("RemovableMediaToolbar"))->isChecked());
	cfg->writeEntry("RemovableMediaToolbar Pos", (int) toolBar("RemovableMediaToolbar")->barPos());
	cfg->writeEntry("Show SleepModeToolbar", static_cast<KToggleAction*>(ac->action("SleepModeToolbar"))->isChecked());
	cfg->writeEntry("SleepModeToolbar Pos", (int) toolBar("SleepModeToolbar")->barPos());
	//Statsbar
// 	cfg->writeEntry("Show Statusbar",viewStatusBar->isChecked());

	//TODO
	//save: current folder, autoplay mode/folder, column order and visibility
}


void YammiGui::readOptions()
{
	kdDebug()<<"readOptions()"<<endl;
	KConfig *cfg = kapp->config();
	KActionCollection *ac = actionCollection( );
	
	cfg->setGroup("General Options");

// Toolbar settings status settings
	bool b;
	KToolBar::BarPosition pos;
	b = cfg->readBoolEntry("Show MainToolbar", true);
	static_cast<KToggleAction*>(ac->action("MainToolbar"))->setChecked(b);
	pos=(KToolBar::BarPosition) cfg->readNumEntry("MainToolbar Pos", KToolBar::Top);
	toolBar("MainToolbar")->setBarPos(pos);
	toolbarToggled("MainToolbar");
	b = cfg->readBoolEntry("Show MediaPlayerToolbar", true);
	static_cast<KToggleAction*>(ac->action("MediaPlayerToolbar"))->setChecked(b);
	pos=(KToolBar::BarPosition) cfg->readNumEntry("MediaPlayerToolbar Pos", KToolBar::Top);
	toolBar("MediaPlayerToolbar")->setBarPos(pos);
	toolbarToggled("MediaPlayerToolbar");
	b = cfg->readBoolEntry("Show SongActionsToolbar", true);
	static_cast<KToggleAction*>(ac->action("SongActionsToolbar"))->setChecked(b);
	pos=(KToolBar::BarPosition) cfg->readNumEntry("SongActionsToolbar Pos", KToolBar::Top);
	toolBar("SongActionsToolbar")->setBarPos(pos);
	toolbarToggled("SongActionsToolbar");
	b = cfg->readBoolEntry("Show PrelistenToolbar", true);
	static_cast<KToggleAction*>(ac->action("PrelistenToolbar"))->setChecked(b);
	pos=(KToolBar::BarPosition) cfg->readNumEntry("PrelistenToolbar Pos", KToolBar::Top);
	toolBar("PrelistenToolbar")->setBarPos(pos);
	toolbarToggled("PrelistenToolbar");
	b = cfg->readBoolEntry("Show RemovableMediaToolbar", true);
	static_cast<KToggleAction*>(ac->action("RemovableMediaToolbar"))->setChecked(b);
	pos=(KToolBar::BarPosition) cfg->readNumEntry("RemovableMediaToolbar Pos", KToolBar::Top);
	toolBar("RemovableMediaToolbar")->setBarPos(pos);
	toolbarToggled("RemovableMediaToolbar");
	b = cfg->readBoolEntry("Show SleepModeToolbar", true);
	static_cast<KToggleAction*>(ac->action("SleepModeToolbar"))->setChecked(b);
	pos=(KToolBar::BarPosition) cfg->readNumEntry("SleepModeToolbar Pos", KToolBar::Top);
	toolBar("SleepModeToolbar")->setBarPos(pos);
	toolbarToggled("SleepModeToolbar");

     //Statusbar
// 	b = cfg->readBoolEntry("Show Statusbar", true);
// 	viewStatusBar->setChecked(b);
//   slotViewStatusBar();

	QSize size=cfg->readSizeEntry("Geometry");
	if(!size.isEmpty())
	{
		resize(size);
	}
	
//FIXME 
	//TODO
	//save: current folder, autoplay mode/folder, column order and visibility
}



void YammiGui::saveProperties(KConfig *config)
{
	kdDebug()<<"saveProperties(KConfig *config)"<<endl;
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored

//     if (!m_view->currentURL().isNull())
//         config->writeEntry("lastURL", m_view->currentURL());
}

void YammiGui::readProperties(KConfig *config)
{
	kdDebug()<<"readProperties(KConfig *config)"<<endl;
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'

//     QString url = config->readEntry("lastURL");
// 
//     if (!url.isNull())
//         m_view->openURL(KURL(url));
}


bool YammiGui::queryClose()
{kdDebug()<<"queryClose()"<<endl;
	if(model->allSongsChanged() || model->categoriesChanged()) 
	{
		QString msg=i18n("The Song Database has been modified.\nDo you want to save the changes?");
		switch( KMessageBox::warningYesNoCancel(this,msg,i18n("Database modified")))
		{
			case KMessageBox::Yes:
						model->save( );
						break;
			case KMessageBox::No:
						break;
			case KMessageBox::Cancel:
						return false;
		}
	}
	else
	{ //the db has not changed, but save history anyways...(only if more than 2 songs to add)
		if(m_config.logging && model->songsPlayed.count()>2)
			model->saveHistory();
	}
return true;
}

bool YammiGui::queryExit()
{kdDebug()<<"queryExit()"<<endl;
	player->quit( );
	saveOptions();
	return true;
}

// void YammiGui::finishInitialization()
// {
//   // restore session settings
// 	statusBar( )->message(tr("Welcome to Yammi ")+m_config.yammiVersion, 10000);
// 	changeToFolder(chosenFolder, true);
// 	songListView->setSelected( songListView->firstChild(), TRUE );
// 	songListView->setCurrentItem( songListView->firstChild() );
// 	updateSongPopup();
//  
// }

void YammiGui::shutdownSequence( )
{
	
	QString msg(i18n("Shutting down in %1 seconds"));
	KProgressDialog d(this,0,i18n("Shutting down..."));
	
	d.setMinimumDuration(0);
	d.setAllowCancel(true);
	QTimer *t;
	int total = 10;
	d.progressBar()->setTotalSteps(total);
	for( int i = 0; i < 10; ++i )
	{
		d.setLabel( msg.arg(total-i) );
		d.progressBar()->setProgress( i );
		t = new QTimer();
		t->start(1000,true);
		while(t->isActive())
		{
			kapp->processEvents();
		}
		delete t;
		if( d.wasCancelled() )
		{
			kdDebug()<<"Shutdown cancelled"<<endl;
			changeSleepMode();
			return;
		}
	}
	model->save( );
	if(m_config.shutdownScript.isEmpty())
		this->close();
	else
		system(m_config.shutdownScript+" &");
}

void YammiGui::toolbarToggled( const QString& name )
{
	QString n = name;
	if(n.isNull())
	{//see if we were called by an action
		const QObject *s = 0L;
		s = sender();
		if(s)
		{
			n = s->name();
		}
	}
	KToggleAction *action = dynamic_cast<KToggleAction*>(actionCollection()->action( n ));
	if(!action)
	{
		kdError()<<"toolbarToggled( const QString& name ) : action not found n = "
		<<n<<" (name = "<<name<<")"<<endl;
		return;
	}
	if(!action->isChecked())
	{
		toolBar(n)->hide();
	}
	else
	{
		toolBar(n)->show();
	}
}

//****************************************************************************************************//
YammiGui::~YammiGui()
{
  player->syncYammi2Player(true);    
  delete player;
}


/**
 * This slot is called on changes in the playlist (model->songsToPlay),
 * signalled by the mediaplayer or on changes from within yammigui (enqueing, dequeing songs, ...)
 */
void YammiGui::updatePlaylist()
{
  if(folderActual->songlist().count()>0) {
    model->currentSongFilenameAtStartPlay=folderActual->firstSong()->location();
  }

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

/**
 * TODO: document, when is this method called???
 */
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
  // but first check, whether already in there as last entry
  // (due to xmms status change bug, we would sometimes insert a song twice)
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
  if(newSong==0) 
  {
    setCaption(i18n("Yammi - not playing"));
    currentFile="";
    m_seekSlider->setValue(0);
    m_seekSlider->setRange(0, 0);
    return;
  }
  // TODO: take swapped file?
  currentFile=newSong->location();
  currentSongStarted=currentSongStarted.currentDateTime();

  if(m_sleepMode)
  {
	int left = m_sleepModeSpinBox->value() - 1;
	if(left > 0 )
		m_sleepModeSpinBox->setValue(left);
	else
		shutdownSequence();
  }

  setCaption("Yammi: "+currentSong->displayName());

  // setup songSlider
  kdDebug() << "calling setRange, length: " << currentSong->length*1000 <<endl;
  m_seekSlider->setValue(0);
  m_seekSlider->setRange(0, currentSong->length*1000);
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
    m_playPauseAction->setIcon("player_pause");
    m_playPauseAction->setText(i18n("Pause"));
    if(currentSong==0 || currentFile!=player->getCurrentFile()) {
      handleLastSong(currentSong);
      (model->getSongFromFilename(player->getCurrentFile()));
    }
  }
  else {
    m_playPauseAction->setIcon("player_play");
    m_playPauseAction->setText(i18n("Play"));
  }

  // check, if we stopped because playlist empty and sleep mode activated
  if(m_sleepMode && player->getStatus() == STOPPED && model->songsToPlay.count()==0)
  {
	shutdownSequence( );
  }
}



QString YammiGui::getColumnName(int column)
{
  return QString(columnName[column]);
}



/**
 * Switches to the folder best matching the search term.
 * Uses yammi's fuzzy search capabilities.
 */
void YammiGui::gotoFuzzyFolder(bool backward)
{//HERE
  QString searchStr=m_searchField->text();
	if(searchStr.length()<1) {
    return;
  }

  if(fuzzyFolderName!=searchStr) {
    // search term changed => do fuzzy search with new search term
    backward=false;
    fuzzyFolderNo=-1;
    fuzzyFolderName=searchStr;

    searchStr=" "+searchStr+" ";
    FuzzySearch fs;
    fs.initialize(searchStr.lower(), 2, 4);			// STEP 1


    for(QListViewItem* i=folderListView->firstChild(); i; i=i->nextSibling()) {
      Folder* f=(Folder*)i;
      fs.checkNext(f->folderName().lower(), (void*)f);				// STEP 2 (top-level folder)
      for(QListViewItem* i2=i->firstChild(); i2; i2=i2->nextSibling()) {
        Folder* f2=(Folder*)i2;
        fs.checkNext(f2->folderName().lower(), (void*)f2);				// STEP 2 (subfolders)
      }
    }

    // step 3 does not work here? => not necessary anyway...
//	fs.newOrder();											// STEP 3
    BestMatchEntry** bme;
    bme=fs.getBestMatchesList();				// STEP 4
    int noResults=0;
    for(; noResults<FUZZY_FOLDER_LIST_SIZE && noResults<m_config.searchMaximumNoResults && bme[noResults]; noResults++) {
      Folder* f=(Folder*)(bme[noResults]->objPtr);
//      cout << noResults << ".: " << f->folderName() << ", match: " << bme[noResults]->sim << "\n";
      fuzzyFolderList[noResults]=f;
    }
    // clear rest of fzzyFolderList (if not enough matching entries
    for(; noResults<FUZZY_FOLDER_LIST_SIZE; noResults++) {
      fuzzyFolderList[noResults]=0;
    }
  }

  // switch to first (or on pressing key multiple times n-th) best match folder
  if(backward) {
    if(fuzzyFolderNo<=0) {
      return;
    }
    fuzzyFolderNo--;
  }
  else {
    if(fuzzyFolderNo>=FUZZY_FOLDER_LIST_SIZE-1 || fuzzyFolderList[fuzzyFolderNo+1]==0) {
      return;
    }
    fuzzyFolderNo++;
  }
  Folder* switchTo=fuzzyFolderList[fuzzyFolderNo];
  if(switchTo!=0) {
    changeToFolder(switchTo);
  }
}


Folder* YammiGui::getFolderByName(QString folderName)
{
  for(QListViewItem* i=folderListView->firstChild(); i; i=i->nextSibling()) {
    Folder* f=(Folder*)i;
    if(f->folderName()==folderName) {
      return f;
    }
    for(QListViewItem* i2=i->firstChild(); i2; i2=i2->nextSibling()) {
      Folder* f2=(Folder*)i2;
      if(f2->folderName()==folderName) {
        return f2;
      }
    }
  }
  return 0;
}


/// updates the automatically calculated folders after changes to song database
void YammiGui::updateView(bool startup)
{
  for(Song* s=model->allSongs.firstSong(); s; s=model->allSongs.nextSong()) {
		s->classified=false;
  }
	folderArtists->update(&(model->allSongs), MyList::ByArtist);
	folderAlbums->update(&(model->allSongs), MyList::ByAlbum);
	folderGenres->update(&(model->allSongs), MyList::ByGenre);
	folderYears->update(&(model->allSongs), MyList::ByYear);
	folderMedia->update(&(model->allSongs));
	folderSearchResults->update(searchResults);
  
  if(startup) {
    // this is only necessary on startup
    folderActual->update(model->songsToPlay);
    folderCategories->update(model->allCategories, model->categoryNames);
    folderMedia->update(&(model->allSongs));
    createSongPopup();
  }

	model->unclassifiedSongs.clear();
	for(Song* s=model->allSongs.firstSong(); s; s=model->allSongs.nextSong()) {
		if(!s->classified)
			model->unclassifiedSongs.appendSong(s);
	}
	folderUnclassified->update(model->unclassifiedSongs);

  // recently added songs:
  // at least 20, but if delta is less than 1 hour, add the following one, too
  model->recentSongs.clear();
  model->allSongs.setSortOrderAndSort(MyList::ByAddedTo, true);
  int count=0;
  int timeDelta=0;  // time delta between two additions in seconds
  Song* last=0;
	for(Song* s=model->allSongs.firstSong(); s; s=model->allSongs.nextSong()) {
    count++;
    if(last!=0) {
      timeDelta=s->addedTo.secsTo(last->addedTo);
    }
    last=s;
    if(count>20 && timeDelta>60*60) {
      break;
    }
    model->recentSongs.appendSong(s);
	}
  folderRecentAdditions->update(model->recentSongs);
  if(!startup) {
    folderContentChanged();
  }
}

/// returns true if the column should be shown
bool YammiGui::columnIsVisible(int column)
{
  return columnVisible[column];
}


int YammiGui::mapToRealColumn(int column)
{
  return realColumnMap[column];
}

void YammiGui::mapVisibleColumnsToOriginals()
{
  int visibleColumn=0;
  for(int realColumn=0; realColumn<MAX_COLUMN_NO; realColumn++) {
    if(columnVisible[realColumn]) {
      realColumnMap[visibleColumn]=realColumn;
      visibleColumn++;
    }
  }
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
  int current=0;

  if(chosenFolder==folderActual) {
		songListView->addColumn( i18n("Pos"), 35);
    current++;
  }
	if(chosenFolder==folderHistory || chosenFolder==folderSongsPlayed) {
		songListView->addColumn( i18n("Played on"), 135);
    current++;
  }
	if(chosenFolder==folderSearchResults) {
		songListView->addColumn( i18n("Match"), 45);
    current++;
  }
	if(chosenFolder==folderProblematic) {
		songListView->addColumn( i18n("Reason"), 120);
    current++;
  }
	if(((QListViewItem*)chosenFolder)->parent()==folderCategories) {
		songListView->addColumn( i18n("Pos"), 35);
    current++;
  }

  if(columnIsVisible(COLUMN_ARTIST)) {
    songListView->addColumn( getColumnName(COLUMN_ARTIST), 200);
    current++;
  }
	if(columnIsVisible(COLUMN_TITLE)) {
    songListView->addColumn( getColumnName(COLUMN_TITLE), 200);
    current++;
  }
  if(columnIsVisible(COLUMN_ALBUM)) {
    songListView->addColumn( getColumnName(COLUMN_ALBUM), 150);
    current++;
  }
	if(columnIsVisible(COLUMN_LENGTH)) {
    songListView->addColumn( getColumnName(COLUMN_LENGTH), 50);
    songListView->setColumnAlignment( current, Qt::AlignRight );
    current++;
  }
  if(columnIsVisible(COLUMN_YEAR)) {
    songListView->addColumn( getColumnName(COLUMN_YEAR), 50);
    songListView->setColumnAlignment( current, Qt::AlignRight );
    current++;
  }
	if(columnIsVisible(COLUMN_TRACKNR)) {
    songListView->addColumn( getColumnName(COLUMN_TRACKNR), 40);
    songListView->setColumnAlignment( current, Qt::AlignRight );
    current++;
  }
	if(columnIsVisible(COLUMN_GENRE)) {
    songListView->addColumn( getColumnName(COLUMN_GENRE), 40);
    current++;
  }
	if(columnIsVisible(COLUMN_ADDED_TO)) {
    songListView->addColumn( getColumnName(COLUMN_ADDED_TO), 60);
    songListView->setColumnAlignment( current, Qt::AlignRight );
    current++;
  }
	if(columnIsVisible(COLUMN_BITRATE)) {
    songListView->addColumn( getColumnName(COLUMN_BITRATE), 40);
    songListView->setColumnAlignment( current, Qt::AlignRight );
    current++;
  }
	if(columnIsVisible(COLUMN_FILENAME)) {
    songListView->addColumn( getColumnName(COLUMN_FILENAME), 80);
    current++;
  }
	if(columnIsVisible(COLUMN_PATH)) {
    songListView->addColumn( getColumnName(COLUMN_PATH), 80);
    current++;
  }
	if(columnIsVisible(COLUMN_COMMENT)) {
 	  songListView->addColumn( getColumnName(COLUMN_COMMENT), 100);
    current++;
  }
	if(columnIsVisible(COLUMN_LAST_PLAYED)) {
    songListView->addColumn( getColumnName(COLUMN_LAST_PLAYED), 100);
    current++;
  }

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
  // now move those that have not existed before to the front
  target=0;
  for(int j=0; j<noNewColumns; j++) {
    if(!exists[j]) {
      header->moveSection(j, target);
      target++;
    }
  }
  saveColumnSettings();
  mapVisibleColumnsToOriginals();
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
//    cout << "j: " << j << ", section: " << section << "label: " << header->label(section) << ", size: " << header->sectionSize(section) << "\n";
  }
}

/// opens the preferences dialogue
void YammiGui::setPreferences()
{
	PreferencesDialog d(this, "preferencesDialog", true, &m_config);

	// show dialog
	int result=d.exec();

	if(result==QDialog::Accepted) {
		updateSongPopup();
		m_config.saveConfig();
	}
}


/**
 * Updates the songPopup submenus with available categories and plugins
 */
void YammiGui::updateSongPopup()
{
  cout << "updating song popup\n";
  playListPopup->clear();
	playListPopup->insertItem(QIconSet( QPixmap(newCategory_xpm)), i18n("New Category..."), this, SLOT(toCategory(int)), 0, 9999);
	for(unsigned int i=0; i<model->categoryNames.count(); i++) {
		playListPopup->insertItem(QIconSet( QPixmap(in_xpm)), model->categoryNames[i], this, SLOT(toCategory(int)), 0, 10000+i);
	}
  pluginPopup->clear();
  for(unsigned int i=0; i<m_config.pluginMenuEntry.count(); i++) {
		pluginPopup->insertItem( m_config.pluginMenuEntry[i], this, SLOT(forSelectionPlugin(int)), 0, 2000+i);
	}
}


/**
 * returns a specific icon for popup menu
 * (according to configured action for doubleclick, etc.)
 * TODO: fix for xmlui
 */
QIconSet YammiGui::getPopupIcon(Song::action whichAction)
{
	if(m_config.doubleClickAction==whichAction)
		return QIconSet(QPixmap(defaultDoubleClick_xpm));
	if((m_config.middleClickAction==whichAction))
		return QIconSet(QPixmap(defaultMiddleClick_xpm));
	if((m_config.controlClickAction==whichAction))
		return QIconSet(QPixmap(defaultControlClick_xpm));
	if((m_config.shiftClickAction==whichAction))
		return QIconSet(QPixmap(defaultShiftClick_xpm));
	else
		return (QIconSet) NULL;
}


/// adds the text in search field to the wishlist
void YammiGui::addToWishList()
{
	QString toAdd = m_searchField->text();
	MyDateTime wishDate=wishDate.currentDateTime();
	Song* newSong=new Song("{wish}", toAdd, "", "", "", 0, 0, wishDate, 0, "", 0, 0);
	folderAll->addSong(newSong);
  // FIXME: selectionMode for custom list?
  //forSong(newSong, Song::SongInfo, NULL);
	model->allSongsChanged(true);
	m_searchField->setText("{wish}");
  folderContentChanged(folderAll);
  m_searchField->setText("");
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
		if( ((Folder*)f)->folderName()==chosen ) {
			categoryFolder=(FolderSorted*)f;
    }
	}

	if(categoryFolder==0) {
		cout << "folder not found!\n";
		return;
	}
	// go through list of songs
	for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
		if(!remove) {
      if(!categoryFolder->songlist().containsSong(s)) {
        categoryFolder->addSong(s);
      }
    }
		else {
			categoryFolder->removeSong(s);
    }
	}

	model->categoriesChanged(true);
  folderContentChanged(categoryFolder);
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
		
	QString msg = i18n("Two identical songs: \ns1: %1\ns2: %2\nDo you want to delete one of them?");
	int what = KMessageBox::warningYesNoCancel( this, msg.arg(str1).arg(str2),QString::null,i18n("Delete s1"), i18n("Delete s2"));
	if(what == KMessageBox::Yes) 
	{	
		kdDebug()<< "deleting s1\n";
		s1->deleteFile(m_config.trashDir);
		folderAll->removeSong(s1);
	}
	if(what == KMessageBox::No)
	{
		kdDebug()<< "deleting s2\n";
		s2->deleteFile(m_config.trashDir);
		folderAll->removeSong(s2);
	}
	model->allSongsChanged(true);
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
	searchSong(searchFor);
// 	m_searchField->setText(searchFor);
}

/**
 * Go to a specific folder (album/artist) of selected song.
 */
void YammiGui::goToFolder(int what)
{
	what-=2000;
	Song* s=selectedSongs.firstSong();
  QString folderName;

  switch(what)
	{
		case 1:
      folderName=s->artist;
      break;

    case 2:
      folderName=s->artist+" - "+s->album;
      break;

    case 3:
      folderName=CMP3Info::getGenre(s->genreNr);
      break;

    default:
      folderName="";
  }

  Folder* folder=this->getFolderByName(folderName);
  if(folder==0) {
    cout << "folder " << folderName << " not existing\n";
  }
  else {
    changeToFolder(folder);
  }
}

/**
 * search field changed => update search results
 *
 */
void YammiGui::searchSong( const QString &fuzzy )
{
	if( fuzzy.length() < 3 )
		return;
	
	fuzzyFolderName="";
	QString searchStr=" " + fuzzy +" ";
	
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
	for(; noResults<m_config.searchMaximumNoResults && bme[noResults] && bme[noResults]->sim>(m_config.searchThreshold*10); noResults++) {
		searchResults.append( new SongEntryInt2 ((Song*)bme[noResults]->objPtr, bme[noResults]->sim) );
	}
  folderSearchResults->updateTitle();
  folderContentChanged(folderSearchResults);
  if(chosenFolder!=folderSearchResults) {
    // we better reset these settings to not confuse the user with his own changes
    // (sort order and scroll position)
    folderSearchResults->saveScrollPos(0, 0);
    folderSearchResults->saveSorting(0);
    changeToFolder(folderSearchResults);
  }
	songListView->setContentsPos( 0, 0);			// set scroll position to top
	QListViewItem* item=songListView->firstChild();
	if(item) {
		item->setSelected(true);											// select first anyway
  }
	int threshold=700;
	for(int j=0; j<noResults && bme[j] && bme[j]->sim>threshold; j++, item=item->nextSibling()) {
    if(item==0) {
      cout << "item==0, this should not happen!\n";
      break;
    }
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
  if(chosenFolder)
  {
	if(newFolder == chosenFolder && (!changeAnyway)) 
	{
    // don't do anything if current folder is already the specified one
		return;
	}

  KApplication::setOverrideCursor( Qt::waitCursor );
  // TODO: history of visited folders, something like:
  //visitedFoldersHistory->add(chosenFolder);

  chosenFolder->saveSorting(songListView->sortedBy);
  chosenFolder->saveScrollPos(songListView->contentsX(), songListView->contentsY());
 }

  // now we really change to new folder
  chosenFolder = newFolder;

  if(chosenFolder==folderActual) {
		songListView->dontTouchFirst=true;				// don't allow moving the first
  }
	else {
		songListView->dontTouchFirst=false;
  }

  updateListViewColumns();

  folderListView->setCurrentItem( (QListViewItem*)chosenFolder );
  folderListView->setSelected( (QListViewItem*)chosenFolder , TRUE );
  folderListView->ensureItemVisible((QListViewItem*)chosenFolder);
  folderContentChanged();
}


void YammiGui::folderContentChanged()
{
	songListView->clear();
	if(chosenFolder)
	{
		addFolderContent(chosenFolder);
		if(chosenFolder == folderActual)
		{
			updateCurrentSongStatus();
		}
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


/**
 * recursively add the content of folder and all subfolders
 * for now: folder contains songs OR subfolders, but not both!
 */
void YammiGui::addFolderContent(Folder* folder)
{
  folderToAdd=folder;
	alreadyAdded=0;

	if(folder->songlist().count()!=0) {
		songListView->setSorting(-1);
		songListView->setUpdatesEnabled(false);
		addFolderContentSnappy();
	}
	else {		// no songList in that folder
		QApplication::restoreOverrideCursor();
  }
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
  // any songs left to add?
  if(entry) {
    // yes, add them after processing events
		QTimer* timer=new QTimer(this);
		connect(timer, SIGNAL(timeout()), this, SLOT(addFolderContentSnappy()) );
		timer->start(0, TRUE);
	}
	else {		// no, we are finished
		QApplication::restoreOverrideCursor();
		songListView->setUpdatesEnabled(true);

    int saved=chosenFolder->getSavedSorting();
    if(saved!=0) {
      bool asc=true;
      int column;
      if(saved<0) {
        column=-saved-1;
        asc=false;
      }
      else {
        column=saved-1;
      }
      songListView->sortedBy=saved;
      songListView->setSorting(column, asc);
    }
    else {
      // special default sorting for certain folders
      if(((QListViewItem*)chosenFolder)->parent()==folderAlbums) {
        songListView->sortedBy=COLUMN_TRACKNR+1;
        songListView->setSorting(COLUMN_TRACKNR, true);
      }
      else if(chosenFolder==folderRecentAdditions) {
        songListView->sortedBy=COLUMN_ADDED_TO+1;
        songListView->setSorting(COLUMN_ADDED_TO, true);
      }
      else {
        // default sort order: first column
        songListView->sortedBy=1;
        songListView->setSorting(0);
      }
    }
    int x=chosenFolder->getScrollPosX();
    int y=chosenFolder->getScrollPosY();
    songListView->setContentsPos(x, y);

	}
}


/// user clicked on a song
void YammiGui::slotSongChanged()
{
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
	if( selected<=0 ) {
		return;										// only if at least one song selected
  }
	adjustSongPopup();
 	songPopup->popup( point );
}


/// adjust SongPopup corresponding to <selectedSongs>
void YammiGui::adjustSongPopup()
{
	int selected=selectedSongs.count();
  QString label;
 	Song* first=selectedSongs.firstSong();
	if (selected>1) { 							// more than one song selected
		label=QString(i18n("%1 songs selected")).arg(selected);
  }
	else {
		label=first->displayName();
  }
  songPopup->changeItem ( 113, label);

/*
  songGoToPopup->changeItem( 2001, first->artist);
  songGoToPopup->setItemEnabled(2001, getFolderByName(first->artist)!=0);
  songGoToPopup->changeItem( 2002, first->artist+" - "+first->album);
  songGoToPopup->setItemEnabled(2002, getFolderByName(first->artist+" - "+first->album)!=0);
  songGoToPopup->changeItem( 2003, CMP3Info::getGenre(first->genreNr));
  songGoToPopup->setItemEnabled(2003, getFolderByName(CMP3Info::getGenre(first->genreNr))!=0);
*/

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

 	songPopup->setItemEnabled(Song::PlayNow, enable);
 	songPopup->setItemEnabled(Song::Dequeue, enable);
 	songPopup->setItemEnabled(Song::PrelistenStart, enable);
 	songPopup->setItemEnabled(Song::PrelistenMiddle, enable);
 	songPopup->setItemEnabled(Song::PrelistenEnd, enable);
 	songPopup->setItemEnabled(Song::CheckConsistency, enable);
 	songPopup->setItemEnabled(Song::MoveTo, enable);
 	songPopup->setItemEnabled(Song::BurnToMedia, enable);
}


/**
 * Popup menu on a folder
 */
void YammiGui::slotFolderPopup( QListViewItem* Item, const QPoint & point, int )
{
	QListViewItem *i = folderListView->currentItem();
	Folder* chosenFolder = ( Folder* )i;
  setSelectionMode(SELECTION_MODE_FOLDER);
  getSelectedSongs();
	if(selectedSongs.count()==0) {
		// no songs in this folder
		chosenFolder->popup( point, 0);
		return;
	}
	adjustSongPopup();
	chosenFolder->popup( point, songPopup);
  setSelectionMode(SELECTION_MODE_USER_SELECTED);
}



// executes a plugin on a selection of songs
void YammiGui::forSelectionPlugin(int pluginIndex)
{
	pluginIndex-=2000;

  bool confirm=m_config.pluginConfirm[pluginIndex]=="true";
  QString mode=m_config.pluginMode[pluginIndex];
  QString cmd=m_config.pluginCommand[pluginIndex];

  if(cmd.contains("{directoryDialog}")>0) {
    QString dir = KFileDialog::getExistingDirectory("", this, i18n("choose directory for plugin"));
    if(dir.isNull())
      return;
    cmd.replace(QRegExp("{directoryDialog}"), dir);
  }
  if(cmd.contains("{fileDialog}")>0) {
    QString file = KFileDialog::getSaveFileName("",QString::null, this, i18n("choose file for plugin"));
    if(file.isNull())
      return;
    cmd.replace(QRegExp("{fileDialog}"), file);
  }
  if(cmd.contains("{inputString}")>0) {
    bool ok;
    QString inputString=QString(QInputDialog::getText( i18n("Get input parameter"), i18n("Type in argument for plugin:"), QLineEdit::Normal, QString(""), &ok, this ));
    if(!ok) {
      return;
    }
    cmd.replace(QRegExp("{inputString}"), inputString);
  }


  if(mode=="single") {
    KProgressDialog progress( this,0, i18n("Yammi"),i18n("Executing song plugin cmd..."),true);
    progress.progressBar()->setTotalSteps(selectedSongs.count());
    kapp->processEvents();

    int index=1;
    for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong(), index++) {
      QString cmd2=s->replacePlaceholders(cmd, index);

      if(index==1 && confirm) {
        // before  executing cmd on first song, ask user
        QString msg=i18n("Execute the following command on each selected song?\n");
        msg+=i18n("(here shown: values for first song)\n\n");
        for(unsigned int i=0; i<cmd2.length(); i+=80) {
          msg+=cmd2.mid(i, 80)+"\n";
        }
        if( KMessageBox::warningYesNo( this,msg ) != KMessageBox::Yes)
	{
          return;
        }
      }
      progress.progressBar()->setProgress(index);
      kapp->processEvents();
      if(progress.wasCancelled())
        return;
      system(cmd2);
    }
  }

  if(mode=="group") {
    int index=1;
    QString customList="";
    for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong(), index++) {
      QString entry = m_config.pluginCustomList[pluginIndex];
      customList+=s->replacePlaceholders(entry, index);
    }

//     // custom list can be long => we put it into a file...
//     QFile customListFile(m_config.yammiBaseDir+"/customlist.txt");
//     customListFile.open(IO_WriteOnly);
//     customListFile.writeBlock( customList, qstrlen(customList) );
//     customListFile.close();
//     cmd.replace(QRegExp("{customListViaFile}"), "`cat "+m_config.yammiBaseDir+"/customlist.txt`");
//     cmd.replace(QRegExp("{customList}"), customList);
#warning fixme

    if(confirm) {
      cout << "plugin command: " << cmd << "\n";
      cout << "custom list: " << customList << "\n";
      QString msg=i18n("Execute the following command:\n");
      for(unsigned int i=0; i<cmd.length(); i+=80) {
        msg+=cmd.mid(i, 80)+"\n";
        if(i>1200) {
          msg+=i18n("\n...\n(command truncated)");
          break;
        }
      }
      if( KMessageBox::warningYesNo( this, msg) != KMessageBox::Yes)
      {
        return;
      }
    }
    system(cmd);
  }
}


/**
 * move the files of the selected songs to a different location
 */
void YammiGui::forSelectionMove()
{
	getSelectedSongs();
	int count = selectedSongs.count();
	if(count < 1) {
		return;
  }
	// let user choose destination directory
  QString startPath=selectedSongs.firstSong()->path;
  QString dir=KFileDialog::getOpenFileName(QString("/mm"), QString("*.mp3"), this, QString("yammi"));
	if(dir.isNull()) {
		return;
  }
	if(dir.right(1)=="/") { 						// strip trailing slash
		dir=dir.left(dir.length()-1);
  }
	for(Song* s = selectedSongs.firstSong(); s; s=selectedSongs.nextSong())
	{
		s->moveTo(dir);
	}
}



/**
 * prepare burning selection to media
 * (burning order will be the order of the selected songs)
 */
void YammiGui::forSelectionBurnToMedia()
{
	long double totalSize=0;
	long double size=-1;

	bool ok;
	QString collName=QString(QInputDialog::getText( i18n("collection name"), i18n("Please enter collection name:"), QLineEdit::Normal, QString(i18n("my mp3 collection")), &ok, this ));
	if(!ok)
		return;

	QString startIndexStr=QString(QInputDialog::getText( i18n("collection start number"), i18n("Please enter start index:"), QLineEdit::Normal, QString("1"), &ok, this ));
	if(!ok)
		return;

	KProgressDialog progress(this, i18n("Preparing media...") );
	progress.setMinimumDuration(0);
	progress.setAutoReset(false);
        progress.progressBar()->setProgress(0);
	progress.progressBar()->setTotalSteps(selectedSongs.count());
	kapp->processEvents();

	int startIndex=atoi(startIndexStr);
  int mediaNo=startIndex-1;
	QString mediaName=QString("%1_%2").arg(collName).arg(mediaNo);
	QString mediaDir = KGlobal::dirs()->saveLocation("appdata","media/" + mediaName + "/",true);
	long double sizeLimit=(long double)m_config.criticalSize*1024.0*1024.0;
	int count=0;
	for(Song* s=selectedSongs.firstSong(); s; ) {
	  progress.progressBar()->setProgress(count);
		if(progress.wasCancelled())
			break;

		QFileInfo fi(s->location());
		if(size==-1 || size+fi.size()>sizeLimit) {
      // medium is full, prepare new one
			mediaNo++;
			mediaName=QString("%1_%2").arg(collName).arg(mediaNo);
			mediaDir= KGlobal::dirs()->saveLocation("appdata","media/"+ mediaName + "/",true);
			progress.setLabel(i18n("Preparing media ")+mediaName);
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

    count++;
    // check, whether song already contained on media
    if(s->mediaName.contains(mediaName)) {
      cout << "song already existing on this media, skipping...\n";
      s=selectedSongs.nextSong();
      continue;
    }

    // okay, we really add the song to the current media
		size+=fi.size();
    totalSize+=fi.size();
		// linux specific
		QString cmd=QString("ln -s \"%1\" \"%2/%3\"").arg(s->location()).arg(mediaDir).arg(s->filename);
		system(cmd);
		s->addMediaLocation(mediaName, s->filename);
    s=selectedSongs.nextSong();
	}

	cout << "no of media: " << mediaNo+1-startIndex << " (size limit: " << m_config.criticalSize << " MB, ";
  cout << "index " << startIndex << " to " << mediaNo << ")\n";
	cout << "no of files: " << count << "\n";
	cout << "size of last media: " << (int)(size/1024.0/1024.0) << " MB\n";
	cout << "size in total: " << (int)(totalSize/1024.0/1024.0) << " MB\n";
	folderMedia->update(&(model->allSongs));
	model->allSongsChanged(true);
	QString msg=QString(i18n("Result of \"Burn to media\" process:\n\n\
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
  to check an option \"follow symlinks\" or similar)."))
  .arg(mediaNo+1-startIndex).arg(m_config.criticalSize).arg(startIndex).arg(mediaNo)
  .arg(count).arg((int)(size/1024.0/1024.0)).arg((int)(totalSize/1024.0/1024.0))
  .arg(KGlobal::dirs()->saveLocation("appdata","media/",false));
	KMessageBox::information( this, msg );
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



/**
 * Fills the selection list with the songs selected in listview
 */
void YammiGui::getSelectedSongs()
{
	selectedSongs.clear();

  if(selectionMode == SELECTION_MODE_USER_SELECTED) {
    // get songs selected in listview
  	QListViewItem* i=songListView->firstChild();
	  for(; i; i=i->itemBelow()) {						// go through list of songs
		  if(i->isSelected()) {
			  Song* s=((SongListItem*) i)->song();
  			selectedSongs.appendSong(s);
	  	}
  	}
  }
  if(selectionMode == SELECTION_MODE_FOLDER) {
    // get songs from currently selected folder: complete folder content
  	// take the order as shown in the songlist
	  for(QListViewItem* i=songListView->firstChild(); i; i=i->itemBelow()) {
		  selectedSongs.appendSong(((SongListItem*) i)->song());
    }
  }

  if(selectionMode == SELECTION_MODE_ALL) {
    // select all songs in database
	  for(Song* s=model->allSongs.firstSong(); s; s=model->allSongs.nextSong()) {
		  selectedSongs.appendSong(s);
  	}
  }

  if(selectionMode == SELECTION_MODE_CURRENT) {
    // select only the currently selected item in the listview
  	QListViewItem* i=songListView->currentItem();
	  selectedSongs.appendSong(((SongListItem*) i)->song());
  }

}


/// check consistency, fill up list of problematic songs
void YammiGui::forAllCheckConsistency()
{
  setSelectionMode(SELECTION_MODE_ALL);
  forSelectionCheckConsistency();
  setSelectionMode(SELECTION_MODE_USER_SELECTED);
}

void YammiGui::forSelectionCheckConsistency()
{
	CheckConsistencyDialog d(this, i18n("Check consistency - settings"));
  d.TextLabel1->setText(QString(i18n("checking %1 songs")).arg(selectedSongs.count()));

	// show dialog
	int result=d.exec();
	if(result!=QDialog::Accepted)
    return;

  ConsistencyCheckParameter p;
  p.checkForExistence=d.CheckBoxCheckForExistence->isChecked();
  p.updateNonExisting=d.CheckBoxUpdateNonExisting->isChecked();
  p.checkTags=d.CheckBoxCheckTags->isChecked();
  p.correctTags=d.CheckBoxCorrectTags->isChecked();
  p.correctTagsDirection=d.ComboBoxCorrectTagsDirection->currentItem();
  p.checkFilenames=d.CheckBoxCheckFilenames->isChecked();
  p.ignoreCaseInFilenames=m_config.ignoreCaseInFilenames;
  p.correctFilenames=d.CheckBoxCorrectFilenames->isChecked();
  p.checkDirectories=d.CheckBoxCheckDirectories->isChecked();
  p.correctDirectories=d.CheckBoxCorrectDirectories->isChecked();
  p.checkDoubles=d.CheckBoxCheckDoubles->isChecked();

  KProgressDialog progress( this, 0,i18n("Yammi"), i18n("Checking consistency..."),true);
  progress.setMinimumDuration(0);
	progress.setAutoReset(false);
  progress.progressBar()->setProgress(0);
	kapp->processEvents();
	model->checkConsistency(&progress, &selectedSongs, &p);

  folderProblematic->update(model->problematicSongs);
	folderContentChanged(folderProblematic);
  folderContentChanged(chosenFolder);

	QString msg=QString(
  i18n("Result of consistency check:\n\n\
  %1 problematic issues identified (check folder \"Problematic Songs\")\n\
      (that folder won't be saved)\n\
  %2 songs not existing, of these:\n\
      %3 entries updated (filename cleared)\n\
      %4 entries deleted (because not existing on any media)\n\
  %5 songs with inconsistent tags, of these:\n\
      %6 tags corrected\n\
  %7 songs with inconsistent filename, of these:\n\
      %8 filenames corrected\n")).arg(model->problematicSongs.count()).arg(p.nonExisting).arg(p.nonExistingUpdated)\
  .arg(p.nonExistingDeleted).arg(p.dirtyTags).arg(p.tagsCorrected).arg(p.dirtyFilenames).arg(p.filenamesCorrected);
  msg+=QString(i18n("%1 songs with inconsistent path, of these:\n\
      %2 paths corrected\n\
  %3 double entries found\n")).arg(p.dirtyDirectories).arg(p.directoriesCorrected).arg(p.doublesFound);
	KMessageBox::information( this, msg );
}

void YammiGui::forSelectionAppend( )
{
	kdDebug()<<"appendSelected( )"<<endl;
	getSelectedSongs();
	int count = selectedSongs.count();
	if(count < 1) {
		return;
  }
	if(shiftPressed) {
		selectedSongs.shuffle();
  }
	for(Song* s = selectedSongs.firstSong(); s; s=selectedSongs.nextSong())
	{
		model->songsToPlay.append(new SongEntryInt(s, 13));
	}
	folderActual->correctOrder();
	player->syncYammi2Player(false);
	folderContentChanged(folderActual);
	statusBar( )->message(QString(i18n("%1 Songs equeued at end of playlist")).arg(count), 2000);
}

void YammiGui::forSelectionPrepend( )
{
	kdDebug()<<"prependSelected( )"<<endl;
	// reverse the order, to get intended play order
	getSelectedSongs();
	int count = selectedSongs.count();
	if(count < 1) {
		return;
  }
	if(shiftPressed) {
		selectedSongs.shuffle();
  }
	selectedSongs.reverse();
	//FIXME - If the player is not playing, we can insert the itmes at pos 0
	for(Song* s = selectedSongs.firstSong(); s; s=selectedSongs.nextSong())
	{
		if(model->songsToPlay.count()==0 || currentSong!=model->songsToPlay.at(0)->song())
			model->songsToPlay.insert(0, new SongEntryInt(s, 13));
		else
			model->songsToPlay.insert(1, new SongEntryInt(s, 13));
	}
	folderActual->correctOrder();
	player->syncYammi2Player(false);
	folderContentChanged(folderActual);
	statusBar( )->message(QString(i18n("%1 Songs equeued as next")).arg(count), 2000);
}


void YammiGui::forSelectionPrelisten(int where )
{
	getSelectedSongs();
	int count = selectedSongs.count();
	if(count < 1) {
		return;
  }
	for(Song* s = selectedSongs.firstSong(); s; s=selectedSongs.nextSong())
	{
		preListen(s, where);
    break;
	}
}


void YammiGui::forSelectionPlay( )
{//FIXME - this does not work too well....
	kdDebug()<<"playSelected( )"<<endl;
	getSelectedSongs();
	int count = selectedSongs.count();
	if(count < 1) {
		return;
  }
	player->stop( );
	forSelectionPrepend( );
	//this is not really clean, but...
	player->skipForward(shiftPressed);
	player->play( );
}

void YammiGui::forSelectionDequeue( )
{
	getSelectedSongs();
  int sortedByBefore=songListView->sortedBy;
	for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
    if(chosenFolder==folderActual) {
      // song chosen from playlist => dequeue only the selected song entry (not all songs with that id)
      QListViewItem* i=songListView->firstChild();
      for(; i; i=i->itemBelow()) {						// go through list of songs
        if(i->isSelected() && ((SongListItem*) i)->song()==s) {
          SongEntry* entry=((SongListItem*) i)->songEntry;
          int pos=((SongEntryInt*)entry)->intInfo-1;
          if(pos!=0 || player->getStatus()==STOPPED) {
            // only dequeue if not currently played song (or player stopped)
            model->songsToPlay.remove(pos);
            statusBar( )->message(QString(i18n("song %1 dequeued")).arg(s->displayName()), 3000);
          }
          break;
        }
      }
    }
    else {
      // song chosen from other folder => dequeue ALL occurrences with that id
      int i=1;
      if(player->getStatus()==STOPPED) {
        i=0;
      }
      for(; i<(int)model->songsToPlay.count(); i++) {
        Song* check=model->songsToPlay.at(i)->song();
        if(check==s) {
          model->songsToPlay.remove(i);
          statusBar( )->message(QString(i18n("song %1 dequeued")).arg(s->displayName()), 3000);
          i--;
        }
      }
    }
	}
	folderActual->correctOrder();
  player->syncYammi2Player(false);
  folderContentChanged(folderActual);
  bool ascending = (sortedByBefore>0);
  if(!ascending) {
    sortedByBefore = -sortedByBefore;
  }
  int column=sortedByBefore-1;
  songListView->setSorting(column, ascending);
  checkPlaylistAvailability();
}


void YammiGui::forSelectionSongInfo( )
{
	getSelectedSongs();
	int count = selectedSongs.count();
	if(count < 1) {
		return;
  }
	if(count == 1 ) {
		Song *s = selectedSongs.first( )->song();
		songInfo(s);
		return;
	}
	else //FIXME -
	{
		KMessageBox::information(this,QString(i18n("%1 Songs selected").arg(count)));
	}
}


void YammiGui::forSelectionDelete( )
{
	getSelectedSongs();
	int count = selectedSongs.count();
	if(count < 1) {
		return;
  }

  // determine delete mode
  DeleteDialog dd( this,  "deleteDialog", true);
	if(selectedSongs.count()==1)
		dd.LabelSongname->setText(selectedSongs.firstSong()->displayName());
	else
		dd.LabelSongname->setText(QString(i18n("Delete %1 songs")).arg(selectedSongs.count()));
	// fill dialog with onMedia info...(for all toDelete songs)
	for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
		for(unsigned int i=0; i<s->mediaName.count(); i++) {
			QString toInsert(s->mediaName[i]);
			bool exists=false;
			for(int j=0; j<dd.ComboBoxOnMedia->count(); j++) {
				if(dd.ComboBoxOnMedia->text(j)==toInsert)
					exists=true;
			}
			if(!exists) {
				dd.ComboBoxOnMedia->insertItem(toInsert);
      }
		}
	}
	int result=dd.exec();
	if(result!=QDialog::Accepted) {
    return;
  }
	bool deleteFileFlag=dd.CheckBoxDeleteFile->isChecked();
	bool deleteEntryFlag=dd.CheckBoxDeleteDbEntry->isChecked();

	// go through list of songs
	for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
    if(deleteFileFlag) {
     	s->deleteFile(m_config.trashDir);
    }
		if(deleteEntryFlag)	{
      deleteEntry(s);
    }
	}

	// update view
	model->allSongsChanged(true);
	folderContentChanged(folderAll);
	if(deleteEntryFlag) {
		updateView();
    model->categoriesChanged(true);
	}
}

void YammiGui::songInfo( Song *s )
{
//FIXME move all this stuff to the dialog..
//ie, SongInfoDialog d(s);
//    if(d.exec( ))
//		updateSongInfo(s);

	QString _artist, _title, _album, _comment, _path, _filename, _year, _trackNr, _bitrate, _proposedFilename;
	MyDateTime _addedTo, _lastTimePlayed;
	int _length=0;
	long double _size=0;
	int _genreNr=0;

	SongInfoDialog si(this, i18n("test"), true);

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
	// get filesize
	QFile file(s->location());
	if(file.exists())
		_size+=file.size();
	_length+=s->length;

	// insert all media, over that songs are distributed
	for(unsigned int m=0; m<s->mediaName.count(); m++)
	{
		bool found=false;
		for(int n=0; n<si.ComboBoxMedia->count(); n++)
		{
			if(si.ComboBoxMedia->text(n)==s->mediaName[m])
					found=true;
		}
			if(!found)
				si.ComboBoxMedia->insertItem(s->mediaName[m]);
	}


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

	// now edit the (common) info
	si.LineEditArtist->setText(_artist);
	si.LineEditTitle->setText(_title);
	si.LineEditAlbum->setText(_album);
	si.LineEditComment->setText(_comment);
	if(_year!="0")
		si.LineEditYear->setText(_year);
	if(_trackNr!="0")
		si.LineEditTrack->setText(_trackNr);
//	MyDateTime d=_addedTo;
//	if(_addedTo.isValid())
	if(_addedTo.isValid())
		si.LineEditAddedTo->setText(_addedTo.writeToString());
	else
		si.LineEditAddedTo->setText("!");
	if(_lastTimePlayed.isValid())
	{
		MyDateTime never;
		never.setDate(QDate(1900,1,1));
		never.setTime(QTime(0,0,0));
		if(_lastTimePlayed!=never)
			si.LineEditLastPlayed->setText(_lastTimePlayed.writeToString());
		else
		si.LineEditLastPlayed->setText("never");
	}
	else
	{
		si.LineEditLastPlayed->setText("!");
	}

	si.ReadOnlyPath->setText(_path);
	si.ReadOnlyFilename->setText(_filename);
	si.ReadOnlyProposedFilename->setText(_proposedFilename);


	si.LabelHeading->setText(_artist+" - "+_title);
	QString x;
	si.ReadOnlyLength->setText(x.sprintf(i18n("%2d:%02d (mm:ss)"), _length/60, _length % 60));

	si.ReadOnlySize->setText( QString(i18n("%1 MB (%2 Bytes)"))
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
	if(result!=QDialog::Accepted)
		return;

	// get genreNr
	int sortedGenreNr=si.ComboBoxGenre->currentItem();
	int tryGenreNr=-1;
	if(sortedGenreNr!=0) {
    tryGenreNr=CMP3Info::getGenreIndex(genreList[sortedGenreNr]);
  }


	bool change=false;
	if(si.LineEditArtist->text()!="!" && si.LineEditArtist->text()!=s->artist)
	{
		s->artist=si.LineEditArtist->text();
		change=true;
	}
	if(si.LineEditTitle->text()!="!" && si.LineEditTitle->text()!=s->title)
	{
		s->title=si.LineEditTitle->text();
		change=true;
	}
	if(change)
	{ // for artist and title: mark categories as dirty on change!
		model->markPlaylists(s);
	}
	if(si.LineEditAlbum->text()!="!" && si.LineEditAlbum->text()!=s->album) 		{ s->album=si.LineEditAlbum->text(); change=true; }
	if(si.LineEditComment->text()!="!" && si.LineEditComment->text()!=s->comment)	{s->comment=si.LineEditComment->text(); change=true; }
	if(si.LineEditYear->text()!="!")
	{
		int tryYear=atoi(si.LineEditYear->text());
		if(tryYear!=s->year) {s->year=tryYear; change=true; }
	}
	if(si.LineEditTrack->text()!="!")
	{
		int tryTrackNr=atoi(si.LineEditTrack->text());
		if(tryTrackNr!=s->trackNr) {s->trackNr=tryTrackNr; change=true; }
	}
	MyDateTime newAddedTo;
	newAddedTo.readFromString(si.LineEditAddedTo->text());
	if(newAddedTo.isValid())
	{
		if(newAddedTo!=s->addedTo) { s->addedTo=newAddedTo; change=true; }
	}

	if(tryGenreNr!=-1)
	{
		if(tryGenreNr!=s->genreNr) { s->genreNr=tryGenreNr; change=true; }
	}

	if(change)
	{
		model->allSongsChanged(true);
		s->tagsDirty=true; // mark song as dirty(tags)
		s->filenameDirty=(s->checkFilename(m_config.ignoreCaseInFilenames)==false);
			// manual update: go through list of songs and correct, if necessary
		for(SongListItem* i=(SongListItem*)songListView->firstChild(); i; i=(SongListItem*)i->itemBelow())
		{
			if(i->song()!=s)
				continue;
			i->setColumns(i->songEntry);
		}
	}
}


/**
 * Delete a song entry.
 * TODO: move this (partly) to yammimodel?
 */
void YammiGui::deleteEntry(Song* s)
{
  // remove from database
  folderAll->removeSong(s);

  // ...and from categories
  for( QListViewItem* f=folderCategories->firstChild(); f; f=f->nextSibling() ) {
    FolderSorted* category=(FolderSorted*)f;
    category->removeSong(s);
  }
  // ...and from playlist
  folderActual->removeSong(s);
}


/// doubleClick on song
void YammiGui::doubleClick()
{
  // TODO: fix
  //forCurrent(m_config.doubleClickAction);
}

/// middleClick on song
void YammiGui::middleClick(int button)
{
  // TODO: fix
//	cout << "button: " << button << "\n";
//	if(button==1) {			// left button
//	}
/*
	if(button==4) {				// middle button
		if(!controlPressed && !shiftPressed)
			forCurrent(m_config.middleClickAction);
		if(controlPressed && !shiftPressed)
			forCurrent(m_config.controlClickAction);
		if(shiftPressed && !controlPressed)
			forCurrent(m_config.shiftClickAction);
//		if(shiftPressed && controlPressed)
//			cout << "both\n";
	}
*/
}



/**
 * Creates a new category, querying the user for the name.
 */
bool YammiGui::newCategory(){
	bool ok = false;
	QString caption(i18n("Enter name for category"));
	QString message(i18n("Please enter name of category"));
	QString newName=QString(QInputDialog::getText( caption, message, QLineEdit::Normal, QString(i18n("new category")), &ok, this ));
	if(!ok) {
    return false;
  }
	model->newCategory(newName);
	folderCategories->update(model->allCategories, model->categoryNames);
	updateSongPopup();
  return true;
}


/**
 * Removes the selected category.
 */
void YammiGui::removeCategory()
{
	QListViewItem* i = folderListView->currentItem();
	QString name=((Folder*)i)->folderName();
	QString msg(i18n("Delete category %1 ?\n (will be deleted immediately!)"));
	if( KMessageBox::warningYesNo( this, msg) == KMessageBox::Yes )
	{
		model->removeCategory(name);
		folderCategories->update(model->allCategories, model->categoryNames);
		updateSongPopup();
	}
}

/**
 * Renames the selected category, querying the user for the new name.
 */
void YammiGui::renameCategory()
{
	QListViewItem* i = folderListView->currentItem();
	QString oldName=((Folder*)i)->folderName();
	bool ok;
	QString newName=QString(QInputDialog::getText( i18n("collection name"), i18n("Please enter new name:"), QLineEdit::Normal, oldName, &ok, this ));
	if(!ok) {
		return;
  }

	model->renameCategory(oldName, newName);
	folderCategories->update(model->allCategories, model->categoryNames);
	updateSongPopup();
}

/**
 * Inserts all songs from a .m3u playlist into a category.
 * Only inserts those songs already existing in yammi's database.
 * Inserts in the order of the playlist.
 */
void YammiGui::loadM3uIntoCategory()
{
	QListViewItem* i = folderListView->currentItem();
  FolderSorted* categoryFolder=(FolderSorted*)i;
  QString filename = KFileDialog::getOpenFileName("/", "Playlists (*.m3u)", this,i18n("Choose a Playlist to insert" ));
	if(filename.isNull()) {
    return;
  }
  QStringList* list=model->readM3uFile(filename);

  QStringList::Iterator it = list->begin();
  while( it != list->end() ) {
    QString filename(*it);
    Song* toAdd=model->getSongFromFilename(filename);
    if(toAdd==0) {
      // no song found with that filename
      cout << "no song in database found with filename \"" << filename << "\" (not in Yammi database yet?), skipping\n";
    }
    else {
      categoryFolder->addSong(toAdd);
    }
    ++it;
  }
  delete(list);
  // update category content
	model->categoriesChanged(true);
  folderContentChanged(categoryFolder);
}




/**
 * Sets the selected autoplay folder to the currently selected folder.
 * Updates the autoplayMenu.
 */
void YammiGui::autoplayFolder()
{
	Folder* f = (FolderSorted*)folderListView->currentItem();
  autoplayFoldername=f->folderName();
  m_currentAutoPlay->setText(i18n("Folder: ")+autoplayFoldername);
}


void YammiGui::autoplayOff()
{
	autoplayMode = AUTOPLAY_OFF;
}
void YammiGui::autoplayLNP()
{
	autoplayMode = AUTOPLAY_LNP;
}
void YammiGui::autoplayRandom()
{
	autoplayMode = AUTOPLAY_RANDOM;
}

/// remove media
// uahhaa... ugly! make mediaName + mediaLocation a struct/class, oli!
void YammiGui::removeMedia()
{
	QListViewItem *i = folderListView->currentItem();
	Folder* chosenFolder = ( Folder* )i;
	QString mediaName=chosenFolder->folderName();
	QString msg = QString(i18n("Remove media %1 and the corresponding directory?\n\
(which contains the symbolic links to the songs)")).arg(mediaName);
	if( KMessageBox::warningYesNo( this, msg )!=KMessageBox::Yes)
		return;
	model->removeMedia(mediaName);
	folderMedia->update(&(model->allSongs));
}


void YammiGui::renameMedia()
{
	QListViewItem* i = folderListView->currentItem();
	QString oldName=((Folder*)i)->folderName();
	bool ok;
	QString newName=QString(QInputDialog::getText( i18n("Rename Media"), i18n("Please enter new name:"), QLineEdit::Normal, oldName, &ok, this ));
	if(!ok)
		return;
	model->renameMedia(oldName, newName);
	folderMedia->update(&(model->allSongs));
}


/// invoke an externally configured program/script on the content of a folder
void YammiGui::pluginOnFolder()
{
	QFile f(KGlobal::dirs()->saveLocation("appdata")+"plugin.temp" );
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


/**
 * clear all playlist items (except currently played song
 */
void YammiGui::clearPlaylist()
{
	if(m_config.childSafe)
		return;
	Song* save=0;

  bool saveCurrentSong=(currentSong!=0 && player->getStatus()!=STOPPED && model->songsToPlay.count()>1);
  ApplyToAllBase confirm(this, "confirmDialog", true);
  QString msg=QString(i18n("Clear complete playlist?\n(%1 entries)")).arg(model->songsToPlay.count());
  confirm.TextLabel->setText(msg);
  confirm.CheckBoxApply->setText(i18n("including current song"));
  confirm.CheckBoxApply->setChecked(!saveCurrentSong);
  // show dialog
  int result=confirm.exec();
  if(result!=QDialog::Accepted) {
    return;
  }
  if(!confirm.CheckBoxApply->isChecked() && model->songsToPlay.count()>0) {
    save=model->songsToPlay.firstSong();
  }
  model->songsToPlay.clear();
  if(save != 0) {
    folderActual->addSong(save);
  }
  folderActual->updateTitle();
  player->syncYammi2Player(false);
  folderContentChanged(folderActual);
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
  if(folderActual->songlist().count()<5 && autoplayMode!=AUTOPLAY_OFF && this->autoplayFoldername!="") {
    autoFillPlaylist();
  }

	// perform these actions only if player is playing or paused
	if(player->getStatus()!=STOPPED)
	{
		// adjust songSlider (if user is not currently dragging it around)
		int current = player->getCurrentTime();
// 		if(!isSongSliderGrabbed && player->getStatus() != PAUSED) {
			m_seekSlider->setValue(current);
//     }
	}
}


void YammiGui::autoFillPlaylist()
{
  Folder* toAddFrom=getFolderByName(autoplayFoldername);
  if(toAddFrom!=0 && toAddFrom->songlist().count()>0) {
    // fill up from chosen autoplay folder
    int total=toAddFrom->songlist().count();
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
      songToAdd=toAddFrom->songlist().at(chosen)->song();
      if(folderActual->songlist().containsSong(songToAdd)) {
        // don't add a song that is already in playlist
        songToAdd=0;
      }
    }

    if(autoplayMode==AUTOPLAY_LNP) {
      // method 2: try to pick the song we didn't play for longest time
      int rememberSortOrder=toAddFrom->songlist().getSortOrder();
      toAddFrom->songlist().setSortOrderAndSort(MyList::ByLastPlayed, true);

      // if more than one song have the same timestamp: choose randomly
      QDateTime lnpTimestamp;
      MyList candidateList;
      for(unsigned int i=0; i<toAddFrom->songlist().count(); i++) {
        Song* check=toAddFrom->songlist().at(i)->song();
        if(folderActual->songlist().containsSong(check)) {
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
          QDateTime dt = QDateTime::currentDateTime();
          QDateTime xmas( QDate(2050,12,24), QTime(17,00) );
          int chosen=(dt.secsTo(xmas) + dt.time().msec()) % candidates;
          songToAdd=candidateList.at(chosen)->song();
        }
      }
      toAddFrom->songlist().setSortOrderAndSort(rememberSortOrder, true);
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
	QString caption(i18n("Enter track number"));
	QString message(i18n("Please enter track number"));
	QString trackNrStr(QInputDialog::getText( caption, message, QLineEdit::Normal, QString("1"), &ok, this ));
	if(!ok)
		return;
	int trackNr=atoi(trackNrStr);
	if(trackNr<1)
		return;

	caption=i18n("Enter artist");
	message=i18n("Please enter artist");
	QString artist(QInputDialog::getText( caption, message, QLineEdit::Normal, QString(i18n("MyArtist")), &ok, this ));
	if(!ok)
		return;

	caption=i18n("Enter title");
	message=i18n("Please enter title");
	QString title(QInputDialog::getText( caption, message, QLineEdit::Normal, QString(i18n("Fantastic Song")), &ok, this ));
	if(!ok)
		return;
	
	QString filename=QString(tr("%1%2 - %3.mp3")).arg(m_config.scanDir).arg(artist).arg(title);
  QFileInfo fi(filename);
  if(fi.exists())
  {
	QString msg = i18n("The file\n%1\nalready exists!\n\nPlease choose a different artist/title combination.");
	KMessageBox::information( this, msg.arg(filename));
	return;
  }
	// linux specific
	QString cmd=QString("%1 %2 \"%3\" \"%4\" \"%5\" &").arg(m_config.grabAndEncodeCmd).arg(trackNr).arg(artist).arg(title).arg(filename);
	system(cmd);
	grabbedTrackFilename=filename;
	statusBar( )->message(i18n("grabbing track, will be available shortly..."), 30000);
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
	statusBar( )->message(i18n("grabbed song available"), 20000);
  model->entriesAdded=0;
	model->corruptSongs=0;
  model->problematicSongs.clear();

  model->addSongToDatabase(grabbedTrackFilename, 0);
	updateView();
	folderProblematic->update(model->problematicSongs);
	folderAll->updateTitle();
  QString msg=tr("Yammi tried to add the grabbed song to the database.\n\nSome statistics: \n\n");
  msg+=QString(tr("%1 songs added to database\n")).arg(model->entriesAdded);
  msg+=QString(tr("%1 songs corrupt (=not added)\n")).arg(model->corruptSongs);
  msg+=QString(tr("%1 problematic issues(check in folder Problematic Songs)")).arg(model->problematicSongs.count());
	KMessageBox::information( this,msg);
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

		case Key_Pause:									// exit program (press twice for shutting down computer)
			changeSleepMode();
			break;

		case Key_PageUp:
			if(m_sleepMode)
			{
				int left = m_sleepModeSpinBox->value() + 1;
				m_sleepModeSpinBox->setValue(left);
			}
			break;
		case Key_PageDown:
			if(m_sleepMode)
			{
				int left = m_sleepModeSpinBox->value() - 1;
				if(left > 0 )
					m_sleepModeSpinBox->setValue(left);
			}
			break;

		case Key_Up:
			for(QListViewItem* i=songListView->firstChild(); i; i=i->itemBelow())
			{
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
			for(QListViewItem* i=songListView->firstChild(); i; i=i->itemBelow())
			{
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
			if(e->state()!=ControlButton) {
				break;
      }
			m_searchField->setText("");
			m_searchField->setFocus();
			break;

    case Key_G:  // Ctrl-G
			if(e->state()!=ControlButton) {
				break;
      }
			gotoFuzzyFolder(false);
      break;

    case Key_R:  // Ctrl-R
			if(e->state()!=ControlButton) {
				break;
      }
			gotoFuzzyFolder(true);
      break;

		case Key_Escape:
			m_searchField->setText("");
			m_searchField->setFocus();
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
	kapp->beep();
	if(!m_sleepMode)
	{
		m_sleepMode = true;
		m_sleepModeSpinBox->setValue(3);
		if(model->allSongsChanged() || model->categoriesChanged()) 
		{
			QString msg = i18n("The Database has been modified. Save changes?\n(answering no will cancel sleep mode)");
 			if( KMessageBox::warningYesNo(this,msg) == KMessageBox::Yes )
 				model->save();
 			else
 				m_sleepMode = false; //leave sleep mode off
 		}
 	}
	else
	{
		m_sleepMode = false;
	}
	m_sleepModeButton->setText(m_sleepMode? i18n("shutdown"):i18n("(disabled)"));
	m_sleepModeSpinBox->setEnabled(m_sleepMode);
}



/**
 * stops playback on headphone
 */
void YammiGui::stopPrelisten()
{
  if(m_config.secondSoundDevice=="") {
    cout << "prelisten feature: no sound device configured\n";
    return;
  }
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

/**
 * sends the song to headphones
 * skipTo: 0 = beginning of song, 100 = end
 */
void YammiGui::preListen(Song* s, int skipTo)
{
  if(m_config.secondSoundDevice=="") {
    cout << "prelisten feature: no sound device configured\n";
    return;
  }

  int seconds=s->length;

  // first, kill any previous mpg123 prelisten process
  stopPrelisten();

  // now play song via mpg123, ogg123 or aplay on sound device configured in prefs
	if(s->filename.right(3).upper()=="MP3") {
		QString skip=QString(" --skip %1").arg(seconds*skipTo*38/100);
		QString cmd=QString("mpg123 -a %1 %2 \"%3\" &").arg(m_config.secondSoundDevice).arg(skip).arg(s->location());
    qDebug("command: %s", cmd.latin1());
    system(cmd);
    lastPrelistened="MP3";
	}
  if(s->filename.right(3).upper()=="OGG") {
		QString skip=QString(" --skip %1").arg(seconds*skipTo/100);
		QString cmd=QString("ogg123 -d oss -odsp:%1 %2 \"%3\" &").arg(m_config.secondSoundDevice).arg(skip).arg(s->location());
		system(cmd);
    lastPrelistened="OGG";
  }
	if(s->filename.right(3).upper()=="WAV") {
		QString skip=QString(" trim %1s").arg(seconds*skipTo*441);
		QString cmd=QString("play -d %1 \"%2\" %3 &").arg(m_config.secondSoundDevice).arg(s->location()).arg(skip);
    cout << cmd << "\n";
		system(cmd);
    lastPrelistened="WAV";
	}

}

void YammiGui::updateSongDatabaseHarddisk()
{
	UpdateDatabaseDialog d(this, i18n("Update Database (harddisk) Dialog"));

  d.LineEditScanDir->setText(m_config.scanDir);
  d.LineEditFilePattern->setText("*.mp3 *.ogg *.wav");
	// show dialog
	int result=d.exec();
	if(result!=QDialog::Accepted)
    return;

  QString scanDir=d.LineEditScanDir->text();
  QString filePattern=d.LineEditFilePattern->text();
  updateSongDatabase(scanDir, filePattern, 0);
}

void YammiGui::updateSongDatabaseSingleFile()
{
  QStringList files = KFileDialog::getOpenFileNames( m_config.scanDir,QString::null, this, i18n("Open file(s) to import"));
  if(files.count()==0) {
    return;
  }
  QStringList list = files;
  model->updateSongDatabase(list);
	updateView();
	folderProblematic->update(model->problematicSongs);
	folderAll->updateTitle();
  changeToFolder(folderRecentAdditions);
  QString msg=tr("Updated your database.\n\nStatistics: \n\n");
  msg+=QString(i18n("%1 songs added to database\n")).arg(model->entriesAdded);
  msg+=QString(i18n("%1 songs corrupt (=not added)\n")).arg(model->corruptSongs);
  msg+=QString(i18n("%1 songs problematic (check in folder Problematic Songs)\n")).arg(model->problematicSongs.count());
	KMessageBox::information( this, msg );
}


void YammiGui::updateSongDatabaseMedia()
{
	UpdateDatabaseMediaDialog d(this, i18n("Update Database (media) Dialog"));

  d.LineEditMediaDir->setText(m_config.mediaDir);
  d.LineEditFilePattern->setText("*.mp3 *.ogg *.wav");
  d.CheckBoxMountMediaDir->setChecked(m_config.mountMediaDir);
	// show dialog
	int result=d.exec();
	if(result!=QDialog::Accepted)
    return;
  if(d.LineEditMediaDir->text()=="") {
    KMessageBox::information( this,i18n("You have to enter a name for the media!") );
    return;
  }

  m_config.mountMediaDir=d.CheckBoxMountMediaDir->isChecked();
  QString mediaName=d.LineEditMediaName->text();
  QString mediaDir=d.LineEditMediaDir->text();
  QString filePattern=d.LineEditFilePattern->text();
  updateSongDatabase(mediaDir, filePattern, mediaName);
}



void YammiGui::updateSongDatabase(QString scanDir, QString filePattern, QString media)
{
  if(m_config.childSafe) {
    return;
  }
  KProgressDialog progress( this,0,i18n("Yammi"),i18n ("Scanning..."),true);
	progress.setMinimumDuration(0);
	progress.setAutoReset(false);
  progress.progressBar()->setProgress(0);
	kapp->processEvents();

  isScanning=true;
	model->updateSongDatabase(scanDir, filePattern, media, &progress);

	updateView();
	folderProblematic->update(model->problematicSongs);
	folderAll->updateTitle();
  changeToFolder(folderRecentAdditions);
  QString msg=i18n("Updated your database.\n\nStatistics: \n\n");
  msg+=QString(i18n("%1 songs added to database\n")).arg(model->entriesAdded);
  msg+=QString(i18n("%1 songs corrupt (=not added)\n")).arg(model->corruptSongs);
  msg+=QString(i18n("%1 songs problematic (check in folder Problematic Songs)\n")).arg(model->problematicSongs.count());
	KMessageBox::information( this,msg);

  // TODO: check update actions after scanning...without danger? (=> don't need to stop xmms?)
//  player->syncPlayer2Yammi(folderActual);
  isScanning=false;
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


/**
 * Tries to load all songs of the playlist that need to be swapped into the swap dir,
 * assuming the chosen media is inserted.
 * Mounts the media if necessary/configured.
 */
void YammiGui::loadSongsFromMedia(QString mediaName)
{
  // find out how many songs we try to load from the chosen media
  // (to indicate proper progress)
  int songsToLoad=0;
	for(unsigned int i=1; i<model->songsToPlay.count(); i++) {
		Song* s=model->songsToPlay.at(i)->song();
		for(unsigned int j=0; j<s->mediaLocation.count(); j++) {
		  if(s->mediaName[j]==mediaName) {
  		  if(model->checkAvailability(s)=="") {
  			  songsToLoad++;
        }
      }
    }
	}	
	KProgressDialog progress( this,0, i18n("Yammi"), i18n("Loading song files..."),true);
	progress.progressBar()->setTotalSteps(songsToLoad);
	progress.setMinimumDuration(0);
  progress.progressBar()->setProgress(0);
	kapp->processEvents();
	
	QString mediaDir=m_config.mediaDir;
	QString swapDir=m_config.swapDir;
	if(m_config.mountMediaDir) {
		// linux specific
		QString cmd;
		cmd=QString("mount %1").arg(m_config.mediaDir);
		system(cmd);
	}
	// iterate through playlist and load all songs on that media into swap dir
	// (that are not available so far)
	int loaded=0;
	for(unsigned int i=1; i<model->songsToPlay.count(); i++) {
		if(progress.wasCancelled()) {
			break;
    }
		Song* s=model->songsToPlay.at(i)->song();
		for(unsigned int j=0; j<s->mediaLocation.count(); j++) {
			if(s->mediaName[j]==mediaName) {
		    if(model->checkAvailability(s)=="") {
					cout << "loading song " << s->displayName() << "from " << mediaDir << s->mediaLocation[j] << "\n";
					progress.setLabel(tr("loading song: ")+s->displayName()+" ("+QString("%1").arg(i+1)+tr(". in playlist)"));
			    progress.progressBar()->setProgress(loaded);
  			  kapp->processEvents();
					if(progress.wasCancelled()) {
						break;
          }
					QString filename=s->constructFilename();
          // linux specific
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
	progress.progressBar()->setProgress(loaded);
	kapp->processEvents();
	// unmount swap dir
	if(m_config.mountMediaDir) {
		// linux specific
		QString cmd;
		cmd=QString("umount %1").arg(m_config.mediaDir);
		system(cmd);
	}

  player->syncYammi2Player(false);
	checkPlaylistAvailability();
//	folderContentChanged(folderActual);
  songListView->triggerUpdate();
}

/**
 * Manages loading songfiles from removable media
 */
void YammiGui::checkPlaylistAvailability()
{
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
		mediaListCombo->insertItem(i18n("<none>"));
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
 * Checks whether the swapped songs take more space than the given limit.
 * If they do, we delete the least recently used song files, until we are below
 * the limit again.
 */
void YammiGui::checkSwapSize()
{
	long double sizeLimit=(long double)m_config.swapSize*1024.0*1024.0;
	long double size=0.0;
 	QString path=m_config.swapDir;
 	cout << "checking swap size in directory " << path << ", limit: " << m_config.swapSize << " MB\n";
	QDir d(path);

	d.setFilter(QDir::Files);
	d.setSorting(QDir::Time);			// most recent first
	const QFileInfoList *list = d.entryInfoList();
	QFileInfoListIterator it( *list );

	for(QFileInfo *fi; (fi=it.current()); ++it ) {
		if (fi->isDir()) {							// if directory...		=> skip
			continue;
    }

		if(size+fi->size()>sizeLimit) {
			// swap dir too full, delete this entry
			cout << "removing from swap dir: " << fi->fileName() << "\n";
			QDir dir;
			if(!dir.remove(path+fi->fileName())) {
				cout << "could not remove LRU song from swapdir";
      }
		}
		else {
			size+=fi->size();
    }
	}
}


/**
 * Tells the media player to skip to the next song.
 */
void YammiGui::skipForward()
{
  player->skipForward(shiftPressed);
}

/**
 * Performs a skip backward with a little trick:
 * Prepends the last played song to the playlist,
 * then tells the media player to skip backward.
 * Also removes this last played song from the songsPlayed folder,
 * as it will be inserted again after being played.
 */
void YammiGui::skipBackward()
{
  int count=model->songsPlayed.count();
	if(count==0) {
    // empty folder songsPlayed => can't skip backwards
    return;
  }

	// 1. get and remove last song from songsPlayed
	Song* last=model->songsPlayed.at(count-1)->song();
	model->songsPlayed.remove(count-1);
	folderActual->insertSong(last, 0);
  player->skipBackward(shiftPressed);

  // skipping backward creates a new entry in songsPlayed that we don't want
  // => remove it again
	model->songsPlayed.remove(model->songsPlayed.count()-1);
  folderSongsPlayed->updateTitle();
  folderContentChanged(folderActual);
  folderContentChanged(folderSongsPlayed);
}


void YammiGui::toggleColumnVisibility(int column)
{
  columnsMenu->setItemChecked(column, !columnsMenu->isItemChecked(column));
  columnVisible[column]=columnsMenu->isItemChecked(column);
  changeToFolder(chosenFolder, true);
}



void YammiGui::loadMediaPlayer( )
{
  switch( m_config.player )
  {
  	case 0: player = new XmmsPlayer(0, model);
		break;
	case 1: player = new NoatunPlayer( model );
		break;
	case 2: player = new Yammi::ArtsPlayer( model );
		break;
	default:player = new DummyPlayer( model );
  }
  kdDebug()<<"Media Player : "<<player->getName( )<<endl;
}

void YammiGui::createMainWidget( )
{
	QSplitter* centralWidget=new QSplitter(Qt::Horizontal, this);

	// set up the quick browser on the left
	folderListView = new QListView( centralWidget );
	folderListView->header()->setClickEnabled( FALSE );
	folderListView->addColumn( i18n("Quick Browser") );
	folderListView->setRootIsDecorated( TRUE );
	folderListView->setSorting(-1);
	centralWidget->setResizeMode( folderListView, QSplitter::KeepSize );

  // set up the songlist on the right
	songListView = new MyListView( centralWidget );
	for(int i=0; i<MAX_COLUMN_NO; i++)
	{
		columnVisible[i]=true;
	}

	QValueList<int> lst;
	lst.append( 150 );
	centralWidget->setSizes( lst );

	setCentralWidget(centralWidget);

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
}


void YammiGui::createFolders( )
{
	// folder containing all music
	folderAll=new Folder( folderListView, QString(i18n("All Music")), &(model->allSongs));

	// folder containing all artists with more than <n> songs
	folderArtists = new FolderGroups( folderListView, QString( i18n("Artists") ));
	folderArtists->moveItem(folderAll);

	// folder containing all albums with more than <n> songs
	folderAlbums = new FolderGroups( folderListView, QString( i18n("Albums") ));
	folderAlbums->moveItem(folderArtists);

	// folder containing all genres with more than <n> songs
	folderGenres = new FolderGroups( folderListView, QString( i18n("Genre") ));
	folderGenres->moveItem(folderAlbums);

	// folder containing all songs from a year (if more than <n> songs)
	folderYears = new FolderGroups( folderListView, QString( i18n("Year") ));
	folderYears->moveItem(folderGenres);

	// folder containing all categories
	folderCategories = new FolderCategories( folderListView, QString(i18n("Categories")));
	folderCategories->moveItem(folderYears);

	// folder containing media
	folderMedia = new FolderMedia( folderListView, QString(i18n("Media")));
	folderMedia->moveItem(folderCategories);

	// folder containing currently played song
	folderActual = new FolderSorted(folderListView, QString(i18n("Playlist")), &(model->songsToPlay));

	// folder containing songs played in this session
	folderSongsPlayed = new Folder(folderListView, QString(i18n("Songs Played")), &(model->songsPlayed));
	folderSongsPlayed->moveItem(folderCategories);

	// folder containing history
	folderHistory = new Folder(folderListView, QString(i18n("History")), &(model->songHistory));
	folderHistory->moveItem(folderSongsPlayed);

	// folder containing unclassified songs
	folderUnclassified = new Folder(folderListView, QString(i18n("Unclassified")), &(model->unclassifiedSongs));
	folderUnclassified->moveItem(folderMedia);

	folderSearchResults = new Folder( folderListView, QString(i18n("Search Results")), &searchResults );
	folderSearchResults->moveItem(folderActual);

	folderProblematic = new Folder( folderListView, QString(i18n("Problematic Songs")) );
	folderProblematic->moveItem(folderUnclassified);

	folderRecentAdditions = new Folder(folderListView, QString(i18n("Recent Additions")), &(model->recentSongs));
	folderRecentAdditions->moveItem(folderUnclassified);

	// signals of folders
	connect(folderCategories, SIGNAL( CategoryNew() ), this, SLOT(newCategory()));
	connect(folderCategories, SIGNAL( CategoryRemoved() ), this, SLOT(removeCategory()));
	connect(folderCategories, SIGNAL( CategoryRenamed() ), this, SLOT(renameCategory()));
	connect(folderCategories, SIGNAL( LoadM3uIntoCategory() ), this, SLOT(loadM3uIntoCategory()));
	connect(folderMedia, SIGNAL( RemoveMedia() ), this, SLOT(removeMedia()));
	connect(folderMedia, SIGNAL( RenameMedia() ), this, SLOT(renameMedia()));
}


void YammiGui::setupActions( )
{
	KStdAction::quit(this, SLOT(endProgram()), actionCollection());

	//Selection actions
	KStdAction::selectAll(this,SLOT(selectAll()),actionCollection());
	new KAction(i18n("Invert selection"),0,0,this,SLOT(invertSelection()), actionCollection(),"invert_selection");

	//Media player actions
	m_playPauseAction =
		new KAction(i18n("Play"),"player_play",KShortcut(Key_F1),player,SLOT(playPause()), actionCollection(),"play_pause");
	new KAction(i18n("Stop"),"player_stop",KShortcut(Key_F4),player,SLOT(stop()),actionCollection(),"stop");
	new KAction(i18n("Skip Backward"),"player_rew",KShortcut(Key_F2),this,SLOT(skipBackward()), actionCollection(),"skip_backward");
	new KAction(i18n("Skip Forward"),"player_fwd",KShortcut(Key_F3),this,SLOT(skipForward()), actionCollection(),"skip_forward");
	m_seekSlider = new QSlider( QSlider::Horizontal, 0L, "seek_slider");
	//m_seekSlider->setTickmarks( QSlider::Below );
	m_seekSlider->setTracking( false );
	connect(m_seekSlider,SIGNAL(sliderMoved(int)),this,SLOT(seek(int)));
	new KWidgetAction( m_seekSlider ,"text",0, 0, 0,actionCollection(),"seek");

	//Database actions
	new KAction(i18n("Save Database"),"save",KShortcut(QKeySequence(Key_Control,Key_S)),model,SLOT(save()), actionCollection(),"save_db");
	new KAction(i18n("Scan Harddisk..."),0,0,this,SLOT(updateSongDatabaseHarddisk()), actionCollection(),"scan_hd");
	new KAction(i18n("Scan Removable Media..."),0,0,this,SLOT(updateSongDatabaseMedia()), actionCollection(),"scan_media");
	new KAction(i18n("Import Selected File(s)..."),0,0,this,SLOT(updateSongDatabaseSingleFile()), actionCollection(),"import_file");
	new KAction(i18n("Check Consistency..."),0,0,this,SLOT(forAllCheckConsistency()), actionCollection(),"check_consistency");
	new KAction(i18n("Grab And Encode CD-Track..."),0,0,this,SLOT(grabAndEncode()), actionCollection(),"grab");

	// playlist actions
	new KAction(i18n("Switch to/from Playlist"),0,0,this,SLOT(toFromPlaylist()),actionCollection(),"to_from_playlist");
	new KAction(i18n("Clear Playlist..."),0,0,this,SLOT(clearPlaylist()),actionCollection(),"clear_playlist");
	new KAction(i18n("Shuffle Playlist..."),0,0,this,SLOT(shufflePlaylist()),actionCollection(),"shuffle_playlists");
	new KAction(i18n("Clear Playlist"),"dequeue_all",KShortcut(QKeySequence(Key_Shift,Key_F8)),this,SLOT(clearPlaylist()),actionCollection(),"clear_playlist");
	new KAction(i18n("Switch to/from Playlist"),"toggle_playlist",KShortcut(Key_P),this,SLOT(toFromPlaylist()),actionCollection(),"toggle_playlist");

  // selection actions
	new KAction(i18n("Enqueue as next (prepend)"),"enqueue_asnext",KShortcut(Key_F6),this,SLOT(forSelectionPrepend()),actionCollection(),"prepend_selected");
	new KAction(i18n("Enqueue at end (append)"),"enqueue",KShortcut(Key_F5),this,SLOT(forSelectionAppend()),actionCollection(),"append_selected");
	new KAction(i18n("Play Now!"),"play_now",KShortcut(Key_F7),this,SLOT(forSelectionPlay()),actionCollection(),"play_selected");
	new KAction(i18n("Dequeue Songs"),"dequeue_song",KShortcut(Key_F8),this,SLOT(forSelectionDequeue()),actionCollection(),"dequeue_selected");
	new KAction(i18n("Prelisten start"),"prelisten_start",KShortcut(Key_F9),this,SLOT(forSelectionPrelistenStart()),actionCollection(),"prelisten_start");
	new KAction(i18n("Prelisten middle"),"prelisten_middle",KShortcut(Key_F10),this,SLOT(forSelectionPrelistenMiddle()),actionCollection(),"prelisten_middle");
	new KAction(i18n("Prelisten end"),"prelisten_end",KShortcut(Key_F11),this,SLOT(forSelectionPrelistenEnd()),actionCollection(),"prelisten_end");
  new KAction(i18n("Song Info..."),"song_info",KShortcut(Key_I),this,SLOT(forSelectionSongInfo()),actionCollection(),"info_selected");
	new KAction(i18n("Delete Song..."),0,0,this,SLOT(forSelectionDelete()),actionCollection(),"delete_selected");
  new KAction(i18n("Burn To Media"),0,0,this,SLOT(forSelectionBurnToMedia()),actionCollection(),"burn_selected");
  new KAction(i18n("Move Files"),0,0,this,SLOT(forSelectionMove()),actionCollection(),"move_selected");
  new KAction(i18n("Search for similar entry"),0,0,this,SLOT(searchForSimilarEntry()),actionCollection(),"search_similar_entry");
  new KAction(i18n("Search for similar artist"),0,0,this,SLOT(searchForSimilarArtist()),actionCollection(),"search_similar_artist");
  new KAction(i18n("Search for similar title"),0,0,this,SLOT(searchForSimilarTitle()),actionCollection(),"search_similar_title");
  new KAction(i18n("Search for similar album"),0,0,this,SLOT(searchForSimilarAlbum()),actionCollection(),"search_similar_album");
  new KAction(i18n("Goto artist"),0,0,this,SLOT(gotoFolderArtist()),actionCollection(),"goto_artist_folder");
  new KAction(i18n("Goto album"),0,0,this,SLOT(gotoFolderAlbum()),actionCollection(),"goto_album_folder");
  new KAction(i18n("Goto genre"),0,0,this,SLOT(gotoFolderGenre()),actionCollection(),"goto_genre_folder");

	new KAction(i18n("Stop prelisten"),"stop_prelisten",KShortcut(Key_F12),this,SLOT(stopPrelisten()),actionCollection(),"stop_prelisten");
	// autoplay actions
	KToggleAction *ta;
	ta = new KRadioAction(i18n("Off"),0,0,this,SLOT(autoplayOff()),actionCollection(),"autoplay_off");
	ta->setExclusiveGroup("autoplay");
	ta = new KRadioAction(i18n("Longest not played"),0,0,this,SLOT(autoplayLNP()),actionCollection(),"autoplay_longest");
	ta->setExclusiveGroup("autoplay");
	ta = new KRadioAction(i18n("Random"),0,0,this,SLOT(autoplayRandom()),actionCollection(),"autoplay_random");
	ta->setExclusiveGroup("autoplay");
	m_currentAutoPlay = new KAction(i18n("Unknown"),0,0,0,0,actionCollection(),"autoplay_folder");

	// toggle toolbar actions
	ta = new KToggleAction("Main ToolBar",0,0,this,SLOT(toolbarToggled()),actionCollection(),"MainToolbar");	
	ta = new KToggleAction("Media Player",0,0,this,SLOT(toolbarToggled()),actionCollection(),"MediaPlayerToolbar");	
	ta = new KToggleAction("Song Actions",0,0,this,SLOT(toolbarToggled()),actionCollection(),"SongActionsToolbar");	
	ta = new KToggleAction("Removable Media",0,0,this,SLOT(toolbarToggled()),actionCollection(),"RemovableMediaToolbar");	
	ta = new KToggleAction("Sleep Mode",0,0,this,SLOT(toolbarToggled()),actionCollection(),"SleepModeToolbar");	
	ta = new KToggleAction("Prelisten",0,0,this,SLOT(toolbarToggled()),actionCollection(),"PrelistenToolbar");
	
	// other actions
	new KAction(i18n("Update Automatic Folder Structure"),0,0,this,SLOT(updateView()),actionCollection(),"update_view");
  
	// setup
	KStdAction::preferences(this,SLOT(setPreferences()),actionCollection());
	
	// - "insert custom widgets in toolbar using KWidgetAction
	//search
	QHBox *w = new QHBox( );
	new QLabel(i18n("Search:"),w);
	m_searchField = new LineEditShift(w);
	m_searchField->setFixedWidth(175);
	QToolTip::add( m_searchField, i18n("Fuzzy search (Ctrl-F)"));
	connect( m_searchField, SIGNAL(textChanged(const QString&)), SLOT(searchSong(const QString&)));
	QPushButton *btn = new QPushButton(i18n("to wishlist"),w);
	connect( btn, SIGNAL( clicked() ), this, SLOT( addToWishList() ) );
	QToolTip::add( btn, i18n("Add this entry to the database as a \"wish\""));
	new KWidgetAction( w ,"Search",0, 0, 0,actionCollection(),"search");
	
	// removable media management	
  w = new QHBox( );
	new QLabel(i18n("Needed media:"),w);
	mediaListCombo = new QComboBox( FALSE, w );
	mediaListCombo->setFixedWidth(150);
	loadFromMediaButton = new QPushButton(i18n("load"), w);
	connect( loadFromMediaButton, SIGNAL( clicked() ), this, SLOT( loadMedia() ) );
	new KWidgetAction( w ,"Load media",0, 0, 0,actionCollection(),"load_media");
	
	// Sleep mode
	w = new QHBox( );
	new QLabel(i18n("Sleep mode:"),w);
	m_sleepModeButton = new QPushButton(i18n("(disabled)"), w);
	QToolTip::add( m_sleepModeButton, i18n("toggle sleep mode"));
	connect( m_sleepModeButton, SIGNAL( clicked() ), this, SLOT( changeSleepMode() ) );
	m_sleepModeSpinBox=new QSpinBox(1, 99, 1, w);
	QToolTip::add( m_sleepModeSpinBox, i18n("number songs until shutdown"));
	m_sleepMode = false;
	m_sleepModeSpinBox->setEnabled(m_sleepMode);
	new KWidgetAction( w ,"Sleep Mode",0, 0, 0,actionCollection(),"sleep_mode");	
	
  // if the rc file is properly installed yammi finds it automatically
  // (eg. /opt/kde3/share/apps/yammi/yammiui.rc)
  createGUI();
}
//////////////////

void YammiGui::createMenuBar( )
{
	KMenuBar *mainMenu = menuBar( );

	//FIXME Integrate this into the menu generated by the XML-GUI Framework!!!
  // columns submenu
	columnsMenu = new QPopupMenu;
	for(int column=0; column<13; column++) {
	  columnsMenu->insertItem( columnName[column],  this, SLOT(toggleColumnVisibility(int)), 0, column);
	}
  // view menu
	QPopupMenu* viewMenu = new QPopupMenu;
	//viewMenu->insertItem( i18n("Toolbars"), toolbarsMenu );
	viewMenu->insertItem( i18n("Columns"), columnsMenu );
	mainMenu->insertItem( i18n("&View"), viewMenu );
}

/**
 * Creates the song popup menu from the xml gui framework.
 * Also calls updateSongPopup to populate the submenus for categories and plugins.
 * These have to be updated with updateSongPopup on each change in existing categories
 * or plugins.
 */
void YammiGui::createSongPopup()
{
  songPopup = (QPopupMenu *)factory()->container("song_popup", this);
  songPopup->insertItem( "", 113, 0);
  songPopup->insertSeparator(1);
  playListPopup = new QPopupMenu(songPopup);
	songPopup->insertItem( i18n("Insert Into/Remove From..."), playListPopup, -1, -1);
	pluginPopup = new QPopupMenu(songPopup);
	songPopup->insertItem( i18n("Plugins..."), pluginPopup, -1, -1);
  // populate the submenus
  updateSongPopup();
}

/**
 * Tells the media player to jump with the playback to the given position
 * within the currently played song.
 * 0 = beginning, 100 = end?
 */
void YammiGui::seek( int pos )
{
	kdDebug()<<"seek song to pos "<<pos<<endl;
	player->jumpTo(pos);
}


void YammiGui::setSelectionMode(SelectionMode mode)
{
  selectionMode = mode;
}
