/***************************************************************************
                          foldersorted.cpp  -  description
                             -------------------
    begin                : Thu Mar 28 2002
    copyright            : (C) 2002 by Oliver Nölle
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

#include "foldersorted.h"

#include "mylistview.h"
#include "songlistitem.h"
#include "songentryint.h"


using namespace std;

// construct a folder (top-level)
FolderSorted::FolderSorted( QTreeWidget *parent, const QString &name )
			: Folder(parent, name)
{
	sorted=true;
}

// construct a folder (not top-level)
FolderSorted::FolderSorted( QTreeWidgetItem* parent, const QString &name )
			: Folder(parent, name)
{
	sorted=true;
}

// construct a folder (top-level)
FolderSorted::FolderSorted( QTreeWidget *parent, const QString &name, MyList* songList)
			: Folder(parent, name, songList)
{
	sorted=true;  
}

// construct a folder (not top-level)
FolderSorted::FolderSorted( QTreeWidgetItem* parent, const QString &name, MyList* songList)
			: Folder(parent, name, songList)
{
	sorted=true;  
}


  
FolderSorted::~FolderSorted()
{
}

// update the view (after changes in songlist)
void FolderSorted::update( MyList& songList)
{
	Folder::update(songList);	
	updateTitle();
}



// add a song to the end of the sorted folder
void FolderSorted::addSong(Song* s)
{
	addEntry(new SongEntryInt(s, songList->count()+1));
}


// inserts a song at the specified position
void FolderSorted::insertSong(Song* s, int index)
{
	songList->insert(index, new SongEntryInt(s, index));
	songList->dirty=true;
	correctOrder();
}


// remove a given song
void FolderSorted::removeSong(Song* s)
{
	Folder::removeSong(s);
	correctOrder();
}


/**
 * corrects the index entries (intInfo) in all the songEntries
 * (= takes the current order)
 */
void FolderSorted::correctOrder()
{
	int index=1;
    for (MyList::iterator it = songList->begin(); it != songList->end(); it++) {
        ((SongEntryInt*)(*it))->intInfo=index;
		index++;
	}
	setText(0, fName+QString(" (%1)").arg(songList->count()));
	songList->dirty=true;
}

/**
 * syncs the folder entries with the ones shown in the list
 * so far: only works if sorted after position
 * => only for sorted folders
 */
void FolderSorted::syncWithListView(MyListView* listView)
{
	songList->clear();		// don't use folder->clearSongs() as it affects the list!
	// what if order reverse?
	int index=1;
	for(Q3ListViewItem* i=listView->firstChild(); i; i=i->itemBelow()) {
		// don't use addEntry for the same reason...
		songList->append(new SongEntryInt( ((SongListItem*)i)->song(), index));
		index++;
	}
	updateTitle();
	songList->dirty=true;
}

