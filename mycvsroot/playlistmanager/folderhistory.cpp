/***************************************************************************
                          folderhistory.cpp  -  description
                             -------------------
    begin                : Sun Nov 4 2001
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

#include "folderhistory.h"

FolderHistory::FolderHistory( QListView* parent, QString title)
		: Folder( parent, title )
{
}

FolderHistory::~FolderHistory(){
}

// update the view
void FolderHistory::update(QList<Song> history)
{
	clearSongs();
	for(unsigned int i=0; i<history.count(); i++) {
		addSong(history.at(i));
	}
}