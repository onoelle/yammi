/***************************************************************************
                          folder.cpp  -  description
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

#include "folder.h"
#include "yammigui.h"

extern YammiGui* gYammiGui;

// constructs a top-level folder
Folder::Folder( QListView *parent, const QString &name )
			: QListViewItem( parent )
{
	init(name);
	this->songList=new MyList();
}

// construct a top-level folder
Folder::Folder( QListView* parent, const QString &name, MyList* songList)
			: QListViewItem( parent )
{
	init(name);
	this->songList=songList;
	updateTitle();
}

// construct a folder (not top-level)
Folder::Folder( QListViewItem* parent, const QString &name )
			: QListViewItem( parent )
{
	init(name);
	this->songList=new MyList();
}

// construct a folder (not top-level)
Folder::Folder( QListViewItem* parent, const QString &name, MyList* songList)
			: QListViewItem( parent )
{
	init(name);
	this->songList=songList;
	updateTitle();
}


// update the view (after changes in songlist)
void Folder::update(MyList* songList)
{
	this->songList=songList;
	updateTitle();
}

// clear songlist
void Folder::clearSongs()
{
	songList->clear();
	songList->dirty=true;
	updateTitle();
}


// inits the folder with a name
// tooltip???
void Folder::init(QString name)
{
	fName=name;
	setText( 0, fName );
	folderPopup=0;
	songList=0;
	sorted=false;
}

// clean up
// - songlist (if necessary)
Folder::~Folder()
{
	// following line: causing a crash???
//	delete(songList);
}


// adds a Song as simple SongEntry
void Folder::addSong(Song* s)
{
	addEntry(new SongEntry(s));
}

void Folder::removeSong(Song* s)
{
	songList->removeSong(s);
	updateTitle();
}

// adds a given SongEntry
void Folder::addEntry( SongEntry* entry)
{
	songList->append(entry);
	songList->dirty=true;
	updateTitle();
}


// removes an item(=Song)
void Folder::removeEntry(SongEntry* entry)
{
	songList->removeRef(entry);
	songList->dirty=true;
	updateTitle();
}


Song* Folder::firstSong()
{
	SongEntry* entry=songList->first();
	if(entry)
		return entry->song();
	else
		return 0;
}

Song* Folder::nextSong()
{
	SongEntry* entry=songList->next();
	if(entry)
		return entry->song();
	else
		return 0;
}



void Folder::updateTitle()
{
	setText(0, fName+QString(" (%1)").arg(songList->count()));
	if(this==gYammiGui->chosenFolder)
		gYammiGui->slotFolderChanged();
}

// insert content menu...
void Folder::popup(QPoint point, QPopupMenu* contentMenu)
{
	allPopup=new QPopupMenu();
	if (folderPopup)
		allPopup->insertItem("Folder...", folderPopup);
	if (contentMenu)
		allPopup->insertItem("Content...", contentMenu);
	if(folderPopup || contentMenu)
		allPopup->popup(point);
}


