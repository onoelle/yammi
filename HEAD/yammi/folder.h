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
#include "songentryint.h"
#include "mylist.h"

// represents a folder on the left
class Folder : public QObject, public QListViewItem
{
	Q_OBJECT
	
public:
	Folder					(QListView* parent, const QString &name );						// top-level folder
	Folder					(QListView* parent, const QString &name, MyList* songList);
	Folder					(QListViewItem* parent, const QString &name );				// subfolder
	Folder					(QListViewItem* parent, const QString &name, MyList* songList);
	~Folder();
	
  void						init(QString name);
	virtual void		update(MyList* songList);
	void 						clearSongs();
	void						updateTitle();

	Song*						firstSong();
	Song*						nextSong();

	// these methods add songs..
	virtual void		addSong(Song* s);
	virtual void		removeSong(Song* s);
	
	// ..and these song entries
	virtual void		addEntry(SongEntry* entry);
	virtual void		removeEntry(SongEntry* entry);
	

	QString					folderName()								{ return fName; }
	SongEntry*			firstEntry()								{ return songList->first(); }
	SongEntry*			nextEntry()									{ return songList->next(); }

	void 						popup(QPoint point, QPopupMenu* contentMenu);
	
	
	QPopupMenu*			folderPopup;
	QPopupMenu*			allPopup;
	MyList*					songList;
	bool						sorted;

protected:
	QString					fName;
};

#endif
