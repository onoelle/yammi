/***************************************************************************
                          folderplaylists.cpp  -  description
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

#include "folderplaylists.h"

FolderPlayLists::FolderPlayLists( QListView* parent, QString title)
		: Folder( parent, title )
{
}


FolderPlayLists::~FolderPlayLists()
{
}


// update all playlists according to given structures
void FolderPlayLists::update(QList<MyList> allPlaylists, QStrList playlistNames)
{
	// we have to delete all existing items first!
	while(firstChild()) {
		QListViewItem* toDelete=firstChild();
		delete(toDelete);
	}
	
	MyList* ptr=allPlaylists.first();
	QString name=playlistNames.first();
	for(; ptr; ptr=allPlaylists.next(), name=playlistNames.next())
	{
		Folder *f = new Folder( this, name );
		f->folderPopup = new QPopupMenu( 0 );
		f->folderPopup->insertItem( "Remove playlist", this, SLOT(removePlaylist()));
		f->folderPopup->insertItem( "Enqueue", this, SLOT(enqueueFolder()));
		f->folderPopup->insertItem( "Burn folder...", this, SLOT(burnFolder()));
		
		for(Song* s=ptr->first(); s; s=ptr->next()) {
			f->addSong(s);
			s->classified=true;
		}
	}
}

void FolderPlayLists::removePlaylist()
{
	emit PlaylistRemoved();
}


