/***************************************************************************
                          folder.h  -  description
                             -------------------
    begin                : Fri Feb 23 2001
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
#ifndef FOLDER_H
#define FOLDER_H

#include <stream.h>
#include <qobject.h>
#include <qlistview.h>
#include <qlist.h>
#include <qpopupmenu.h>

#include "song.h"
#include "songentry.h"
#include "mylist.h"

// represents a folder on the left
// a folder can
// - contain subfolders
// - have a popup-menu
// - can contain items
class Folder : public QObject, public QListViewItem
{
	Q_OBJECT
	
public:
	Folder		(QListView *parent, const QString &name );						// top-level folder
	Folder		(QListViewItem *parent, const QString &name );				// subfolder
	~Folder()	{};
	virtual 	void update(MyList* allSongs);					// common update method

	Song*			firstSong();
	Song*			nextSong();

	void			addSong(Song* s)    				{ songList.append(new SongEntry(s)); }	// adds a Song as simple SongEntry
	void			addEntry(SongEntry* entry)	{ songList.append(entry); }							// adds a given SongEntry
	
	void 			clearSongs()						{ songList.clear(); }						// clear songlist
	void 			removeSongEntry(SongEntry* entry )  { songList.removeRef(entry); }			// removes an item(=Song)

	QString		folderName()				{ return fName; }
	SongEntry* firstEntry()						{ return songList.first(); }
	SongEntry* nextEntry()						{ return songList.next(); }

	void popup(QPoint point, QPopupMenu* contentMenu);
	QPopupMenu* folderPopup;
	QPopupMenu* allPopup;
	MyList songList;

public slots:
	void enqueueFolder();
	void burnFolder();

signals:
	void EnqueueFolder();
	void BurnFolder();

protected:
	QString fName;
};
//	void insertSubFolders( const QObjectList *lst );

#endif
