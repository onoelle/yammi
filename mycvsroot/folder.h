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
	Folder( QListView *parent, const QString &name );						// top-level folder
	Folder( QListViewItem *parent, const QString &name );				// subfolder
	~Folder()    	{}
	virtual void update(MyList* allSongs);


//	void insertSubFolders( const QObjectList *lst );
	void addSong( Song *s )    	{ songList.append(s); }					// adds an item(=Song)
	
	void clearSongs()						{ songList.clear(); }						// clear songlist
	void removeSong( Song *s )  { songList.removeRef(s); }			// removes an item(=Song)

	QString folderName()				{ return fName; }
	Song *firstSong()						{ return songList.first(); }
	Song *nextSong()						{ return songList.next(); }

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

#endif
