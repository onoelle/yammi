/***************************************************************************
                          folderall.cpp  -  description
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
// olitest
#include "folderall.h"

FolderAll::FolderAll( QListView* parent, QString title)
		: Folder( parent, title )
{
}
	
// update the view (after changes in list allSongs)
void FolderAll::update(MyList* allSongs)
{
	clearSongs();
	Song* s=allSongs->first();
	while(s) {
		addSong(s);
		s=allSongs->next();
	}
	folderPopup = new QPopupMenu( 0 );
	folderPopup->insertItem( "Burn folder...", this, SLOT(burnFolder()));

}

FolderAll::~FolderAll()
{
}
