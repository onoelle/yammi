/***************************************************************************
                          yammigui.h  -  description
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

#ifndef YAMMIGUI_H
#define YAMMIGUI_H

#define MAX_COLUMN_NO 15

#include "options.h"

// general includes
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace std;

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
#include <qmainwindow.h>
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

#ifdef ENABLE_XMMS
#include "xmmsplayer.h"
#endif

#ifdef ENABLE_NOATUN
#include "noatunplayer.h"
#endif
// -----------------------------------------------------------------


/**
 * This is the main class: horribly huge... a total mess ... sorry...
 * I am working on cleaning it up...honestly...
 */
class YammiGui : public QMainWindow
{
	Q_OBJECT

// constructors
//*************
public:
	YammiGui	(QWidget *parent = 0, const char *name = 0);
	~YammiGui	();
  
// public members
//***************
public:
	MyListView*		songListView;
	Folder*		    chosenFolder;
	QToolButton*	tbPlayPause;
	Song*					currentSong;					// song that is currently played or 0 if not in database
	MyDateTime		currentSongStarted;		// timestamp when song was started playing
	bool					controlPressed;
	bool					shiftPressed;
	FolderSorted*	folderActual;
	FolderCategories* folderCategories;
	Folder*				folderSongsPlayed;		// songs played in this session
  FolderSorted* folderAutoplay;


// public methods
//***************
public:
  /// checks whether the swapped songs take more space than the given limit
  void          checkSwapSize();
  void			    stopDragging();
  YammiModel*   getModel() { return model; };
  void          commitData(QSessionManager& sm);
  void          saveState(QSessionManager& sm);
  QString       makeReplacements(QString input, Song* s, int index);


// public slots
//*************
public slots:
	void				  forSong(Song* s, action act, QString dir=0);
  void				  checkForGrabbedTrack();
	void				  slotFolderChanged();
  void          updatePlaylist();
  void          updatePlayerStatus();
  void          selectAll();									  /** selects all in songListView */
  void          invertSelection();							/** inverts selection in songListView */



// protected members
//******************
protected:
	int				    shuttingDown;
  QString       lastPrelistened;
	MyList		    selectedSongs;
	MyList		    searchResults;
  bool          isScanning;
  void          readSettings();
  void          writeSettings();
  void          moveEvent(QMoveEvent* e)         { updateGeometrySettings(); }
  void          resizeEvent(QResizeEvent* e)     { updateGeometrySettings(); }
  void          updateGeometrySettings();
	void			    updateSongPopup();
	void			    updateListViewColumns();

	// gui
	//***************
	QListView*		folderListView;
	LineEditShift*	searchField;
	QPushButton*	currentSongLabel;
	QComboBox*		mediaListCombo;
	QPushButton*	loadFromMediaButton;
	QSpinBox*			sleepModeSpinBox;
	QLabel*				sleepModeLabel;
	QPushButton* 	sleepModeButton;
	QPopupMenu*		playListPopup;
	QPopupMenu* 	songPopup;
	QPopupMenu* 	songPlayPopup;
	QPopupMenu* 	songPrelistenPopup;
	QPopupMenu* 	songSearchPopup;
	QPopupMenu* 	songAdvancedPopup;
	QPopupMenu* 	pluginPopup;
	QPopupMenu* 	folderPopup;
	QStatusBar* 	mainStatusBar;
	QSlider*			songSlider;
	bool					isSongSliderGrabbed;
	YammiModel*		model;								// pointer to our model
  MediaPlayer*  player;
	QString				currentFile;					// file that is currently played by player
	QString				grabbedTrackFilename;	// filename of new track being grabbed


// move the following into a (desktop)settings class?
  int           geometryX;
  int           geometryY;
  int           geometryWidth;
  int           geometryHeight;
  QStringList   columnOrder;
  int           columnWidth[MAX_COLUMN_NO];


	QTimer				regularTimer;
	QTimer				checkTimer;
	QTimer				typeTimer;
	int						songsUntilShutdown;



