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

#include "foldercategories.h"
#include "foldersorted.h"
#include "mylistview.h"
#include "yammigui.h"
#include "yammimodel.h"

extern YammiGui* gYammiGui;





// constructs a top-level folder
Folder::Folder( QTreeWidget *parent, const QString &name )
            : QTreeWidgetItem( parent ),
				owner(true)
{
	init(name);
	songList=new MyList();
	owner=true;
  sortedBy=-1;
}

// construct a top-level folder
Folder::Folder( QTreeWidget* parent, const QString &name, MyList* songList)
            : QTreeWidgetItem( parent )
{
	init(name);
	this->songList=songList;
	updateTitle();
}

// construct a folder (not top-level)
Folder::Folder( QTreeWidgetItem* parent, const QString &name )
            : QTreeWidgetItem( parent )
{
	init(name);
	songList=new MyList();
}

// construct a folder (not top-level)
Folder::Folder( QTreeWidgetItem* parent, const QString &name, MyList* songList)
            : QTreeWidgetItem( parent )
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
    m_actionAutoPlayAction = new QAction(tr("Autoplay"), this);
    m_actionAutoPlayAction->setCheckable(true);
    connect(m_actionAutoPlayAction, SIGNAL(triggered()), this, SLOT(autoplayFolder()));
	fName=name;
	setText( 0, fName );
	folderPopup=0;
	songList=0;
	owner=false;
	sorted=false;
  sortedBy=0;
  scrollPosX=0;
  scrollPosY=0;
    saveSorting(0, Qt::AscendingOrder);
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
  allPopup->addAction(m_actionAutoPlayAction);
  if(gYammiGui->autoplayFoldername==this->folderName()) {
      m_actionAutoPlayAction->setChecked(true);
  } else {
      m_actionAutoPlayAction->setChecked(false);
  }

  // folder popup as defined in subclass
  if (folderPopup) {
        allPopup->addMenu(folderPopup);
  }

  // content menu (if folder contains at least one song)
	if (contentMenu) {
        allPopup->addMenu(contentMenu);
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
void Folder::saveSorting(int sortedBy, Qt::SortOrder sortOrder)
{
    this->sortedBy = sortedBy;
    this->sortOrder = sortOrder;
}

/**
 * Saves the view settings (ordering, which song is at top).
 */
void Folder::saveScrollPos(int scrollPosX, int scrollPosY)
{
  this->scrollPosX=scrollPosX;
  this->scrollPosY=scrollPosY;
}

