/***************************************************************************
                          yammigui.h  -  description
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

#ifndef YAMMIGUI_H
#define YAMMIGUI_H


// general includes
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>

// qt includes (non gui)
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
#include <qtooltip.h>

// qt includes (gui-stuff)
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

// xmms control
#include <xmmsctrl.h>

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

// dialog includes
#include "preferencesdialog.h"
#include "DeleteDialog.h"
#include "WorkDialogBase.h"


// -----------------------------------------------------------------


/**
 * This is the main class: application + view
 */
class YammiGui : public QMainWindow
{
	Q_OBJECT

public:
	YammiGui	(QWidget *parent = 0, const char *name = 0);
	~YammiGui	();
	void			stopDragging();
  YammiModel* getModel() { return model; };

protected:
	void			myWait(int msecs);
	int				shuttingDown;
	bool			xmmsShuffleWasActivated;
	void 			decide(Song* s1, Song* s2);
	void			keyPressEvent(QKeyEvent* e);
	void			keyReleaseEvent(QKeyEvent* e);
		
	void			getCurrentSong();
	void			getCurrentlyPlayedSong();
	void			getSelectedSongs();
	void			getAllSongs();
	MyList		selectedSongs;
	MyList		searchResults;
public:
	Folder*		chosenFolder;
protected:
	void			updateSongPopup();
	void			updateListViewColumns();

	void			syncYammi2Xmms(bool syncAll=false);	// yammi -> xmms
	void			syncXmms2Yammi();		// xmms -> yammi
public:
	QString		checkAvailability(Song* s, bool touch=false);
  /** checks whether the swapped songs take more space than the given limit
 */
  void checkSwapSize();
protected:
		
	// gui
	//***************
	MyListView*		songListView;
	QListView*		folderListView;
	QLineEdit*		searchField;
	QPushButton*	currentSongLabel;
	QComboBox*		mediaListCombo;
	QPushButton*	loadFromMediaButton;
	QSpinBox*			shutdownSpinBox;
	QLabel*				shutdownLabel;
	QPushButton* 	shutdownButton;
	QPopupMenu*		playListPopup;
	QPopupMenu* 	songPopup;
	QPopupMenu* 	songPlayPopup;
	QPopupMenu* 	songPrelistenPopup;
	QPopupMenu* 	songSearchPopup;
	QPopupMenu* 	songAdvancedPopup;
	QPopupMenu* 	songPluginPopup;
	QPopupMenu* 	playlistPluginPopup;
	QPopupMenu* 	folderPopup;
	QMenuBar*			mainMenu;
	QPopupMenu* 	fileMenu;
	QPopupMenu* 	viewMenu;
	QPopupMenu* 	xmmsMenu;
	QPopupMenu* 	helpMenu;
	QPopupMenu*		currentSongMenu;
	QStatusBar* 	mainStatusBar;					// status bar
	QToolBar*			toolBar;								// tool bar
	QSlider*			songSlider;
	bool					isSongSliderGrabbed;
public:	
	QToolButton*	tbSaveDatabase;
protected:
	QToolButton*	tbForward;
	QToolButton*	tbBackward;
	QToolButton*	tbPause;
	QToolButton*	tbClearPlaylist;
	QToolButton*	tbEnqueue;
	QToolButton*	tbEnqueueAsNext;
	QToolButton*	tbPlayNow;
	QToolButton*	tbPrelistenStart;
	QToolButton*	tbPrelistenMiddle;
	QToolButton*	tbPrelistenEnd;
	QToolButton*	tbStopPrelisten;
	QToolButton*	tbDequeueSong;
	QToolButton*	tbSongInfo;

protected slots:
	void				endProgram();
	void				shutDown();
	void				changeShutdownMode();
	void				setPreferences();
	void				openHelp();
	void				aboutDialog();
	void				userTyped( const QString& searchStr );
	void				songSliderMoved();
	void				songSliderGrabbed();
	void				searchSimilar(int what);
	void				searchFieldChanged();
	void				slotSongChanged();
	void				currentlyPlayedSongPopup();
	void				songListPopup( QListViewItem*, const QPoint &, int );
	void				doSongPopup(QPoint point);
	void				slotFolderPopup( QListViewItem*, const QPoint &, int );
	void				adjustSongPopup();
	QIconSet		getPopupIcon(action whichAction);