	// folders
	Folder*				folderAll;
	Folder*				folderSearchResults;


// protected methods
//******************
protected:
	void 			    decide(Song* s1, Song* s2);
  long double   diskUsage(QString path, long double sizeLimit);
	void			    keyPressEvent(QKeyEvent* e);
	void			    keyReleaseEvent(QKeyEvent* e);

	void			    getCurrentSong();
	void			    getCurrentlyPlayedSong();
	void			    getSelectedSongs();
	void			    getAllSongs();
	FolderGroups*	folderArtists;
	FolderGroups*	folderAlbums;
	FolderGroups* folderGenres;
	FolderMedia*	folderMedia;
	Folder* 			folderUnclassified;
	Folder*				folderProblematic;
	Folder*				folderHistory;				// songs played sometime

  Folder*				folderToAdd;					// for snappy folder adding in background
	int						alreadyAdded;
	void					addFolderContent(Folder* folder);

		


// protected slots
//****************
protected slots:
  void          finishInitialization();

	void				  forAllSelectedEnqueue()            { forAllSelected(Enqueue); }
	void				  forAllSelectedEnqueueAsNext()      { forAllSelected(EnqueueAsNext); }
	void				  forAllSelectedPlayNow()            { forAllSelected(PlayNow); }
	void				  forAllSelectedPrelistenStart()     { forAllSelected(PrelistenStart); }
	void				  forAllSelectedPrelistenMiddle()    { forAllSelected(PrelistenMiddle); }
	void				  forAllSelectedPrelistenEnd()       { forAllSelected(PrelistenEnd); }
	void				  forAllSelectedDequeue()            { forAllSelected(Dequeue); }
	void				  forAllSelectedSongInfo()           { forAllSelected(SongInfo); }
  void				  preListen(Song* s, int skipTo);  ///< sends the song to headphones
  void				  stopPrelisten();
  void          clearPlaylist();
  void          saveColumnSettings();
	void				  endProgram();
	void				  shutDown();
	void				  changeSleepMode();
  void          changeShutdownValue(int value);
	void				  setPreferences();
	void				  openHelp();
	void				  aboutDialog();
	void				  userTyped( const QString& searchStr );
	void				  songSliderMoved();
	void				  songSliderGrabbed();
	void				  searchSimilar(int what);
	void				  searchFieldChanged();
	void				  slotSongChanged();
	void				  currentlyPlayedSongPopup();
	void				  songListPopup( QListViewItem*, const QPoint &, int );
	void				  doSongPopup(QPoint point);
	void				  slotFolderPopup( QListViewItem*, const QPoint &, int );
	void				  adjustSongPopup();
	QIconSet		  getPopupIcon(action whichAction);

	void				  doubleClick();
	void				  middleClick(int button);

	void				  updateSongDatabaseHarddisk();
	void				  updateSongDatabaseMedia();
	void				  updateSongDatabase(QString scanDir, QString filePattern, QString media);
	void				  updateView();
		
	// song action slots
	void				  forAllCheckConsistency();
		
	// these three methods perform an action for...
	void				  forCurrent(action act);						// ..current song
	void				  forAllSelected(action act);				// ..all selected (in songlist)
	void				  forAll(action act);								// ..all songs
		
		
	// this works on the songs in selection
	// (can be different than currently selected songs!)
	void				  forSelection(action act);																	// perform <action> on <selectedSongs>
	void				  forSelection(int act) {forSelection((action) act);}				// just needed for menu receivers with ints

  // special treatment needed for the following cases
	void				  forSelectionSongInfo();
	void				  forSelectionPlugin(int pluginIndex);
	void				  forSelectionBurnToMedia();
  void          forSelectionCheckConsistency();
  void          skipBackward();
  void          skipForward();
	void 					addToWishList();
	void					toCategory(int index);

  void 					onTimer();
	// removable media management
	void					checkPlaylistAvailability();
	void					loadSongsFromMedia(QString mediaName);
	void					loadMedia();
		
  bool 					newCategory();	  					/** create new category */
	void 					removeCategory();
	void					renameCategory();
	void					autoplayCategory();
	
	void					pluginOnFolder();
	void					removeMedia();
	void					renameMedia();
	void					grabAndEncode();
	void 					addFolderContentSnappy();
		
};

#endif

