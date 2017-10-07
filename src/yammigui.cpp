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

#ifdef USE_TAGLIB
#include <id3v1genres.h>
#endif

#include <QActionGroup>
#include <QCheckBox>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMenuBar>
#include <QPixmap>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QSplitter>
#include <QTextEdit>
#include <QTextStream>
#ifdef USE_QDBUS
#include <QtDBus>
#endif
#include <QToolBar>
#include <QToolTip>

#include "ConsistencyCheckParameter.h"
#include "folder.h"
#include "foldercategories.h"
#include "foldergroups.h"
#include "foldermodel.h"
#include "foldersorted.h"
#include "fuzzsrch.h"
#include "dummyplayer.h"
#include "mediaplayer.h"
#include "mylistview.h"
#include "prefs.h"
#include "qmediaplayer-engine.h"
#include "searchthread.h"
#include "song.h"
#include "songentry.h"
#include "songentryint.h"
#include "songentrystring.h"
#include "songentrytimestamp.h"
#include "songinfo.h"
#include "trackpositionslider.h"
#include "util.h"
#include "vlc-engine.h"
#include "xine-engine.h"
#include "yammilcdnumber.h"
#include "yammimodel.h"

// dialog includes
#include "applytoalldialog.h"
#include "ConsistencyCheckDialog.h"
#include "ui_DeleteDialog.h"
#include "preferencesdialog.h"
#include "updatedatabasedialog.h"

#ifdef Q_OS_WIN32
#undef DeleteFile
/* some include pulls windows headers in which define DeleteFile to DeleteFileW/A - then the Song::DeleteFile could not be found anymore */
#endif


extern YammiGui* gYammiGui;

class DeleteDialog : public QDialog, public Ui::DeleteDialog
{
public:
    DeleteDialog(QWidget *parent)
        : QDialog(parent)
    {
        setupUi(this);
    };
};

QIcon loadAndConvertIconToGrayScale(QString path)
{
    QImage image(path);
    QRgb col;
    int gray;
    int width = image.width();
    int height = image.height();
    for (int i = 0; i < width; ++i)
    {
        for (int j = 0; j < height; ++j)
        {
            col = image.pixel(i, j);
            if (qAlpha(col)) {
                gray = qGray(col);
                image.setPixel(i, j, qRgb(gray, gray, gray));
            }
        }
    }
    return QIcon(QPixmap().fromImage(image));
}


/////////////////////////////////////////////////////////////////////////////////////////////
YammiGui::YammiGui() : QMainWindow( ) {
    currentSongStarted = new MyDateTime();
    gYammiGui = this;
    setGeometry(0, 0, 800, 600);

    setWindowIcon(QIcon("icons:yammi.png"));

    prelistenProcess = new QProcess;

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

    createMainWidget( );
    setupActions();
    createFolders( );
    createToolbars();
    createMenuBar();
    createTrayIcon();

    if (config()->thisIsSecondYammi) {
        setWindowIcon(loadAndConvertIconToGrayScale("icons:yammi.png"));
        qApp->setStyleSheet("QTreeWidget, QListView, MyListView, QTextEdit#playlistPart { background-color: #404040; color: white }");
    }

    // final touches before start up
    readOptions( );

    // from here: stuff that needs the options to be read already
    validState = true;

    updateHtmlPlaylist();
    /* only to update playlistPart. For some reason the background is only drawn
       when the image was already loaded as normal image inside the playlistPart
       (see playqueueTemplate) */

    trayIcon->show();
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



void YammiGui::loadDatabase() {
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

    qDebug() << "trying to load database from directory " << config()->databaseDir;

    model->readSongDatabase();
    // finish initialization of player
    //player->syncPlayer2Yammi(&(model->songsToPlay));
    QSettings cfg;
    cfg.beginGroup("General Options");

    SongKeyMap map;
    model->createSongKeyMap(&model->allSongs, &map);

    model->readCategories(&map);
    model->readHistory(&map);
    player->finishInitialization();

    bool restorePlaylistOnStartup = true;            // TODO: make this configurable
    if(restorePlaylistOnStartup) {
        model->readList(&(model->songsToPlay), config()->databaseDir + "playqueue.xml", &map);
        folderActual->update(folderActual->songlist());
        player->syncYammi2Player();
        if(folderActual->songlist().count() > 0) {
            m_seekSlider->setupTickmarks(folderActual->firstSong());
            if (!config()->thisIsSecondYammi) {
                int savedSongPosition = cfg.value("savedSongPosition", 0).toInt();
                if (savedSongPosition != 0) {
                    player->jumpTo(savedSongPosition);
                }
                int savedPlayingStatus = cfg.value("savedPlayingStatus", STOPPED).toInt();
                if (savedPlayingStatus == PLAYING && player->getStatus() != PLAYING) {
                    player->play();
                }
            }
        }
    }
    // update dynamic folders based on database contents
    updateView(true);

    Folder* f=getFolderByName(cfg.value("CurrentFolder").toString());
    if(f != 0) {
        changeToFolder(f, true);
    } else {
        changeToFolder(folderAll, true);
    }
    folderContentChanged(folderActual);
    
    checkTimer.setSingleShot(false);
    checkTimer.start(100);
    regularTimer.setSingleShot(false);
    regularTimer.start(500);
    searchResultsTimer.setSingleShot(false);
    searchResultsTimer.start(10);
    cfg.endGroup();
}

void YammiGui::saveOptions() {

    if (config()->thisIsSecondYammi)
        return;

    qDebug() << "saveOptions() ";

    QSettings cfg;

    cfg.setValue("geometry", saveGeometry());
    cfg.setValue("windowState", saveState());
    cfg.setValue("splitterHorizontal", centralWidget->saveState());
    cfg.setValue("splitterVertical", leftWidget->saveState());

    cfg.beginGroup("General Options");
    cfg.setValue("CurrentFolder", chosenFolder->folderName());
    cfg.setValue("savedSongPosition", player->getCurrentTime());
    cfg.setValue("savedPlayingStatus", player->getStatus());

    foreach (QAction* action, m_actionGroupColumnVisibility->actions()) {
        QVariant property = action->property("column");
        if (property.isValid()) {
            int column = property.toInt();
            cfg.setValue(QString("Column%1Visible").arg(column), action->isChecked());
        }
    }

    cfg.setValue( "columnsState" , songListView->horizontalHeader()->saveState());

    cfg.setValue("AutoplayFolder", autoplayFoldername);
    cfg.setValue("AutoplayMode", autoplayMode);
    
    cfg.endGroup();
}


void YammiGui::readOptions() {
    qDebug() << "readOptions()";

    QSettings cfg;

    restoreGeometry(cfg.value("geometry").toByteArray());
    restoreState(cfg.value("windowState").toByteArray());
    centralWidget->restoreState(cfg.value("splitterHorizontal").toByteArray());
    leftWidget->restoreState(cfg.value("splitterVertical").toByteArray());

    QAction* a;
    a = findChild<QAction*>("MainToolbar");
    if (a) a->setChecked(!findChild<QToolBar*>("MainToolbar")->isHidden());

    a = findChild<QAction*>("MediaPlayerToolbar");
    if (a) a->setChecked(!findChild<QToolBar*>("MediaPlayerToolbar")->isHidden());

    a = findChild<QAction*>("TimeDisplayToolbar");
    if (a) a->setChecked(!findChild<QToolBar*>("TimeDisplayToolbar")->isHidden());

    a = findChild<QAction*>("SongActionsToolbar");
    if (a) a->setChecked(!findChild<QToolBar*>("SongActionsToolbar")->isHidden());

    a = findChild<QAction*>("PrelistenToolbar");
    if (a) a->setChecked(!findChild<QToolBar*>("PrelistenToolbar")->isHidden());

    cfg.beginGroup("General Options");


    foreach (QAction* action, m_actionGroupColumnVisibility->actions()) {
        QVariant property = action->property("column");
        if (property.isValid()) {
            int column = property.toInt();
            bool columnVisible = cfg.value(QString("Column%1Visible").arg(column), true).toBool();
            action->setChecked(columnVisible);
        }
    }

    QByteArray columnsState = cfg.value("columnsState").toByteArray();
    if (!columnsState.isEmpty()) {
        songListView->horizontalHeader()->restoreState(columnsState);
    }

    autoplayFoldername=cfg.value("AutoplayFolder", tr("All Music")).toString();
    m_actionCurrentAutoPlay->setText(tr("Folder: ")+autoplayFoldername);
    autoplayMode=cfg.value( "AutoplayMode", AUTOPLAY_OFF).toInt();
    switch(autoplayMode) {
    case AUTOPLAY_OFF:
        m_actionAutoplayOff->setChecked(true);
        break;
    case AUTOPLAY_LNP:
        m_actionAutoplayLnp->setChecked(true);
        break;
    case AUTOPLAY_RANDOM:
        m_actionAutoplayRandom->setChecked(true);
        break;
    }

    cfg.endGroup();
}


void YammiGui::closeEvent(QCloseEvent *event)
{
    if (queryClose()) {
        queryExit();
        event->accept();
    } else {
        event->ignore();
    }
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
    }

    if (config()->thisIsSecondYammi)
        return true;

    if(config()->logging && model->songsPlayed.count()>2) {
        model->saveHistory();
    }
    // save playlist
    model->saveList(&(model->songsToPlay), config()->databaseDir, "playqueue");    
    return true;
}

bool YammiGui::queryExit() {
    qDebug() << "queryExit() ";
    if (!config()->thisIsSecondYammi) {
        saveOptions();
    }
    player->quit( );
    return true;
}


