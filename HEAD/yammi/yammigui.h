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


#include "options.h"
 

// qt includes (non gui)
#include <qsessionmanager.h>
#include <qapplication.h>
#include <qheader.h>
#include <qregexp.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qstring.h>
#include <qobject.h>
#include <qdatetime.h>
#include <qlist.h>
#include <qtimer.h>
#include <qevent.h>

// qt includes (gui-stuff)
#include <qtooltip.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qmenudata.h>
#include <qmenubar.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qfiledialog.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qslider.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qprogressdialog.h>
#include <qsettings.h>


// project includes
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

// -----------------------------------------------------------------



/**
 * This is the main class: horribly huge... a total mess ... sorry...
 * I am working on cleaning it up...honestly...
 */
class YammiGui : public KMainWindow
{
	Q_OBJECT
public:
// constants
  enum { AUTOPLAY_OFF = 0, AUTOPLAY_LNP = 1, AUTOPLAY_RANDOM = 2, AUTOPLAY_FOLDER = 10, FUZZY_FOLDER_LIST_SIZE = 50 };
  enum Columns {COLUMN_ARTIST = 0, COLUMN_TITLE = 1, COLUMN_ALBUM = 2, COLUMN_LENGTH = 3, COLUMN_YEAR = 4,  COLUMN_TRACKNR = 5,
	COLUMN_GENRE = 6, COLUMN_ADDED_TO = 7, COLUMN_BITRATE = 8, COLUMN_FILENAME = 9, COLUMN_PATH = 10, COLUMN_COMMENT = 11,
	COLUMN_LAST_PLAYED = 12 };


public:
	YammiGui(QString baseDir);
	virtual ~YammiGui();
  
	// checks whether the swapped songs take more space than the given limit
	void checkSwapSize();
	void stopDragging();
	YammiModel* getModel() { return model; };
	void commitData(QSessionManager& sm);
	void saveState(QSessionManager& sm);
	QString makeReplacements(QString input, Song* s, int index);
	bool columnIsVisible(int column);
	int mapToRealColumn(int column);
	void mapVisibleColumnsToOriginals();
	QString getColumnName(int column);
	
	public slots:
	void forSong(Song* s, Song::action act, QString dir=0);
	void checkForGrabbedTrack();
	void slotFolderChanged();
	void updatePlaylist();
	void updatePlayerStatus();
	void selectAll();
	void invertSelection();
	
	/** Enqueue the selected songs at the end of the Playlist.
	  * If the Shift key is pressed, the songs are shuffled before being appended */
	void appendSelected( );
	/** Enqueue the selected songs at the beginning of the Playlist
	  * If the Shift key is pressed, the songs are shuffled before being prepended */
	void prependSelected( );
	/** Put the selected songs at the beginning of the playlist and start the player */
	void playSelected( );
	/** Remove all selected songs from the Playlist */
	void dequeueSelected( );
	/** Remove all songs from the playlist */
	void clearPlaylist();
	/** Show information about selected songs.
	  * If there is only one song selected, this function just calls songInfo(s) for the selected song */
	void infoSelected( );
	/** Show Dialog to display/edit the song info */
	void songInfo( Song *s );
	
	void autoplayFolder();


	MediaPlayer*  player;
	MyListView* songListView;
	Folder* chosenFolder;
	
	
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
	int shuttingDown;
	QString lastPrelistened;
	MyList selectedSongs;
	MyList searchResults;
	bool isScanning;
	void readSettings();
	void writeSettings();
	void moveEvent(QMoveEvent* e)         { updateGeometrySettings(); }
	void resizeEvent(QResizeEvent* e)     { updateGeometrySettings(); }
	void updateGeometrySettings();
	void updateSongPopup();
	void updateListViewColumns();

protected:

	void setupActions( );

	void setupToolBars( );
	void createMenuBar( );

	
	void createFolders( );
	void createMainWidget( );
	void loadMediaPlayer( );
	
