/***************************************************************************
                          foldersorted.cpp  -  description
                             -------------------
    begin                : Thu Mar 28 2002
    copyright            : (C) 2002 by Oliver N�lle
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

#include "foldersorted.h"

#include "yammigui.h"

extern YammiGui* gYammiGui;


// construct a folder (top-level)
FolderSorted::FolderSorted( QListView *parent, const QString &name )
			: Folder(parent, name)
{
	sorted=true;
}

// construct a folder (not top-level)
FolderSorted::FolderSorted( QListViewItem* parent, const QString &name )
			: Folder(parent, name)
{
	sorted=true;
}

FolderSorted::~FolderSorted()
{
// the following line causes a crash???
//	delete(songList);
}

// update the view (after changes in songlist)
void FolderSorted::update(MyList* songList)
{
	this->songList=songList;
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


// corrects the index entries (intInfo) in all the songEntries
// (= takes the current order)
void FolderSorted::correctOrder()
{
	int index=1;
	for(SongEntry* entry=songList->first(); entry; entry=songList->next()) {
		((SongEntryInt*)entry)->intInfo=index;
		index++;
	}
	setText(0, fName+QString(" (%1)").arg(songList->count()));
	songList->dirty=true;
}

// syncs the folder entries with the ones shown in the list
// so far: only works if sorted after position
// => only for sorted folders
void FolderSorted::syncWithListView(MyListView* listView)
{
	songList->clear();		// don't use folder->clearSongs() as it affects the list!
	// what if order reverse?
	int index=1;
	for(QListViewItem* i=listView->firstChild(); i; i=i->itemBelow()) {
		// don't use addEntry for the same reason...
		songList->append(new SongEntryInt( ((SongListItem*)i)->song(), index));
		index++;
	}
	updateTitle();
	songList->dirty=true;
}


// insert content menu...
void FolderSorted::popup(QPoint point, QPopupMenu* contentMenu)
{
	allPopup=new QPopupMenu();
	if (folderPopup) {
		allPopup->insertItem("Folder...", folderPopup);
    cout << "checking..\n";
    if(gYammiGui->folderAutoplay==this) {
      cout << "yep!\n";
      this->folderPopup->setItemChecked(13, true);
    }
    else {
      cout << "no!\n";
      this->folderPopup->setItemChecked(13, false);      
    }

  }
	if (contentMenu)
		allPopup->insertItem("Content...", contentMenu);
	if(folderPopup || contentMenu)
		allPopup->popup(point);
}

