/***************************************************************************
                          yammigui.h  -  description
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

#ifndef YAMMIGUI_H
#define YAMMIGUI_H

#include <QMainWindow>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QWaitCondition>

#include "folder.h"
#include "mylist.h"


class QActionGroup;
class QMenu;
class QModelIndex;
class QKeyEvent;
class QLabel;
class QLineEdit;
class QPushButton;
class QProcess;
class QShortcut;
class QSlider;
class QSpinBox;
class QSplitter;
class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;

class YammiModel;
class YammiLCDNumber;
class MediaPlayer;
class MyDateTime;
class MyListView;
class Folder;
class FolderCategories;
class FolderGroups;
class FolderModel;
class FolderSorted;
class Song;
class SearchThread;
class TrackPositionSlider;
class Prefs;
// -----------------------------------------------------------------



/**
 * This is the main class.
 *
 * ...still way to big and unordered...
 */
class YammiGui : public QMainWindow {
    Q_OBJECT
public:    
    /**
     * Determines the way the playlist is filled up when it contains less then 5 songs.
     */
    enum AutoplayMode { AUTOPLAY_OFF = 0, AUTOPLAY_LNP = 1, AUTOPLAY_RANDOM = 2, AUTOPLAY_FOLDER = 10, FUZZY_FOLDER_LIST_SIZE = 50 };

    /**
     * The selection mode determines for which songs an action is to be executed
     * for all forSelection...() methods.
     * The default is SELECTION_MODE_USER_SELECTED, which means the action is executed for
     * all songs selected in the songlistview by the user.
     * The selection mode enables to use the same slots for toolbuttons and context menu entries alike,
     * although their "scope" might be different. A context menu on a folder eg. just has to ensure
     * that the selection mode is set properly before invoking the slot, and to restore the selection
     * mode afterwards again.
     */
    enum SelectionMode {SELECTION_MODE_USER_SELECTED = 0, SELECTION_MODE_FOLDER = 1, SELECTION_MODE_ALL = 2};

public:
    YammiGui();
    virtual ~YammiGui();

    /** Load the Song database.
      * @param db Database file. */
    void loadDatabase();

    /** Yammi config options (preferences) */
    Prefs* config();
    
public slots:
    /** Seeks in the current song (if any) to position pos */
    Q_SCRIPTABLE void seek( int pos );
    void seekWithWheel(int rotation);

    /** Request a fuzzy search in the song database, and switch to the search-results view */
    void searchFieldChanged( const QString &fuzzy );

    /* (Remote) functions, skipForward and skipBackward are used as is */
    Q_SCRIPTABLE void play();
    Q_SCRIPTABLE void pause();

    Q_SCRIPTABLE void aplayOff();
    Q_SCRIPTABLE void aplayLNP();
    Q_SCRIPTABLE void aplayRandom();

    Q_SCRIPTABLE int currentTime();
    Q_SCRIPTABLE int totalTime();

    Q_SCRIPTABLE QString songInfo();
    Q_SCRIPTABLE QString songArtist();
    Q_SCRIPTABLE QString songTitle();
    Q_SCRIPTABLE int songTrack();
    Q_SCRIPTABLE QString songTrack2D();
    Q_SCRIPTABLE QString songAlbum();
    Q_SCRIPTABLE QString songGenre();
    Q_SCRIPTABLE QString songComment();
    Q_SCRIPTABLE int songYear();

    Q_SCRIPTABLE void skipBackward();
    Q_SCRIPTABLE void skipForward();
    Q_SCRIPTABLE void stop();
    Q_SCRIPTABLE void playPause();

    Q_SCRIPTABLE void slotEnqueueAsNextByLocation(QString location);

    /* At the command line this call is possible then:
             qdbus net.sf.yammi.yammi.YammiGui /YammiGui net.sourceforge.yammi.yammi.YammiGui.playPause
    */

protected:
    /** creates the internal MediaPlayer */
    void loadMediaPlayer( );
    /** connect media player and yammi via signals */
    void connectMediaPlayer();
    void disconnectMediaPlayer();

