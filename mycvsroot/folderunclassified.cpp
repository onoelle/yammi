/***************************************************************************
                          folderunclassified.cpp  -  description
                             -------------------
    begin                : Mon Oct 1 2001
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

#include "folderunclassified.h"

FolderUnclassified::FolderUnclassified( QListView* parent, QString title)
		: Folder( parent, title )
{
}


FolderUnclassified::~FolderUnclassified(){
}

// update the view (after changes in list allSongs)
void FolderUnclassified::update(MyList* allSongs)
{
	clearSongs();
	for(Song* s=allSongs->first(); s; s=allSongs->next()) {
		if(!s->classified)
			addSong(s);
	}
//	folderPopup = new QPopupMenu( 0 );
//	folderPopup->insertItem( "Burn folder...", this, SLOT(burnFolder()));
}
