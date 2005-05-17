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

#include "searchthread.h"

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

#include <taglib/id3v1genres.h>
#include "ConsistencyCheckParameter.h"

#include <qsimplerichtext.h>
#include <khtmlview.h>
#include <khtml_part.h>
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
#include <kkeydialog.h>
#include <kprocess.h>
#include <dcopobject.h>


#include <qheader.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qlistview.h>
#include <qtooltip.h>
#include <qcombobox.h>


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
#include "trackpositionslider.h"
#include "searchthread.h"
#include "util.h"

#include "mediaplayer.h"
#include "dummyplayer.h"
#include "noatunplayer.h"
#include "artsplayer.h"
#include "gstplayer.h"
#ifdef ENABLE_XMMS
#include "xmmsplayer.h"
#else
#define XmmsPlayer DummyPlayer
#endif


static QString columnName[] = { i18n("Artist"), i18n("Title"), i18n("Album"), i18n("Length"),
                                i18n("Year"), i18n("TrackNr"), i18n("Genre"), i18n("AddedTo"), i18n("Bitrate"),
                                i18n("Filename"), i18n("Path"), i18n("Comment"), i18n("Last Played") };


extern YammiGui* gYammiGui;

/////////////////////////////////////////////////////////////////////////////////////////////
YammiGui::YammiGui() : DCOPObject("YammiPlayer"), KMainWindow( ) {
    gYammiGui = this;
    setGeometry(0, 0, 800, 600);

    // initialize some fields
    validState = false;
    currentSong = 0;
    chosenFolder = 0;
    folderToAdd = 0;
    selectionMode = SELECTION_MODE_USER_SELECTED;
    isScanning = false;
    controlPressed = false;
    shiftPressed = false;
    toFromRememberFolder = 0;


    model = new YammiModel( this );
    config()->loadConfig();

    if(!setupActions()) {
        return;
    }
    createMainWidget( );
    createFolders( );

    // final touches before start up
    readOptions( );

    // from here: stuff that needs the options to be read already
    createMenuBar( );
    validState = true;
}

/**
 * connect media player and yammi via signals
 */
void YammiGui::connectMediaPlayer() {
    connect( player, SIGNAL(playlistChanged()), this, SLOT(updatePlaylist()) );
    connect( player, SIGNAL(statusChanged()), this, SLOT(updatePlayerStatus()) );
    connect( &checkTimer, SIGNAL(timeout()), player, SLOT(check()) );
}

void YammiGui::disconnectMediaPlayer() {
    disconnect( player, SIGNAL(playlistChanged()), this, SLOT(updatePlaylist()) );
    disconnect( player, SIGNAL(statusChanged()), this, SLOT(updatePlayerStatus()) );
    disconnect( &checkTimer, SIGNAL(timeout()), player, SLOT(check()) );
}



void YammiGui::loadDatabase(QString databaseDir) {
    loadMediaPlayer( );
    // TODO: replace this checkTimer with a thread owned by the media player
    connectMediaPlayer( );

    // connect all timers
    searchResultsUpdateNeeded=false;
    searchThread = new SearchThread(this);
    searchThread->start();
    connect( &regularTimer, SIGNAL(timeout()), SLOT(onTimer()) );
    connect( &searchResultsTimer, SIGNAL(timeout()), SLOT(updateSearchResults()) );

    // TODO: make yammi topmost window (does not work: setActiveWindow())

    if(databaseDir.isEmpty()) {
        databaseDir = KGlobal::dirs()->findResourceDir("appdata", "songdb.xml");
        if(databaseDir.isNull()) {
            databaseDir=(KGlobal::dirs()->saveLocation("appdata"));
        }
    }
    if(!databaseDir.endsWith("/")) {
        databaseDir+="/";
    }
    kdDebug() << "trying to load database from directory " << databaseDir << endl;
    QDir d(databaseDir);
    bool importOld=false;
    config()->databaseDir = databaseDir;
    QString oldYammiDir(QDir::homeDirPath()+"/.yammi/");
    if(!d.exists("songdb.xml")) {
        QDir oldDir(oldYammiDir);
        if(oldDir.exists() && oldDir.exists("songdb.xml")) {
            kdDebug() << "no database existing yet, database from previous yammi versions found\n";
            kdDebug() << "importing your old song database from " << oldYammiDir << endl;
            /*
            TODO: KMessageBox crashes after any button pressed... event loop problem???

            QString msg("No yammi database has been found at\n");
            msg+="\t" + databaseDir + "\n";
            msg+="However, an old yammi database\n";
            msg+="(from a previous version of yammi)\n";
            msg+="has been found at\n";
            msg+="\t" + oldYammiDir +"\n\n";
            msg+="Do you wish to import this database?\n";
            int result=KMessageBox::questionYesNoCancel(this, msg, "import existing database");
            if(result==KMessageBox::Yes) {
            */
            importOld=true;
            config()->databaseDir=oldYammiDir;
            //}
        }
    }
    model->readSongDatabase();
    // finish initialization of player
    //player->syncPlayer2Yammi(&(model->songsToPlay));
    KConfig* cfg = kapp->config();
    cfg->setGroup("General Options");

    model->readCategories();
    model->readHistory();
    player->finishInitialization();
    
    bool restorePlaylistOnStartup = true;            // TODO: make this configurable
    if(restorePlaylistOnStartup) {
        model->readList(&(model->songsToPlay), config()->databaseDir + "/" + "playqueue.xml");
        folderActual->update(folderActual->songlist());
        player->syncYammi2Player();
        if(folderActual->songlist().count() > 0) {
            m_seekSlider->setupTickmarks(folderActual->firstSong());
            int savedSongPosition = cfg->readNumEntry("savedSongPosition", 0);
            if(savedSongPosition != 0) {
                player->jumpTo(savedSongPosition);
            }
            int savedPlayingStatus = cfg->readNumEntry("savedPlayingStatus", STOPPED);
            if(savedPlayingStatus==PLAYING && player->getStatus()!=PLAYING ) {
                player->play();
            }
        }
    }
    checkPlaylistAvailability();
    // update dynamic folders based on database contents
    updateView(true);

    if(importOld) {
        config()->databaseDir=databaseDir;
        model->saveAll();
        model->saveHistory();
    }

    Folder* f=getFolderByName(cfg->readEntry("CurrentFolder"));
    if(f != 0) {
        changeToFolder(f, true);
    } else {
        changeToFolder(folderAll, true);
    }
    folderContentChanged(folderActual);
    
    checkTimer.start( 100, FALSE );
    regularTimer.start( 500, FALSE );
    searchResultsTimer.start( 10, FALSE );
}

void YammiGui::saveOptions() {
    kdDebug() << "saveOptions() " << endl;
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

    cfg->writeEntry("CurrentFolder", chosenFolder->folderName());
    cfg->writeEntry("savedSongPosition", player->getCurrentTime());
    cfg->writeEntry("savedPlayingStatus", player->getStatus());
    for(int i=0; i<MAX_COLUMN_NO; i++) {
        cfg->writeEntry(QString("Column%1Visible").arg(i), columnIsVisible(i));
    }
    cfg->writeEntry( "columnOrder" , columnOrder);
    for(int i=0; i<MAX_COLUMN_NO; i++) {
        cfg->writeEntry( QString("column%1Width").arg(i), columnWidth[i]);
    }
    cfg->writeEntry("AutoplayFolder", autoplayFoldername);
    cfg->writeEntry("AutoplayMode", autoplayMode);
    
    cfg->writeEntry("LeftSplitterWidth", centralWidget->sizes().first());    
    cfg->writeEntry("LeftUpperSplitterHeight", leftWidget->sizes().first());
}


void YammiGui::readOptions() {
    kdDebug() << "readOptions()" << endl;
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
    if(!size.isEmpty()) {
        resize(size);
    }
    for(int i=0; i<MAX_COLUMN_NO; i++) {
        columnVisible[i]=cfg->readBoolEntry(QString("Column%1Visible").arg(i), true);
    }
    columnOrder=cfg->readListEntry("columnOrder");
    if(columnOrder.count()==0) {
        kdDebug() << "no column order found, taking default order\n";
    }
    for(int i=0; i<MAX_COLUMN_NO; i++) {
        columnWidth[i]=cfg->readNumEntry(QString("column%1Width").arg(i));
    }
    autoplayFoldername=cfg->readEntry("AutoplayFolder", i18n("All Music"));
    m_currentAutoPlay->setText(i18n("Folder: ")+autoplayFoldername);
    autoplayMode=cfg->readNumEntry( "AutoplayMode", AUTOPLAY_OFF);
    switch(autoplayMode) {
    case AUTOPLAY_OFF:
        m_autoplayActionOff->setChecked(true);
        break;
    case AUTOPLAY_LNP:
        m_autoplayActionLnp->setChecked(true);
        break;
    case AUTOPLAY_RANDOM:
        m_autoplayActionRandom->setChecked(true);
        break;
    }
        
    QValueList<int> lst2;
    lst2.append(cfg->readNumEntry("LeftUpperSplitterHeight", 200));
    leftWidget->setSizes( lst2 );
    
    QValueList<int> lst;
    lst.append( cfg->readNumEntry("LeftSplitterWidth", 250));
    centralWidget->setSizes( lst );
}


void YammiGui::saveProperties(KConfig *config) {
    kdDebug() << "saveProperties(KConfig *config, name: " << config->name() << ")" << endl;
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
}

void YammiGui::readProperties(KConfig *config) {
    kdDebug() << "readProperties(KConfig *config, name: " << config->name() << ")" << endl;
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'
}


bool YammiGui::queryClose() {
    kdDebug() << "queryClose()" << endl;
    if(model->allSongsChanged() || model->categoriesChanged()) {
        QString msg=i18n("The Song Database has been modified.\nDo you want to save the changes?");
        switch( KMessageBox::warningYesNoCancel(this,msg,i18n("Database modified"))) {
        case KMessageBox::Yes:
            saveDatabase();
            break;
        case KMessageBox::No:
            break;
        case KMessageBox::Cancel:
            return false;
        }
    } else { //the db has not changed, but save history anyways...(only if more than 2 songs to add)
        if(config()->logging && model->songsPlayed.count()>2) {
            model->saveHistory();
        }
    }
    // save playlist
    model->saveList(&(model->songsToPlay), config()->databaseDir, "playqueue");    
    return true;
}

bool YammiGui::queryExit() {
    kdDebug() << "queryExit() " << endl;
    saveOptions();
    player->quit( );
    return true;
}


void YammiGui::shutdownSequence( ) {
    QString msg(i18n("Shutting down in %1 seconds"));
    KProgressDialog d(this,0,i18n("Shutting down..."));

    d.setMinimumDuration(0);
    d.setAllowCancel(true);
    QTimer *t;
    int total = 10;
    d.progressBar()->setTotalSteps(total);
    for( int i = 0; i < 10; ++i ) {
        d.setLabel( msg.arg(total-i) );
        d.progressBar()->setProgress( i );
        t = new QTimer();
        t->start(1000,true);
        while(t->isActive()) {
            kapp->processEvents();
        }
        delete t;
        if( d.wasCancelled() ) {
            kdDebug()<<"Shutdown cancelled"<<endl;
            changeSleepMode();
            return;
        }
    }
    model->save();
    //TODO: change to saveDatabase()
    // (what if the user touched the database in between? => save immediately or cancel sleep mode)
    if(config()->shutdownScript.isEmpty()) {
        this->close();
    } else {
        system(config()->shutdownScript+" &");
    }
}