    /** save general Options like all bar positions and status as well as the geometry*/
    void saveOptions();
    /** read general Options again and initialize all variables*/
    void readOptions();

    virtual void closeEvent(QCloseEvent *event);

    /**
     * queryClose is called before the window is closed, either by the user
     * or by the session manager. If data has been modified, this function can 
     * be used to ask if the changes should be saved.
     * if queryClose returns false, the close event is rejected.
     * 
     * @return	True if window may be closed.
     */
    virtual bool queryClose();

    /**
     * queryExit is called just before the last window is closed (and the
     * application exits ).
     * There should not be any user interaction here (only severe errors), but
     * the function can be used to save configuration back, etc.
     *
     * @return	True if window may be closed.
     */
    virtual bool queryExit();

protected slots:
    /** Show/hide a toolbar after the correspondingt action is toggled by the user*/
    void toolbarToggled(QAction* action);
private:
    /** Setup UI-actions*/
    void setupActions();

private:
    bool validState;
    TrackPositionSlider* m_seekSlider;
    YammiLCDNumber* m_lcdDisplay;
    QLineEdit* m_searchField;
    bool searchResultsUpdateNeeded;
    bool m_acceptSearchResults;

public:
    QString replacePrelistenSkip(QString input, int lengthInSeconds, int skipTo);
    void stopDragging();
    bool dragDropAllowed(int row);
    void requestSearchResultsUpdate(MyList* results);
    YammiModel* getModel() {
        return model;
    };
    bool isValidState() {
        return validState;
    };

    QString getColumnName(int column);

public slots:
    void songListPopup(const QPoint& pos);
    void deleteEntry(Song* s);
    void slotFolderChanged(const QModelIndex& current, const QModelIndex& previous);
    void updatePlaylist();
    void updateHtmlPlaylist();
    
    void updatePlayerStatus();

    // forSelection methods
    // ********************
    // all forSelection...() methods perform an action on a selection of songs
    // see selectionMode for possible selections of song

    void forSelectionPrelistenStart()     {
        forSelectionPrelisten(0);
    }
    void forSelectionPrelistenMiddle()    {
        forSelectionPrelisten(33);
    }
    void forSelectionPrelistenEnd()       {
        forSelectionPrelisten(95);
    }
    void forSelectionPrelisten(int where);
    void forSelectionMove();
    void forSelectionPlugin();
    void forSelectionCheckConsistency();
    void openFolderInFileManager();

    /** Enqueue the selected songs at the end of the Playlist.
     * If the Shift key is pressed, the songs are shuffled before being appended */
    void forSelectionEnqueue( );
    /** Enqueue the selected songs at the beginning of the Playlist
      * If the Shift key is pressed, the songs are shuffled before being prepended */
    void forSelectionEnqueueAsNext( );
    /** Put the selected songs at the beginning of the playlist and start the player */
    void forSelectionPlayNow( );
    /** Remove all selected songs from the Playlist */
    void forSelectionDequeue( );
    /** Show information about selected songs.
      * If there is only one song selected, this function just calls songInfo(s) for the selected song */
    void forSelectionSongInfo( );
    /** Delete selected songs */
    void forSelectionDelete( );


    void forAllCheckConsistency();
    void fixGenres();

    void searchForSimilarEntry()            {
        searchSimilar(0);
    }
    void searchForSimilarArtist()           {
        searchSimilar(1);
    }
    void searchForSimilarTitle()            {
        searchSimilar(2);
    }
    void searchForSimilarAlbum()            {
        searchSimilar(3);
    }
    void gotoFolderArtist()                 {
        goToFolder(1);
    }
    void gotoFolderAlbum()                  {
        goToFolder(2);
    }
    void gotoFolderGenre()                  {
        goToFolder(3);
    }
    void gotoFolderYear()                   {
        goToFolder(4);
    }

    /** Remove all songs from the playlist */
    void clearPlaylist();

    void autoplayFolder();

