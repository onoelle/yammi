/***************************************************************************
                          folder.h  -  description
                             -------------------
    begin                : Fri Feb 23 2001
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
#ifndef FOLDER_H
#define FOLDER_H

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
	virtual void		update( MyList& songList);
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
  void					  setFolderName(QString folderName)	{ fName=folderName; }
	SongEntry*			firstEntry()								{ return songList->first(); }
	SongEntry*			nextEntry()									{ return songList->next(); }

	virtual void 		popup(QPoint point, QPopupMenu* contentMenu);
	
	MyList& songlist() { return *songList; }
	
	QPopupMenu*			folderPopup;
	QPopupMenu*			allPopup;	
  bool            isSorted()                  { return sorted; }
public slots:
  void            autoplayFolder();


protected:
	QString					fName;
	bool						sorted;
	bool            owner;

	MyList*					songList;

	Folder(const Folder&);
	Folder& operator=(const Folder&);
	
};

#endif