	// gui
	//***************
	QListView* folderListView;
	LineEditShift* searchField;

// 	KToolBar *m_mediaPlayerToolBar;
// 	KToolBar*     mainToolBar;
// 	KToolBar*     songActionsToolBar;
// 	KToolBar*     prelistenToolBar;
// 	KToolBar*     removableMediaToolBar;
// 	KToolBar*     sleepModeToolBar;
	
//	QPushButton*	currentSongLabel;
	QComboBox* mediaListCombo;
	QPushButton* loadFromMediaButton;
	QSpinBox* sleepModeSpinBox;
	QLabel* sleepModeLabel;
	QPushButton* sleepModeButton;
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
  
	QSlider* songSlider;
	bool isSongSliderGrabbed;
	YammiModel* model;
	// file that is currently played by player
	QString currentFile;
	// filename of new track being grabbed
	QString grabbedTrackFilename;

// move the following into a (desktop)settings class?
  int           geometryX;
  int           geometryY;
  int           geometryWidth;
  int           geometryHeight;
  QStringList   columnOrder;
  int           columnWidth[MAX_COLUMN_NO];


	QTimer regularTimer;
	QTimer checkTimer;
	QTimer typeTimer;
	int songsUntilShutdown;

	// folders
	Folder* folderAll;
	Folder* folderSearchResults;


// protected methods
//******************
protected:
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
	void			    getCurrentlyPlayedSong();
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

		


// protected slots
//****************
protected slots:
  void          toggleColumnVisibility(int column);
  void toggleToolbar( const QString& name = QString::null );
  void          finishInitialization();

	void forAllSelectedEnqueue();
	void forAllSelectedEnqueueAsNext();
	void forAllSelectedPlayNow()            { forAllSelected(Song::PlayNow); }
	void forAllSelectedPrelistenStart()     { forAllSelected(Song::PrelistenStart); }
	void forAllSelectedPrelistenMiddle()    { forAllSelected(Song::PrelistenMiddle); }
	void forAllSelectedPrelistenEnd()       { forAllSelected(Song::PrelistenEnd); }
	void forAllSelectedDequeue()            { forAllSelected(Song::Dequeue); }
  void preListen(Song* s, int skipTo);  ///< sends the song to headphones
  void stopPrelisten();
  void shufflePlaylist();
  
  void toFromPlaylist();
  void saveColumnSettings();
	void endProgram();
	void shutDown();
	void changeSleepMode();
  void changeShutdownValue(int value);
	void setPreferences();
	
	void userTyped( const QString& searchStr );
	void songSliderMoved();
	void songSliderGrabbed();
	void searchSimilar(int what);
	void goToFolder(int what);
	void searchFieldChanged();
	void slotSongChanged();
	void autoplayOff();
	void autoplayLNP();
	void autoplayRandom();
//	void currentlyPlayedSongPopup();
	void songListPopup( QListViewItem*, const QPoint &, int );
	void doSongPopup(QPoint point);
	void slotFolderPopup( QListViewItem*, const QPoint &, int );
	void adjustSongPopup();
	QIconSet getPopupIcon(Song::action whichAction);

	void doubleClick();
	void middleClick(int button);

	void updateSongDatabaseHarddisk();
	void updateSongDatabaseMedia();
	void updateSongDatabaseSingleFile();
	void updateSongDatabase(QString scanDir, QString filePattern, QString media);
	void updateView(bool startup=false);
		
	// song action slots
	void forAllCheckConsistency();
		
	// these three methods perform an action for...
	void forCurrent(Song::action act);
	// ..all selected (in songlist)
	void forAllSelected(Song::action act);
	void forAll(Song::action act);
		
		
	// this works on the songs in selection
	// (can be different than currently selected songs!)
	void forSelection(Song::action act); // perform <action> on <selectedSongs>
	void forSelection(int act) {forSelection((Song::action) act);} // just needed for menu receivers with ints

  // special treatment needed for the following cases
	void forSelectionPlugin(int pluginIndex);
	void forSelectionBurnToMedia();
	void forSelectionCheckConsistency();
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
	void  addFolderContentSnappy();
	
	// UI - actions
	//need to keep track of this so that we can change the icon/text
	KAction *m_playPauseAction;
	//FIXME - hack.. ugly?
	KAction *m_currentAutoPlay;
};

#endif