    virtual void changeEvent(QEvent *e);

public:
    MediaPlayer*  player;
    MyListView* songListView;
    FolderModel* songListViewModel;
    QTextEdit* playlistPart;
    Folder* chosenFolder;
    QWaitCondition searchFieldChangedIndicator;


    // song that is currently played or 0 if not in database
    Song* currentSong;
    // timestamp when song was started playing
    MyDateTime* currentSongStarted;
    bool controlPressed;
    bool shiftPressed;
    FolderSorted* folderActual;
    FolderCategories* folderCategories;
    // songs played in this session
    Folder* folderSongsPlayed;
    Folder* folderRecentAdditions;
    // name of chosen autoplay folder, or empty if autoplay off
    QString autoplayFoldername;

    Folder* fuzzyFolderList[FUZZY_FOLDER_LIST_SIZE];
    QString fuzzyFolderName;
    int fuzzyFolderNo;
    
    /** this list contains a selection of songs to work on */
    MyList selectedSongs;
    MyList searchResults;
    bool isScanning;
    void updateSongPopup();
    void setSelectionMode(SelectionMode mode);

protected:
    void createMenuBar( );
    void createSongPopup( );
    void createFolders( );
    void createMainWidget( );
    void createToolbars();
    void createTrayIcon();
    static int randomNum(int numbers = RAND_MAX);

    // gui
    //***************
    QTreeWidget* folderListView;
    QSplitter* centralWidget;
    QSplitter* leftWidget;
    QSystemTrayIcon* trayIcon;

    QMenu* songCategoryPopup;
    QMenu* songPopup;
    QMenu* songPrelistenPopup;
    QMenu* songAdvancedPopup;
    QMenu* pluginPopup;
    QMenu* folderPopup;
    QMenu* gotoPopup;
    QMenu* searchSimilarPopup;
    QMenu* advancedPopup;


    YammiModel* model;
    // the thread doing the fuzzy search in background
    SearchThread* searchThread;

    SelectionMode selectionMode;

    QTimer regularTimer;
    QTimer searchResultsTimer;
    QTimer checkTimer;
    QProcess* prelistenProcess;

    // folders
    Folder* folderAll;


    // protected methods
    //******************
protected:
    void		forSelection(int action);
    void          gotoFuzzyFolder(bool backward);
    void          changeToFolder(Folder* newFolder, bool changeAnyway=false);
    void          folderContentChanged(Folder* folder = NULL);
    void          updateCurrentSongStatus();
    void          autoFillPlaylist();
    void          feedAutoplayQueueIntoMixxxAutoDJ();
    Folder*       getFolderByName(QString foldername);
    void 			    decide(Song* s1, Song* s2);
    long double   diskUsage(QString path, long double sizeLimit);
    void			    keyPressEvent(QKeyEvent* e);
    void			    keyReleaseEvent(QKeyEvent* e);

    void          handleLastSong(Song* lastSong);
    void          handleNewSong(Song* newSong);
    void			    getCurrentSong();
    void			    getSelectedSongs();
    void			    getAllSongs();
    FolderGroups*	folderArtists;
    FolderGroups*	folderAlbums;
    FolderGroups* folderGenres;
    FolderGroups* folderYears;
    Folder* 			folderUnclassified;
public:
    Folder* folderProblematic;
    Folder* folderHistory; // songs played sometime
    Folder* folderSearchResults;
protected:

    Folder*				folderToAdd;					// for snappy folder adding in background
    Folder*       toFromRememberFolder;
    int           autoplayMode;
    int           skipBigFolderRefresh;


    // protected slots
    //****************
protected slots:
    void toggleColumnVisibility(QAction* action);
    void preListen(Song* s, int skipTo);  ///< sends the song to headphones
    void stopPrelisten();
    void shufflePlaylist();
    void updateSearchResults();


    void toFromPlaylist();

    void setPreferences();
    void configureKeys();

    void searchSimilar(int what);
    void goToFolder(int what);
    void slotSongChanged();
    void autoplayOff();
    void autoplayLNP();
    void autoplayRandom();
    void doSongPopup(QPoint point);
    void slotFolderPopup(const QPoint& point);
    void adjustSongPopup();

