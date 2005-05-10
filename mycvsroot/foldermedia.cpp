/***************************************************************************
                          foldermedia.cpp  -  description
                             -------------------
    begin                : Thu Sep 6 2001
    copyright            : (C) 2001 by Brian O.Nlle
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

#include "foldermedia.h"

FolderMedia::FolderMedia( QListView* parent, QString title)
		: Folder( parent, title )
{
}

FolderMedia::~FolderMedia()
{
}


void FolderMedia::update(MyList* allSongs)
{	
	// we have to delete all existing items first!
	while(firstChild()) {
		QListViewItem* toDelete=firstChild();
		delete(toDelete);
	}
	
//	allSongs->setSortOrder(sortBy);
//	allSongs->sort();
	
	Song* s=allSongs->first();
	for(; s; s=allSongs->next()) {
		
		for(unsigned int i=0; i<s->mediaName.count(); i++) {
			// try to find an existing folder for that media
			bool found=false;
			for(QListViewItem* li=firstChild(); li; li=li->nextSibling()) {
				Folder* f2=(Folder*) li;
				if(f2->folderName()==s->mediaName[i]) {
					f2->addSong(s);
					found=true;
					break;
				}
			}

			if(!found) { 		// folder not existing => create
				Folder *f = new Folder( this, s->mediaName[i] );
				f->folderPopup = new QPopupMenu( 0 );
				f->folderPopup->insertItem( "Enqueue", this, SLOT(enqueueFolder()));
				f->folderPopup->insertItem( "Remove media", this, SLOT(removeMedia()));
				f->addSong(s);
			}
		}
	}
}

void FolderMedia::removeMedia()
{
	emit RemoveMedia();
}