void YammiGui::toolbarToggled( const QString& name ) {
    QString n = name;
    if(n.isNull()) {//see if we were called by an action
        const QObject *s = 0L;
        s = sender();
        if(s) {
            n = s->name();
        }
    }
    KToggleAction *action = dynamic_cast<KToggleAction*>(actionCollection()->action( n ));
    if(!action) {
        kdError()<<"toolbarToggled( const QString& name ) : action not found n = "
        <<n<<" (name = "<<name<<")"<<endl;
        return;
    }
    if(!action->isChecked()) {
        toolBar(n)->hide();
    } else {
        toolBar(n)->show();
    }
}

//****************************************************************************************************//
YammiGui::~YammiGui() {
    delete player;
    searchThread->stopThread();
    searchFieldChangedIndicator.wakeOne();
    searchThread->wait();
    delete searchThread;
    searchThread = NULL;
}


/**
 * This slot is called on changes in the playlist (model->songsToPlay),
 * signalled by the mediaplayer or on changes from within yammigui (enqueing, dequeing songs, ...)
 */
void YammiGui::updatePlaylist() {
    kdDebug() << "updatePlaylist called" << endl;
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
    player->syncYammi2Player();
    folderContentChanged(folderActual);
}

/**
 * Update the html playlist view after changes
 */
void YammiGui::updateHtmlPlaylist()
{
    QString htmlTemplate = config()->playqueueTemplate;
    
    int noSongsToPlay = folderActual->songlist().count();
    int length = 0;
    for(unsigned int i=0; i<folderActual->songlist().count(); i++) {
        length += folderActual->songlist().at(i)->song()->length;
    }
    QString formattedTime;
    formattedTime.sprintf(i18n("%d:%02d"), length/(60*60), (length % (60*60))/60);
    htmlTemplate.replace(QRegExp("\\{noSongsToPlay\\}"), QString("%1").arg(noSongsToPlay));
    htmlTemplate.replace(QRegExp("\\{timeToPlay\\}"), formattedTime);

    QString htmlSource("");
    
    QStringList entries = QStringList::split(QString("{scope:"), htmlTemplate);
    
    for ( QStringList::Iterator it = entries.begin(); it != entries.end(); ++it ) {
        QString entry = *it;
        int pos = entry.find('}');
        if(pos != -1) {
            QString scopeNumberStr = entry.left(pos);
            bool ok;
            int scopeNumber = scopeNumberStr.toInt(&ok);
            QString entryTemplate = entry.mid(pos +1);
            if(ok) {
                if(scopeNumber < 0) {
                    if(((int)folderSongsPlayed->songlist().count()) >= (-scopeNumber)) {
                        int listIndex = folderSongsPlayed->songlist().count() + scopeNumber;
                        Song* s = folderSongsPlayed->songlist().at(listIndex)->song();
                        htmlSource += s->replacePlaceholders(entryTemplate, scopeNumber);
                    }
                }
                else {
                    if(((int)folderActual->songlist().count()) > scopeNumber) {
                        Song* s = folderActual->songlist().at(scopeNumber)->song();
                        htmlSource += s->replacePlaceholders(entryTemplate, scopeNumber);
                    }
                }
            }
            else {
                // out of scope: add without replacing anything
                htmlSource += entryTemplate;
            }
        }
        else {
            kdDebug() << "invalid scope definition in template!" << endl;
        }
    }
    
    playlistPart->begin();
    playlistPart->write(htmlSource);
    playlistPart->end();

    playlistPart->show();
    playlistPart->view()->setHScrollBarMode(QScrollView::AlwaysOn);
}

/**
 * TODO: document, when is this method called???
 */
void YammiGui::updateCurrentSongStatus() {
    Song* firstInPlaylist = 0;
    if(model->songsToPlay.count()>0) {
        firstInPlaylist = model->songsToPlay.firstSong();
    }
    if(firstInPlaylist!=currentSong) {
        // a change in the first (=currently played) song!
        handleLastSong(currentSong);
        handleNewSong(model->songsToPlay.firstSong());
    }
}

/**
 * Puts the song that was just played into folder songsPlayed.
 */