    void doubleClicked();
    void middleClicked();

    void saveDatabase();
    void updateSongDatabaseHarddisk();
    void updateSongDatabaseSingleFile();
    void updateSongDatabase(QString scanDir, QString filePattern);
    void updateView(bool startup=false);

    void addToWishList();
    void toCategory();

    void onTimer();

    /** create new category */
    bool newCategory();
    void removeCategory();
    void renameCategory();
    void loadM3uIntoCategory();

    void pluginOnFolder();

    void slotLoadInMixxxDeck1();
    void slotLoadInMixxxDeck2();
    void loadSelectedSongInMixxxDeck(int deckNumber);
    void slotLoadSelectedSongInMixxxAutoDJ();
    void slotEnqueueAsNextInOtherYammi();

    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void trayIconMenuAboutToShow();
    void trayIconMenuAboutToHide();

    void startSecondYammi();

private:

    QAction* m_actionSecondYammi;
    QAction* m_actionQuit;
    QAction* m_actionSelectAll;
    QAction* m_actionInvertSelection;
    QAction* m_actionUpdateView;
    QActionGroup* m_actionGroupToggleToolbar;
    QAction* m_actionToggleMainToolbar;
    QAction* m_actionToggleMediaPlayerToolbar;
    QAction* m_actionToggleTimeDisplayToolbar;
    QAction* m_actionToggleSongActionsToolbar;
    QAction* m_actionTogglePrelistenToolbar;
    QActionGroup* m_actionGroupColumnVisibility;
    QAction* m_actionPlayPause;
    QAction* m_actionSkipBackward;
    QAction* m_actionSkipForward;
    QAction* m_actionStop;
    QAction* m_actionToFromPlaylist;
    QAction* m_actionClearPlayList;
    QAction* m_actionShufflePlaylist;
    QAction* m_actionSaveDatabase;
    QAction* m_actionScanHarddisk;
    QAction* m_actionImportSelectedFiles;
    QAction* m_actionCheckConsistencyAll;
    QAction* m_actionFixGenres;
    QActionGroup* m_actionGroupAutoplay;
    QAction* m_actionAutoplayOff;
    QAction* m_actionAutoplayLnp;
    QAction* m_actionAutoplayRandom;
    QAction* m_actionCurrentAutoPlay;
    QAction* m_actionAutoplayFeedMixxx;
    QAction* m_actionConfigureYammi;
    QAction* m_actionEnqueueAtEnd;
    QAction* m_actionEnqueueAsNext;
    QAction* m_actionPlayNow;
    QAction* m_actionDequeueSong;
    QAction* m_actionPrelistenStart;
    QAction* m_actionPrelistenMiddle;
    QAction* m_actionPrelistenEnd;
    QAction* m_actionStopPrelisten;
    QAction* m_actionLoadInMixxxDeck1;
    QAction* m_actionLoadInMixxxDeck2;
    QAction* m_actionLoadInMixxxAutoDJ;
    QAction* m_actionEnqueueAsNextInOtherYammi;
    QAction* m_actionSongInfo;
    QAction* m_actionGotoFolderArtist;
    QAction* m_actionGotoFolderAlbum;
    QAction* m_actionGotoFolderGenre;
    QAction* m_actionGotoFolderYear;
    QAction* m_actionSearchSimilarEntry;
    QAction* m_actionSearchSimilarArtist;
    QAction* m_actionSearchSimilarTitle;
    QAction* m_actionSimilarAlbum;
    QAction* m_actionOpenFolderInFilemanager;
    QAction* m_actionCheckConsistencySelection;
    QAction* m_actionDeleteSong;
    QAction* m_actionMoveFiles;
    QAction* m_actionSongPopupHeader;
    QAction* m_actionMinimize;
    QAction* m_actionMaximize;
    QAction* m_actionRestore;

    QLabel* m_current_song_label;

    QShortcut* m_shortcutSkipBackward;
    QShortcut* m_shortcutPlayPause;
    QShortcut* m_shortcutStop;
    QShortcut* m_shortcutSkipForward;
};

#endif
