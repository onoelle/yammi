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
		// delete on Folder?
		delete(toDelete);
	}
	
	int noMedia=0;
	for(Song* s=allSongs->firstSong() ; s; s=allSongs->nextSong()) {
		
		for(unsigned int i=0; i<s->mediaName.count(); i++) {
			// try to find an existing folder for that media
			bool found=false;
			for(QListViewItem* li=firstChild(); li; li=li->nextSibling()) {
				Folder* f2=(Folder*) li;
				if(f2->folderName()==s->mediaName[i]) {
					f2->addSong(s);
					f2->setText( 0, s->mediaName[i]+QString(" (%1)").arg(f2->songList->count()) );
					found=true;
					break;
				}
			}

			if(!found) { 		// folder not existing => create
				noMedia++;
				Folder *f = new Folder( this, s->mediaName[i] );
				f->folderPopup = new QPopupMenu( 0 );
				f->folderPopup->insertItem( "Remove media", this, SLOT(removeMedia()));
				f->folderPopup->insertItem( "Rename media", this, SLOT(renameMedia()));
				f->addSong(s);
				f->setText(0, s->mediaName[i]+QString(" (1)"));
			}
		}
	}
	setText(0, fName+QString(" (%1)").arg(noMedia));
  sortChildItems(0, true);
}

