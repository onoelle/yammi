/***************************************************************************
                          folder.cpp  -  description
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

#include "folder.h"

#include <QMenu>

#include "yammigui.h"

extern YammiGui* gYammiGui;


// constructs a top-level folder
Folder::Folder( Q3ListView *parent, const QString &name )
			: Q3ListViewItem( parent ),
				owner(true)
{
	init(name);
	songList=new MyList();
	owner=true;
  sortedBy=-1;
}

// construct a top-level folder
Folder::Folder( Q3ListView* parent, const QString &name, MyList* songList)
			: Q3ListViewItem( parent )				
{
	init(name);
	this->songList=songList;
	updateTitle();
}

// construct a folder (not top-level)
Folder::Folder( Q3ListViewItem* parent, const QString &name )
			: Q3ListViewItem( parent )				
{
	init(name);
	songList=new MyList();
}

// construct a folder (not top-level)
Folder::Folder( Q3ListViewItem* parent, const QString &name, MyList* songList)
			: Q3ListViewItem( parent )			  
{
	init(name);
	this->songList=songList;
	updateTitle();
}


// update the view (after changes in songlist)
void Folder::update( MyList& songList)
{
	this->songList=&songList;
	owner = false;
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
	owner=false;
	sorted=false;
  sortedBy=0;
  scrollPosX=0;
  scrollPosY=0;
}

// clean up
// - songlist (if necessary)
Folder::~Folder()
{
	if( owner) delete(songList);
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
    songList->removeAll(entry);
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

void Folder::updateTitle()
{
	setText(0, fName+QString(" (%1)").arg(songList->count()));
}

/**
 * insert content menu...
 */
void Folder::popup(QPoint point, QMenu* contentMenu)
{
  allPopup=new QMenu();
  // autoplay
  allPopup->insertItem(tr("Autoplay"), this, SLOT(autoplayFolder()), 0, 13);
  if(gYammiGui->autoplayFoldername==this->folderName()) {
    allPopup->setItemChecked(13, true);
  }
  else {
    allPopup->setItemChecked(13, false);
  }

  // folder popup as defined in subclass
  if (folderPopup) {
        allPopup->insertItem(tr("Folder ..."), folderPopup);
  }

  // content menu (if folder contains at least one song)
	if (contentMenu) {
        allPopup->insertItem(tr("Content ..."), contentMenu);
  }
	allPopup->exec(point);
}

void Folder::autoplayFolder()
{
  gYammiGui->autoplayFolder();
}

/**
 * Saves the view settings (ordering, which song is at top).
 */
void Folder::saveSorting(int sortedBy)
{
  this->sortedBy=sortedBy;
}

/**
 * Saves the view settings (ordering, which song is at top).
 */
void Folder::saveScrollPos(int scrollPosX, int scrollPosY)
{
  this->scrollPosX=scrollPosX;
  this->scrollPosY=scrollPosY;
}

