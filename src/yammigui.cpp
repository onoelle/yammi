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
#include "ConsistencyCheckDialog.h"
#include "ApplyToAllBase.h"

#include <taglib/id3v1genres.h>
#include "ConsistencyCheckParameter.h"

#include <qtextedit.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <qfiledialog.h>
#include <kconfig.h>
#include <kaction.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <kkeydialog.h>


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
#include "foldersorted.h"
#include "mylistview.h"
#include "lineeditshift.h"
#include "trackpositionslider.h"
#include "util.h"

#include "mediaplayer.h"
#include "dummyplayer.h"
#include "xine-engine.h"

static QString columnName[] = { QObject::tr("Artist"), QObject::tr("Title"), QObject::tr("Album"),QObject:: tr("Length"),
                                QObject::tr("Year"), QObject::tr("TrackNr"), QObject::tr("Genre"), QObject::tr("AddedTo"), QObject::tr("Bitrate"),
                                QObject::tr("Filename"), QObject::tr("Path"), QObject::tr("Comment"), QObject::tr("Last Played") };


extern YammiGui* gYammiGui;

/////////////////////////////////////////////////////////////////////////////////////////////
YammiGui::YammiGui() : KMainWindow( ) {
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
    qDebug() << "trying to load database from directory " << databaseDir;
    QDir d(databaseDir);
    bool importOld=false;
    config()->databaseDir = databaseDir;
    QString oldYammiDir(QDir::homeDirPath()+"/.yammi/");
    if(!d.exists("songdb.xml")) {
        QDir oldDir(oldYammiDir);
        if(oldDir.exists() && oldDir.exists("songdb.xml")) {
            qDebug() << "no database existing yet, database from previous yammi versions found";
            qDebug() << "importing your old song database from " << oldYammiDir;

            importOld=true;
            config()->databaseDir=oldYammiDir;
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
    qDebug() << "saveOptions() ";
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
    qDebug() << "readOptions()";
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
        qDebug() << "no column order found, taking default order";
    }
    for(int i=0; i<MAX_COLUMN_NO; i++) {
        columnWidth[i]=cfg->readNumEntry(QString("column%1Width").arg(i));
    }
    autoplayFoldername=cfg->readEntry("AutoplayFolder", tr("All Music"));
    m_currentAutoPlay->setText(tr("Folder: ")+autoplayFoldername);
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


bool YammiGui::queryClose() {
    qDebug() << "queryClose()";
    if(model->allSongsChanged() || model->categoriesChanged()) {
        QString msg=tr("The Song Database has been modified.\nDo you want to save the changes?");
        switch (QMessageBox::warning(this, tr("Database modified"), msg,
                                     QMessageBox::Yes | QMessageBox::Default,
                                     QMessageBox::No,
                                     QMessageBox::Cancel | QMessageBox::Escape))
        {
        case QMessageBox::Yes:
            saveDatabase();
            break;
        case QMessageBox::No:
            break;
        case QMessageBox::Cancel:
        default:
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
    qDebug() << "queryExit() ";
    saveOptions();
    player->quit( );
    return true;
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
        qError()<<"toolbarToggled( const QString& name ) : action not found n = "
        <<n<<" (name = "<<name<<")";
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
    qDebug() << "updatePlaylist called";
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
    
    int noSongsToPlay = folderActual->songlist().count() -1;
    if(noSongsToPlay < 0) {
      noSongsToPlay = 0;
    }
    int length = 0;
    for(unsigned int i=1; i<folderActual->songlist().count(); i++) {
        length += folderActual->songlist().at(i)->song()->length;
    }
    QString formattedTime;
    formattedTime.sprintf(tr("%d:%02d"), length/(60*60), (length % (60*60))/60);
    htmlTemplate.replace(QRegExp("\\{noSongsToPlay\\}"), QString("%1").arg(noSongsToPlay));
    htmlTemplate.replace(QRegExp("\\{timeToPlay\\}"), formattedTime);
    
    QString yammiDir = KGlobal::dirs()->findResourceDir("appdata","pics/nowplaying.png");    
    htmlTemplate.replace(QRegExp("\\{yammiPicsDir\\}"), yammiDir + "pics");

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
            qDebug() << "invalid scope definition in template!";
        }
    }
    
    playlistPart->setText(htmlSource);
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
 * Called when a new song is played: updates title bar, songSlider
 */
void YammiGui::handleNewSong(Song* newSong) {
    currentSong=newSong;
    if(newSong==0) {
        setCaption(tr("Yammi - not playing"));
        currentFile="";
        m_seekSlider->setupTickmarks(0);
        return;
    }
    // TODO: take swapped file?
    currentFile=newSong->location();
    currentSongStarted=currentSongStarted.currentDateTime();

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
        m_playPauseAction->setText(tr("Pause"));
    } else {
        m_playPauseAction->setIcon("player_play");
        m_playPauseAction->setText(tr("Play"));
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
    folderSearchResults->update(searchResults);

    if(startup) {
        // this is only necessary on startup
        folderActual->update(model->songsToPlay);
        folderCategories->update(model->allCategories, model->categoryNames);
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
        songListView->addColumn( tr("Pos"), 35);
        current++;
    }
    if(chosenFolder==folderHistory || chosenFolder==folderSongsPlayed) {
        songListView->addColumn( tr("Played on"), 135);
        current++;
    }
    if(chosenFolder==folderSearchResults) {
        songListView->addColumn( tr("Match"), 45);
        current++;
    }
    if(chosenFolder==folderProblematic) {
        songListView->addColumn( tr("Reason"), 120);
        current++;
    }
    if(((QListViewItem*)chosenFolder)->parent()==folderCategories) {
        songListView->addColumn( tr("Pos"), 35);
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
        //    qDebug() << "j: " << j << ", section: " << section << "label: " << header->label(section) << ", size: " << header->sectionSize(section);
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
        qDebug() << "trying to switch media player...";
        int savedSongPosition = player->getCurrentTime();
        int savedPlayingState = player->getStatus();
        player->stop();
        disconnectMediaPlayer();
        qDebug() << "old player disconnected";
        delete player;
        qDebug() << "old player deleted";
        loadMediaPlayer();
        qDebug() << "new media player loaded...";
        connectMediaPlayer();
        qDebug() << "new media player connected...";
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
    qDebug() << "updating song popup";
    playListPopup->clear();
    playListPopup->insertItem(QIconSet( QPixmap(newCategory_xpm)), tr("New Category..."), this, SLOT(toCategory(int)), 0, 9999);
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
        qDebug() << "folder not found!";
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
    qDebug() << str1;
    qDebug() << str2;

    int def=0;									// 1=keep s1, 2=keep s2, 0=keep both
    if(s1->bitrate > s2->bitrate) {
        def=1;
    }
    if(s1->bitrate < s2->bitrate) {
        def=2;
    }

    QString msg = tr("Two identical songs: \ns1: %1\ns2: %2\nDo you want to delete one of them?");
    switch (QMessageBox::warning(this, "", msg.arg(str1).arg(str2),
                                 tr("Delete s1"),
                                 tr("Delete s2"),
                                 tr("Keep both"), 0, 2))
    {
    case 0:
        qDebug()<< "deleting s1";
        s1->deleteFile(config()->trashDir);
        folderAll->removeSong(s1);
        break;
    case 1:
        qDebug()<< "deleting s2";
        s2->deleteFile(config()->trashDir);
        folderAll->removeSong(s2);
        break;
    case 2:
    default:
        break;
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
        qDebug() << "goto artist/album/genre/year: folder '" << folderName << "' not existing";
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
    qDebug() << "adding folder content of folder " << folder->folderName();

    if(folder->songlist().count() != 0) {
        songListView->setSorting(-1);
        songListView->setUpdatesEnabled(false);
        addFolderContentSnappy();
    } else {		// no songList in that folder
        QApplication::restoreOverrideCursor();
    }
}

void YammiGui::addFolderContentSnappy() {
    qDebug() << "snappy adding folder content of folder " << folderToAdd->folderName();
    int i=0;
    SongEntry* entry = 0;
    
    SongListItem* lastOne=(SongListItem*)songListView->firstChild();
    if(folderToAdd->songlist().count() != 0) {
        for (entry = folderToAdd->firstEntry(); entry && i<alreadyAdded; entry = folderToAdd->nextEntry(), i++ ) {
            SongListItem* check=(SongListItem*)lastOne->itemBelow();
            if(check!=0)
                lastOne=check;
        }
    
        for (; entry && i<=alreadyAdded+200; entry = folderToAdd->nextEntry(), i++ ) {
            lastOne=new SongListItem( songListView, entry); //, lastOne);
        }
        alreadyAdded=i;
    }
    
    // any songs left to add?
    if(entry) {
        // yes, add them after processing events
        QTimer* timer=new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(addFolderContentSnappy()) );
        timer->start(0, TRUE);
    } else {		// no, we are finished
        qDebug() << "finishing off...";
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
        label=QString(tr("%1 songs selected")).arg(selected);
    } else {
        label=first->displayName();
    }
    songPopup->changeItem ( 113, label);


    int id = songGoToPopup->idAt(0);
    QString folderName = first->artist;
    if(folderName == "") {
        folderName = "- no artist -";
    }
    songGoToPopup->changeItem(id, tr("Artist: ") + folderName);
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
    songGoToPopup->changeItem(id, tr("Album: ") + folderName);
    songGoToPopup->setItemEnabled(id, f!=0);

    id = songGoToPopup->idAt(2);
    folderName = first->genre;
    if(folderName == "") {
        folderName = "- no genre -";
    }
    songGoToPopup->changeItem(id, tr("Genre: ") + folderName);
    songGoToPopup->setItemEnabled(id, getFolderByName(folderName)!=0);

    id = songGoToPopup->idAt(3);
    if(first->year == 0) {
        folderName = "- no year -";
    } else {
        folderName = QString("%1").arg(first->year);
    }
    songGoToPopup->changeItem(id, tr("Year: ") + folderName);
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
    songPopup->setItemEnabled(Song::CheckConsistency, enable);
    songPopup->setItemEnabled(Song::MoveTo, enable);
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
        QString dir = QFileDialog::getExistingDirectory("", this, NULL, tr("choose directory for plugin"));
        if(dir.isNull())
            return;
        cmd.replace(QRegExp("\\{directoryDialog\\}"), dir);
    }
    if(cmd.contains("{fileDialog}")>0) {
        QString file = QFileDialog::getSaveFileName("", QString::null, this, NULL, tr("choose file for plugin"));
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
        QString prompt = QString(tr("Type in plugin parameter"));
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
            QString msg=tr("Execute the following command on each selected song?\n");
            msg+=tr("(here shown: values for first song)\n\n");
            for(unsigned int i=0; i<sampleCmd.length(); i+=80) {
                msg+=sampleCmd.mid(i, 80)+"\n";
            }
            if (QMessageBox::warning(this, "", msg, QMessageBox::Yes, QMessageBox::No | QMessageBox::Escape) != QMessageBox::Yes) {
                return;
            }
        }
        int index=1;
        QProgressDialog progress(this);
        progress.setLabelText(tr("Executing song plugin cmd..."));
        progress.setModal(true);
        progress.setTotalSteps(selectedSongs.count());
        for(Song* s=selectedSongs.firstSong(); s; s=selectedSongs.nextSong(), index++) {
            QString cmd2=s->replacePlaceholders(cmd, index);
            progress.setProgress(index);
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
            QString msg=tr("Execute the following command:\n");
            for(unsigned int i=0; i<cmd.length(); i+=80) {
                msg+=cmd.mid(i, 80)+"\n";
                if(i>1200) {
                    msg+=tr("\n...\n(command truncated)");
                    break;
                }
            }
            if (QMessageBox::warning(this, "", msg, QMessageBox::Yes, QMessageBox::No | QMessageBox::Escape) != QMessageBox::Yes) {
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
    QString dir = QFileDialog::getExistingDirectory(startPath, this, NULL, tr("Select destination directory"));
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



/** calculate disk usage for a directory (including all subdirectories)
 * returns -1 if too full
 */
long double YammiGui::diskUsage(QString path, long double sizeLimit) {
    QDir d(path);

    d.setFilter(QDir::Files);
    const QFileInfoList* list = d.entryInfoList();
    if (!list) {
        qError() << "Error: cannot read access swap directory: " << path;
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
        qWarning() << "Error: Skipping unreadable directory under swap directory: " << path;
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

    qDebug() << "disk usage in directory " << path << ": " << ((int)size/1024.0/1024.0) << " MB";
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
            qDebug() << "taking songs directly from list...";
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
    statusBar( )->message(QString(tr("%1 Songs equeued at end of playlist")).arg(count), 2000);
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
                qDebug() << "song dequeued: " << entry->song()->displayName();
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
                    qDebug() << "song dequeued: " << s->displayName();
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
            _bitrate=QString(tr("%1 kb/s")).arg(s->bitrate);
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
        si.LabelHeading->setText(QString(tr("Mass editing: %1 songs")).arg(selected));
        si.LabelSize->setText(tr("Size (total)"));
        si.LabelLength->setText(tr("Length (total)"));
        QString x;
        si.ReadOnlyLength->setText(x.sprintf(tr("%d:%02d:%02d (hh:mm:ss)"), _length/(60*60), (_length % (60*60))/60, _length % 60));
    } else {
        si.LabelHeading->setText(_artist+" - "+_title);
        QString x;
        si.ReadOnlyLength->setText(x.sprintf(tr("%2d:%02d (mm:ss)"), _length/60, _length % 60));
    }
    if(_size < (1024 * 1024)) {
        si.ReadOnlySize->setText( QString(tr("%1 KB")).arg( (float)_size/(float)(1024) , 4,'f', 2));
    }
    else {
        si.ReadOnlySize->setText( QString(tr("%1 MB")).arg( (float)_size/(float)(1024*1024) , 4,'f', 2));
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
        msg += QString(tr("Your changes will affect %1 song entries.\n")).arg(selected);
        if(si.CheckBoxCorrectFilename->isChecked()) {
            msg += tr("\nNote: Your changes may change the filenames of the selected files!\n");
        }
        if(si.CheckBoxCorrectPath->isChecked()) {
            msg += tr("\nNote: Your changes may affect the location of the selected files!\n");
        }
        msg += tr("\n\nDo you want to continue?");
        if (QMessageBox::warning(this, "", msg, QMessageBox::Yes, QMessageBox::No | QMessageBox::Escape) != QMessageBox::Yes) {
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
        dd.LabelSongname->setText(QString(tr("Delete %1 songs")).arg(selectedSongs.count()));
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
    default:
        qWarning() << "unknown action for double click: " << action;
    }
}


/**
 * Creates a new category, querying the user for the name.
 */
bool YammiGui::newCategory() {
    bool ok = false;
    QString caption(tr("Enter name for category"));
    QString message(tr("Please enter name of category"));
    QString newName=QString(QInputDialog::getText( caption, message, QLineEdit::Normal, QString(tr("new category")), &ok, this ));
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
    QString msg(tr("Delete category %1 ?\n (will be deleted immediately!)").arg(name));
    if (QMessageBox::warning(this, "", msg, QMessageBox::Yes, QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes) {
        model->removeCategory(name);
        folderListView->setCurrentItem( (QListViewItem*)folderCategories );
        folderListView->setSelected( (QListViewItem*)folderCategories , TRUE );
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
    QString newName=QString(QInputDialog::getText( tr("collection name"), tr("Please enter new name:"), QLineEdit::Normal, oldName, &ok, this ));
    if(!ok) {
        return;
    }

    model->renameCategory(oldName, newName);
    qDebug() << "renamed in model...";
    folderListView->setCurrentItem( (QListViewItem*)folderCategories );
    folderListView->setSelected( (QListViewItem*)folderCategories , TRUE );
    folderCategories->update(model->allCategories, model->categoryNames);
    qDebug() << "categories updated...";
    updateSongPopup();
    qDebug() << "updated song popup...";
    //changeToFolder((Folder*)i, TRUE);
}

/**
 * Inserts all songs from a .m3u playlist into a category.
 * Only inserts those songs already existing in yammi's database.
 * Inserts in the order of the playlist.
 */
void YammiGui::loadM3uIntoCategory() {
    QListViewItem* i = folderListView->currentItem();
    FolderSorted* categoryFolder=(FolderSorted*)i;
    QString filename = QFileDialog::getOpenFileName("/", "Playlists (*.m3u)", this, NULL, tr("Choose a Playlist to insert" ));
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
            qDebug() << "no song in database found with filename \"" << filename << "\" (not in Yammi database yet?), skipping";
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
    m_currentAutoPlay->setText(tr("Folder: ")+autoplayFoldername);
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


/// invoke an externally configured program/script on the content of a folder
void YammiGui::pluginOnFolder() {
    QFile f(config()->databaseDir + "plugin.temp" );
    if ( !f.open( IO_WriteOnly  ) ) {
        return;
    }
    QTextStream str(&f);

    for(Song* s=chosenFolder->firstSong(); s; s=chosenFolder->nextSong()) {
        qDebug() << s->path << "/" << s->filename;
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
    QString msg=QString(tr("Clear complete playlist?\n(%1 entries)")).arg(model->songsToPlay.count());
    confirm.TextLabel->setText(msg);
    confirm.CheckBoxApply->setText(tr("including current song"));
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


/**
 * Fix all genres by re-reading them from the files.
 */
void YammiGui::fixGenres() {
    QString msg = tr("Do you want to fix the genre of all songs (potentially broken or incomplete from earlier versions of yammi) by re-reading all genres from available files now?");
    if (QMessageBox::warning(this, "", msg, QMessageBox::Yes, QMessageBox::No | QMessageBox::Escape) != QMessageBox::Yes) {
        return;
    }
    isScanning=true;
    QProgressDialog progress(this);
    progress.setLabelText(tr("Re-Reading all genres from your files..."));
    progress.setTotalSteps(model->allSongs.count());
    progress.setModal(true);
    progress.setMinimumDuration(0);
    model->fixGenres(&progress);
    updateView();
    isScanning=false;
}

void YammiGui::keyPressEvent(QKeyEvent* e) {
    //  qDebug() << "x: " << this->x() << "pos.x: " << this->pos().x();
    //  qDebug() << "geometry.x: " << this->geometry().left() << "frameGeometry.x: " << this->frameGeometry().left();
    //  qDebug() << "key(): " << e->key() << "text(): " << e->text() << "ascii(): " << e->ascii();
    int key=e->key();
    switch(key) {
    case Key_Control:
        controlPressed=true;
        break;
    case Key_Shift:
        shiftPressed=true;
        break;

    case Key_PageUp:
        songListView->simulateKeyPressEvent(e);
        break;
    case Key_PageDown:
        songListView->simulateKeyPressEvent(e);
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
    //	qDebug() << "release key(): " << e->key() << "text(): " << e->text() << "ascii(): " << e->ascii();
    int key=e->key();
    switch(key) {
    case Key_Control:
        controlPressed=false;
        //      qDebug() << "control released";
        break;
    case Key_Shift:
        shiftPressed=false;
        //      qDebug() << "shift released";
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


void YammiGui::updateSongDatabaseHarddisk() {
    UpdateDatabaseDialog d(this, config());
    // show dialog
    int result=d.exec();
    if(result!=QDialog::Accepted) {
        return;
    }
    config()->saveConfig();
    updateSongDatabase(config()->scanDir, config()->scanPattern);
}

void YammiGui::updateSongDatabaseSingleFile() {
    QStringList files = QFileDialog::getOpenFileNames( ":singleFile", QString::null, this, NULL, tr("Open file(s) to import"));
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
    msg+=QString(tr("%1 songs added to database\n")).arg(model->entriesAdded);
    msg+=QString(tr("%1 songs corrupt (=not added)\n")).arg(model->corruptSongs);
    msg+=QString(tr("%1 songs problematic (check in folder Problematic Songs)\n")).arg(model->problematicSongs.count());
    QMessageBox::information(this, "Yammi", msg);
}


void YammiGui::updateSongDatabase(QString scanDir, QString filePattern) {
    if(config()->childSafe) {
        return;
    }
    QProgressDialog progress(this);
    progress.setLabelText(tr("Scanning..."));
    progress.setModal(true);
    progress.setMinimumDuration(0);
    progress.setAutoReset(false);
    progress.setAutoClose(false);
    progress.setProgress(0);

    isScanning=true;
    model->updateSongDatabase(scanDir, config()->followSymLinks, filePattern, &progress);

    progress.close();
    updateView();
    folderProblematic->update(model->problematicSongs);
    folderAll->updateTitle();
    QString msg=tr("Updated your database.\n\nStatistics: \n\n");
    msg+=QString(tr("%1 songs added to database\n")).arg(model->entriesAdded);
    msg+=QString(tr("%1 songs corrupt (=not added)\n")).arg(model->corruptSongs);
    msg+=QString(tr("%1 songs problematic (check in folder Problematic Songs)\n")).arg(model->problematicSongs.count());
    QMessageBox::information(this, "", msg);
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
    case Prefs::MEDIA_PLAYER_XINEENGINE:
        player = new Yammi::XineEngine(model);
        break;
    case Prefs::MEDIA_PLAYER_DUMMY:
    default:
        player = new DummyPlayer( model );
        break;
    }
    if (!player) {
        player = new DummyPlayer( model );
        /*        QMessageBox::error(this, tr("Can't create the player object.\n"
                                              "Please select a suitable backend player\n"
                                              "from the Preferences Dialog"), tr("Error"));*/
        qDebug() << "Can't create player backend, select a suitable backend in the Preferences Dialog";
    }
    qDebug() << "Media Player : " << player->getName( );
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
    playlistPart = new QTextEdit(leftWidget);
    playlistPart->setReadOnly(true);
    playlistPart->setHScrollBarMode(QScrollView::AlwaysOn);
    
    centralWidget->setResizeMode( leftWidget, QSplitter::KeepSize );
    leftWidget->setResizeMode(playlistPart, QSplitter::KeepSize );
    
    // set up the quick browser on the left
    folderListView = new QListView( leftWidget );
    folderListView->header()->setClickEnabled( FALSE );
    folderListView->addColumn( tr("Quick Browser"), -1 );
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
    folderAll=new Folder( folderListView, QString(tr("All Music")), &(model->allSongs));

    // folder containing all artists with more than <n> songs
    folderArtists = new FolderGroups( folderListView, QString( tr("Artists") ));
    folderArtists->moveItem(folderAll);

    // folder containing all albums with more than <n> songs
    folderAlbums = new FolderGroups( folderListView, QString( tr("Albums") ));
    folderAlbums->moveItem(folderArtists);

    // folder containing all genres with more than <n> songs
    folderGenres = new FolderGroups( folderListView, QString( tr("Genre") ));
    folderGenres->moveItem(folderAlbums);

    // folder containing all songs from a year (if more than <n> songs)
    folderYears = new FolderGroups( folderListView, QString( tr("Year") ));
    folderYears->moveItem(folderGenres);

    // folder containing all categories
    folderCategories = new FolderCategories( folderListView, QString(tr("Categories")));
    folderCategories->moveItem(folderYears);

    // folder containing currently played song
    folderActual = new FolderSorted(folderListView, QString(tr("Playlist")), &(model->songsToPlay));

    // folder containing songs played in this session
    folderSongsPlayed = new Folder(folderListView, QString(tr("Songs Played")), &(model->songsPlayed));
    folderSongsPlayed->moveItem(folderCategories);

    // folder containing history
    folderHistory = new Folder(folderListView, QString(tr("History")), &(model->songHistory));
    folderHistory->moveItem(folderSongsPlayed);

    // folder containing unclassified songs
    folderUnclassified = new Folder(folderListView, QString(tr("Unclassified")), &(model->unclassifiedSongs));

    folderSearchResults = new Folder( folderListView, QString(tr("Search Results")), &searchResults );
    folderSearchResults->moveItem(folderActual);

    folderProblematic = new Folder( folderListView, QString(tr("Problematic Songs")) );
    folderProblematic->moveItem(folderUnclassified);

    folderRecentAdditions = new Folder(folderListView, QString(tr("Recent Additions")), &(model->recentSongs));
    folderRecentAdditions->moveItem(folderUnclassified);

    // signals of folders
    connect(folderCategories, SIGNAL( CategoryNew() ), this, SLOT(newCategory()));
    connect(folderCategories, SIGNAL( CategoryRemoved() ), this, SLOT(removeCategory()));
    connect(folderCategories, SIGNAL( CategoryRenamed() ), this, SLOT(renameCategory()));
    connect(folderCategories, SIGNAL( LoadM3uIntoCategory() ), this, SLOT(loadM3uIntoCategory()));
}


bool YammiGui::setupActions( ) {
    KStdAction::quit(this, SLOT(close()), actionCollection());

    //Selection actions
    KStdAction::selectAll(this,SLOT(selectAll()),actionCollection());
    new KAction(tr("Invert selection"),0,0,this,SLOT(invertSelection()), actionCollection(),"invert_selection");

    //Media player actions
    m_playPauseAction = new KAction(tr("Play"), "player_play", KShortcut(Key_F1), this, SLOT(playPause()), actionCollection(),"play_pause");
    new KAction(tr("Stop"), "player_stop", KShortcut(Key_F4), this, SLOT(stop()), actionCollection(), "stop");
    new KAction(tr("Skip Backward"),"player_rew",KShortcut(Key_F2),this,SLOT(skipBackward()), actionCollection(),"skip_backward");
    new KAction(tr("Skip Forward"),"player_fwd",KShortcut(Key_F3),this,SLOT(skipForward()), actionCollection(),"skip_forward");
    m_seekSlider = new TrackPositionSlider( QSlider::Horizontal, 0L, "seek_slider");
    m_seekSlider->setFixedWidth(200);
    QToolTip::add
        (m_seekSlider, tr("Track position"));
    connect(m_seekSlider,SIGNAL(sliderMoved(int)),this,SLOT(seek(int)));
    connect(m_seekSlider,SIGNAL(myWheelEvent(int)),this,SLOT(seekWithWheel(int)));
    new KWidgetAction( m_seekSlider ,"text",0, 0, 0,actionCollection(),"seek");

    //Database actions
    new KAction(tr("Save Database"),"filesave",KShortcut(QKeySequence(Key_Control,Key_S)),this,SLOT(saveDatabase()), actionCollection(),"save_db");
    new KAction(tr("Scan Harddisk..."),"fileimport",0,this,SLOT(updateSongDatabaseHarddisk()), actionCollection(),"scan_hd");
    new KAction(tr("Import Selected File(s)..."),"edit_add",0,this,SLOT(updateSongDatabaseSingleFile()), actionCollection(),"import_file");
    new KAction(tr("Check Consistency..."),"spellcheck",0,this,SLOT(forAllCheckConsistency()), actionCollection(),"check_consistency_all");
    new KAction(tr("Fix genres..."),0,0,this,SLOT(fixGenres()), actionCollection(),"fix_genres");

    // playlist actions
    new KAction(tr("Shuffle Playlist..."),"roll",0,this,SLOT(shufflePlaylist()),actionCollection(),"shuffle_playlist");
    new KAction(tr("Clear Playlist"),"edittrash",KShortcut(QKeySequence(Key_Shift,Key_F8)),this,SLOT(clearPlaylist()),actionCollection(),"clear_playlist");
    new KAction(tr("Switch to/from Playlist"),"toggle_playlist",KShortcut(QKeySequence(Key_Control,Key_P)),this,SLOT(toFromPlaylist()),actionCollection(),"toggle_playlist");

    // selection actions
    new KAction(tr("Enqueue as next (prepend)"), "enqueue_asnext", KShortcut(Key_F6), this, SLOT(forSelectionEnqueueAsNext()), actionCollection(), "prepend_selected");
    new KAction(tr("Enqueue at end (append)"), "enqueue", KShortcut(Key_F5), this, SLOT(forSelectionEnqueue()), actionCollection(), "append_selected");
    new KAction(tr("Play Now!"), "play_now", KShortcut(Key_F7), this, SLOT(forSelectionPlayNow()), actionCollection(), "play_selected");
    new KAction(tr("Dequeue Songs"), "stop", KShortcut(Key_F8), this, SLOT(forSelectionDequeue()), actionCollection(), "dequeue_selected");
    new KAction(tr("Song Info..."), "info", KShortcut(Key_I), this, SLOT(forSelectionSongInfo()), actionCollection(), "info_selected");
    new KAction(tr("Delete Song..."), 0, 0, this, SLOT(forSelectionDelete()), actionCollection(), "delete_selected");
    new KAction(tr("Check Consistency..."),"spellcheck",0,this,SLOT(forSelectionCheckConsistency()), actionCollection(),"check_consistency");
    new KAction(tr("Move Files"), 0, 0, this, SLOT(forSelectionMove()), actionCollection(), "move_selected");
    new KAction(tr("Search for similar entry"), 0, 0, this, SLOT(searchForSimilarEntry()), actionCollection(), "search_similar_entry");
    new KAction(tr("Search for similar artist"), 0, 0, this, SLOT(searchForSimilarArtist()), actionCollection(), "search_similar_artist");
    new KAction(tr("Search for similar title"), 0, 0, this, SLOT(searchForSimilarTitle()), actionCollection(), "search_similar_title");
    new KAction(tr("Search for similar album"), 0, 0, this, SLOT(searchForSimilarAlbum()), actionCollection(), "search_similar_album");
    new KAction(tr("Goto artist"), 0, 0, this, SLOT(gotoFolderArtist()), actionCollection(), "goto_artist_folder");
    new KAction(tr("Goto album"), 0, 0, this, SLOT(gotoFolderAlbum()), actionCollection(), "goto_album_folder");
    new KAction(tr("Goto genre"), 0, 0, this, SLOT(gotoFolderGenre()), actionCollection(), "goto_genre_folder");
    new KAction(tr("Goto year"), 0, 0, this, SLOT(gotoFolderYear()), actionCollection(), "goto_year_folder");

    // autoplay actions
    KToggleAction *ta;
    m_autoplayActionOff = new KRadioAction(tr("Off"),0,0,this,SLOT(autoplayOff()),actionCollection(),"autoplay_off");
    m_autoplayActionOff->setExclusiveGroup("autoplay");
    m_autoplayActionLnp = new KRadioAction(tr("Longest not played"),0,0,this,SLOT(autoplayLNP()),actionCollection(),"autoplay_longest");
    m_autoplayActionLnp->setExclusiveGroup("autoplay");
    m_autoplayActionRandom = new KRadioAction(tr("Random"),0,0,this,SLOT(autoplayRandom()),actionCollection(),"autoplay_random");
    m_autoplayActionRandom->setExclusiveGroup("autoplay");
    m_currentAutoPlay = new KAction(tr("Unknown"),0,0,0,0,actionCollection(),"autoplay_folder");

    // toggle toolbar actions
    ta = new KToggleAction("Main ToolBar",0,0,this,SLOT(toolbarToggled()),actionCollection(),"MainToolbar");
    ta = new KToggleAction("Media Player",0,0,this,SLOT(toolbarToggled()),actionCollection(),"MediaPlayerToolbar");
    ta = new KToggleAction("Song Actions",0,0,this,SLOT(toolbarToggled()),actionCollection(),"SongActionsToolbar");

    // other actions
    new KAction(tr("Update Automatic Folder Structure"),"reload",0,this,SLOT(updateView()),actionCollection(),"update_view");

    // setup
    KStdAction::preferences(this,SLOT(setPreferences()),actionCollection());
    KStdAction::keyBindings(this,SLOT(configureKeys()),actionCollection());

    // - "insert custom widgets in toolbar using KWidgetAction
    //search
    QHBox *w = new QHBox( );
    new QLabel(tr("Search:"),w);
    m_searchField = new LineEditShift(w);
    m_searchField->setFixedWidth(175);
    QToolTip::add
        ( m_searchField, tr("Fuzzy search (Ctrl-F)"));
    connect( m_searchField, SIGNAL(textChanged(const QString&)), SLOT(searchFieldChanged(const QString&)));
    /*
        TODO: temporarily disabled before we have a better concept and have it documented properly...
        QPushButton *btn = new QPushButton(tr("to wishlist"),w);
        connect( btn, SIGNAL( clicked() ), this, SLOT( addToWishList() ) );
        QToolTip::add( btn, tr("Add this entry to the database as a \"wish\""));
    */
    new KWidgetAction(w, "Search", 0, 0, 0, actionCollection(), "search");

    // the rc file must be installed (eg. in /opt/kde3/share/apps/yammi/yammiui.rc)
    QString curDir = QDir::currentDirPath();
    KGlobal::dirs()->addResourceDir("data", curDir); // to find yammiui.rc e.g. when developing
    createGUI();
    // test whether file was found and loaded
    QPopupMenu* anyMenu = (QPopupMenu *)factory()->container("player", this);
    if(anyMenu == 0) {
        qError() << "you must have the file 'yammiui.rc' installed (eg. in /opt/kde3/share/apps/yammi/yammiui.rc)";
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
    mainMenu->insertItem( tr("Columns"), columnsMenu, -1, 3 );
}

/**
 * Creates the song popup menu from the xml gui framework.
 * Also calls updateSongPopup to populate the submenus for categories and plugins.
 * These have to be updated with updateSongPopup on each change in existing categories
 * or plugins.
 */
void YammiGui::createSongPopup() {
    qDebug() << "creating song popup";
    songPopup = (QPopupMenu *)factory()->container("song_popup", this);
    songGoToPopup = (QPopupMenu *)factory()->container("goto", this);
    if(songPopup == 0 || songGoToPopup == 0) {
        qFatal() << "yammiui.rc not installed correctly!!!";
        return;
    }
    songPopup->insertItem( "", 113, 0);
    songPopup->insertSeparator(1);
    playListPopup = new QPopupMenu(songPopup);
    songPopup->insertItem( tr("Insert Into/Remove From..."), playListPopup, -1, -1);
    pluginPopup = new QPopupMenu(songPopup);
    songPopup->insertItem( tr("Plugins..."), pluginPopup, -1, -1);
    // populate the submenus
    updateSongPopup();
}

/**
 * Tells the media player to jump with the playback to the given position
 * within the currently played song.
 * @param pos time in milliseconds
 */
void YammiGui::seek( int pos ) {
//    qDebug() << "seek song to pos " << pos;
    player->jumpTo(pos);
}


void YammiGui::seekWithWheel(int rotation) {
    qDebug() << "seekWithWheel() called";
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


/*     (Remote)-functions     */
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
