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


// gui-stuff
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
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qmainwindow.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qfiledialog.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>


#include <qapplication.h>
#include <qheader.h>
#include <qregexp.h>
#include <qdir.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <qstring.h>
#include <qobject.h>
#include <qdatetime.h>
#include <qlist.h>
#include <qtimer.h>
#include <qevent.h>
#include <qtooltip.h>

// xmms control
#include <xmms/xmmsctrl.h>

// my includes
#include "yammimodel.h"
#include "song.h"
#include "songentry.h"
#include "songentryint.h"
#include "songentrystring.h"
#include "songentrytimestamp.h"
#include "songlistitem.h"
#include "songinfo.h"
#include "fuzzsrch.h"
#include "folder.h"
#include "foldergroups.h"
#include "foldercategories.h"
#include "foldermedia.h"
#include "preferencesdialog.h"
#include "DeleteDialog.h"
#include "mylistview.h"



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
	void 			decide(Song* s1, Song* s2);
	void			keyPressEvent(QKeyEvent* e);
		
	void			getCurrentSong();
	void			getCurrentlyPlayedSong();
	void			getSelectedSongs();
	void			getAllSongs();
	MyList		selectedSongs;
	MyList		searchResults;
	Folder*		chosenFolder;
	void			updateSongPopup();
	void			updateListViewColumns();

	void			syncYammi2Xmms(bool syncAll=false);	// yammi -> xmms
	void			syncXmms2Yammi();		// xmms -> yammi

		
	// gui
	//***************
	MyListView*		songListView;			// draggggg
	QListView*		folderListView;
	QLineEdit*		searchField;
	QPushButton*	currentSongLabel;
	QPopupMenu*		playListPopup;
	QPopupMenu* 	songPopup;
	QPopupMenu* 	songPlayPopup;
	QPopupMenu* 	songPrelistenPopup;
	QPopupMenu* 	songAdvancedPopup;
	QPopupMenu* 	folderPopup;
	QMenuBar*			mainMenu;
	QPopupMenu* 	fileMenu;
	QPopupMenu* 	viewMenu;
	QPopupMenu* 	xmmsMenu;
	QPopupMenu* 	helpMenu;
	QStatusBar* 	mainStatusBar;					// status bar
	QToolBar*			toolBar;								// tool bar
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
	void				setPreferences();
	void				openHelp();
	void				aboutDialog();
	void				userTyped( const QString& searchStr );
	void				searchFieldChanged();
	void				slotFolderChanged();
	void				slotSongChanged();
	void				currentlyPlayedSongPopup();
	void				songListPopup( QListViewItem*, const QPoint &, int );
	void				doSongPopup(QPoint point);
	void				slotFolderPopup( QListViewItem*, const QPoint &, int );
	void				adjustSongPopup();
	QIconSet		getPopupIcon(action whichAction);

	void				doubleClick();
	void				middleClick(int button);

	void				updateSongDatabase();
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
	
protected:
	YammiModel*		model;								// pointer to our model
	QString				currentFile;					// file that is currently played by xmms
	QString				grabbedTrackFilename;	// filename of new track being grabbed

	QTimer				regularTimer;
	QTimer				typeTimer;
	int						songsUntilShutdown;


	// folders
	Folder*				folderAll;
	Folder*				folderSearchResults;
	Folder*				folderActual;
	FolderGroups*	folderArtists;
	FolderGroups*	folderAlbums;
	FolderCategories* folderCategories;
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

protected slots:		
	void 					addToWishList();
	void					toPlayList(int index);
	void					xmms_playPause();
	void					xmms_skipForward();
	void					xmms_skipBackward();
	void					xmms_clearPlaylist();
		
	void 					onTimer();
	Song*					getSongFromFilename(QString filename);
	
  void 					newCategory();	  					/** create new category */
	void 					removeCategory();
	void					enqueueFolder();
	void					burnFolder();
	void					removeMedia();
	void					enqueueCdTrack();
	void 					addFolderContentSnappy();
		
};

#endif

