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

#define MAX_COLUMN_NO 15

#include <kmainwindow.h>

#include <qtimer.h>
#include <qwaitcondition.h>
#include "options.h"
//#include "prefs.h"
#include "mylist.h"
#include "yammidcopiface.h"



class QSlider;
class QListView;
class QListViewItem;
class QComboBox;
class QPushButton;
class QSpinBox;

class YammiModel;
class MediaPlayer;
class MyListView;
class LineEditShift;
class Folder;
class FolderGroups;
class FolderSorted;
class FolderCategories;
class FolderMedia;
class Song;
class KToggleAction;
class SearchThread;
class TrackPositionSlider;
class Prefs;
// -----------------------------------------------------------------



/**
 * This is the main class.
 *
 * ...still way to big and unordered...cleaning in progress (luis and oliver)
 */
class YammiGui : public KMainWindow, virtual public YammiDcopIface {
    Q_OBJECT
public:
    /**
     * Determines the way the playlist is filled up when it contains less then 5 songs.
     */
    enum AutoplayMode { AUTOPLAY_OFF = 0, AUTOPLAY_LNP = 1, AUTOPLAY_RANDOM = 2, AUTOPLAY_FOLDER = 10, FUZZY_FOLDER_LIST_SIZE = 50 };
    /**
     * Enumeration for the available columns.
     */
    enum Columns {COLUMN_ARTIST = 0, COLUMN_TITLE = 1, COLUMN_ALBUM = 2, COLUMN_LENGTH = 3, COLUMN_YEAR = 4,  COLUMN_TRACKNR = 5,
                  COLUMN_GENRE = 6, COLUMN_ADDED_TO = 7, COLUMN_BITRATE = 8, COLUMN_FILENAME = 9, COLUMN_PATH = 10, COLUMN_COMMENT = 11,
                  COLUMN_LAST_PLAYED = 12 };

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
    enum SelectionMode {SELECTION_MODE_USER_SELECTED = 0, SELECTION_MODE_FOLDER = 1, SELECTION_MODE_ALL = 2, SELECTION_MODE_CURRENT = 3,
                        SELECTION_MODE_CUSTOM = 4};

public:
    YammiGui();
    virtual ~YammiGui();

    /** Load the Song database.
      * @param db Database file. If empty the file set in yammi's config file, or 
      *           the default will be used */
    void loadDatabase(QString databaseDir);

    /** Start a count-down and then shut down yammi unless the user decides to cancel */
    void shutdownSequence( );

    /** Yammi config options (preferences) */
    Prefs* config();
    
public slots:
    /** Seeks in the current song (if any) to position pos */
    void seek( int pos );
    void seekWithWheel(int rotation);

    /** Request a fuzzy search in the song database, and switch to the search-results view */
    void searchFieldChanged( const QString &fuzzy );

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

    /** Save information that should be recovered when the app is restored*/
    void saveProperties(KConfig *config);
    /** Read information from last session when the app is restored by KDE*/
    void readProperties(KConfig *config);
    /** Return the configuration (preferences) object. This is not KDE's KConfig object*/

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
    void toolbarToggled( const QString& name = QString::null );
    /** Turn on/off the sleep mode */
    void changeSleepMode();
private:
    /** Setup UI-actions*/
    bool setupActions( );

private:
//    Prefs m_config;
    bool validState;
    TrackPositionSlider* m_seekSlider;
    LineEditShift* m_searchField;
    bool searchResultsUpdateNeeded;
    bool m_acceptSearchResults;
    bool m_sleepMode;
    QSpinBox* m_sleepModeSpinBox;
    QPushButton* m_sleepModeButton;

public:
    // checks whether the swapped songs take more space than the given limit
    void checkSwapSize();
    void stopDragging();
    void requestSearchResultsUpdate(MyList* results);
    YammiModel* getModel() {
        return model;
    };
    bool isValidState() {
        return validState;
    };

    bool columnIsVisible(int column);
    int mapToRealColumn(int column);
    void mapVisibleColumnsToOriginals();
    QString getColumnName(int column);

public slots:
    void songListPopup(QListViewItem*, const QPoint&, int);
    void deleteEntry(Song* s);
    void checkForGrabbedTrack();
    void slotFolderChanged();
    void updatePlaylist();
    void updatePlayerStatus();

    void selectAll();
    void invertSelection();

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
    void forSelectionPlugin(int pluginIndex);
    void forSelectionBurnToMedia();
    void forSelectionCheckConsistency();

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

public:
    MediaPlayer*  player;
    MyListView* songListView;
    Folder* chosenFolder;
    QWaitCondition searchFieldChangedIndicator;


    // song that is currently played or 0 if not in database
    Song* currentSong;
    // timestamp when song was started playing
    MyDateTime currentSongStarted;
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
    bool columnVisible[MAX_COLUMN_NO];
    int realColumnMap[MAX_COLUMN_NO];
    QString lastPrelistened;
    /** this list contains a selection of songs to work on */
    MyList selectedSongs;
    MyList searchResults;
    bool isScanning;
    void updateSongPopup();
    void updateListViewColumns();
    void setSelectionMode(SelectionMode mode);