void YammiGui::toolbarToggled(QAction* action)
{
    QToolBar* toolBar = findChild<QToolBar*>(action->objectName());

    if (!toolBar) {
        qDebug() << "toolbarToggled called without named action.";
        return;
    } else {
        qDebug() << "toolbarToggled name=" << action->objectName();

        if(!action->isChecked()) {
            toolBar->hide();
        } else {
            toolBar->show();
        }
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
    delete prelistenProcess;
    delete currentSongStarted;
}


/**
 * This slot is called on changes in the playlist (model->songsToPlay),
 * signalled by the mediaplayer or on changes from within yammigui (enqueing, dequeing songs, ...)
 */
void YammiGui::updatePlaylist() {
    //qDebug() << "updatePlaylist called";
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
    for(int i=1; i<folderActual->songlist().count(); i++) {
        length += folderActual->songlist().at(i)->song()->length;
    }
    QString formattedTime;
    formattedTime.sprintf("%d:%02d", length/(60*60), (length % (60*60))/60);
    htmlTemplate.replace(QRegExp("\\{noSongsToPlay\\}"), QString("%1").arg(noSongsToPlay));
    htmlTemplate.replace(QRegExp("\\{timeToPlay\\}"), formattedTime);

    if (config()->thisIsSecondYammi) {
        htmlTemplate.replace("\"icons:playlistbackground.jpg\"", "\"\"");
        htmlTemplate.replace("#0000ff", "#9999ff");
        htmlTemplate.replace("#00ff00", "#99ff99");
        htmlTemplate.replace("#ff0000", "#ff9999");
    }
    
    QString htmlSource("");
    
    QStringList entries = htmlTemplate.split("{scope:", QString::SkipEmptyParts, Qt::CaseInsensitive);
    
    for ( QStringList::Iterator it = entries.begin(); it != entries.end(); ++it ) {
        QString entry = *it;
        int pos = entry.indexOf('}');
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
        Song* lastLogged=((SongEntry*)model->songsPlayed.last())->song();
        if(lastLogged==lastSong) {
            return;
        }
    }
    MyDateTime now;
    SongEntryTimestamp* entry=new SongEntryTimestamp(lastSong, currentSongStarted);
    lastSong->lastPlayed=entry->timestamp;
    folderSongsPlayed->addEntry(entry);		// append to songsPlayed
}

/**
 * Called when a new song is played: updates title bar, songSlider
 */
void YammiGui::handleNewSong(Song* newSong) {
    currentSong=newSong;
    if(newSong==0) {
        setWindowTitle(tr("Yammi - not playing"));
        m_seekSlider->setupTickmarks(0);
        return;
    }
    // TODO: take swapped file?
    *currentSongStarted = currentSongStarted->currentDateTime();

    setWindowTitle("Yammi: "+currentSong->displayName());
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
    QStyle* style = QApplication::style();
    int status = player->getStatus();
    if(status == PLAYING) {
        m_actionPlayPause->setIcon(style->standardIcon(QStyle::SP_MediaPause));
        m_actionPlayPause->setText(tr("Pause"));
    } else {
        m_actionPlayPause->setIcon(style->standardIcon(QStyle::SP_MediaPlay));
        m_actionPlayPause->setText(tr("Play"));
    }

}


QString YammiGui::getColumnName(int column) {
    QString ret;
    switch (column) {
    case FolderModel::COLUMN_POS:            ret = tr("Pos"); break;
    case FolderModel::COLUMN_PLAYED_ON:      ret = tr("Played on"); break;
    case FolderModel::COLUMN_MATCH:          ret = tr("Match"); break;
    case FolderModel::COLUMN_REASON:         ret = tr("Reason"); break;
    case FolderModel::COLUMN_ARTIST:         ret = tr("Artist"); break;
    case FolderModel::COLUMN_TITLE:          ret = tr("Title"); break;
    case FolderModel::COLUMN_ALBUM:          ret = tr("Album"); break;
    case FolderModel::COLUMN_LENGTH:         ret = tr("Length"); break;
    case FolderModel::COLUMN_YEAR:           ret = tr("Year"); break;
    case FolderModel::COLUMN_TRACKNR:        ret = tr("TrackNr"); break;
    case FolderModel::COLUMN_GENRE:          ret = tr("Genre"); break;
    case FolderModel::COLUMN_ADDED_TO:       ret = tr("AddedTo"); break;
    case FolderModel::COLUMN_BITRATE:        ret = tr("Bitrate"); break;
    case FolderModel::COLUMN_FILENAME:       ret = tr("Filename"); break;
    case FolderModel::COLUMN_PATH:           ret = tr("Path"); break;
    case FolderModel::COLUMN_COMMENT:        ret = tr("Comment"); break;
    case FolderModel::COLUMN_LAST_PLAYED:    ret = tr("Last Played"); break;
    case FolderModel::COLUMN_EXTENSION:      ret = tr("Extension"); break;
    default:
        break;
    }

    return ret;
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
        fs.initialize(searchStr.toLower().toLatin1(), 2, 4);			// STEP 1


        for (int i = 0; i < folderListView->topLevelItemCount(); i++) {
            QTreeWidgetItem* item = folderListView->topLevelItem(i);
            Folder* f = (Folder*)item;
            fs.checkNext(f->folderName().toLower().toLatin1(), (void*)f);				// STEP 2 (top-level folder)
            for (int j = 0; j < item->childCount(); j++) {
                QTreeWidgetItem* item2 = item->child(j);
                Folder* f2 = (Folder*)item2;
                fs.checkNext(f2->folderName().toLower().toLatin1(), (void*)f2);				// STEP 2 (subfolders)
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
    for (int i = 0; i < folderListView->topLevelItemCount(); i++) {
        QTreeWidgetItem* item = folderListView->topLevelItem(i);
        Folder* f = (Folder*)item;
        if(f->folderName()==folderName) {
            return f;
        }
        for (int j = 0; j < item->childCount(); j++) {
            QTreeWidgetItem* item2 = item->child(j);
            Folder* f2 = (Folder*)item2;
            if(f2->folderName()==folderName) {
                return f2;
            }
        }
    }
    return 0;
}


/// updates the automatically calculated folders after changes to song database
void YammiGui::updateView(bool startup) {
    for (MyList::iterator it = model->allSongs.begin(); it != model->allSongs.end(); it++) {
        (*it)->song()->classified = false;
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
    for (MyList::iterator it = model->allSongs.begin(); it != model->allSongs.end(); it++) {
        Song* s = (*it)->song();
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
    for (MyList::iterator it = model->allSongs.begin(); it != model->allSongs.end(); it++) {
        Song* s = (*it)->song();
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

/// opens the preferences dialogue
void YammiGui::setPreferences() {
    int playerBefore=config()->mediaPlayer;

    PreferencesDialog d(this, config());

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
    //KKeyDialog::configure(actionCollection());
}

/**
 * Updates the songPopup submenus with available categories and plugins
 */
void YammiGui::updateSongPopup() {
    //qDebug() << "updating song popup";
    songCategoryPopup->clear();

    QAction* action = new QAction(tr("New Category ..."), this);
    action->setIcon(QIcon("icons:newCategory.xpm"));
    action->setProperty("id", QVariant(9999));
    connect(action, SIGNAL(triggered()), this, SLOT(toCategory()));
    songCategoryPopup->addAction(action);

    for(int i=0; i<model->categoryNames.count(); i++) {
        action = new QAction(model->categoryNames[i], this);
        action->setIcon(QIcon("icons:in.xpm"));
        action->setProperty("id", QVariant(10000+i));
        connect(action, SIGNAL(triggered()), this, SLOT(toCategory()));
        songCategoryPopup->addAction(action);
    }
    pluginPopup->clear();
    for(int i=0; i<config()->pluginMenuEntry.count(); i++) {
        action = new QAction(config()->pluginMenuEntry[i], this);
        action->setProperty("pluginIndex", QVariant(2000+i));
        connect(action, SIGNAL(triggered()), this, SLOT(forSelectionPlugin()));
        pluginPopup->addAction(action);
    }
}


/// adds the text in search field to the wishlist
void YammiGui::addToWishList() {
    QString toAdd = m_searchField->text();
    MyDateTime wishDate=wishDate.currentDateTime();
    Song* newSong=new Song(tr("{wish}"), toAdd, "", "", "", 0, 0, wishDate, 0, "", 0, 0);
    folderAll->addSong(newSong);
    // FIXME: selectionMode for custom list?
    //forSong(newSong, Song::SongInfo, NULL);
    model->allSongsChanged(true);
    m_searchField->setText(tr("{wish}"));
    folderContentChanged(folderAll);
    m_searchField->setText("");
}

/**
 * adds all selected songs to the category (specified by index)
 * if current song is already in category => removes all selected from that category (if they are in)
 */
void YammiGui::toCategory() {
    QVariant qv = sender()->property("id");
    if (!qv.isValid())
        return;

    int index = qv.toInt();
    index-=10000;
    if(index==-1) {
        // create new category
        if(!newCategory()) {
            return;
        }
        index=model->allCategories.count()-1;
    }
    // choose the desired category
    MyList* category = model->allCategories.at(index);
    QString chosen=model->categoryNames[index];

    // determine whether all/some/none of the selected songs are already in the chosen category
    int mode=category->containsSelection(&selectedSongs);

    // determine mode (add/remove): we only use remove mode if all selected songs are contained in category
    bool remove
        =(mode==2);


    // get pointer to the folder
    FolderSorted* categoryFolder=0;
    for (int i = 0; i < folderCategories->childCount(); i++) {
        QTreeWidgetItem* f = folderCategories->QTreeWidgetItem::child(i);
        if( ((Folder*)f)->folderName()==chosen ) {
            categoryFolder=(FolderSorted*)f;
        }
    }

    if(categoryFolder==0) {
        qDebug() << "folder not found!";
        return;
    }
    // go through list of songs
    for (MyList::iterator it = selectedSongs.begin(); it != selectedSongs.end(); it++) {
        Song* s = (*it)->song();
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
            folderName = tr("- no artist -");
        }
        break;

    case 2:
        folderName=s->artist+" - "+s->album;
        break;

    case 3:
        folderName=s->genre;
        if(folderName == "") {
            folderName = tr("- no genre -");
        }
        break;

    case 4:
        if(s->year == 0) {
            folderName = tr("- no year -");
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

    // we better reset these settings to not confuse the user with his own changes
    // (sort order and scroll position)
    folderSearchResults->saveScrollPos(0, 0);
    folderSearchResults->saveSorting(FolderModel::COLUMN_MATCH, Qt::DescendingOrder);
    changeToFolder(folderSearchResults);

    songListView->scrollToTop(); // set scroll position to top
    songListView->selectionModel()->select(songListView->model()->index(0, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    searchResultsUpdateNeeded=false;
}

/**
 * user clicked on a folder
 */
void YammiGui::slotFolderChanged(const QModelIndex& /*current*/, const QModelIndex& /*previous*/) {
    QTreeWidgetItem *i = folderListView->currentItem();
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

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        // TODO: history of visited folders, something like:
        //visitedFoldersHistory->add(chosenFolder);

        chosenFolder->saveSorting(songListView->sortedBy(), songListView->sortOrder());
        chosenFolder->saveScrollPos(songListView->horizontalScrollBar()->value(), songListView->verticalScrollBar()->value());
    }

    // now we really change to new folder
    chosenFolder = newFolder;

    if(chosenFolder==folderActual) {
        songListView->dontTouchFirst=true;				// don't allow moving the first
    } else {
        songListView->dontTouchFirst=false;
    }

    folderListView->setCurrentItem(chosenFolder);
    chosenFolder->QTreeWidgetItem::setSelected(true);
    folderListView->scrollTo(folderListView->currentIndex());
    folderContentChanged();
    QApplication::restoreOverrideCursor();
}


void YammiGui::folderContentChanged(Folder* folder) {
    if (!folder) {
        if(chosenFolder) {

            //qDebug() << "adding folder content of folder " << chosenFolder->folderName();
            folderToAdd = chosenFolder;
            songListView->setSorting(true, chosenFolder->getSavedSorting(), chosenFolder->getSavedSortOrder());

            if(chosenFolder == folderActual) {
                updateCurrentSongStatus();
            }
        }
    } else {
        if(folder==chosenFolder) {
            folderContentChanged();
        } else {
            if(folder==folderActual) {
                updateCurrentSongStatus();
            }
            if(folder!=folderSearchResults) {
                m_acceptSearchResults=false;
            }
        }
    }

    QHeaderView* header = songListView->horizontalHeader();
    bool columnVisible;

    columnVisible = ( (chosenFolder == folderActual) ||
                      (chosenFolder != NULL && chosenFolder->QTreeWidgetItem::parent() == folderCategories) );
    header->setSectionHidden(FolderModel::COLUMN_POS, !columnVisible);

    columnVisible = ( (chosenFolder == folderHistory) ||
                      (chosenFolder == folderSongsPlayed) );
    header->setSectionHidden(FolderModel::COLUMN_PLAYED_ON, !columnVisible);

    columnVisible = chosenFolder == folderSearchResults;
    header->setSectionHidden(FolderModel::COLUMN_MATCH, !columnVisible);

    columnVisible = chosenFolder == folderProblematic;
    header->setSectionHidden(FolderModel::COLUMN_REASON, !columnVisible);

    foreach (QAction* action, m_actionGroupColumnVisibility->actions()) {
        QVariant property = action->property("column");
        if (property.isValid()) {
            int column = property.toInt();
            columnVisible = action->isChecked();
            header->setSectionHidden(column, !columnVisible);
        }
    }

    songListViewModel->reset();
    if (songListView && chosenFolder) {
        songListView->scroll(chosenFolder->getScrollPosX(), chosenFolder->getScrollPosY());
    }

    updateHtmlPlaylist();
}

void YammiGui::slotLoadInMixxxDeck1()
{
    loadSelectedSongInMixxxDeck(1);
}

void YammiGui::slotLoadInMixxxDeck2()
{
    loadSelectedSongInMixxxDeck(2);
}

void YammiGui::loadSelectedSongInMixxxDeck(int deckNumber)
{
#ifdef USE_QDBUS
    bool doLoad = true;

    getSelectedSongs();
    if (selectedSongs.count() != 1) {
        doLoad = false;
    }

    QDBusInterface iface("org.mixxx", "/PlayerManager", "", QDBusConnection::sessionBus());
    if (doLoad) {
        if (iface.isValid()) {
            QDBusReply<bool> reply = iface.call("slotIsDeckPlaying", deckNumber);
            if (reply.isValid()) {
                qDebug() << "Reply to slotIsDeckPlaying was:" << reply.value();
                if (reply.value() == true) {
                    switch (QMessageBox::warning(this,
                                                 tr("Load Selected Song In Mixxx Deck"),
                                                 tr("The deck in Mixxx is currently playing. Do you really want to load this song?"),
                                                 QMessageBox::Yes,
                                                 QMessageBox::No | QMessageBox::Default | QMessageBox::Escape))
                    {
                        case QMessageBox::Yes:
                            doLoad = true;
                            break;
                        case QMessageBox::No:
                        default:
                            doLoad = false;
                    }
                }
            } else {
                qDebug() << "Call to slotIsDeckPlaying failed:" << qPrintable(reply.error().message());
            }
        } else {
            qDebug() << "QDBusInterface failed:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }
    }

    if (doLoad) {
        QString file = selectedSongs.first()->song()->location();
        QDBusReply<void> reply = iface.call("slotLoadToDeck", file, deckNumber);
        if (reply.isValid()) {
            qDebug() << "Call to slotLoadToDeck succeeded";
        } else {
            qDebug() << "Call to slotLoadToDeck failed:" << qPrintable(reply.error().message());
        }
    }
#else
    Q_UNUSED(deckNumber);
#endif /*USE_QDBUS*/
}

void YammiGui::slotLoadSelectedSongInMixxxAutoDJ()
{
#ifdef USE_QDBUS
    bool doLoad = true;

    getSelectedSongs();
    if (selectedSongs.count() < 1) {
        doLoad = false;
    }

    QDBusInterface iface("org.mixxx", "/AutoDJFeature", "", QDBusConnection::sessionBus());
    if (doLoad) {
        if (iface.isValid()) {
            for (MyList::iterator it = selectedSongs.begin(); it != selectedSongs.end() && doLoad; it++) {
                SongEntry* s = *it;
                QString file = s->song()->location();
                QDBusReply<void> reply = iface.call("enqueueInAutoDJ", file);
                if (reply.isValid()) {
                    qDebug() << "Call to enqueueInAutoDJ succeeded";
                } else {
                    qDebug() << "Call to enqueueInAutoDJ failed:" << qPrintable(reply.error().message());
                    doLoad = false;
                }
            }
        } else {
            qDebug() << "QDBusInterface failed:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }
    }

#else
    Q_UNUSED(deckNumber);
#endif /*USE_QDBUS*/
}

void YammiGui::slotEnqueueAsNextInOtherYammi()
{
#ifdef USE_QDBUS
    getSelectedSongs();
    if (selectedSongs.count() == 1) {

        QDBusInterface iface(config()->getDBusServiceOtherYammi(), config()->getDBusPathOtherYammi(), "", QDBusConnection::sessionBus());

        QString file = selectedSongs.first()->song()->location();
        QDBusReply<void> reply = iface.call("slotEnqueueAsNextByLocation", file);
        if (reply.isValid()) {
            qDebug() << "Call to slotEnqueueAsNextByLocation succeeded";
        } else {
            qDebug() << "Call to slotEnqueueAsNextByLocation failed:" << qPrintable(reply.error().message());
        }
    }
#endif /*USE_QDBUS*/
}

void YammiGui::slotEnqueueAsNextByLocation(QString location)
{
    Song* s = model->getSongFromFilename(location);
    if (s) {
        if(model->songsToPlay.count()==0 || currentSong!=model->songsToPlay.at(0)->song() || player->getStatus() != PLAYING) {
            model->songsToPlay.insert(0, new SongEntryInt(s, 13));
        } else {
            model->songsToPlay.insert(1, new SongEntryInt(s, 13));
        }
    } else {
        qDebug() << "Song not in database" << location;
    }

    folderActual->correctOrder();
    player->syncYammi2Player();
    folderContentChanged(folderActual);
}

/// user clicked on a song
void YammiGui::slotSongChanged() {}


/// rmb on songlist: song popup for selection
void YammiGui::songListPopup(const QPoint& pos)
{
    QPoint globalPos = songListView->viewport()->mapToGlobal(pos);
    getSelectedSongs();
    doSongPopup(globalPos);
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
    m_actionSongPopupHeader->setText(label);

    // for each category: determine whether all, some or no songs of selection are contained
    int k=0;
    for (QList<MyList*>::iterator it = model->allCategories.begin(); it != model->allCategories.end(); it++, k++) {
        MyList* category = *it;

        QAction* actionToChange = NULL;
        foreach (QAction* action, songCategoryPopup->actions()) {
            QVariant qv = action->property("id");
            if (qv.isValid()) {
                if (qv.toInt() == (10000 + k)) {
                    actionToChange = action;
                }
            }
        }
        if (actionToChange) {
            int mode=category->containsSelection(&selectedSongs);
            switch(mode) {
            case 0:
                actionToChange->setIcon(QIcon("icons:notin.xpm"));
                break;
            case 1:
                actionToChange->setIcon(QIcon("icons:some_in.xpm"));
                break;
            case 2:
                actionToChange->setIcon(QIcon("icons:in.xpm"));
                break;
            }
        }
    }

    // for songs not on local harddisk: disable certain menu entries
    // only if exactly one song selected!
    bool enable=true;
    if(selected==1 && first->filename=="") {
        enable=false;
    }

    m_actionPlayNow->setEnabled(enable);
    m_actionDequeueSong->setEnabled(enable);
    m_actionPrelistenStart->setEnabled(enable);
    m_actionPrelistenMiddle->setEnabled(enable);
    m_actionPrelistenEnd->setEnabled(enable);
    m_actionOpenFolderInFilemanager->setEnabled(enable);
    m_actionCheckConsistencySelection->setEnabled(enable);
    m_actionMoveFiles->setEnabled(enable);

    bool isMixxxRunning = false;
    bool isMixxxAutoDJRunning = false;
    bool isOtherYammiRunning = false;
#ifdef USE_QDBUS
    getSelectedSongs();
    if (selectedSongs.count() == 1) {
        QDBusInterface iface("org.mixxx", "/PlayerManager", "", QDBusConnection::sessionBus());
        if (iface.isValid()) {
            isMixxxRunning = true;
        } else {
            qDebug() << "QDBusInterface failed:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }
    }
    if (selectedSongs.count() >= 1) {
        QDBusInterface iface("org.mixxx", "/AutoDJFeature", "", QDBusConnection::sessionBus());
        if (iface.isValid()) {
            isMixxxAutoDJRunning = true;
        } else {
            qDebug() << "QDBusInterface failed:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }
    }
    if (selectedSongs.count() == 1) {

        QDBusInterface iface(config()->getDBusServiceOtherYammi(), config()->getDBusPathOtherYammi(), "", QDBusConnection::sessionBus());

        if (iface.isValid()) {
            isOtherYammiRunning = true;
        } else {
            qDebug() << "QDBusInterface failed:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }
    }
#endif
    m_actionLoadInMixxxDeck1->setVisible(isMixxxRunning);
    m_actionLoadInMixxxDeck2->setVisible(isMixxxRunning);
    m_actionLoadInMixxxAutoDJ->setVisible(isMixxxAutoDJRunning);
    m_actionEnqueueAsNextInOtherYammi->setVisible(isOtherYammiRunning);
}


/**
 * Popup menu on a folder
 */
void YammiGui::slotFolderPopup(const QPoint& point)
{
    QPoint globalPoint = folderListView->viewport()->mapToGlobal(point);

    folderListView->setCurrentIndex(folderListView->indexAt(point));

    QTreeWidgetItem *i = folderListView->currentItem();
    Folder* chosenFolder = ( Folder* )i;
    if (i) {
        setSelectionMode(SELECTION_MODE_FOLDER);
        getSelectedSongs();
        if(selectedSongs.count()==0) {
            // no songs in this folder
            chosenFolder->popup(globalPoint, 0);
            setSelectionMode(SELECTION_MODE_USER_SELECTED);
            return;
        }
        adjustSongPopup();
        chosenFolder->popup(globalPoint, songPopup);
        setSelectionMode(SELECTION_MODE_USER_SELECTED);
    }
}



// executes a plugin on a selection of songs
void YammiGui::forSelectionPlugin() {
    QVariant qv = sender()->property("pluginIndex");
    if (!qv.isValid())
        return;

    int pluginIndex = qv.toInt();
    pluginIndex-=2000;

    bool confirm=config()->pluginConfirm[pluginIndex]=="true";
    QString mode=config()->pluginMode[pluginIndex];
    QString cmd=config()->pluginCommand[pluginIndex];

    if(cmd.contains("{directoryDialog}")>0) {
        QString dir = QFileDialog::getExistingDirectory(this, tr("choose directory for plugin"));
        if(dir.isNull())
            return;
        cmd.replace(QRegExp("\\{directoryDialog\\}"), dir);
    }
    if(cmd.contains("{fileDialog}")>0) {
        QString file = QFileDialog::getSaveFileName(this, tr("choose file for plugin"));
        if(file.isNull())
            return;
        cmd.replace(QRegExp("\\{fileDialog\\}"), file);
    }

    while(cmd.contains("{inputString")>0) {
        int startPos = cmd.indexOf("{inputString");
        if ( startPos == -1 ) {
            break;
        }
        int endPos = cmd.indexOf("}", startPos);
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
        QString inputString=QString(QInputDialog::getText(this, prompt, prompt, QLineEdit::Normal, QString(""), &ok ));
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
            for(int i=0; i<sampleCmd.length(); i+=80) {
                msg+=sampleCmd.mid(i, 80)+"\n";
            }
            if (QMessageBox::warning(this, "", msg, QMessageBox::Yes, QMessageBox::No | QMessageBox::Escape) != QMessageBox::Yes) {
                return;
            }
        }
        int index=1;
        QProgressDialog progress(this);
        progress.setLabelText(tr("Executing song plugin cmd ..."));
        progress.setModal(true);
        progress.setRange(0, selectedSongs.count());
        for (MyList::iterator it = selectedSongs.begin(); it != selectedSongs.end(); it++) {
            QString cmd2 = (*it)->song()->replacePlaceholders(cmd, index);
            progress.setValue(index);
            if(progress.wasCanceled()) {
                return;
            }
            system(cmd2.toLatin1());
        }
    }

    if(mode=="group") {
        int index=1;
        QString customList="";
        for (MyList::iterator it = selectedSongs.begin(); it != selectedSongs.end(); it++, index++) {
            QString entry = config()->pluginCustomList[pluginIndex];
            customList += (*it)->song()->replacePlaceholders(entry, index);
        }

        // custom list can be long => we put it into a file...
        QString customListFilename(config()->databaseDir+"customlist.temp");
        QFile customListFile(customListFilename);
        customListFile.open(QIODevice::WriteOnly);
        customListFile.write(customList.toLatin1(), customList.length());
        customListFile.close();
        cmd.replace(QRegExp("\\{customList\\}"), customList);
        cmd.replace(QRegExp("\\{customListFile\\}"), customListFilename);
        cmd.replace(QRegExp("\\{customListViaFile\\}"), "`cat " + customListFilename + "`");

        if(confirm) {
            QString msg=tr("Execute the following command:\n");
            for(int i=0; i<cmd.length(); i+=80) {
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
        system(cmd.toLatin1());
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
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select destination directory"), startPath);
    if(dir.isNull()) {
        return;
    }
    if(dir.right(1)=="/") { 						// strip trailing slash
        dir=dir.left(dir.length()-1);
    }
    for (MyList::iterator it = selectedSongs.begin(); it != selectedSongs.end(); it++) {
        (*it)->song()->moveTo(dir);
    }
}



/** calculate disk usage for a directory (including all subdirectories)
 * returns -1 if too full
 */
long double YammiGui::diskUsage(QString path, long double sizeLimit) {
    QDir d(path);

    d.setFilter(QDir::Files);
    QFileInfoList list = d.entryInfoList();

    long double size=0;

    // step 1: sum up all files
    for (QFileInfoList::iterator it = list.begin(); it != list.end(); ++it ) {
        size+=it->size();
        if(size>sizeLimit) {
            return -1;
        }
    }

    // step 2: recursively sum up subdirectories
    QDir d2(path);
    d2.setFilter(QDir::Dirs);
    QFileInfoList list2 = d2.entryInfoList();

    for (QFileInfoList::iterator it2 = list2.begin(); it2 != list2.end(); ++it2) {
        if(it2->fileName()=="." || it2->fileName()=="..")
            continue;
        double long toAdd=diskUsage(it2->filePath(), sizeLimit);
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

    switch (selectionMode) {
    case SELECTION_MODE_USER_SELECTED:
        {
            // get songs selected in listview
            QAbstractItemModel* model = songListView->model();
            foreach(QModelIndex index, songListView->selectionModel()->selection().indexes()) { // go through list of selected songs
                if (index.column() == 0) {
                    QVariant qv = model->data(index, FolderModel::SongEntryPointerRole);
                    SongEntry* se = SongEntry::qvAsSe(qv);
                    if (se) {
                        selectedSongs.append(se);
                    }
                }
            }
            break;
        }

    case SELECTION_MODE_FOLDER:
        // get songs from currently selected folder: complete folder content
        // if the folder is already completely added to GUI, we take the order as shown in the songlist
        if(folderToAdd == 0) {
            QAbstractItemModel* model = songListView->model();
            QModelIndex index;
            for (int row = 0; (index = model->index(row, 0)).isValid(); row++) {
                QVariant qv = model->data(index, FolderModel::SongEntryPointerRole);
                SongEntry* se = SongEntry::qvAsSe(qv);
                if (se) {
                    selectedSongs.append(se);
                }
            }
        } else {
            // here we are in the middle of adding all songs... (lazy adding) => take folder content directly
            qDebug() << "taking songs directly from list...";
            for (MyList::iterator it = chosenFolder->songlist().begin(); it != chosenFolder->songlist().end(); it++) {
                selectedSongs.appendSong((*it)->song());
            }
        }
        break;

    case SELECTION_MODE_ALL:
        // select all songs in database
        for (MyList::iterator it = model->allSongs.begin(); it != model->allSongs.end(); it++) {
            selectedSongs.appendSong((*it)->song());
        }
        break;

    default:
        break;
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
    ConsistencyCheckDialog d(this, config()->consistencyPara, &selectedSongs, model);
    d.exec();
    config()->saveConfig();
    folderProblematic->update(model->problematicSongs);
    folderContentChanged(folderProblematic);
    folderContentChanged(chosenFolder);
}


void YammiGui::openFolderInFileManager() {
    getSelectedSongs();
    if (selectedSongs.count() >= 1) {
        MyList::iterator it = selectedSongs.begin();
        QString cmd = QString("xdg-open \"%1\"").arg((*it)->song()->path);
        system(cmd.toUtf8());
    }
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
    for (MyList::iterator it = selectedSongs.begin(); it != selectedSongs.end(); it++) {
        model->songsToPlay.append(new SongEntryInt((*it)->song(), 13));
    }
    folderActual->correctOrder();
    player->syncYammi2Player();
    folderContentChanged(chosenFolder);
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
    for (MyList::iterator it = selectedSongs.begin(); it != selectedSongs.end(); it++) {
        Song* s = (*it)->song();
        if(model->songsToPlay.count()==0 || currentSong!=model->songsToPlay.at(0)->song() || player->getStatus() != PLAYING) {
            model->songsToPlay.insert(0, new SongEntryInt(s, 13));
        } else {
            model->songsToPlay.insert(1, new SongEntryInt(s, 13));
        }
    }

    folderActual->correctOrder();
    player->syncYammi2Player();
    chosenFolder->saveSorting(songListView->sortedBy(), songListView->sortOrder());
    chosenFolder->saveScrollPos(songListView->horizontalScrollBar()->value(), songListView->verticalScrollBar()->value());
    folderContentChanged(chosenFolder);
}


void YammiGui::forSelectionPrelisten(int where ) {
    getSelectedSongs();
    int count = selectedSongs.count();
    if(count < 1) {
        return;
    }
    for (MyList::iterator it = selectedSongs.begin(); it != selectedSongs.end(); it++) {
        preListen((*it)->song(), where);
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
    bool dirty = false;
    if(chosenFolder==folderActual) {
        // song chosen from playlist => dequeue only the selected song entry (not all songs with that id)
        QList<int> posToDel;
        foreach(QModelIndex index, songListView->selectionModel()->selection().indexes()) {
            if (index.column() == 0) {
                QVariant qv = songListView->model()->data(index, FolderModel::SongEntryPointerRole);
                SongEntry* se = SongEntry::qvAsSe(qv);
                if (se) {
                    int pos=((SongEntryInt*)se)->intInfo-1;
                    if ((pos != 0 || player->getStatus()==STOPPED) && pos < model->songsToPlay.count()) {
                        // only dequeue if not currently played song (or player stopped)
                        qDebug() << "song dequeued: " << se->song()->displayName();
                        posToDel << pos;
                        /* Saved in temporary because deleting here would change index.
                         * TODO: replace this ugly removing of songs - probably altogether
                         * with current list handling ... */
                    }
                }
            }
        }
        qSort(posToDel);
        for (int i = posToDel.count()-1; i >= 0; i--) {
            delete model->songsToPlay.takeAt(posToDel[i]);
            dirty = true;
        }
    } else {
        // song chosen from other folder => dequeue ALL occurrences of each selected song
        for (MyList::iterator it = selectedSongs.begin(); it != selectedSongs.end(); it++) {
            Song* s = (*it)->song();
            int i=1;
            if(player->getStatus()==STOPPED) {
                i=0;
            }
            for(; i<(int)model->songsToPlay.count(); i++) {
                Song* check = model->songsToPlay.at(i)->song();
                if(check == s) {
                    delete model->songsToPlay.takeAt(i);
                    qDebug() << "song dequeued: " << s->displayName();
                    i--;
                    dirty = true;
                }
            }
        }
    }

    if (dirty) {
        folderActual->correctOrder();
        player->syncYammi2Player();
        folderContentChanged(folderActual);
    }
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

    if(count == 1) {
        /*Song* singleSong = */selectedSongs.firstSong();
    }
    SongInfo si(this, &selectedSongs);

    // fill combobox with genres, but sort them first
    QStringList genreList;
    genreList.append("");
#ifdef USE_TAGLIB
    for(int genreNr=0; !(TagLib::ID3v1::genre(genreNr).isEmpty()); genreNr++) {
        genreList.append(TStringToQString(TagLib::ID3v1::genre(genreNr)));
    }
#endif
    genreList.sort();
    for ( QStringList::Iterator it = genreList.begin(); it != genreList.end(); ++it ) {
        si.ComboBoxGenre->addItem(*it);
    }

    int selected=0;
    QDateTime invalid;
    for (MyList::iterator it = selectedSongs.begin(); it != selectedSongs.end(); it++) {
        Song* s = (*it)->song();
        selected++;
        if(selected==10) {			// set wait cursor (summing size of 2000 files may take a while...)
            QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
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
            _bitrate=QString("%1 kb/s").arg(s->bitrate);
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
            si.LineEditLastPlayed->setText(tr("never"));
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
        si.ReadOnlyLength->setText(x.sprintf("%d:%02d:%02d (hh:mm:ss)", _length/(60*60), (_length % (60*60))/60, _length % 60));
    } else {
        si.LabelHeading->setText(_artist+" - "+_title);
        QString x;
        si.ReadOnlyLength->setText(x.sprintf("%2d:%02d (mm:ss)", _length/60, _length % 60));
    }
    if(_size < (1024 * 1024)) {
        si.ReadOnlySize->setText( QString("%1 KB").arg( (float)_size/(float)(1024) , 4,'f', 2));
    }
    else {
        si.ReadOnlySize->setText( QString("%1 MB").arg( (float)_size/(float)(1024*1024) , 4,'f', 2));
    }
    si.ReadOnlyBitrate->setText(_bitrate);
    for (int i = 0; i < si.ComboBoxGenre->count(); i++) {
        if (si.ComboBoxGenre->itemText(i) == _genre)
            si.ComboBoxGenre->setCurrentIndex(i);
    }
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
    for (MyList::iterator it = selectedSongs.begin(); it != selectedSongs.end(); it++) {
        Song* s = (*it)->song();
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
            int tryYear=si.LineEditYear->text().toInt();
            if(tryYear!=s->year) {
                s->year=tryYear;
                change=true;
            }
        }
        if(si.LineEditTrack->text()!="!") {
            int tryTrackNr=si.LineEditTrack->text().toInt();
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
            folderContentChanged(folderActual);
        }
        
        if(si.CheckBoxCorrectFilename->isChecked() && s->checkFilename(config()->consistencyPara->ignoreCaseInFilenames)==false) {
            s->correctFilename();
            model->allSongsChanged(true);
        }
        if(si.CheckBoxCorrectPath->isChecked() && s->checkDirectory(config()->consistencyPara->ignoreCaseInFilenames)==false) {
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
    DeleteDialog dd(this);
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
    for (MyList::iterator it = selectedSongs.begin(); it != selectedSongs.end(); it++) {
        Song* s = (*it)->song();
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
    for (int i = 0; i < folderCategories->childCount(); i++) {
        QTreeWidgetItem* f = folderCategories->QTreeWidgetItem::child(i);
        FolderSorted* category=(FolderSorted*)f;
        category->removeSong(s);
    }
    // ...and from playlist
    folderActual->removeSong(s);
}


/// doubleClick on song
void YammiGui::doubleClicked()
{
    forSelection(config()->doubleClickAction);
}

/// middleClick on song
void YammiGui::middleClicked()
{
    forSelection(config()->middleClickAction);
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
    QString newName=QString(QInputDialog::getText(this, caption, message, QLineEdit::Normal, QString(tr("new category")), &ok ));
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
    QTreeWidgetItem* i = folderListView->currentItem();
    QString name=((Folder*)i)->folderName();
    QString msg(tr("Delete category %1 ?\n (will be deleted immediately!)").arg(name));
    if (QMessageBox::warning(this, "", msg, QMessageBox::Yes, QMessageBox::No | QMessageBox::Escape) == QMessageBox::Yes) {
        model->removeCategory(name);
        folderListView->setCurrentItem(folderCategories);
        folderCategories->QTreeWidgetItem::setSelected(true);
        folderCategories->update(model->allCategories, model->categoryNames);
        updateSongPopup();
    }
}

/**
 * Renames the selected category, querying the user for the new name.
 */
void YammiGui::renameCategory() {
    QTreeWidgetItem* i = folderListView->currentItem();
    QString oldName=((Folder*)i)->folderName();
    bool ok;
    QString newName=QString(QInputDialog::getText(this, tr("Category name"), tr("Please enter new name:"), QLineEdit::Normal, oldName, &ok));
    if(!ok) {
        return;
    }

    model->renameCategory(oldName, newName);
    qDebug() << "renamed in model...";
    folderListView->setCurrentItem(folderCategories);
    folderCategories->QTreeWidgetItem::setSelected(true);
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
    QTreeWidgetItem* i = folderListView->currentItem();
    FolderSorted* categoryFolder=(FolderSorted*)i;
    QString filename = QFileDialog::getOpenFileName(this, tr("Choose a Playlist to insert" ), "/", "Playlists (*.m3u)");
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
    m_actionCurrentAutoPlay->setText(tr("Folder: ")+autoplayFoldername);
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

void YammiGui::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    m_actionMinimize->setEnabled(!isMinimized());
    m_actionMaximize->setEnabled(!isMaximized());
    m_actionRestore->setEnabled(isMaximized() || isMinimized());
}

/// invoke an externally configured program/script on the content of a folder
void YammiGui::pluginOnFolder() {
    QFile f(config()->databaseDir + "plugin.temp" );
    if ( !f.open( QIODevice::WriteOnly  ) ) {
        return;
    }
    QTextStream str(&f);

    for (MyList::iterator it = chosenFolder->songlist().begin(); it != chosenFolder->songlist().end(); it++) {
        Song* s = (*it)->song();
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
        firstSong=model->songsToPlay.takeFirst()->song();
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
    ApplyToAllDialog confirm(this);
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



void YammiGui::feedAutoplayQueueIntoMixxxAutoDJ()
{
#ifdef USE_QDBUS

    QDBusInterface iface("org.mixxx", "/AutoDJFeature", "", QDBusConnection::sessionBus());
    if (iface.isValid()) {
        QDBusReply<int> reply = iface.call("getAutoDJQueueLength");
        if (reply.isValid()) {
            if (reply.value() <= 1) {
                SongEntry* s = model->songsToPlay.takeAt(0);
                QString file = s->song()->location();
                QDBusReply<void> reply = iface.call("enqueueInAutoDJ", file);
                if (reply.isValid()) {
                    qDebug() << "Call to enqueueInAutoDJ succeeded and dequeuing song: " << s->song()->displayName();
                    delete s;
                    folderActual->correctOrder();
                    player->syncYammi2Player();
                    folderContentChanged(folderActual);
                } else {
                    qDebug() << "Call to enqueueInAutoDJ failed:" << qPrintable(reply.error().message());
                }
            }
        } else {
            qDebug() << "Call to getAutoDJQueueLength failed:" << qPrintable(reply.error().message());
        }
    } else {
        qDebug() << "QDBusInterface failed:" << qPrintable(QDBusConnection::sessionBus().lastError().message());
    }

#endif /*USE_QDBUS*/
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

    if (m_actionAutoplayFeedMixxx->isChecked() &&
        folderActual->songlist().count() > 0  &&
        (player->getStatus() != STOPPED || player->getStatus() != PAUSED))
    {
        feedAutoplayQueueIntoMixxxAutoDJ();
    }

    // perform these actions only if player is playing or paused
    if(player->getStatus()!=STOPPED) {
        // adjust songSlider (if user is not currently dragging it around)
        int current = player->getCurrentTime();
        // 		if(!isSongSliderGrabbed && player->getStatus() != PAUSED) {
        m_seekSlider->setValue(current);
        m_lcdDisplay->update(current, player->getTotalTime());
        //     }
    } else {
        m_seekSlider->setValue(0);
        m_lcdDisplay->update(0, 0);
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
            for(int i=0; i<toAddFrom->songlist().count(); i++) {
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
    progress.setLabelText(tr("Re-Reading all genres from your files ..."));
    progress.setRange(0, model->allSongs.count());
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
    case Qt::Key_Control:
        controlPressed=true;
        break;
    case Qt::Key_Shift:
        shiftPressed=true;
        break;

    case Qt::Key_PageUp:
        songListView->simulateKeyPressEvent(e);
        break;
    case Qt::Key_PageDown:
        songListView->simulateKeyPressEvent(e);
        break;

    case Qt::Key_Up:
        songListView->simulateKeyPressEvent(e);
        break;

    case Qt::Key_Down:
        songListView->simulateKeyPressEvent(e);
        break;
    
    case Qt::Key_F:		// Ctrl-F
        if (e->modifiers() != Qt::ControlModifier) {
            break;
        }
        m_searchField->setText("");
        m_searchField->setFocus();
        break;
        
    case Qt::Key_Escape: // ESC
        m_searchField->setText("");
        m_searchField->setFocus();
        break;

    case Qt::Key_G:  // Ctrl-G
        if (e->modifiers() != Qt::ControlModifier) {
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
    case Qt::Key_Control:
        controlPressed=false;
        //      qDebug() << "control released";
        break;
    case Qt::Key_Shift:
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


/**
 * stops playback on headphone
 */
void YammiGui::stopPrelisten() {
    if(prelistenProcess->state() == QProcess::NotRunning) {
        qDebug() << "looks like no prelisten process running...";
        return;
    }
    prelistenProcess->kill();
    /* disabled - probably with a newer qt version
    bool result = prelistenProcess->kill(9);
    if(!result) {
        qWarning() << "could not stop prelisten process!\n";
    }
    */
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
    if(s->filename.right(3).toUpper()=="MP3") {
        prelistenCmd = config()->prelistenMp3Command;
    }
    else if(s->filename.right(3).toUpper()=="OGG") {
        prelistenCmd = config()->prelistenOggCommand;
    }
    else if(s->filename.right(3).toUpper()=="WAV") {
        prelistenCmd = config()->prelistenWavCommand;
    }
    else if(s->filename.right(4).toUpper()=="FLAC") {
        prelistenCmd = config()->prelistenFlacCommand;
    }
    else {
        prelistenCmd = config()->prelistenOtherCommand;
    }

    if(prelistenCmd == "") {
        qDebug() << "no prelistening configured for this file type: " << s->filename;
        return;
    }
    
    if (prelistenProcess->state() != QProcess::NotRunning) {
        qDebug() << "waiting for prelisten process to die...";
        prelistenProcess->kill();
    }

    // prepare command
    prelistenCmd = s->replacePlaceholders(prelistenCmd, 0);
    prelistenCmd = replacePrelistenSkip(prelistenCmd, s->length, skipTo);        

    //prelistenProcess->setUseShell(true);
    QStringList argList = prelistenCmd.split(QChar('|'), QString::SkipEmptyParts, Qt::CaseInsensitive);
    QString program = argList[0];
    argList.removeFirst();

    qDebug() << "program=" << program << "argList=" << argList;
    prelistenProcess->start(program, argList);
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
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Open file(s) to import"));
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
    progress.setLabelText(tr("Scanning ..."));
    progress.setModal(true);
    progress.setMinimumDuration(0);
    progress.setAutoReset(false);
    progress.setAutoClose(false);
    progress.setValue(0);

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
    folderContentChanged();

    if(chosenFolder==folderActual) {
        player->syncYammi2Player();
    }

    if(chosenFolder->QTreeWidgetItem::parent()==folderCategories) {
        // we have to save the order
        model->categoriesChanged(true);
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
    delete model->songsPlayed.takeAt(count-1);
    folderActual->insertSong(last, 0);
    player->skipBackward(shiftPressed);

    // skipping backward creates a new entry in songsPlayed that we don't want
    // => remove it again
    if (!model->songsPlayed.isEmpty()) {
        delete model->songsPlayed.takeAt(model->songsPlayed.count()-1);
    }
    folderSongsPlayed->updateTitle();
    folderContentChanged(folderActual);
    folderContentChanged(folderSongsPlayed);
}


void YammiGui::toggleColumnVisibility(QAction* /*action*/)
{
    changeToFolder(chosenFolder, true);
}



void YammiGui::loadMediaPlayer( ) {
    player = 0;
    switch( config()->mediaPlayer ) {
#ifdef USE_XINE
    case Prefs::MEDIA_PLAYER_XINEENGINE:
        player = new Yammi::XineEngine(model);
        break;
#endif
#ifdef USE_QMEDIAPLAYER
    case Prefs::MEDIA_PLAYER_QMEDIAPLAYERENGINE:
        player = new Yammi::QMediaPlayerEngine(model);
        break;
#endif
#ifdef USE_VLC
    case Prefs::MEDIA_PLAYER_VLCENGINE:
        player = new Yammi::VlcEngine(model);
        break;
#endif
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

void YammiGui::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        if (!isMinimized()) {
            showMinimized();
        } else {
            showNormal();
            activateWindow();
        }
        break;
    default:
        break;
    }
}

void YammiGui::trayIconMenuAboutToShow()
{
    foreach (QAction* action, trayIcon->contextMenu()->actions()) {
        action->setProperty("disabledShortcut", action->shortcut().toString());
        action->setShortcut(QKeySequence());
        if (action == m_actionSkipBackward)
            action->setShortcut(m_shortcutSkipBackward->key());

        if (action == m_actionPlayPause)
            action->setShortcut(m_shortcutPlayPause->key());

        if (action == m_actionStop)
            action->setShortcut(m_shortcutStop->key());

        if (action == m_actionSkipForward)
            action->setShortcut(m_shortcutSkipForward->key());
    }
}

// TODO: aboutToHide not called in Qt 5.7.1 ?
void YammiGui::trayIconMenuAboutToHide()
{
    qDebug() << "YammiGui::trayIconMenuAboutToHide";
    foreach (QAction* action, trayIcon->contextMenu()->actions()) {
        QVariant qv = action->property("disabledShortcut");
        if (qv.isValid() && !qv.toString().isEmpty()) {
            action->setShortcut(QKeySequence::fromString(qv.toString()));
            action->setProperty("disabledShortcut", QVariant());
        }
    }
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
    playlistPart->setObjectName("playlistPart");
    playlistPart->setReadOnly(true);
    playlistPart->setLineWrapMode(QTextEdit::NoWrap);
    
    // set up the quick browser on the left
    folderListView = new QTreeWidget( leftWidget );
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    folderListView->header()->setSectionsClickable(false);
#else
    folderListView->header()->setClickable(false);
#endif
    folderListView->headerItem()->setText(0, tr("Quick Browser"));
    folderListView->setRootIsDecorated(true);

    // set up the songlist on the right
    songListView = new MyListView( centralWidget );
    songListViewModel = new FolderModel();
    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel();
    proxyModel->setSourceModel(songListViewModel);
    proxyModel->setSortRole(FolderModel::SongEntrySortDataRole);
    proxyModel->setDynamicSortFilter(true);
    songListView->setModel(proxyModel);
    songListView->show();

    setCentralWidget(centralWidget);

    // signals of folderListView
    connect(folderListView->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(slotFolderChanged(const QModelIndex&, const QModelIndex&)));
    folderListView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(folderListView, SIGNAL( customContextMenuRequested(const QPoint&)), this, SLOT( slotFolderPopup(const QPoint&) ) );

    // signals of songListView
    connect(songListView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(songListPopup(const QPoint&)));
    connect(songListView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(doubleClicked()));
    connect(songListView, SIGNAL(middleClicked()), this, SLOT(middleClicked()));
}


void YammiGui::createFolders( ) {
    folderActual = new FolderSorted(folderListView, QString(tr("Playlist")), &(model->songsToPlay));
    folderActual->saveSorting(FolderModel::COLUMN_POS);

    folderSearchResults = new Folder(folderListView, QString(tr("Search Results")), &searchResults );
    folderSearchResults->saveSorting(FolderModel::COLUMN_MATCH, Qt::DescendingOrder);

    folderAll = new Folder(folderListView, QString(tr("All Music")), &(model->allSongs));
    folderAll->saveSorting(FolderModel::COLUMN_ARTIST);

    folderArtists = new FolderGroups(folderListView, QString( tr("Artists") ));
    folderArtists->saveSorting(FolderModel::COLUMN_TITLE);

    folderAlbums = new FolderGroups(folderListView, QString( tr("Albums") ));
    folderAlbums->saveSorting(FolderModel::COLUMN_TRACKNR);

    folderGenres = new FolderGroups(folderListView, QString( tr("Genre") ));
    folderGenres->saveSorting(FolderModel::COLUMN_ARTIST);

    folderYears = new FolderGroups(folderListView, QString( tr("Year") ));
    folderYears->saveSorting(FolderModel::COLUMN_ARTIST);

    folderCategories = new FolderCategories(folderListView, QString(tr("Categories")));
    folderCategories->saveSorting(FolderModel::COLUMN_POS);

    folderSongsPlayed = new Folder(folderListView, QString(tr("Songs Played")), &(model->songsPlayed));
    folderSongsPlayed->saveSorting(FolderModel::COLUMN_LAST_PLAYED);

    folderHistory = new Folder(folderListView, QString(tr("History")), &(model->songHistory));
    folderHistory->saveSorting(FolderModel::COLUMN_PLAYED_ON, Qt::DescendingOrder);

    folderUnclassified = new Folder(folderListView, QString(tr("Unclassified")), &(model->unclassifiedSongs));
    folderUnclassified->saveSorting(FolderModel::COLUMN_ARTIST);

    folderRecentAdditions = new Folder(folderListView, QString(tr("Recent Additions")), &(model->recentSongs));
    folderRecentAdditions->saveSorting(FolderModel::COLUMN_ADDED_TO);

    folderProblematic = new Folder(folderListView, QString(tr("Problematic Songs")) );
    folderProblematic->saveSorting(FolderModel::COLUMN_REASON);


    // signals of folders
    connect(folderCategories, SIGNAL( CategoryNew() ), this, SLOT(newCategory()));
    connect(folderCategories, SIGNAL( CategoryRemoved() ), this, SLOT(removeCategory()));
    connect(folderCategories, SIGNAL( CategoryRenamed() ), this, SLOT(renameCategory()));
    connect(folderCategories, SIGNAL( LoadM3uIntoCategory() ), this, SLOT(loadM3uIntoCategory()));
}


void YammiGui::setupActions()
{
    QStyle* style = QApplication::style();

    m_actionSecondYammi = new QAction(tr("&Second Yammi"), this);
    m_actionSecondYammi->setIcon(loadAndConvertIconToGrayScale("icons:yammi.png"));
    connect(m_actionSecondYammi, SIGNAL(triggered()), this, SLOT(startSecondYammi()));

    m_actionQuit = new QAction(tr("&Quit"), this);
    m_actionQuit->setShortcut(QKeySequence::Quit);
    m_actionQuit->setIcon(style->standardIcon(QStyle::SP_DialogCloseButton));
    connect(m_actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    m_actionSelectAll = new QAction(tr("Select &All"), this);
    m_actionSelectAll->setShortcut(QKeySequence::SelectAll);
    connect(m_actionSelectAll, SIGNAL(triggered()), songListView, SLOT(selectAll()));

    m_actionInvertSelection = new QAction(tr("&Invert Selection"), this);
    connect(m_actionInvertSelection, SIGNAL(triggered()), songListView, SLOT(invertSelection()));

    m_actionUpdateView = new QAction(tr("Update Automatic Folder Structure"), this);
    m_actionUpdateView->setIcon(style->standardIcon(QStyle::SP_BrowserReload));
    connect(m_actionUpdateView, SIGNAL(triggered()), this, SLOT(updateView()));

    m_actionGroupToggleToolbar = new QActionGroup(this);
    m_actionGroupToggleToolbar->setExclusive(false);
    connect(m_actionGroupToggleToolbar, SIGNAL(triggered(QAction*)), this, SLOT(toolbarToggled(QAction*)));

    m_actionToggleMainToolbar = new QAction(tr("Main Toolbar"), this);
    m_actionToggleMainToolbar->setCheckable(true);
    m_actionToggleMainToolbar->setObjectName("MainToolbar");
    m_actionGroupToggleToolbar->addAction(m_actionToggleMainToolbar);

    m_actionToggleMediaPlayerToolbar = new QAction(tr("Media Player"), this);
    m_actionToggleMediaPlayerToolbar->setCheckable(true);
    m_actionToggleMediaPlayerToolbar->setObjectName("MediaPlayerToolbar");
    m_actionGroupToggleToolbar->addAction(m_actionToggleMediaPlayerToolbar);

    m_actionToggleTimeDisplayToolbar = new QAction(tr("Time Display"), this);
    m_actionToggleTimeDisplayToolbar->setCheckable(true);
    m_actionToggleTimeDisplayToolbar->setObjectName("TimeDisplayToolbar");
    m_actionGroupToggleToolbar->addAction(m_actionToggleTimeDisplayToolbar);

    m_actionToggleSongActionsToolbar = new QAction(tr("Song Actions"), this);
    m_actionToggleSongActionsToolbar->setCheckable(true);
    m_actionToggleSongActionsToolbar->setObjectName("SongActionsToolbar");
    m_actionGroupToggleToolbar->addAction(m_actionToggleSongActionsToolbar);

    m_actionTogglePrelistenToolbar = new QAction(tr("Prelisten"), this);
    m_actionTogglePrelistenToolbar->setCheckable(true);
    m_actionTogglePrelistenToolbar->setObjectName("PrelistenToolbar");
    m_actionGroupToggleToolbar->addAction(m_actionTogglePrelistenToolbar);

    m_actionGroupColumnVisibility = new QActionGroup(this);
    m_actionGroupColumnVisibility->setExclusive(false);
    connect(m_actionGroupColumnVisibility, SIGNAL(triggered(QAction*)), this, SLOT(toggleColumnVisibility(QAction*)));

    m_actionPlayPause = new QAction(tr("&Play"), this);
    m_actionPlayPause->setShortcut(QKeySequence(Qt::Key_F1));
    m_actionPlayPause->setIcon(style->standardIcon(QStyle::SP_MediaPlay));
    connect(m_actionPlayPause, SIGNAL(triggered()), this, SLOT(playPause()));
    m_shortcutPlayPause = new QShortcut(QKeySequence("Meta+x"), this);
    connect(m_shortcutPlayPause, SIGNAL(activated()), m_actionPlayPause, SLOT(trigger()));

    m_actionSkipBackward = new QAction(tr("&Skip Backward"), this);
    m_actionSkipBackward->setShortcut(QKeySequence(Qt::Key_F2));
    m_actionSkipBackward->setIcon(style->standardIcon(QStyle::SP_MediaSkipBackward));
    connect(m_actionSkipBackward, SIGNAL(triggered()), this, SLOT(skipBackward()));
    m_shortcutSkipBackward = new QShortcut(QKeySequence("Meta+y"), this);
    connect(m_shortcutSkipBackward, SIGNAL(activated()), m_actionSkipBackward, SLOT(trigger()));

    m_actionSkipForward = new QAction(tr("Skip &Forward"), this);
    m_actionSkipForward->setShortcut(QKeySequence(Qt::Key_F3));
    m_actionSkipForward->setIcon(style->standardIcon(QStyle::SP_MediaSkipForward));
    connect(m_actionSkipForward, SIGNAL(triggered()), this, SLOT(skipForward()));
    m_shortcutStop = new QShortcut(QKeySequence("Meta+v"), this);
    connect(m_shortcutStop, SIGNAL(activated()), m_actionSkipForward, SLOT(trigger()));

    m_actionStop = new QAction(tr("S&top"), this);
    m_actionStop->setShortcut(QKeySequence(Qt::Key_F4));
    m_actionStop->setIcon(style->standardIcon(QStyle::SP_MediaStop));
    connect(m_actionStop, SIGNAL(triggered()), this, SLOT(stop()));
    m_shortcutSkipForward = new QShortcut(QKeySequence("Meta+c"), this);
    connect(m_shortcutSkipForward, SIGNAL(activated()), m_actionStop, SLOT(trigger()));

    m_actionToFromPlaylist = new QAction(tr("&Switch to/from Playlist"), this);
    m_actionToFromPlaylist->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));
    m_actionToFromPlaylist->setIcon(QIcon("icons:toggle_playlist.png"));
    connect(m_actionToFromPlaylist, SIGNAL(triggered()), this, SLOT(toFromPlaylist()));

    m_actionClearPlayList = new QAction(tr("&Clear Playlist"), this);
    m_actionClearPlayList->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_F8));
    m_actionClearPlayList->setIcon(style->standardIcon(QStyle::SP_DialogResetButton));
    connect(m_actionClearPlayList, SIGNAL(triggered()), this, SLOT(clearPlaylist()));

    m_actionShufflePlaylist = new QAction(tr("Shuffle &Playlist"), this);
    connect(m_actionShufflePlaylist, SIGNAL(triggered()), this, SLOT(shufflePlaylist()));

    m_actionSaveDatabase = new QAction(tr("&Save Database"), this);
    m_actionSaveDatabase->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    m_actionSaveDatabase->setIcon(style->standardIcon(QStyle::SP_DialogSaveButton));
    connect(m_actionSaveDatabase, SIGNAL(triggered()), this, SLOT(saveDatabase()));

    m_actionScanHarddisk = new QAction(tr("Scan &Harddisk ..."), this);
    m_actionScanHarddisk->setIcon(style->standardIcon(QStyle::QStyle::SP_DirIcon));
    connect(m_actionScanHarddisk, SIGNAL(triggered()), this, SLOT(updateSongDatabaseHarddisk()));

    m_actionImportSelectedFiles = new QAction(tr("&Import Selected File(s) ..."), this);
    m_actionImportSelectedFiles->setIcon(style->standardIcon(QStyle::QStyle::SP_DirIcon));
    connect(m_actionImportSelectedFiles, SIGNAL(triggered()), this, SLOT(updateSongDatabaseSingleFile()));

    m_actionCheckConsistencyAll = new QAction(tr("&Check Consistency ..."), this);
    connect(m_actionCheckConsistencyAll, SIGNAL(triggered()), this, SLOT(forAllCheckConsistency()));

    m_actionFixGenres = new QAction(tr("&Fix Genres ..."), this);
    connect(m_actionFixGenres, SIGNAL(triggered()), this, SLOT(fixGenres()));

    m_actionGroupAutoplay = new QActionGroup(this);
    m_actionGroupAutoplay->setExclusive(true);

    m_actionAutoplayOff = new QAction(tr("Autoplay &Off"), this);
    m_actionAutoplayOff->setCheckable(true);
    m_actionGroupAutoplay->addAction(m_actionAutoplayOff);
    connect(m_actionAutoplayOff, SIGNAL(triggered()), this, SLOT(autoplayOff()));

    m_actionAutoplayLnp = new QAction(tr("&Longest Not Played"), this);
    m_actionAutoplayLnp->setCheckable(true);
    m_actionGroupAutoplay->addAction(m_actionAutoplayLnp);
    connect(m_actionAutoplayLnp, SIGNAL(triggered()), this, SLOT(autoplayLNP()));

    m_actionAutoplayRandom = new QAction(tr("&Random"), this);
    m_actionAutoplayRandom->setCheckable(true);
    m_actionGroupAutoplay->addAction(m_actionAutoplayRandom);
    connect(m_actionAutoplayRandom, SIGNAL(triggered()), this, SLOT(autoplayRandom()));

    m_actionCurrentAutoPlay = new QAction(tr("Unknown"), this); //m_currentAutoPlay = new KAction(tr("Unknown"),0,0,0,0,actionCollection(),"autoplay_folder");

    m_actionAutoplayFeedMixxx = new QAction(tr("Feed into Mixxx AutoDJ"), this);
    m_actionAutoplayFeedMixxx->setCheckable(true);

    m_actionConfigureYammi = new QAction(tr("&Configure Yammi ..."), this);
    connect(m_actionConfigureYammi, SIGNAL(triggered()), this, SLOT(setPreferences()));

    m_actionEnqueueAtEnd = new QAction(tr("Enqueue at end (append)"), this);
    m_actionEnqueueAtEnd->setShortcut(QKeySequence(Qt::Key_F5));
    m_actionEnqueueAtEnd->setIcon(QIcon("icons:enqueue.png"));
    connect(m_actionEnqueueAtEnd, SIGNAL(triggered()), this, SLOT(forSelectionEnqueue()));

    m_actionEnqueueAsNext = new QAction(tr("Enqueue as next (prepend)"), this);
    m_actionEnqueueAsNext->setShortcut(QKeySequence(Qt::Key_F6));
    m_actionEnqueueAsNext->setIcon(QIcon("icons:enqueue_asnext.png"));
    connect(m_actionEnqueueAsNext, SIGNAL(triggered()), this, SLOT(forSelectionEnqueueAsNext()));

    m_actionPlayNow = new QAction(tr("Play Now!"), this);
    m_actionPlayNow->setShortcut(QKeySequence(Qt::Key_F7));
    m_actionPlayNow->setIcon(QIcon("icons:play_now.png"));
    connect(m_actionPlayNow, SIGNAL(triggered()), this, SLOT(forSelectionPlayNow()));

    m_actionDequeueSong = new QAction(tr("Dequeue Songs"), this);
    m_actionDequeueSong->setShortcut(QKeySequence(Qt::Key_F8));
    m_actionDequeueSong->setIcon(style->standardIcon(QStyle::QStyle::SP_DialogCancelButton));
    connect(m_actionDequeueSong, SIGNAL(triggered()), this, SLOT(forSelectionDequeue()));

    m_actionPrelistenStart = new QAction(tr("Prelisten Start"), this);
    m_actionPrelistenStart->setShortcut(QKeySequence(Qt::Key_F9));
    m_actionPrelistenStart->setIcon(QIcon("icons:prelisten_start.png"));
    connect(m_actionPrelistenStart, SIGNAL(triggered()), this, SLOT(forSelectionPrelistenStart()));

    m_actionPrelistenMiddle = new QAction(tr("Prelisten Middle"), this);
    m_actionPrelistenMiddle->setShortcut(QKeySequence(Qt::Key_F10));
    m_actionPrelistenMiddle->setIcon(QIcon("icons:prelisten_middle.png"));
    connect(m_actionPrelistenMiddle, SIGNAL(triggered()), this, SLOT(forSelectionPrelistenMiddle()));

    m_actionPrelistenEnd = new QAction(tr("Prelisten End"), this);
    m_actionPrelistenEnd->setShortcut(QKeySequence(Qt::Key_F11));
    m_actionPrelistenEnd->setIcon(QIcon("icons:prelisten_end.png"));
    connect(m_actionPrelistenEnd, SIGNAL(triggered()), this, SLOT(forSelectionPrelistenEnd()));

    m_actionStopPrelisten = new QAction(tr("Stop Prelisten"), this);
    m_actionStopPrelisten->setShortcut(QKeySequence(Qt::Key_F12));
    m_actionStopPrelisten->setIcon(QIcon("icons:stop_prelisten.png"));
    connect(m_actionStopPrelisten, SIGNAL(triggered()), this, SLOT(stopPrelisten()));

    m_actionLoadInMixxxDeck1 = new QAction(tr("Load in Mixxx Deck 1"), this);
    m_actionLoadInMixxxDeck1->setIcon(QIcon("icons:mixxx-icon.png"));
    connect(m_actionLoadInMixxxDeck1, SIGNAL(triggered()), this, SLOT(slotLoadInMixxxDeck1()));

    m_actionLoadInMixxxDeck2 = new QAction(tr("Load in Mixxx Deck 2"), this);
    m_actionLoadInMixxxDeck2->setIcon(QIcon("icons:mixxx-icon.png"));
    connect(m_actionLoadInMixxxDeck2, SIGNAL(triggered()), this, SLOT(slotLoadInMixxxDeck2()));

    m_actionLoadInMixxxAutoDJ = new QAction(tr("Load in Mixxx AutoDJ"), this);
    m_actionLoadInMixxxAutoDJ->setIcon(QIcon("icons:mixxx-icon.png"));
    connect(m_actionLoadInMixxxAutoDJ, SIGNAL(triggered()), this, SLOT(slotLoadSelectedSongInMixxxAutoDJ()));

    m_actionEnqueueAsNextInOtherYammi = new QAction(tr("Enqueue as next in Other Yammi"), this);
    if (config()->thisIsSecondYammi) {
        m_actionEnqueueAsNextInOtherYammi->setIcon(QIcon("icons:yammi.png"));
    } else {
        m_actionEnqueueAsNextInOtherYammi->setIcon(loadAndConvertIconToGrayScale("icons:yammi.png"));
    }
    connect(m_actionEnqueueAsNextInOtherYammi, SIGNAL(triggered()), this, SLOT(slotEnqueueAsNextInOtherYammi()));

    m_actionSongInfo = new QAction(tr("Song Info ..."), this);
    m_actionSongInfo->setShortcut(QKeySequence(Qt::Key_I));
    m_actionSongInfo->setIcon(style->standardIcon(QStyle::QStyle::SP_FileDialogDetailedView));
    connect(m_actionSongInfo, SIGNAL(triggered()), this, SLOT(forSelectionSongInfo()));

    m_actionGotoFolderArtist = new QAction(tr("Goto artist"), this);
    connect(m_actionGotoFolderArtist, SIGNAL(triggered()), this, SLOT(gotoFolderArtist()));

    m_actionGotoFolderAlbum = new QAction(tr("Goto album"), this);
    connect(m_actionGotoFolderAlbum, SIGNAL(triggered()), this, SLOT(gotoFolderAlbum()));

    m_actionGotoFolderGenre = new QAction(tr("Goto genre"), this);
    connect(m_actionGotoFolderGenre, SIGNAL(triggered()), this, SLOT(gotoFolderGenre()));

    m_actionGotoFolderYear = new QAction(tr("Goto year"), this);
    connect(m_actionGotoFolderYear, SIGNAL(triggered()), this, SLOT(gotoFolderYear()));

    m_actionSearchSimilarEntry = new QAction(tr("Search for similar entry"), this);
    connect(m_actionSearchSimilarEntry, SIGNAL(triggered()), this, SLOT(searchForSimilarEntry()));

    m_actionSearchSimilarArtist = new QAction(tr("Search for similar artist"), this);
    connect(m_actionSearchSimilarArtist, SIGNAL(triggered()), this, SLOT(searchForSimilarArtist()));

    m_actionSearchSimilarTitle = new QAction(tr("Search for similar title"), this);
    connect(m_actionSearchSimilarTitle, SIGNAL(triggered()), this, SLOT(searchForSimilarTitle()));

    m_actionSimilarAlbum = new QAction(tr("Search for similar album"), this);
    connect(m_actionSimilarAlbum, SIGNAL(triggered()), this, SLOT(searchForSimilarAlbum()));

    m_actionCheckConsistencySelection = new QAction(tr("Check Consistency ..."), this);
    connect(m_actionCheckConsistencySelection, SIGNAL(triggered()), this, SLOT(forSelectionCheckConsistency()));

    m_actionDeleteSong = new QAction(tr("Delete Song ..."), this);
    connect(m_actionDeleteSong, SIGNAL(triggered()), this, SLOT(forSelectionDelete()));

    m_actionMoveFiles = new QAction(tr("Move Files"), this);
    connect(m_actionMoveFiles, SIGNAL(triggered()), this, SLOT(forSelectionMove()));

    m_actionOpenFolderInFilemanager = new QAction(tr("Open Folder in Filemanager ..."), this);
    connect(m_actionOpenFolderInFilemanager, SIGNAL(triggered()), this, SLOT(openFolderInFileManager()));

    m_actionMinimize = new QAction(tr("Mi&nimize"), this);
    connect(m_actionMinimize, SIGNAL(triggered()), this, SLOT(showMinimized()));

    m_actionMaximize = new QAction(tr("Ma&ximize"), this);
    connect(m_actionMaximize, SIGNAL(triggered()), this, SLOT(showMaximized()));

    m_actionRestore = new QAction(tr("&Restore"), this);
    connect(m_actionRestore, SIGNAL(triggered()), this, SLOT(showNormal()));
}


void YammiGui::createMenuBar()
{
    QMenu* menu;
    QMenu* subMenu;
    QAction* action;

    menu = menuBar()->addMenu(tr("&File"));
    if (!config()->thisIsSecondYammi) {
        menu->addAction(m_actionSecondYammi);
    }
    menu->addAction(m_actionQuit);

    menu = menuBar()->addMenu(tr("&Edit"));
    menu->addAction(m_actionSelectAll);
    menu->addSeparator();
    menu->addAction(m_actionInvertSelection);

    menu = menuBar()->addMenu(tr("&View"));
    menu->addAction(m_actionUpdateView);
    subMenu = menu->addMenu(tr("Toolbars"));
    subMenu->addAction(m_actionToggleMainToolbar);
    subMenu->addAction(m_actionToggleMediaPlayerToolbar);
    subMenu->addAction(m_actionToggleTimeDisplayToolbar);
    subMenu->addAction(m_actionToggleSongActionsToolbar);
    subMenu->addAction(m_actionTogglePrelistenToolbar);

    menu = menuBar()->addMenu(tr("&Columns"));
    for (int column = FolderModel::COLUMN_ARTIST; column < FolderModel::MAX_COLUMN_NO; column++) {
        action = new QAction(getColumnName(column), this);
        action->setCheckable(true);
        action->setProperty("column", column);
        action->setObjectName(QString("Column%1").arg(column));
        action->setChecked(true);
        m_actionGroupColumnVisibility->addAction(action);
        menu->addAction(action);
    }

    menu = menuBar()->addMenu(tr("&Player"));
    menu->addAction(m_actionPlayPause);
    menu->addAction(m_actionSkipBackward);
    menu->addAction(m_actionSkipForward);
    menu->addAction(m_actionStop);

    menu = menuBar()->addMenu(tr("P&laylist"));
    menu->addAction(m_actionToFromPlaylist);
    menu->addAction(m_actionClearPlayList);
    menu->addAction(m_actionShufflePlaylist);

    menu = menuBar()->addMenu(tr("&Database"));
    menu->addAction(m_actionSaveDatabase);
    menu->addAction(m_actionScanHarddisk);
    menu->addAction(m_actionImportSelectedFiles);
    menu->addAction(m_actionCheckConsistencyAll);
    menu->addAction(m_actionFixGenres);

    menu = menuBar()->addMenu(tr("&Autoplay"));
    menu->addAction(m_actionAutoplayOff);
    menu->addAction(m_actionAutoplayLnp);
    menu->addAction(m_actionAutoplayRandom);
    menu->addSeparator();
    menu->addAction(m_actionCurrentAutoPlay);
    menu->addSeparator();
    menu->addAction(m_actionAutoplayFeedMixxx);

    menu = menuBar()->addMenu(tr("&Settings"));
    menu->addAction(m_actionConfigureYammi);
}

void YammiGui::createTrayIcon()
{
    QStyle* style = QApplication::style();

    QMenu* trayIconMenu = new QMenu(this);

    trayIconMenu->addAction(m_actionQuit);
    m_actionQuit->setIcon(style->standardIcon(QStyle::SP_DialogCloseButton)); /* set icon here again, or in KDE4 from wheezy the icon disappears */

    trayIconMenu->addSeparator();

    trayIconMenu->addAction(m_actionMinimize);
    trayIconMenu->addAction(m_actionMaximize);
    trayIconMenu->addAction(m_actionRestore);

    trayIconMenu->addSeparator();

    trayIconMenu->addAction(m_actionSkipBackward);
    trayIconMenu->addAction(m_actionPlayPause);
    trayIconMenu->addAction(m_actionStop);
    trayIconMenu->addAction(m_actionSkipForward);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    if (!config()->thisIsSecondYammi) {
        trayIcon->setIcon(QIcon("icons:yammi.png"));
    } else {
        trayIcon->setIcon(loadAndConvertIconToGrayScale("icons:yammi.png"));
    }

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    connect(trayIcon->contextMenu(), SIGNAL(aboutToShow()), this, SLOT(trayIconMenuAboutToShow()));
    connect(trayIcon->contextMenu(), SIGNAL(aboutToHide()), this, SLOT(trayIconMenuAboutToHide()));
}

void YammiGui::createToolbars()
{
    QToolBar* mainToolBar = new QToolBar(tr("Main Toolbar"), this);
    mainToolBar->setObjectName("MainToolbar");
    addToolBar(mainToolBar);

    //search
    QWidget *w = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(new QLabel(tr("Search:")));
    layout->addWidget(m_searchField = new QLineEdit(w));
    w->setLayout(layout);

    m_searchField->setFixedWidth(175);
    m_searchField->setToolTip(tr("Fuzzy search (Ctrl-F)\nGoto fuzzy matching folder (Ctrl-G)"));
    connect( m_searchField, SIGNAL(textChanged(const QString&)), SLOT(searchFieldChanged(const QString&)));
    /*
        TODO: temporarily disabled before we have a better concept and have it documented properly...
        QPushButton *btn = new QPushButton(tr("to wishlist"),w);
        connect( btn, SIGNAL( clicked() ), this, SLOT( addToWishList() ) );
        QToolTip::add( btn, tr("Add this entry to the database as a \"wish\""));
    */
    mainToolBar->addWidget(w);


    QToolBar* mediaPlayerToolBar = new QToolBar(tr("Media Player"), this);
    mediaPlayerToolBar->setObjectName("MediaPlayerToolbar");
    addToolBar(mediaPlayerToolBar);
    mediaPlayerToolBar->addAction(m_actionPlayPause);
    mediaPlayerToolBar->addAction(m_actionSkipBackward);
    mediaPlayerToolBar->addAction(m_actionSkipForward);
    mediaPlayerToolBar->addAction(m_actionStop);

    //Media player actions
    m_seekSlider = new TrackPositionSlider( Qt::Horizontal, 0L);
    m_seekSlider->setFixedWidth(200);
    m_seekSlider->setToolTip(tr("Track position"));
    connect(m_seekSlider,SIGNAL(sliderMoved(int)),this,SLOT(seek(int)));
    connect(m_seekSlider,SIGNAL(myWheelEvent(int)),this,SLOT(seekWithWheel(int)));
    mediaPlayerToolBar->addWidget(m_seekSlider);

    QToolBar* timeDisplayToolBar = new QToolBar(tr("Time Display"), this);
    timeDisplayToolBar->setObjectName("TimeDisplayToolbar");
    addToolBar(timeDisplayToolBar);
    m_lcdDisplay = new YammiLCDNumber(timeDisplayToolBar);
    timeDisplayToolBar->setMinimumSize(100, 30);

    QToolBar* songActionsToolBar = new QToolBar(tr("Song Actions"), this);
    songActionsToolBar->setObjectName("SongActionsToolbar");
    addToolBar(songActionsToolBar);
    songActionsToolBar->addAction(m_actionEnqueueAtEnd);
    songActionsToolBar->addAction(m_actionEnqueueAsNext);
    songActionsToolBar->addAction(m_actionPlayNow);
    songActionsToolBar->addAction(m_actionToFromPlaylist);
    songActionsToolBar->addAction(m_actionDequeueSong);
    songActionsToolBar->addAction(m_actionSongInfo);

    QToolBar* prelistenToolBar = new QToolBar(tr("Prelisten"), this);
    prelistenToolBar->setObjectName("PrelistenToolbar");
    addToolBar(prelistenToolBar);
    prelistenToolBar->addAction(m_actionPrelistenStart);
    prelistenToolBar->addAction(m_actionPrelistenMiddle);
    prelistenToolBar->addAction(m_actionPrelistenEnd);
    prelistenToolBar->addAction(m_actionStopPrelisten);
}


/**
 * Creates the song popup menu from the xml gui framework.
 * Also calls updateSongPopup to populate the submenus for categories and plugins.
 * These have to be updated with updateSongPopup on each change in existing categories
 * or plugins.
 */
void YammiGui::createSongPopup() {
    //qDebug() << "creating song popup";

    QMenu* subMenu;

    songPopup = new QMenu(tr("Content ..."), this);

    m_actionSongPopupHeader = new QAction(songPopup);
    //m_actionSongPopupHeader->set(false);
    songPopup->addAction(m_actionSongPopupHeader);
    songPopup->addSeparator();

    //subMenu = songPopup->addMenu(tr("Play/Enqueue"));
    songPopup->addAction(m_actionEnqueueAtEnd);
    songPopup->addAction(m_actionEnqueueAsNext);
    songPopup->addAction(m_actionPlayNow);
    songPopup->addAction(m_actionDequeueSong);
    songPopup->addAction(m_actionSongInfo);
    songPopup->addAction(m_actionLoadInMixxxDeck1);
    songPopup->addAction(m_actionLoadInMixxxDeck2);
    songPopup->addAction(m_actionLoadInMixxxAutoDJ);
    songPopup->addAction(m_actionEnqueueAsNextInOtherYammi);

    subMenu = songPopup->addMenu(tr("Prelisten"));
    subMenu->addAction(m_actionPrelistenStart);
    subMenu->addAction(m_actionPrelistenMiddle);
    subMenu->addAction(m_actionPrelistenEnd);

    subMenu = songPopup->addMenu(tr("Go to folder ..."));
    subMenu->addAction(m_actionGotoFolderArtist);
    subMenu->addAction(m_actionGotoFolderAlbum);
    subMenu->addAction(m_actionGotoFolderGenre);
    subMenu->addAction(m_actionGotoFolderYear);

    subMenu = songPopup->addMenu(tr("Search for similar ..."));
    subMenu->addAction(m_actionSearchSimilarEntry);
    subMenu->addAction(m_actionSearchSimilarArtist);
    subMenu->addAction(m_actionSearchSimilarTitle);
    subMenu->addAction(m_actionSimilarAlbum);

    subMenu = songPopup->addMenu(tr("Advanced ..."));
    subMenu->addAction(m_actionOpenFolderInFilemanager);
    subMenu->addAction(m_actionCheckConsistencySelection);
    subMenu->addAction(m_actionDeleteSong);
    subMenu->addAction(m_actionMoveFiles);

    songCategoryPopup = new QMenu(tr("Insert Into/Remove From ..."), songPopup);
    songPopup->addMenu(songCategoryPopup);

    pluginPopup = new QMenu(tr("Plugins ..."), songPopup);
    songPopup->addMenu(pluginPopup);

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
//    qDebug() << "seekWithWheel() called";
    if(rotation<0) {
        m_seekSlider->triggerAction(QAbstractSlider::SliderPageStepAdd);
    } else {
        m_seekSlider->triggerAction(QAbstractSlider::SliderPageStepSub);
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
    m_actionAutoplayOff->setChecked(true);
}

void YammiGui::aplayLNP() {

    autoplayLNP();
    m_actionAutoplayLnp->setChecked(true);
}

void YammiGui::aplayRandom() {

    autoplayRandom();
    m_actionAutoplayRandom->setChecked(true);
}

QString YammiGui::songInfo() {

    QString info;

    info = tr("Artist :  ") + songArtist() + "\n";
    info += tr("Title :\t  ") + songTitle() + "\n";
    info += tr("Album :\t  ") + songAlbum() + "\n";
    info += tr("Track :\t  ") + songTrack2D() + "\n";
    info += tr("Year :\t  ") + QString::number(songYear()) + "\n";
    info += tr("Genre :\t  ") + songGenre() + "\n" ;
    info += tr("Comment : ") + songComment() + "\n";

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

bool YammiGui::dragDropAllowed(int row)
{
    bool ret = true;

    if (chosenFolder != folderActual &&
        chosenFolder->QTreeWidgetItem::parent() != folderCategories)
    {
        ret = false;
    }

    if (songListView->sortedBy() != FolderModel::COLUMN_POS ||
        songListView->sortOrder() != Qt::AscendingOrder)
    {
        ret = false;
    }

    if (chosenFolder == folderActual &&
        row == 0)
    {
        ret = false;
    }

    return ret;
}

void YammiGui::startSecondYammi()
{
    config()->saveConfig();

    QString program = QCoreApplication::applicationFilePath();
    QStringList arguments;
    arguments << "-secondYammi";

    QProcess* myProcess = new QProcess(this);
    myProcess->start(program, arguments);
}