	void				doubleClick();
	void				middleClick(int button);

	void				updateSongDatabaseHarddisk();
	void				updateSongDatabaseMedia();
	void				updateSongDatabase(QString media);
	void				updateView();
		
	// song action slots
	void				forAllCheckConsistency();
		
	// these three methods perform an action for...
	void				forCurrent(action act);						// ..current song
	void				forAllSelected(action act);				// ..all selected (in songlist)
	void				forAll(action act);								// ..all songs
		
		
	// this works on the songs in selection
	// (can be different than currently selected songs!)
	void				forSelection(action act);																	// perform <action> on <selectedSongs>
	void				forSelection(int act) {forSelection((action) act);}				// just needed for menu receivers with ints
	// special treatment needed?
	void				forSelectionSongInfo();
	void				forSelectionPluginPlaylist(int pluginIndex);
	void				forSelectionPluginSong(int pluginIndex);
	void				forSelectionBurnToMedia();
		
	void				forSong(Song* s, action act, QString dir=0);
	
	void				forAllSelectedEnqueue() {forAllSelected(Enqueue); }
	void				forAllSelectedEnqueueAsNext() {forAllSelected(EnqueueAsNext); }
	void				forAllSelectedPlayNow() {forAllSelected(PlayNow); }
	void				forAllSelectedPrelistenStart() {forAllSelected(PrelistenStart); }
	void				forAllSelectedPrelistenMiddle() {forAllSelected(PrelistenMiddle); }
	void				forAllSelectedPrelistenEnd() {forAllSelected(PrelistenEnd); }
	void				forAllSelectedDequeue() {forAllSelected(Dequeue); }
	void				forAllSelectedSongInfo() {forAllSelected(SongInfo); }
  void				preListen(Song* s, int skipTo);  ///< sends the song to headphones
  void				stopPrelisten();

// model
//***********************
public:
	Song*					currentSong;					// song that is currently played or 0 if not in database
	MyDateTime		currentSongStarted;		// timestamp when song was started playing
	
protected:
	YammiModel*		model;								// pointer to our model
	QString				currentFile;					// file that is currently played by xmms
	QString				grabbedTrackFilename;	// filename of new track being grabbed

	QTimer				regularTimer;
	QTimer				typeTimer;
	int						songsUntilShutdown;
	bool					controlPressed;
	bool					shiftPressed;



	// folders
	Folder*				folderAll;
	Folder*				folderSearchResults;
public:
	FolderSorted*	folderActual;
protected:
	FolderGroups*	folderArtists;
	FolderGroups*	folderAlbums;
	FolderGroups* folderGenres;
public:	
	FolderCategories* folderCategories;
protected:
	FolderMedia*	folderMedia;
	Folder* 			folderUnclassified;
	Folder*				folderProblematic;
	Folder*				folderHistory;				// songs played sometime
	Folder*				folderSongsPlayed;		// songs played in this session
	
	Folder*				folderToAdd;					// for snappy folder adding in background
	int						alreadyAdded;
	void					addFolderContent(Folder* folder);

public slots:
  void					checkForGrabbedTrack();
	void					slotFolderChanged();

  void selectAll();									  /** selects all in songListView */
  void invertSelection();							/** inverts selection in songListView */

protected slots:		
	void 					addToWishList();
	void					toPlayList(int index);
	void					xmms_playPause();
	void					xmms_stop();
	void					xmms_skipForward();
	void					xmms_skipForwardIm();
	void					xmms_skipBackward();
	void					xmms_skipBackwardIm();
	void					xmms_clearPlaylist();
		
	void 					onTimer();
	// removable media management
	void					checkPlaylistAvailability();
	void					loadSongsFromMedia(QString mediaName);
	void					loadMedia();
	
	Song*					getSongFromFilename(QString filename);
	
  void 					newCategory();	  					/** create new category */
	void 					removeCategory();
	void					renameCategory();
	
	void					pluginOnFolder();
	void					removeMedia();
	void					renameMedia();
	void					grabAndEncode();
	void 					addFolderContentSnappy();
		
};

#endif