    /* DCOP functions, skipForward and skipBackward are used as is */
    void play();
    void pause();

    void aplayOff();
    void aplayLNP();
    void aplayRandom();

    int currentTime();
    int totalTime();

    QString songInfo();
    QString songArtist();
    QString songTitle();
    int songTrack();
    QString songTrack2D();
    QString songAlbum();
    QString songGenre();
    QString songComment();
    int songYear();

protected:
    void createMenuBar( );
    void createSongPopup( );
    void createFolders( );
    void createMainWidget( );
    static int randomNum(int numbers = RAND_MAX);

    // gui
    //***************
    QListView* folderListView;

    QComboBox* mediaListCombo;
    QPushButton* loadFromMediaButton;

    QPopupMenu* playListPopup;
    QPopupMenu* songPopup;
    QPopupMenu* songPlayPopup;
    QPopupMenu* songPrelistenPopup;
    QPopupMenu* songGoToPopup;
    QPopupMenu* songSearchPopup;
    QPopupMenu* songAdvancedPopup;
    QPopupMenu* pluginPopup;
    QPopupMenu* folderPopup;

    QPopupMenu*   lastSongPopupMenu;
    QPopupMenu*   currentSongPopupMenu;
    QPopupMenu*   nextSongPopupMenu;

    QPopupMenu*   toolbarsMenu;
    QPopupMenu*   columnsMenu;



    YammiModel* model;
    // file that is currently played by player
    QString currentFile;
    // filename of new track being grabbed
    QString grabbedTrackFilename;
    // the thread doing the fuzzy search in background
    SearchThread* searchThread;



    // move the following into a (desktop)settings class?
    int           geometryX;
    int           geometryY;
    int           geometryWidth;
    int           geometryHeight;
    QStringList   columnOrder;
    int           columnWidth[MAX_COLUMN_NO];

    SelectionMode selectionMode;

    QTimer regularTimer;
    QTimer searchResultsTimer;
    QTimer checkTimer;

    // folders
    Folder* folderAll;
    Folder* folderSearchResults;


    // protected methods
    //******************
protected:
    //	QIconSet 	getPopupIcon(Song::action whichAction);
    void		forSelection(int action);
    void          gotoFuzzyFolder(bool backward);
    void          changeToFolder(Folder* newFolder, bool changeAnyway=false);
    void          folderContentChanged();
    void          folderContentChanged(Folder* folder);
    void          updateCurrentSongStatus();
    void          autoFillPlaylist();
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
    FolderMedia*	folderMedia;
    Folder* 			folderUnclassified;
    Folder*				folderProblematic;
    Folder*				folderHistory;				// songs played sometime

    Folder*				folderToAdd;					// for snappy folder adding in background
    Folder*       toFromRememberFolder;
    int						alreadyAdded;
    void					addFolderContent(Folder* folder);
    int           autoplayMode;

    // UI - actions
    //need to keep track of this so that we can change the icon/text
    KAction* m_playPauseAction;
    KAction* m_currentAutoPlay;
    KToggleAction* m_autoplayActionOff;
    KToggleAction* m_autoplayActionLnp;
    KToggleAction* m_autoplayActionRandom;




    // protected slots
    //****************
protected slots:
    void toggleColumnVisibility(int column);
    void preListen(Song* s, int skipTo);  ///< sends the song to headphones
    void stopPrelisten();
    void shufflePlaylist();
    void updateSearchResults();


    void toFromPlaylist();
    void saveColumnSettings();

    void setPreferences();
    void configureKeys();

    void searchSimilar(int what);
    void goToFolder(int what);
    void slotSongChanged();
    void autoplayOff();
    void autoplayLNP();
    void autoplayRandom();
    void doSongPopup(QPoint point);
    void slotFolderPopup( QListViewItem*, const QPoint &, int );
    void adjustSongPopup();

    void doubleClick();
    void middleClick(int button);

    void saveDatabase();
    void updateSongDatabaseHarddisk();
    void updateSongDatabaseMedia();
    void updateSongDatabaseSingleFile();
    void updateSongDatabase(QString scanDir, QString filePattern, QString media);
    void updateView(bool startup=false);

    void skipBackward();
    void skipForward();
    void addToWishList();
    void toCategory(int index);

    void onTimer();
    // removable media management
    void checkPlaylistAvailability();
    void loadSongsFromMedia(QString mediaName);
    void loadMedia();

    /** create new category */
    bool newCategory();
    void removeCategory();
    void renameCategory();
    void loadM3uIntoCategory();

    void pluginOnFolder();
    void removeMedia();
    void renameMedia();
    void grabAndEncode();
    void addFolderContentSnappy();

    // dcop acessible functions
    void stop();
    void playPause();

};

#endif
