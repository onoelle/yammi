/***************************************************************************
                          folderactual.cpp  -  description
                             -------------------
    begin                : Sat Sep 15 2001
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

#include "folderactual.h"

FolderActual::FolderActual( QListView* parent, QString title)
		: Folder( parent, title )
{
}

FolderActual::~FolderActual()
{
}

// update the view
void FolderActual::update(QList<Song> lastPlayed)
{
	clearSongs();
	for(unsigned int i=0; i<lastPlayed.count(); i++) {
		addSong(lastPlayed.at(i));
	}
}