void YammiGui::handleLastSong(Song* lastSong) {
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
void YammiGui::handleNewSong(Song* newSong) {
    currentSong=newSong;
    if(newSong==0) {
        setCaption(i18n("Yammi - not playing"));
        currentFile="";
        m_seekSlider->setupTickmarks(0);
        return;
    }
    // TODO: take swapped file?
    currentFile=newSong->location();
    currentSongStarted=currentSongStarted.currentDateTime();

    if(m_sleepMode) {
        int left = m_sleepModeSpinBox->value() - 1;
        if(left > 0 ) {
            m_sleepModeSpinBox->setValue(left);
        } else {
            shutdownSequence();
        }
    }

    setCaption("Yammi: "+currentSong->displayName());
    m_seekSlider->setupTickmarks(currentSong);
}

/**
 * This slot should be called on changes in the player status.
 * eg. signalled by the mediaplayer
 */
void YammiGui::updatePlayerStatus() {
    if(player==0) {
        return;
    }
    if(player->getStatus()==PLAYING) {
        m_playPauseAction->setIcon("player_pause");
        m_playPauseAction->setText(i18n("Pause"));
    } else {
        m_playPauseAction->setIcon("player_play");
        m_playPauseAction->setText(i18n("Play"));
    }

    // check, if we stopped because playlist empty and sleep mode activated
    if(m_sleepMode && player->getStatus() == STOPPED && model->songsToPlay.count()==0) {
        shutdownSequence( );
    }
}


QString YammiGui::getColumnName(int column) {
    return QString(columnName[column]);
}



/**
 * Switches to the folder best matching the search term.
 * Uses yammi's fuzzy search capabilities.
 */
void YammiGui::gotoFuzzyFolder(bool backward) {//HERE
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
        for(; noResults<FUZZY_FOLDER_LIST_SIZE && noResults<200 && bme[noResults]; noResults++) {
            Folder* f=(Folder*)(bme[noResults]->objPtr);
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
    } else {
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


/**
 * Return the first found folder with the given name, or 0 if no folder with that name exists
 */
Folder* YammiGui::getFolderByName(QString folderName) {
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
void YammiGui::updateView(bool startup) {
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
        folderAll->updateTitle();
        folderHistory->updateTitle();
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
bool YammiGui::columnIsVisible(int column) {
    return columnVisible[column];
}


int YammiGui::mapToRealColumn(int column) {
    return realColumnMap[column];
}

void YammiGui::mapVisibleColumnsToOriginals() {
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
void YammiGui::updateListViewColumns() {
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

void YammiGui::saveColumnSettings() {
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
void YammiGui::setPreferences() {
    int playerBefore=config()->mediaPlayer;

    PreferencesDialog d(this, "preferencesDialog", true, config());

    // show dialog
    int result=d.exec();

    if(result!=QDialog::Accepted) {
        return;
    }
    updateSongPopup();
    updateHtmlPlaylist();
    config()->saveConfig();
    if(playerBefore != config()->mediaPlayer) {
        kdDebug() << "trying to switch media player...\n";
        int savedSongPosition = player->getCurrentTime();
        int savedPlayingState = player->getStatus();
        player->stop();
        disconnectMediaPlayer();
        kdDebug() << "old player disconnected\n";
        delete player;
        kdDebug() << "old player deleted\n";
        loadMediaPlayer();
        kdDebug() << "new media player loaded...\n";
        connectMediaPlayer();
        kdDebug() << "new media player connected...\n";
        player->syncYammi2Player();
        if(savedSongPosition != 0) {
            player->jumpTo(savedSongPosition);
        }
        if(folderActual->songlist().count() > 0 && savedPlayingState == PLAYING && player->getStatus() != PLAYING) {
            player->play();
        }
    }
}

/**
 * Configures the key bindings.
 */
void YammiGui::configureKeys() {
    KKeyDialog::configure(actionCollection());
}

/**
 * Updates the songPopup submenus with available categories and plugins
 */
void YammiGui::updateSongPopup() {
    kdDebug() << "updating song popup\n";
    playListPopup->clear();
    playListPopup->insertItem(QIconSet( QPixmap(newCategory_xpm)), i18n("New Category..."), this, SLOT(toCategory(int)), 0, 9999);
    for(unsigned int i=0; i<model->categoryNames.count(); i++) {
        playListPopup->insertItem(QIconSet( QPixmap(in_xpm)), model->categoryNames[i], this, SLOT(toCategory(int)), 0, 10000+i);
    }
    pluginPopup->clear();
    for(unsigned int i=0; i<config()->pluginMenuEntry.count(); i++) {
        pluginPopup->insertItem( config()->pluginMenuEntry[i], this, SLOT(forSelectionPlugin(int)), 0, 2000+i);
    }
}


/**
 * returns a specific icon for popup menu
 * (according to configured action for doubleclick, etc.)
 * TODO: fix for xmlui
 */
/*
QIconSet YammiGui::getPopupIcon(Song::action whichAction) {
    if(config()->doubleClickAction==whichAction) {
        return QIconSet(QPixmap(defaultDoubleClick_xpm));
    }
    if((config()->middleClickAction==whichAction)) {
        return QIconSet(QPixmap(defaultMiddleClick_xpm));
    }
    return (QIconSet) NULL;
}
*/

/// adds the text in search field to the wishlist
void YammiGui::addToWishList() {
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
void YammiGui::toCategory(int index) {
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
    bool remove
        =(mode==2);


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
        if(!remove
          ) {
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


void YammiGui::decide(Song* s1, Song* s2) {
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
    if(what == KMessageBox::Yes) {
        kdDebug()<< "deleting s1\n";
        s1->deleteFile(config()->trashDir);
        folderAll->removeSong(s1);
    }
    if(what == KMessageBox::No) {
        kdDebug()<< "deleting s2\n";
        s2->deleteFile(config()->trashDir);
        folderAll->removeSong(s2);
    }
    model->allSongsChanged(true);
}





/// searches for similar entries like the current song
void YammiGui::searchSimilar(int what) {
    QString searchFor;
    //	getCurrentSong();
    Song* refSong=selectedSongs.firstSong();
    switch(what) {
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
    m_searchField->setText(searchFor);
}

/**
 * Go to a specific folder (album/artist/genre) of selected song.
 */
void YammiGui::goToFolder(int what) {
    getSelectedSongs();
    Song* s=selectedSongs.firstSong();
    QString folderName;

    switch(what) {
    case 1:
        folderName=s->artist;
        if(folderName == "") {
            folderName = "- no artist -";
        }
        break;

    case 2:
        folderName=s->artist+" - "+s->album;
        break;

    case 3:
        folderName=s->genre;
        if(folderName == "") {
            folderName = "- no genre -";
        }
        break;

    case 4:
        if(s->year == 0) {
            folderName = "- no year -";
        } else {
            folderName=QString("%1").arg(s->year);
        }
        break;

    default:
        folderName="";
    }

    Folder* folder=getFolderByName(folderName);
    if(folder == 0 && what == 2) {
        // compilations with different artists are named just like the album
        folderName = s->album;
        folder=getFolderByName(folderName);
    }
    if(folder==0) {
        kdDebug() << "goto artist/album/genre/year: folder '" << folderName << "' not existing\n";
    } else {
        changeToFolder(folder);
    }
}

/**
 * search field changed => update search results
 *
 */
void YammiGui::searchFieldChanged( const QString &fuzzy ) {
    searchResultsUpdateNeeded=false;
    searchThread->setSearchTerm(fuzzy);
    searchFieldChangedIndicator.wakeOne();
    m_acceptSearchResults=true;
}

/**
 * Update the search results and request GUI update.
 */
void YammiGui::requestSearchResultsUpdate(MyList* results) {
    searchResults.clear();
    searchResults.appendList(results);
    searchResultsUpdateNeeded=true;
}

void YammiGui::updateSearchResults() {
    if(!searchResultsUpdateNeeded) {
        return;
    }
    folderSearchResults->updateTitle();
    folderContentChanged(folderSearchResults);
    if(!m_acceptSearchResults) {
        return;
    }

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
        item->setSelected(true);
        songListView->setCurrentItem(item);
    }
    searchResultsUpdateNeeded=false;
}

/**
 * user clicked on a folder
 */
void YammiGui::slotFolderChanged() {
    QListViewItem *i = folderListView->currentItem();
    if ( !i )
        return;
    changeToFolder((Folder*)i);
}

/**
 * Change to the specified folder and do necessary view updates.
 */
void YammiGui::changeToFolder(Folder* newFolder, bool changeAnyway) {
    if(chosenFolder) {
        if(newFolder == chosenFolder && (!changeAnyway)) {
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
    } else {
        songListView->dontTouchFirst=false;
    }

    updateListViewColumns();

    folderListView->setCurrentItem( (QListViewItem*)chosenFolder );
    folderListView->setSelected( (QListViewItem*)chosenFolder , TRUE );
    folderListView->ensureItemVisible((QListViewItem*)chosenFolder);
    folderContentChanged();
}


void YammiGui::folderContentChanged() {
    songListView->clear();
    if(chosenFolder) {
        addFolderContent(chosenFolder);
        if(chosenFolder == folderActual) {
            updateCurrentSongStatus();
            updateHtmlPlaylist();
        }
    }
}

void YammiGui::folderContentChanged(Folder* folder) {
    if(folder==chosenFolder) {
        folderContentChanged();
    } else {
        songListView->triggerUpdate();
        if(folder==folderActual) {
            updateCurrentSongStatus();
            updateHtmlPlaylist();
        }
        if(folder!=folderSearchResults) {
            m_acceptSearchResults=false;
        }
    }
}


/**
 * recursively add the content of folder and all subfolders
 * for now: folder contains songs OR subfolders, but not both!
 */
void YammiGui::addFolderContent(Folder* folder) {
    folderToAdd=folder;
    alreadyAdded=0;

    if(folder->songlist().count()!=0) {
        songListView->setSorting(-1);
        songListView->setUpdatesEnabled(false);
        addFolderContentSnappy();
    } else {		// no songList in that folder
        QApplication::restoreOverrideCursor();
    }
}

void YammiGui::addFolderContentSnappy() {
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
    } else {		// no, we are finished
        QApplication::restoreOverrideCursor();
        songListView->setUpdatesEnabled(true);

        int saved=chosenFolder->getSavedSorting();
        if(saved!=0 && chosenFolder != folderRecentAdditions) {
            bool asc=true;
            int column;
            if(saved<0) {
                column=-saved-1;
                asc=false;
            } else {
                column=saved-1;
            }
            songListView->sortedBy=saved;
            songListView->setSorting(column, asc);
        } else {
            // special default sorting for certain folders
            if(((QListViewItem*)chosenFolder)->parent()==folderAlbums) {
                songListView->sortedBy=COLUMN_TRACKNR + 1;
                songListView->setSorting(COLUMN_TRACKNR, true);
            } else if(chosenFolder==folderRecentAdditions) {
                songListView->sortedBy=COLUMN_ADDED_TO + 1;
                songListView->setSorting(COLUMN_ADDED_TO, true);
            } else {
                // default sort order: first column
                songListView->sortedBy=1;
                songListView->setSorting(0);
            }
        }
        int x=chosenFolder->getScrollPosX();
        int y=chosenFolder->getScrollPosY();
        songListView->setContentsPos(x, y);
        folderToAdd = 0;
    }
}


/// user clicked on a song
void YammiGui::slotSongChanged() {}


/// rmb on songlist: song popup for selection
void YammiGui::songListPopup( QListViewItem*, const QPoint & point, int) {
    // get selection
    getSelectedSongs();
    doSongPopup(point);
}

void YammiGui::doSongPopup(QPoint point) {
    int selected=selectedSongs.count();
    if( selected<=0 ) {
        return;
    }
    adjustSongPopup();
    songPopup->popup( point );
}

/// adjust SongPopup corresponding to <selectedSongs>
void YammiGui::adjustSongPopup() {
    int selected=selectedSongs.count();
    QString label;
    Song* first=selectedSongs.firstSong();
    if (selected>1) { 							// more than one song selected
        label=QString(i18n("%1 songs selected")).arg(selected);
    } else {
        label=first->displayName();
    }
    songPopup->changeItem ( 113, label);


    int id = songGoToPopup->idAt(0);
    QString folderName = first->artist;
    if(folderName == "") {
        folderName = "- no artist -";
    }
    songGoToPopup->changeItem(id, i18n("Artist: ") + folderName);
    songGoToPopup->setItemEnabled(id, getFolderByName(folderName)!=0);

    id = songGoToPopup->idAt(1);
    folderName = first->artist + " - " + first->album;
    Folder* f = getFolderByName(folderName);
    if(f == 0) {
        folderName = first->album;
        if(folderName == "") {
            folderName = "- no album -";
        }
        f = getFolderByName(folderName);
    }
    songGoToPopup->changeItem(id, i18n("Album: ") + folderName);
    songGoToPopup->setItemEnabled(id, f!=0);

    id = songGoToPopup->idAt(2);
    folderName = first->genre;
    if(folderName == "") {
        folderName = "- no genre -";
    }
    songGoToPopup->changeItem(id, i18n("Genre: ") + folderName);
    songGoToPopup->setItemEnabled(id, getFolderByName(folderName)!=0);

    id = songGoToPopup->idAt(3);
    if(first->year == 0) {
        folderName = "- no year -";
    } else {
        folderName = QString("%1").arg(first->year);
    }
    songGoToPopup->changeItem(id, i18n("Year: ") + folderName);
    songGoToPopup->setItemEnabled(id, getFolderByName(folderName)!=0);


    // for each category: determine whether all, some or no songs of selection are contained
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
void YammiGui::slotFolderPopup( QListViewItem*, const QPoint & point, int ) {
    QListViewItem *i = folderListView->currentItem();
    Folder* chosenFolder = ( Folder* )i;
    setSelectionMode(SELECTION_MODE_FOLDER);
    getSelectedSongs();
    if(selectedSongs.count()==0) {
        // no songs in this folder
        chosenFolder->popup( point, 0);
        setSelectionMode(SELECTION_MODE_USER_SELECTED);
        return;
    }
    adjustSongPopup();
    chosenFolder->popup( point, songPopup);
    setSelectionMode(SELECTION_MODE_USER_SELECTED);
}



// executes a plugin on a selection of songs
void YammiGui::forSelectionPlugin(int pluginIndex) {
    pluginIndex-=2000;

    bool confirm=config()->pluginConfirm[pluginIndex]=="true";
    QString mode=config()->pluginMode[pluginIndex];
    QString cmd=config()->pluginCommand[pluginIndex];

    if(cmd.contains("{directoryDialog}")>0) {
        QString dir = KFileDialog::getExistingDirectory("", this, i18n("choose directory for plugin"));
        if(dir.isNull())
            return;
        cmd.replace(QRegExp("\\{directoryDialog\\}"), dir);
    }
    if(cmd.contains("{fileDialog}")>0) {
        QString file = KFileDialog::getSaveFileName("",QString::null, this, i18n("choose file for plugin"));
        if(file.isNull())
            return;
        cmd.replace(QRegExp("\\{fileDialog\\}"), file);
    }

    while(cmd.contains("{inputString")>0) {
        int startPos = cmd.find("{inputString");
        if ( startPos == -1 ) {
            break;
        }
        int endPos = cmd.find("}", startPos);
        if ( endPos == -1 ) {
            break;
        }
        int midPos = startPos + 13;
        int argumentLength  = endPos - midPos;
        QString prompt = QString(i18n("Type in plugin parameter"));
        if ( argumentLength > 0 ) {
            prompt += QString(" \"%1\"").arg(cmd.mid(midPos, argumentLength));
        }
        bool ok;
        QString inputString=QString(QInputDialog::getText(prompt, prompt, QLineEdit::Normal, QString(""), &ok, this ));
        if(!ok) {
            return;
        }
        cmd.replace(startPos, endPos - startPos + 1, inputString);
    }

    if(mode=="single") {
        if(confirm) {
            QString sampleCmd=selectedSongs.firstSong()->replacePlaceholders(cmd, 1);
            QString msg=i18n("Execute the following command on each selected song?\n");
            msg+=i18n("(here shown: values for first song)\n\n");
            for(unsigned int i=0; i<sampleCmd.length(); i+=80) {
                msg+=sampleCmd.mid(i, 80)+"\n";
            }
            if( KMessageBox::warningYesNo( this,msg ) != KMessageBox::Yes) {
                return;
            }
        }
        int index=1;
        KProgressDialog progress( this,0, i18n("Yammi"),i18n("Executing song plugin cmd..."),true);
        progress.progressBar()->setTotalSteps(selectedSongs.count());
        for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong(), index++) {
            QString cmd2=s->replacePlaceholders(cmd, index);
            progress.progressBar()->setProgress(index);
            kapp->processEvents();
            if(progress.wasCancelled()) {
                return;
            }
            system(cmd2);
        }
    }

    if(mode=="group") {
        int index=1;
        QString customList="";
        for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong(), index++) {
            QString entry = config()->pluginCustomList[pluginIndex];
            customList+=s->replacePlaceholders(entry, index);
        }

        // custom list can be long => we put it into a file...
        QString customListFilename(config()->databaseDir+"customlist.temp");
        QFile customListFile(customListFilename);
        customListFile.open(IO_WriteOnly);
        customListFile.writeBlock( customList, qstrlen(customList) );
        customListFile.close();
        cmd.replace(QRegExp("\\{customList\\}"), customList);
        cmd.replace(QRegExp("\\{customListFile\\}"), customListFilename);
        cmd.replace(QRegExp("\\{customListViaFile\\}"), "`cat " + customListFilename + "`");

        if(confirm) {
            QString msg=i18n("Execute the following command:\n");
            for(unsigned int i=0; i<cmd.length(); i+=80) {
                msg+=cmd.mid(i, 80)+"\n";
                if(i>1200) {
                    msg+=i18n("\n...\n(command truncated)");
                    break;
                }
            }
            if( KMessageBox::warningYesNo( this, msg) != KMessageBox::Yes) {
                return;
            }
        }
        system(cmd);
    }
}


/**
 * move the files of the selected songs to a different location
 */
void YammiGui::forSelectionMove() {
    getSelectedSongs();
    int count = selectedSongs.count();
    if(count < 1) {
        return;
    }
    // let user choose destination directory
    QString startPath=selectedSongs.firstSong()->path;
    QString dir=KFileDialog::getExistingDirectory(startPath, this, i18n("Select destination directory"));
    if(dir.isNull()) {
        return;
    }
    if(dir.right(1)=="/") { 						// strip trailing slash
        dir=dir.left(dir.length()-1);
    }
    for(Song* s = selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
        s->moveTo(dir);
    }
}



/**
 * prepare burning selection to media
 * (burning order will be the order of the selected songs)
 */
void YammiGui::forSelectionBurnToMedia() {
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
    QString mediaDir = config()->databaseDir + "media/" + mediaName + "/";
    long double sizeLimit=(long double)config()->criticalSize*1024.0*1024.0;
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
            mediaDir= config()->databaseDir + "media/"+ mediaName + "/";
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
            } else {
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

    cout << "no of media: " << mediaNo+1-startIndex << " (size limit: " << config()->criticalSize << " MB, ";
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
                .arg(mediaNo+1-startIndex).arg(config()->criticalSize).arg(startIndex).arg(mediaNo)
                .arg(count).arg((int)(size/1024.0/1024.0)).arg((int)(totalSize/1024.0/1024.0))
                .arg(config()->databaseDir + "media/");
    KMessageBox::information( this, msg );
}



/** calculate disk usage for a directory (including all subdirectories)
 * returns -1 if too full
 */
long double YammiGui::diskUsage(QString path, long double sizeLimit) {
    QDir d(path);

    d.setFilter(QDir::Files);
    const QFileInfoList* list = d.entryInfoList();
    if (!list) {
        kdError() << "Error: cannot read access swap directory: " << path << "\n";
        return 0;
    }
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
    if (!list2) {
        kdWarning() << "Error: Skipping unreadable directory under swap directory: " << path << "\n";
        return 0;
    }
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
void YammiGui::getSelectedSongs() {
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
        // if the folder is already completely added to GUI, we take the order as shown in the songlist
        if(folderToAdd == 0) {
            for(QListViewItem* i=songListView->firstChild(); i; i=i->itemBelow()) {
                selectedSongs.appendSong(((SongListItem*) i)->song());
            }
        } else {
            // here we are in the middle of adding all songs... (lazy adding) => take folder content directly
            kdDebug() << "taking songs directly from list...\n";
            for(Song* s=chosenFolder->firstSong(); s; s=chosenFolder->nextSong()) {
                selectedSongs.appendSong(s);
            }
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
void YammiGui::forAllCheckConsistency() {
    setSelectionMode(SELECTION_MODE_ALL);
    forSelectionCheckConsistency();
    setSelectionMode(SELECTION_MODE_USER_SELECTED);
}


void YammiGui::forSelectionCheckConsistency() {
    getSelectedSongs();
    ConsistencyCheckDialog d(this, &(config()->consistencyPara), &selectedSongs, model);
    d.exec();
    config()->saveConfig();
    folderProblematic->update(model->problematicSongs);
    folderContentChanged(folderProblematic);
    folderContentChanged(chosenFolder);
}


void YammiGui::forSelectionEnqueue( ) {
    getSelectedSongs();
    int count = selectedSongs.count();
    if(count < 1) {
        return;
    }
    if(shiftPressed) {
        selectedSongs.shuffle();
    }
    for(Song* s = selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
        model->songsToPlay.append(new SongEntryInt(s, 13));
    }
    folderActual->correctOrder();
    player->syncYammi2Player();
    folderContentChanged(folderActual);
    statusBar( )->message(QString(i18n("%1 Songs equeued at end of playlist")).arg(count), 2000);
    checkPlaylistAvailability();
}

void YammiGui::forSelectionEnqueueAsNext( ) {
    getSelectedSongs();
    int count = selectedSongs.count();
    if(count < 1) {
        return;
    }
    if(shiftPressed) {
        selectedSongs.shuffle();
    }
    // reverse the order, to get intended play order
    selectedSongs.reverse();
    for(Song* s = selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
        if(model->songsToPlay.count()==0 || currentSong!=model->songsToPlay.at(0)->song() || player->getStatus() != PLAYING) {
            model->songsToPlay.insert(0, new SongEntryInt(s, 13));
        } else {
            model->songsToPlay.insert(1, new SongEntryInt(s, 13));
        }
    }

    folderActual->correctOrder();
    player->syncYammi2Player();
    folderContentChanged(folderActual);
    checkPlaylistAvailability();
}


void YammiGui::forSelectionPrelisten(int where ) {
    getSelectedSongs();
    int count = selectedSongs.count();
    if(count < 1) {
        return;
    }
    for(Song* s = selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
        preListen(s, where);
        break;
    }
}


void YammiGui::forSelectionPlayNow() { //FIXME - this does not work too well....
    getSelectedSongs();
    if(selectedSongs.count() < 1) {
        return;
    }
    int stateBefore = player->getStatus();
    forSelectionEnqueueAsNext();
    //this is not really clean, but...
    if(stateBefore == PLAYING) {
        player->skipForward(shiftPressed);
    }
    player->play();
}

void YammiGui::forSelectionDequeue( ) {
    getSelectedSongs();
    int sortedByBefore=songListView->sortedBy;
    if(chosenFolder==folderActual) {
        // song chosen from playlist => dequeue only the selected song entry (not all songs with that id)
        for(QListViewItem* i=songListView->lastItem(); i; i=i->itemAbove()) {
            if(!i->isSelected()) {
                continue;
            }
            SongEntry* entry=((SongListItem*) i)->songEntry;
            int pos=((SongEntryInt*)entry)->intInfo-1;
            if(pos!=0 || player->getStatus()==STOPPED) {
                // only dequeue if not currently played song (or player stopped)
                kdDebug() << "song dequeued: " << entry->song()->displayName() << endl;
                model->songsToPlay.remove(pos);
            }
        }
    } else {
        // song chosen from other folder => dequeue ALL occurrences of each selected song
        for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
            int i=1;
            if(player->getStatus()==STOPPED) {
                i=0;
            }
            for(; i<(int)model->songsToPlay.count(); i++) {
                Song* check = model->songsToPlay.at(i)->song();
                if(check == s) {
                    model->songsToPlay.remove(i);
                    kdDebug() << "song dequeued: " << s->displayName() << endl;
                    i--;
                }
            }
        }
    }
    folderActual->correctOrder();
    player->syncYammi2Player();
    folderContentChanged(folderActual);
    bool ascending = (sortedByBefore>0);
    if(!ascending) {
        sortedByBefore = -sortedByBefore;
    }
    int column=sortedByBefore-1;
    songListView->setSorting(column, ascending);
    checkPlaylistAvailability();
}



/**
 * Mass viewing/editing of song info.
 * (I know, this needs some cleanup...)
 */
void YammiGui::forSelectionSongInfo( ) {
    getSelectedSongs();
    int count = selectedSongs.count();
    if(count < 1) {
        return;
    }

    QString _artist, _title, _album, _comment, _genre, _path, _filename, _year, _trackNr, _bitrate, _proposedFilename, _proposedPath;
    MyDateTime _addedTo, _lastTimePlayed;
    int _length=0;
    long double _size=0;

    Song* singleSong = 0;
    if(count == 1) {
        singleSong = selectedSongs.firstSong();
    }
    SongInfo si(this, &selectedSongs);

    // fill combobox with genres, but sort them first
    QStringList genreList;
    genreList.append("");
    for(int genreNr=0; !(TagLib::ID3v1::genre(genreNr).isNull()); genreNr++) {
        genreList.append(TStringToQString(TagLib::ID3v1::genre(genreNr)));
    }
    genreList.sort();
    for ( QStringList::Iterator it = genreList.begin(); it != genreList.end(); ++it ) {
        si.ComboBoxGenre->insertItem(*it);
    }

    int selected=0;
    QDateTime invalid;
    for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
        selected++;
        if(selected==10) {			// set wait cursor (summing size of 2000 files may take a while...)
            QApplication::setOverrideCursor( Qt::waitCursor );
        }

        // get filesize
        QFile file(s->location());
        if(file.exists()) {
            _size+=file.size();
        }
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
            _bitrate=QString(i18n("%1 kb/s")).arg(s->bitrate);
            _genre=s->genre;
            _proposedFilename=s->constructFilename();
            _proposedPath=s->constructPath();
            _lastTimePlayed=s->lastPlayed;
        } else {
            if(_addedTo!=s->addedTo)
                _addedTo=invalid;
            if(_album!=s->album)
                _album="!";
            if(_artist!=s->artist)
                _artist="!";
            if(_comment!=s->comment)
                _comment="!";
            if(_title!=s->title)
                _title="!";
            if(_trackNr!=QString("%1").arg(s->trackNr))
                _trackNr="!";
            if(_year!=QString("%1").arg(s->year))
                _year="!";
            if(_path!=s->path)
                _path="!";
            if(_filename!=s->filename)
                _filename="!";
            if(_bitrate!=QString("%1").arg(s->bitrate))
                _bitrate="!";
            if(_genre!=s->genre)
                _genre="!";
            if(_proposedFilename!=s->constructFilename())
                _proposedFilename="!";
            if(_proposedPath!=s->constructPath())
                _proposedPath="!";
            if(_lastTimePlayed!=s->lastPlayed)
                _lastTimePlayed=invalid; //.setDate(QDate::fromString(""));
        }
    }

    if(selected>=10) {
        QApplication::restoreOverrideCursor();
    }

    // now edit the (common) info
    si.LineEditArtist->setText(_artist);
    si.LineEditTitle->setText(_title);
    si.LineEditAlbum->setText(_album);
    si.LineEditComment->setText(_comment);
    if(_year!="0")
        si.LineEditYear->setText(_year);
    if(_trackNr!="0")
        si.LineEditTrack->setText(_trackNr);
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
    } else {
        si.LineEditLastPlayed->setText("!");
    }

    si.ReadOnlyPath->setText(_path);
    si.ReadOnlyFilename->setText(_filename);
    si.ReadOnlyProposedFilename->setText(_proposedFilename);
    si.ReadOnlyProposedPath->setText(_proposedPath);
    if(selected>1) {
        si.LabelHeading->setText(QString(i18n("Mass editing: %1 songs")).arg(selected));
        si.LabelSize->setText(i18n("Size (total)"));
        si.LabelLength->setText(i18n("Length (total)"));
        QString x;
        si.ReadOnlyLength->setText(x.sprintf(i18n("%d:%02d:%02d (hh:mm:ss)"), _length/(60*60), (_length % (60*60))/60, _length % 60));
    } else {
        si.LabelHeading->setText(_artist+" - "+_title);
        QString x;
        si.ReadOnlyLength->setText(x.sprintf(i18n("%2d:%02d (mm:ss)"), _length/60, _length % 60));
    }
    if(_size < (1024 * 1024)) {
        si.ReadOnlySize->setText( QString(i18n("%1 KB")).arg( (float)_size/(float)(1024) , 4,'f', 2));
    }
    else {
        si.ReadOnlySize->setText( QString(i18n("%1 MB")).arg( (float)_size/(float)(1024*1024) , 4,'f', 2));
    }
    si.ReadOnlyBitrate->setText(_bitrate);
    si.ComboBoxGenre->setCurrentText(_genre);
    si.CheckBoxCorrectFilename->setChecked(config()->filenamesConsistent);
    si.CheckBoxCorrectPath->setChecked(config()->directoriesConsistent);
    
    // show dialog
    si.activateUpdates();
    int result=si.exec();
    if(result!=QDialog::Accepted) {
        return;
    }
    
    if(selected > 20) {
        QString msg("");
        msg += QString(i18n("Your changes will affect %1 song entries.\n")).arg(selected);
        if(si.CheckBoxCorrectFilename->isChecked()) {
            msg += i18n("\nNote: Your changes may change the filenames of the selected files!\n");
        }
        if(si.CheckBoxCorrectPath->isChecked()) {
            msg += i18n("\nNote: Your changes may affect the location of the selected files!\n");
        }
        msg += i18n("\n\nDo you want to continue?");
        if( KMessageBox::warningYesNo( this, msg) != KMessageBox::Yes ) {
            return;
        }
    }

    // now set the edited info for all selected songs
    for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong()) {
        bool change=false;
        if(si.LineEditArtist->text()!="!" && si.LineEditArtist->text()!=s->artist)	{
            s->artist=si.LineEditArtist->text();
            change=true;
        }
        if(si.LineEditTitle->text()!="!" && si.LineEditTitle->text()!=s->title)			{
            s->title=si.LineEditTitle->text();
            change=true;
        }
        if(si.LineEditAlbum->text()!="!" && si.LineEditAlbum->text()!=s->album) 		{
            s->album=si.LineEditAlbum->text();
            change=true;
        }
        if(change) { 		// for artist and title: mark categories as dirty on change!
            model->markPlaylists(s);
        }
        if(si.LineEditComment->text()!="!" && si.LineEditComment->text()!=s->comment)	{
            s->comment=si.LineEditComment->text();
            change=true;
        }
        if(si.LineEditYear->text()!="!") {
            int tryYear=atoi(si.LineEditYear->text());
            if(tryYear!=s->year) {
                s->year=tryYear;
                change=true;
            }
        }
        if(si.LineEditTrack->text()!="!") {
            int tryTrackNr=atoi(si.LineEditTrack->text());
            if(tryTrackNr!=s->trackNr) {
                s->trackNr=tryTrackNr;
                change=true;
            }
        }

        if(si.ComboBoxGenre->currentText() != "!" && si.ComboBoxGenre->currentText() != s->genre) {
            s->genre=si.ComboBoxGenre->currentText();
            change=true;
        }

        if(change) {
            // changes in tags
            model->allSongsChanged(true);            
            if(config()->tagsConsistent) {
                s->saveTags();
            }
            // update affected songs in view
            for(SongListItem* i=(SongListItem*)songListView->firstChild(); i; i=(SongListItem*)i->itemBelow()) {
                if(i->song()!=s) {
                    continue;
                }
                i->setColumns(i->songEntry);
            }
        }
        
        if(si.CheckBoxCorrectFilename->isChecked() && s->checkFilename(config()->consistencyPara.ignoreCaseInFilenames)==false) {
            s->correctFilename();
            model->allSongsChanged(true);
        }
        if(si.CheckBoxCorrectPath->isChecked() && s->checkDirectory(config()->consistencyPara.ignoreCaseInFilenames)==false) {
            QString pathBefore = s->path;
            s->correctPath();
            Util::deleteDirectoryIfEmpty(pathBefore, config()->scanDir);
            model->allSongsChanged(true);
        }        
    }
}



void YammiGui::forSelectionDelete( ) {
    getSelectedSongs();
    int count = selectedSongs.count();
    if(count < 1) {
        return;
    }

    // determine delete mode
    DeleteDialog dd( this,  "deleteDialog", true);
    if(selectedSongs.count()==1) {
        dd.LabelSongname->setText(selectedSongs.firstSong()->displayName());
    } else {
        dd.LabelSongname->setText(QString(i18n("Delete %1 songs")).arg(selectedSongs.count()));
    }
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
            s->deleteFile(config()->trashDir);
        }
        if(deleteEntryFlag)	{
            deleteEntry(s);
        }
    }

    // update view
    model->allSongsChanged(true);
    if(deleteEntryFlag) {
        model->categoriesChanged(true);
        folderContentChanged(folderAll);
        if(chosenFolder != folderAll) {
            updateView();
        }
    }
}


/**
 * Delete a song entry.
 * TODO: delete from *all* folders
 */
void YammiGui::deleteEntry(Song* s) {
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
void YammiGui::doubleClick() {
    int action=config()->doubleClickAction;
    forSelection(action);
}


/// middleClick on song
void YammiGui::middleClick(int button) {
    //	if(button==1) {			// left button
    //    }

    if(button==4) {				// middle button
        forSelection(config()->middleClickAction);
    }
}


/**
 * Perform an action on a song, defined by a Song::action int value
 */
void YammiGui::forSelection(int action) {
    switch(action) {
    case Song::None:
        break;
    case Song::Enqueue:
        forSelectionEnqueue();
        break;
    case Song::EnqueueAsNext:
        forSelectionEnqueueAsNext();
        break;
    case Song::PlayNow:
        forSelectionPlayNow();
        break;
    case Song::SongInfo:
        forSelectionSongInfo();
        break;
    case Song::PrelistenStart:
        forSelectionPrelistenStart();
        break;
    case Song::PrelistenMiddle:
        forSelectionPrelistenMiddle();
        break;
    case Song::PrelistenEnd:
        forSelectionPrelistenEnd();
        break;
    case Song::Delete:
        forSelectionDelete();
        break;
    case Song::DeleteFile:
        break;
    case Song::DeleteEntry:
        break;
    case Song::CheckConsistency:
        forSelectionCheckConsistency();
        break;
    case Song::MoveTo:
        forSelectionMove();
        break;
    case Song::Dequeue:
        forSelectionDequeue();
        break;
    case Song::BurnToMedia:
        forSelectionBurnToMedia();
        break;
    default:
        kdWarning() << "unknown action for double click: " << action << endl;
    }
}


/**
 * Creates a new category, querying the user for the name.
 */
bool YammiGui::newCategory() {
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
void YammiGui::removeCategory() {
    QListViewItem* i = folderListView->currentItem();
    QString name=((Folder*)i)->folderName();
    QString msg(i18n("Delete category %1 ?\n (will be deleted immediately!)"));
    if( KMessageBox::warningYesNo( this, msg) == KMessageBox::Yes ) {
        model->removeCategory(name);
        folderCategories->update(model->allCategories, model->categoryNames);
        updateSongPopup();
    }
}

/**
 * Renames the selected category, querying the user for the new name.
 */
void YammiGui::renameCategory() {
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
void YammiGui::loadM3uIntoCategory() {
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
        } else {
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
void YammiGui::autoplayFolder() {
    Folder* f = (FolderSorted*)folderListView->currentItem();
    autoplayFoldername=f->folderName();
    m_currentAutoPlay->setText(i18n("Folder: ")+autoplayFoldername);
}


void YammiGui::autoplayOff() {
    autoplayMode = AUTOPLAY_OFF;
}
void YammiGui::autoplayLNP() {
    autoplayMode = AUTOPLAY_LNP;
}
void YammiGui::autoplayRandom() {
    autoplayMode = AUTOPLAY_RANDOM;
}

/// remove media
// uahhaa... ugly! make mediaName + mediaLocation a struct/class, oli!
void YammiGui::removeMedia() {
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


void YammiGui::renameMedia() {
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
void YammiGui::pluginOnFolder() {
    QFile f(config()->databaseDir + "plugin.temp" );
    if ( !f.open( IO_WriteOnly  ) ) {
        return;
    }
    QTextStream str(&f);

    for(Song* s=chosenFolder->firstSong(); s; s=chosenFolder->nextSong()) {
        cout << s->path << "/" << s->filename << "\n";
        str << s->path <<"/" << s->filename << "\n";
    }
    f.close();
}



/// clear all playlist items except currently played song
void YammiGui::shufflePlaylist() {
    if(model->songsToPlay.count() < 2) {
        // shuffling less than 2 songs does not make too much sense...
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
    player->syncYammi2Player();
}


/**
 * clear all playlist items (except currently played song
 */
void YammiGui::clearPlaylist() {
    if(config()->childSafe)
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
    player->syncYammi2Player();
    folderContentChanged(folderActual);
}






/**
 * onTimer is called periodically to do some things independently of any user action
 * - logging
 * - updating folderActual
 */
void YammiGui::onTimer() {
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
    if(player->getStatus()!=STOPPED) {
        // adjust songSlider (if user is not currently dragging it around)
        int current = player->getCurrentTime();
        // 		if(!isSongSliderGrabbed && player->getStatus() != PAUSED) {
        m_seekSlider->setValue(current);
        //     }
    }
}


void YammiGui::autoFillPlaylist() {
    Folder* toAddFrom=getFolderByName(autoplayFoldername);
    if(toAddFrom!=0 && toAddFrom->songlist().count()>0) {
        // fill up from chosen autoplay folder
        int total=toAddFrom->songlist().count();
        Song* songToAdd=0;

        if(autoplayMode==AUTOPLAY_RANDOM) {
            // method 1: randomly pick a song, no further intelligence
            songToAdd=toAddFrom->songlist().at(randomNum(total))->song();
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
                } else if(check->lastPlayed!=lnpTimestamp) {
                    break;
                }
                candidateList.appendSong(check);
            }

            int candidates=candidateList.count();
            if(candidates>0) {
                if(candidates==1) {
                    songToAdd=candidateList.firstSong();
                } else {
                    songToAdd=candidateList.at(randomNum(candidates))->song();
                }
            }
            toAddFrom->songlist().setSortOrderAndSort(rememberSortOrder, true);
        }

        if(songToAdd!=0) {
            folderActual->addSong(songToAdd);
            player->syncYammi2Player();
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
void YammiGui::grabAndEncode() {
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

    QString filename=QString(i18n("%1%2 - %3.mp3")).arg(config()->scanDir).arg(artist).arg(title);
    QFileInfo fi(filename);
    if(fi.exists()) {
        QString msg = i18n("The file\n%1\nalready exists!\n\nPlease choose a different artist/title combination.");
        KMessageBox::information( this, msg.arg(filename));
        return;
    }
    // linux specific
    QString cmd=QString("%1 %2 \"%3\" \"%4\" \"%5\" &").arg(config()->grabAndEncodeCmd).arg(trackNr).arg(artist).arg(title).arg(filename);
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
void YammiGui::checkForGrabbedTrack() {
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
    QString msg=i18n("Yammi tried to add the grabbed song to the database.\n\nSome statistics: \n\n");
    msg+=QString(i18n("%1 songs added to database\n")).arg(model->entriesAdded);
    msg+=QString(i18n("%1 songs corrupt (=not added)\n")).arg(model->corruptSongs);
    msg+=QString(i18n("%1 problematic issues(check in folder Problematic Songs)")).arg(model->problematicSongs.count());
    KMessageBox::information( this,msg);
}


/**
 * Fix all genres by re-reading them from the files.
 */
void YammiGui::fixGenres() {
    if( KMessageBox::warningYesNo(this, "Do you want to fix the genre of all songs (potentially broken or incomplete from earlier versions of yammi) by re-reading all genres from available files now?" ) != KMessageBox::Yes ) {
        return;
    }
    isScanning=true;
    KProgressDialog progress( this, 0, i18n("Yammi"), i18n ("Re-Reading all genres from your files..."), true);
    progress.setMinimumDuration(0);
    progress.setAllowCancel(true);
    progress.progressBar()->setTotalSteps(model->allSongs.count());
    progress.progressBar()->setProgress(0);
    kapp->processEvents();
    model->fixGenres(&progress);
    updateView();
    isScanning=false;
}

void YammiGui::keyPressEvent(QKeyEvent* e) {
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
        if(m_sleepMode) {
            int left = m_sleepModeSpinBox->value() + 1;
            m_sleepModeSpinBox->setValue(left);
        }
        else {
            songListView->simulateKeyPressEvent(e);
        }
        break;
    case Key_PageDown:
        if(m_sleepMode) {
            int left = m_sleepModeSpinBox->value() - 1;
            if(left > 0 )
                m_sleepModeSpinBox->setValue(left);
        }
        else {
            songListView->simulateKeyPressEvent(e);
        }
        break;

    case Key_Up:
        songListView->simulateKeyPressEvent(e);
        break;

    case Key_Down:
        songListView->simulateKeyPressEvent(e);
        break;
    
    case Key_F:		// Ctrl-F
        if(e->state()!=ControlButton) {
            break;
        }
        m_searchField->setText("");
        m_searchField->setFocus();
        break;
        
    case Key_Escape: // ESC
        m_searchField->setText("");
        m_searchField->setFocus();
        break;

    case Key_G:  // Ctrl-G
        if(e->state()!=ControlButton) {
            break;
        }
        gotoFuzzyFolder(false);
        break;


    default:
        e->ignore();
    }
}

void YammiGui::keyReleaseEvent(QKeyEvent* e) {
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


void YammiGui::toFromPlaylist() {
    if(chosenFolder!=folderActual) {
        // switch to playlist
        toFromRememberFolder = chosenFolder;
        changeToFolder(folderActual);
    } else {
        // switch back to last open folder
        if(toFromRememberFolder != 0) {
            changeToFolder(toFromRememberFolder);
        }
    }
}

void YammiGui::saveDatabase() {
    model->save();
    return;
}


void YammiGui::changeSleepMode() {
    kapp->beep();
    if(!m_sleepMode) {
        m_sleepMode = true;
        m_sleepModeSpinBox->setValue(3);
        if(model->allSongsChanged() || model->categoriesChanged()) {
            QString msg = i18n("The Database has been modified. Save changes?\n(answering no will cancel sleep mode)");
            if( KMessageBox::warningYesNo(this,msg) == KMessageBox::Yes ) {
                saveDatabase();
            } else {
                m_sleepMode = false; //leave sleep mode off
            }
        }
    } else {
        m_sleepMode = false;
    }
    m_sleepModeButton->setText(m_sleepMode? i18n("shutdown"):i18n("(disabled)"));
    m_sleepModeSpinBox->setEnabled(m_sleepMode);
}



/**
 * stops playback on headphone
 */
void YammiGui::stopPrelisten() {
    if(!prelistenProcess.isRunning()) {
        kdDebug() << "looks like no prelisten process running..." << endl;
        return;
    }
    bool result=prelistenProcess.kill(9);
    if(!result) {
        kdWarning() << "could not stop prelisten process!\n";
    }
}

/**
 * Replace skipXYZ placeholders with computed values
 */
QString YammiGui::replacePrelistenSkip(QString input, int lengthInSeconds, int skipTo)
{
    int skipSeconds = lengthInSeconds * skipTo / 100;
    int skipMilliSeconds = skipSeconds * 1000;
    int skipFrames = skipSeconds * 38;
//    int skipSamples = skipSeconds * 441;

    input.replace(QRegExp("\\{skipSeconds\\}"), QString("%1").arg(skipSeconds));
    input.replace(QRegExp("\\{skipMilliSeconds\\}"), QString("%1").arg(skipMilliSeconds));
    input.replace(QRegExp("\\{skipFrames\\}"), QString("%1").arg(skipFrames));
//    input.replace(QRegExp("\\{skipSamples\\}"), QString("%1").arg(skipSamples));
    return input;
}
    
/**
 * sends the song to headphones
 * skipTo: 0 = beginning of song, 100 = end
 */
void YammiGui::preListen(Song* s, int skipTo) {
    // first, kill any previous prelisten process
    stopPrelisten();
    
    // now play song via configured command line tools
    QString prelistenCmd;
    if(s->filename.right(3).upper()=="MP3") {
        prelistenCmd = config()->prelistenMp3Command;
    }
    else if(s->filename.right(3).upper()=="OGG") {
        prelistenCmd = config()->prelistenOggCommand;
    }
    else if(s->filename.right(3).upper()=="WAV") {
        prelistenCmd = config()->prelistenWavCommand;
    }
    else if(s->filename.right(4).upper()=="FLAC") {
        prelistenCmd = config()->prelistenFlacCommand;
    }
    else {
        prelistenCmd = config()->prelistenOtherCommand;
    }

    if(prelistenCmd == "") {
        kdDebug() << "no prelistening configured for this file type: " << s->filename << endl;
        return;
    }
    
    // prepare command
    prelistenCmd = s->replacePlaceholders(prelistenCmd, 0);
    prelistenCmd = replacePrelistenSkip(prelistenCmd, s->length, skipTo);        
    kdDebug() << "prelisten command: " << prelistenCmd << endl;
    
    prelistenProcess.clearArguments();
    prelistenProcess.setUseShell(true);
    QStringList argList = QStringList::split( QChar('|'), prelistenCmd );
    for ( QStringList::Iterator it = argList.begin(); it != argList.end(); ++it ) {
        prelistenProcess << *it;
    }
    
    if(prelistenProcess.isRunning()) {
        kdDebug() << "waiting for prelisten process to die..." << endl;
        prelistenProcess.detach();
    }
    
    bool result = prelistenProcess.start(KProcess::OwnGroup, KProcess::NoCommunication);
    if(!result) {
        kdWarning() << "could not start prelisten process!\n";
    }
}

void YammiGui::updateSongDatabaseHarddisk() {
    UpdateDatabaseDialog d(this, config());
    // show dialog
    int result=d.exec();
    if(result!=QDialog::Accepted) {
        return;
    }
    config()->saveConfig();
    updateSongDatabase(config()->scanDir, config()->scanPattern, 0);
}

void YammiGui::updateSongDatabaseSingleFile() {
    QStringList files = KFileDialog::getOpenFileNames( ":singleFile", QString::null, this, i18n("Open file(s) to import"));
    //    QStringList files = KFileDialog::getOpenFileNames( config()->scanDir, QString::null, this, i18n("Open file(s) to import"));
    if(files.count()==0) {
        return;
    }
    QStringList list = files;
    model->updateSongDatabase(list);
    updateView();
    folderProblematic->update(model->problematicSongs);
    folderAll->updateTitle();
    changeToFolder(folderRecentAdditions);
    QString msg=i18n("Updated your database.\n\nStatistics: \n\n");
    msg+=QString(i18n("%1 songs added to database\n")).arg(model->entriesAdded);
    msg+=QString(i18n("%1 songs corrupt (=not added)\n")).arg(model->corruptSongs);
    msg+=QString(i18n("%1 songs problematic (check in folder Problematic Songs)\n")).arg(model->problematicSongs.count());
    KMessageBox::information( this, msg );
}


void YammiGui::updateSongDatabaseMedia() {
    UpdateDatabaseMediaDialog d(this, config());
    // show dialog
    int result=d.exec();
    if(result!=QDialog::Accepted) {
        return;
    }
    config()->saveConfig();
    if(d.LineEditMediaDir->text()=="") {
        KMessageBox::information( this,i18n("You have to enter a name for the media!") );
        return;
    }
    QString mediaName = d.LineEditMediaName->text();
    updateSongDatabase(config()->mediaDir, config()->scanPattern, mediaName);
}



void YammiGui::updateSongDatabase(QString scanDir, QString filePattern, QString media) {
    if(config()->childSafe) {
        return;
    }
    KProgressDialog progress( this, 0, i18n("Yammi"), i18n ("Scanning..."), true);
    progress.setMinimumDuration(0);
    progress.setAutoReset(false);
    progress.setAutoClose(false);
    progress.setAllowCancel(true);
    progress.progressBar()->setProgress(0);
    kapp->processEvents();

    isScanning=true;
    model->updateSongDatabase(scanDir, config()->followSymLinks, filePattern, media, &progress);

    progress.close();
    updateView();
    folderProblematic->update(model->problematicSongs);
    folderAll->updateTitle();
    QString msg=i18n("Updated your database.\n\nStatistics: \n\n");
    msg+=QString(i18n("%1 songs added to database\n")).arg(model->entriesAdded);
    msg+=QString(i18n("%1 songs corrupt (=not added)\n")).arg(model->corruptSongs);
    msg+=QString(i18n("%1 songs problematic (check in folder Problematic Songs)\n")).arg(model->problematicSongs.count());
    KMessageBox::information( this,msg);
    changeToFolder(folderRecentAdditions);

    isScanning=false;
}


void YammiGui::stopDragging() {
    ((FolderSorted*)chosenFolder)->syncWithListView(songListView);
    folderContentChanged();

    if(chosenFolder==folderActual) {
        player->syncYammi2Player();
    }

    if(((QListViewItem*)chosenFolder)->parent()==folderCategories) {
        // we have to save the order
        model->categoriesChanged(true);
    }
}



/** selects all in songListView */
void YammiGui::selectAll() {
    for(QListViewItem* i=songListView->firstChild(); i; i=i->itemBelow()) {
        i->setSelected(true);
    }
    songListView->triggerUpdate();
}

/** inverts selection in songListView */
void YammiGui::invertSelection() {
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
void YammiGui::loadSongsFromMedia(QString mediaName) {
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

    QString mediaDir=config()->mediaDir;
    QString swapDir=config()->swapDir;
    if(config()->mountMediaDir) {
        // linux specific
        QString cmd;
        cmd=QString("mount %1").arg(config()->mediaDir);
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
                    progress.setLabel(i18n("loading song: ")+s->displayName()+" ("+QString("%1").arg(i+1)+i18n(". in playlist)"));
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
    if(config()->mountMediaDir) {
        // linux specific
        QString cmd;
        cmd=QString("umount %1").arg(config()->mediaDir);
        system(cmd);
    }

    player->syncYammi2Player();
    checkPlaylistAvailability();
    //	folderContentChanged(folderActual);
    songListView->triggerUpdate();
}

/**
 * Manages loading songfiles from removable media
 */
void YammiGui::checkPlaylistAvailability() {
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
    } else {
        loadFromMediaButton->setEnabled(true);
    }
}

/// loads the currently in the media list chosen media
void YammiGui::loadMedia() {
    QString mediaName=mediaListCombo->currentText();
    loadSongsFromMedia(mediaName);
}

/**
 * Checks whether the swapped songs take more space than the given limit.
 * If they do, we delete the least recently used song files, until we are below
 * the limit again.
 */
void YammiGui::checkSwapSize() {
    long double sizeLimit=(long double)config()->swapSize*1024.0*1024.0;
    long double size=0.0;
    QString path=config()->swapDir;
    cout << "checking swap size in directory " << path << ", limit: " << config()->swapSize << " MB\n";
    QDir d(path);

    d.setFilter(QDir::Files);
    d.setSorting(QDir::Time);			// most recent first
    const QFileInfoList *list = d.entryInfoList();
    if (!list) {
        kdError() << "Error: cannot access swap directory: " << path << "\n";
        return;
    }

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
        } else {
            size+=fi->size();
        }
    }
}


/**
 * Tells the media player to skip to the next song.
 */
void YammiGui::skipForward() {
    player->skipForward(shiftPressed);
}

/**
 * Performs a skip backward with a little trick:
 * Prepends the last played song to the playlist,
 * then tells the media player to skip backward.
 * Also removes this last played song from the songsPlayed folder,
 * as it will be inserted again after being played.
 */
void YammiGui::skipBackward() {
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


void YammiGui::toggleColumnVisibility(int column) {
    columnsMenu->setItemChecked(column, !columnsMenu->isItemChecked(column));
    columnVisible[column]=columnsMenu->isItemChecked(column);
    changeToFolder(chosenFolder, true);
}



void YammiGui::loadMediaPlayer( ) {
    player = 0;
    switch( config()->mediaPlayer ) {
        #ifdef ENABLE_XMMS
    case 0:
        player = new XmmsPlayer(0, model);
        break;
        #endif

    case 1:
        int retval;
        retval = system("which noatun");
        retval = WEXITSTATUS(retval);
        if (retval == 0) {
            player = new NoatunPlayer( model );
        } else {
            kdDebug() << "WARNING: looks like you want to use noatun, but noatun cannot be found\n";
        }
        break;
    case 2:
        player = new Yammi::ArtsPlayer( model );
        break;
    case 3:
        player = new Yammi::GstPlayer( model );
        break;
        //    default:
        //        player = new DummyPlayer( model );
    }
    if (!player) {
        player = new DummyPlayer( model );
        /*        KMessageBox::error(this, i18n("Can't create the player object.\n"
                                              "Please select a suitable backend player\n"
                                              "from the Preferences Dialog"), i18n("Error"));*/
        kdDebug() << "Can't create player backend, select a suitable backend in the Preferences Dialog\n";
    }
    kdDebug() << "Media Player : " << player->getName( ) << endl;
}

void YammiGui::playlistViewPopup(const QString &url, const QPoint &point) {
    kdDebug() << "url: " << url << endl;
    xxx
}

/**
 * Create main gui of Yammi
 */
void YammiGui::createMainWidget( ) {
    // separate into left and right area
    centralWidget=new QSplitter(Qt::Horizontal, this);
    
    // separate left area into upper (playlist) and lower part (quick browser)
    leftWidget=new QSplitter(Qt::Vertical, centralWidget);

    // setup html playlist view
    playlistPart = new KHTMLPart(leftWidget, 0, 0L, 0);
    connect( playlistPart, SIGNAL( popupMenu(const QString &url, const QPoint &point)), this, SLOT(playlistViewPopup(const QString &url, const QPoint &point)) );
    playlistPart->view()->setHScrollBarMode(QScrollView::AlwaysOn);
    
    centralWidget->setResizeMode( leftWidget, QSplitter::KeepSize );
    leftWidget->setResizeMode( playlistPart->view(), QSplitter::KeepSize );
    
    // set up the quick browser on the left
    folderListView = new QListView( leftWidget );
    folderListView->header()->setClickEnabled( FALSE );
    folderListView->addColumn( i18n("Quick Browser"), -1 );
    folderListView->setRootIsDecorated( TRUE );
    folderListView->setSorting(-1);

    // set up the songlist on the right
    songListView = new MyListView( centralWidget );
    
    setCentralWidget(centralWidget);

    // signals of folderListView
    connect( folderListView, SIGNAL( currentChanged( QListViewItem* ) ), this, SLOT( slotFolderChanged() ) );
    connect(folderListView, SIGNAL( rightButtonPressed( QListViewItem *, const QPoint& , int ) ), this, SLOT( slotFolderPopup( QListViewItem *, const QPoint &, int ) ) );

    // signals of songListView
    connect(songListView, SIGNAL( rightButtonPressed( QListViewItem *, const QPoint& , int ) ), this, SLOT( songListPopup( QListViewItem *, const QPoint&, int ) ) );
    connect(songListView, SIGNAL( doubleClicked( QListViewItem * ) ), this, SLOT( doubleClick() ) );
    connect(songListView, SIGNAL( mouseButtonClicked( int, QListViewItem *, const QPoint&, int ) ), this, SLOT( middleClick(int) ) );
    // for saving column settings
    connect(songListView->header(), SIGNAL( sizeChange(int, int, int) ), this, SLOT( saveColumnSettings() ) );
    connect(songListView->header(), SIGNAL( indexChange(int, int, int) ), this, SLOT( saveColumnSettings() ) );
}


void YammiGui::createFolders( ) {
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


bool YammiGui::setupActions( ) {
    KStdAction::quit(this, SLOT(close()), actionCollection());

    //Selection actions
    KStdAction::selectAll(this,SLOT(selectAll()),actionCollection());
    new KAction(i18n("Invert selection"),0,0,this,SLOT(invertSelection()), actionCollection(),"invert_selection");

    //Media player actions
    m_playPauseAction = new KAction(i18n("Play"), "player_play", KShortcut(Key_F1), this, SLOT(playPause()), actionCollection(),"play_pause");
    new KAction(i18n("Stop"), "player_stop", KShortcut(Key_F4), this, SLOT(stop()), actionCollection(), "stop");
    new KAction(i18n("Skip Backward"),"player_rew",KShortcut(Key_F2),this,SLOT(skipBackward()), actionCollection(),"skip_backward");
    new KAction(i18n("Skip Forward"),"player_fwd",KShortcut(Key_F3),this,SLOT(skipForward()), actionCollection(),"skip_forward");
    m_seekSlider = new TrackPositionSlider( QSlider::Horizontal, 0L, "seek_slider");
    m_seekSlider->setFixedWidth(200);
    QToolTip::add
        (m_seekSlider, i18n("Track position"));
    connect(m_seekSlider,SIGNAL(sliderMoved(int)),this,SLOT(seek(int)));
    connect(m_seekSlider,SIGNAL(myWheelEvent(int)),this,SLOT(seekWithWheel(int)));
    new KWidgetAction( m_seekSlider ,"text",0, 0, 0,actionCollection(),"seek");

    //Database actions
    new KAction(i18n("Save Database"),"filesave",KShortcut(QKeySequence(Key_Control,Key_S)),this,SLOT(saveDatabase()), actionCollection(),"save_db");
    new KAction(i18n("Scan Harddisk..."),"fileimport",0,this,SLOT(updateSongDatabaseHarddisk()), actionCollection(),"scan_hd");
    new KAction(i18n("Scan Removable Media..."),"fileimport",0,this,SLOT(updateSongDatabaseMedia()), actionCollection(),"scan_media");
    new KAction(i18n("Import Selected File(s)..."),"edit_add",0,this,SLOT(updateSongDatabaseSingleFile()), actionCollection(),"import_file");
    new KAction(i18n("Check Consistency..."),"spellcheck",0,this,SLOT(forAllCheckConsistency()), actionCollection(),"check_consistency_all");
    new KAction(i18n("Fix genres..."),0,0,this,SLOT(fixGenres()), actionCollection(),"fix_genres");
    new KAction(i18n("Grab And Encode CD-Track..."),"cd",0,this,SLOT(grabAndEncode()), actionCollection(),"grab");

    // playlist actions
    new KAction(i18n("Shuffle Playlist..."),"roll",0,this,SLOT(shufflePlaylist()),actionCollection(),"shuffle_playlist");
    new KAction(i18n("Clear Playlist"),"edittrash",KShortcut(QKeySequence(Key_Shift,Key_F8)),this,SLOT(clearPlaylist()),actionCollection(),"clear_playlist");
    new KAction(i18n("Switch to/from Playlist"),"toggle_playlist",KShortcut(QKeySequence(Key_Control,Key_P)),this,SLOT(toFromPlaylist()),actionCollection(),"toggle_playlist");

    // selection actions
    new KAction(i18n("Enqueue as next (prepend)"), "enqueue_asnext", KShortcut(Key_F6), this, SLOT(forSelectionEnqueueAsNext()), actionCollection(), "prepend_selected");
    new KAction(i18n("Enqueue at end (append)"), "enqueue", KShortcut(Key_F5), this, SLOT(forSelectionEnqueue()), actionCollection(), "append_selected");
    new KAction(i18n("Play Now!"), "play_now", KShortcut(Key_F7), this, SLOT(forSelectionPlayNow()), actionCollection(), "play_selected");
    new KAction(i18n("Dequeue Songs"), "stop", KShortcut(Key_F8), this, SLOT(forSelectionDequeue()), actionCollection(), "dequeue_selected");
    new KAction(i18n("Prelisten start"), "prelisten_start", KShortcut(Key_F9), this, SLOT(forSelectionPrelistenStart()), actionCollection(), "prelisten_start");
    new KAction(i18n("Prelisten middle"), "prelisten_middle", KShortcut(Key_F10), this, SLOT(forSelectionPrelistenMiddle()), actionCollection(), "prelisten_middle");
    new KAction(i18n("Prelisten end"), "prelisten_end", KShortcut(Key_F11), this, SLOT(forSelectionPrelistenEnd()), actionCollection(), "prelisten_end");
    new KAction(i18n("Song Info..."), "info", KShortcut(Key_I), this, SLOT(forSelectionSongInfo()), actionCollection(), "info_selected");
    new KAction(i18n("Delete Song..."), 0, 0, this, SLOT(forSelectionDelete()), actionCollection(), "delete_selected");
    new KAction(i18n("Burn To Media"), 0, 0, this, SLOT(forSelectionBurnToMedia()), actionCollection(), "burn_selected");
    new KAction(i18n("Check Consistency..."),"spellcheck",0,this,SLOT(forSelectionCheckConsistency()), actionCollection(),"check_consistency");
    new KAction(i18n("Move Files"), 0, 0, this, SLOT(forSelectionMove()), actionCollection(), "move_selected");
    new KAction(i18n("Search for similar entry"), 0, 0, this, SLOT(searchForSimilarEntry()), actionCollection(), "search_similar_entry");
    new KAction(i18n("Search for similar artist"), 0, 0, this, SLOT(searchForSimilarArtist()), actionCollection(), "search_similar_artist");
    new KAction(i18n("Search for similar title"), 0, 0, this, SLOT(searchForSimilarTitle()), actionCollection(), "search_similar_title");
    new KAction(i18n("Search for similar album"), 0, 0, this, SLOT(searchForSimilarAlbum()), actionCollection(), "search_similar_album");
    new KAction(i18n("Goto artist"), 0, 0, this, SLOT(gotoFolderArtist()), actionCollection(), "goto_artist_folder");
    new KAction(i18n("Goto album"), 0, 0, this, SLOT(gotoFolderAlbum()), actionCollection(), "goto_album_folder");
    new KAction(i18n("Goto genre"), 0, 0, this, SLOT(gotoFolderGenre()), actionCollection(), "goto_genre_folder");
    new KAction(i18n("Goto year"), 0, 0, this, SLOT(gotoFolderYear()), actionCollection(), "goto_year_folder");

    new KAction(i18n("Stop prelisten"),"stop_prelisten",KShortcut(Key_F12),this,SLOT(stopPrelisten()),actionCollection(),"stop_prelisten");

    // autoplay actions
    KToggleAction *ta;
    m_autoplayActionOff = new KRadioAction(i18n("Off"),0,0,this,SLOT(autoplayOff()),actionCollection(),"autoplay_off");
    m_autoplayActionOff->setExclusiveGroup("autoplay");
    m_autoplayActionLnp = new KRadioAction(i18n("Longest not played"),0,0,this,SLOT(autoplayLNP()),actionCollection(),"autoplay_longest");
    m_autoplayActionLnp->setExclusiveGroup("autoplay");
    m_autoplayActionRandom = new KRadioAction(i18n("Random"),0,0,this,SLOT(autoplayRandom()),actionCollection(),"autoplay_random");
    m_autoplayActionRandom->setExclusiveGroup("autoplay");
    m_currentAutoPlay = new KAction(i18n("Unknown"),0,0,0,0,actionCollection(),"autoplay_folder");

    // toggle toolbar actions
    ta = new KToggleAction("Main ToolBar",0,0,this,SLOT(toolbarToggled()),actionCollection(),"MainToolbar");
    ta = new KToggleAction("Media Player",0,0,this,SLOT(toolbarToggled()),actionCollection(),"MediaPlayerToolbar");
    ta = new KToggleAction("Song Actions",0,0,this,SLOT(toolbarToggled()),actionCollection(),"SongActionsToolbar");
    ta = new KToggleAction("Removable Media",0,0,this,SLOT(toolbarToggled()),actionCollection(),"RemovableMediaToolbar");
    ta = new KToggleAction("Sleep Mode",0,0,this,SLOT(toolbarToggled()),actionCollection(),"SleepModeToolbar");
    ta = new KToggleAction("Prelisten",0,0,this,SLOT(toolbarToggled()),actionCollection(),"PrelistenToolbar");

    // other actions
    new KAction(i18n("Update Automatic Folder Structure"),"reload",0,this,SLOT(updateView()),actionCollection(),"update_view");

    // setup
    KStdAction::preferences(this,SLOT(setPreferences()),actionCollection());
    KStdAction::keyBindings(this,SLOT(configureKeys()),actionCollection());

    // - "insert custom widgets in toolbar using KWidgetAction
    //search
    QHBox *w = new QHBox( );
    new QLabel(i18n("Search:"),w);
    m_searchField = new LineEditShift(w);
    m_searchField->setFixedWidth(175);
    QToolTip::add
        ( m_searchField, i18n("Fuzzy search (Ctrl-F)"));
    connect( m_searchField, SIGNAL(textChanged(const QString&)), SLOT(searchFieldChanged(const QString&)));
    /*
        TODO: temporarily disabled before we have a better concept and have it documented properly...
        QPushButton *btn = new QPushButton(i18n("to wishlist"),w);
        connect( btn, SIGNAL( clicked() ), this, SLOT( addToWishList() ) );
        QToolTip::add( btn, i18n("Add this entry to the database as a \"wish\""));
    */
    new KWidgetAction(w, "Search", 0, 0, 0, actionCollection(), "search");


    // removable media management
    w = new QHBox( );
    new QLabel(i18n("Needed media:"),w);
    mediaListCombo = new QComboBox( FALSE, w );
    mediaListCombo->setFixedWidth(150);
    loadFromMediaButton = new QPushButton(i18n("load"), w);
    connect( loadFromMediaButton, SIGNAL( clicked() ), this, SLOT( loadMedia() ) );
    new KWidgetAction(w, "Load media", 0, 0, 0, actionCollection(), "load_media");

    // Sleep mode
    w = new QHBox( );
    new QLabel(i18n("Sleep mode:"),w);
    m_sleepModeButton = new QPushButton(i18n("(disabled)"), w);
    QToolTip::add
        ( m_sleepModeButton, i18n("toggle sleep mode"));
    connect( m_sleepModeButton, SIGNAL( clicked() ), this, SLOT( changeSleepMode() ) );
    m_sleepModeSpinBox=new QSpinBox(1, 99, 1, w);
    QToolTip::add
        ( m_sleepModeSpinBox, i18n("number songs until shutdown"));
    m_sleepMode = false;
    m_sleepModeSpinBox->setEnabled(m_sleepMode);
    new KWidgetAction( w ,"Sleep Mode",0, 0, 0,actionCollection(),"sleep_mode");

    // the rc file must be installed (eg. in /opt/kde3/share/apps/yammi/yammiui.rc)
    createGUI();
    // test whether file was found and loaded
    QPopupMenu* anyMenu = (QPopupMenu *)factory()->container("player", this);
    if(anyMenu == 0) {
        kdError() << "you must have the file 'yammiui.rc' installed (eg. in /opt/kde3/share/apps/yammi/yammiui.rc)\n";
        return false;
    }
    return true;
}
//////////////////

void YammiGui::createMenuBar( ) {
    // TODO: Integrate this into the menu generated by the XML-GUI Framework
    // TODO: use states as supported by XML framework
    columnsMenu = new QPopupMenu;
    for(int column=0; column<13; column++) {
        int id=columnsMenu->insertItem( columnName[column],  this, SLOT(toggleColumnVisibility(int)), 0, column);
        columnsMenu->setItemChecked(id, columnVisible[column]);
    }

    KMenuBar *mainMenu = menuBar( );
    mainMenu->insertItem( i18n("Columns"), columnsMenu, -1, 3 );
}

/**
 * Creates the song popup menu from the xml gui framework.
 * Also calls updateSongPopup to populate the submenus for categories and plugins.
 * These have to be updated with updateSongPopup on each change in existing categories
 * or plugins.
 */
void YammiGui::createSongPopup() {
    kdDebug() << "creating song popup\n";
    songPopup = (QPopupMenu *)factory()->container("song_popup", this);
    songGoToPopup = (QPopupMenu *)factory()->container("goto", this);
    if(songPopup == 0 || songGoToPopup == 0) {
        kdFatal() << "yammiui.rc not installed correctly!!!\n";
        return;
    }
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
 * @param pos time in milliseconds
 */
void YammiGui::seek( int pos ) {
//    kdDebug() << "seek song to pos " << pos << endl;
    player->jumpTo(pos);
}


void YammiGui::seekWithWheel(int rotation) {
    kdDebug() << "seekWithWheel() called\n";
    if(rotation<0) {
        m_seekSlider->addPage();
    } else {
        m_seekSlider->subtractPage();
    }
    player->jumpTo(m_seekSlider->value());
}


void YammiGui::setSelectionMode(SelectionMode mode) {
    selectionMode = mode;
}

/// randomNum generates a random number in 0..numbers-1 interval
/// @param numbers designates how many different numbers can be generated
/// @returns random number
int YammiGui::randomNum(int numbers) {
    int chosen = (rand() % numbers); // random number in 0..numbers-1 interval
    if (numbers / RAND_MAX > 0) {
        // RAND_MAX might be smaller than numbers (unlikely, platform dependent)
        int numiter = (rand() % (numbers / RAND_MAX + 1));
        while ( numiter > 0) {
            // add some more random numbers to allow reaching of higher values
            chosen += (rand() % numbers);
            numiter--;
        }
        chosen %= numbers; // make sure the result is < numbers
    }
    return chosen;
}


Prefs* YammiGui::config() {
    return model->config();
}


/*     DCOP-functions     */
void YammiGui::play() {

    player->play();
}

void YammiGui::playPause() {

    player->playPause();
}

void YammiGui::stop() {
    player->stop();
}

void YammiGui::pause() {

    player->pause();
}

int YammiGui::totalTime() {

    return player->getTotalTime();
}

int YammiGui::currentTime() {

    return player->getCurrentTime();
}

void YammiGui::aplayOff() {

    autoplayOff();
    m_autoplayActionOff->setChecked(true);
}

void YammiGui::aplayLNP() {

    autoplayLNP();
    m_autoplayActionLnp->setChecked(true);
}

void YammiGui::aplayRandom() {

    autoplayRandom();
    m_autoplayActionRandom->setChecked(true);
}

QString YammiGui::songInfo() {

    QString info;

    info = "Artist :  " + songArtist() + "\n";
    info += "Title :\t  " + songTitle() + "\n";
    info += "Album :\t  " + songAlbum() + "\n";
    info += "Track :\t  " + songTrack2D() + "\n";
    info += "Year :\t  " + QString::number(songYear()) + "\n";
    info += "Genre :\t  " + songGenre() + "\n" ;
    info += "Comment : " + songComment() + "\n";

    return info;

}

QString YammiGui::songArtist() {

    return currentSong->artist;
}

QString YammiGui::songTitle() {

    return currentSong->title;
}


int YammiGui::songTrack() {

    return currentSong->trackNr;
}

QString YammiGui::songTrack2D() {

    return QString().sprintf("%02d", currentSong->trackNr);
}

QString YammiGui::songAlbum() {

    return currentSong->album;
}

QString YammiGui::songGenre() {

    return currentSong->genre;
}

QString YammiGui::songComment() {

    return currentSong->comment;
}

int YammiGui::songYear() {

    return currentSong->year;
}
