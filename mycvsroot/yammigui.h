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

// xmms control
#include <xmms/xmmsctrl.h>

// my includes
#include "yammimodel.h"
#include "song.h"
#include "songlistitem.h"
#include "songinfo.h"
#include "fuzzsrch.h"
#include "folder.h"
#include "folderall.h"
#include "folderunclassified.h"
#include "foldergroups.h"
#include "folderplaylists.h"
#include "foldermedia.h"
#include "folderactual.h"
#include "folderhistory.h"
#include "preferencesdialog.h"
#include "deletedialog.h"



// -----------------------------------------------------------------


/**
 * This is the main class: application + view
 */
class YammiGui : public QMainWindow
{
    Q_OBJECT

public:
    YammiGui( QWidget *parent = 0, const char *name = 0 );
    ~YammiGui();

protected:
		int				shuttingDown;
		void 			decide(Song* s1, Song* s2);
		void			keyPressEvent(QKeyEvent* e);
		
		void			getCurrentSong();
		void			getCurrentlyPlayedSong();
		void			getSelectedSongs();
		void			getAllSongs();
    MyList		selectedSongs;
    Folder*		chosenFolder;
		void			updateSongPopup();
		
    // gui
    //***************
		QListView*	songListView, *folderListView;
		QLineEdit*	searchField;
		QPushButton*		currentSongLabel;
		QPopupMenu*	playListPopup;
		QPopupMenu* songPopup;
		QPopupMenu* folderPopup;
		QMenuBar*		mainMenu;
		QPopupMenu* fileMenu;
		QPopupMenu* viewMenu;
		QPopupMenu* xmmsMenu;
		QPopupMenu* helpMenu;
		QStatusBar* mainStatusBar;					// status bar
		QToolBar*		toolBar;								// tool bar
		QToolButton* tbSaveDatabase;
		QToolButton* tbForward;
		QToolButton* tbBackward;
		QToolButton* tbPause;
		QToolButton* tbClearPlaylist;

protected slots:
		void				endProgram();
		void				shutDown();
		void				setPreferences();
		void				openHelp();
		void				aboutDialog();
		void				userTyped( const QString& searchStr );
    void				searchFieldChanged();
    void				slotFolderChanged();
    void				slotSortOrderChanged(int sortOrder);
    void				slotSongChanged();
		void				currentlyPlayedSongPopup();
    void				songListPopup( QListViewItem*, const QPoint &, int );
    void				doSongPopup(QPoint point);
    void				slotFolderPopup( QListViewItem*, const QPoint &, int );
		void				adjustSongPopup();

    void				doubleClick();
    void				middleClick(int button);

    void				updateView();
		
		// song action slots
		void				forAllCheckConsistency();
		void				forCurrent(action act);
		void				forAllSelected(action act);
		void				forAll(action act);
//		void				forAllSelected(int act) {forAllSelected((action) act);}	// just needed for menu receivers with ints
		void				forSelection(action act);																	// perform <action> on <selectedSongs>
		void				forSelection(int act) {forSelection((action) act);}				// just needed for menu receivers with ints
		void				forAllSelectedSongInfo();
		void				forSong(Song* s, action act, QString dir=0);
	  void				preListen(int useless);  ///< sends the song to headphones


// model
//***********************
protected:
		YammiModel*	model;							// pointer to our model
		void				addFolderContent(Folder* folder);

		QString			currentFile;				// file that is currently played by xmms
		QList<Song>	lastPlayed;					// contains the last n played songs, lastPlayed[n-1]=current Song
		QTimer			regularTimer;
		QTimer			typeTimer;
		int					songsUntilShutdown;


	// folders
	FolderAll* folderAll;
	FolderGroups* folderArtists;
	FolderGroups* folderAlbums;
	FolderPlayLists* folderPlayLists;
	Folder* folderSearchResults;
	FolderMedia* folderMedia;
	FolderActual* folderActual;
	FolderUnclassified* folderUnclassified;
	FolderHistory* folderHistory;
	Folder* folderProblematic;


protected slots:		
		void addToWishList();
		void toPlayList(int index);
		void xmms_playPause();
		void xmms_skipForward();
		void xmms_skipBackward();
		void xmms_clearPlaylist();
		
		void onTimer();
	
	  void newPlaylist();	  					/** create new playlist */
		void removePlaylist();
		void enqueueFolder();
		void burnFolder();
		void removeMedia();
		void enqueueCdTrack();
		
};

#endif

