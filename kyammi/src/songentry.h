/***************************************************************************
                          songentry.h  -  description
                             -------------------
    begin                : Fri Feb 8 2002
    copyright            : (C) 2002 by Oliver Nölle
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

#ifndef SONGENTRY_H
#define SONGENTRY_H

#include "song.h"

/**pointer to song + some additional info
  *@author Oliver Nölle
  */

class SongEntry {
public: 
	SongEntry(Song* s);
	virtual ~SongEntry();
	
	Song* song() { return songPtr; };
	virtual int getBase() { return 0; } ;
	virtual int compare (int column, SongEntry* other) { return 0; } ;
	virtual QString getKey(int column) { return QString(""); } ;
	virtual QString getColumn(int no) { return QString(""); };
protected:
	Song* songPtr;
};

#endif
