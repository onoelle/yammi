/***************************************************************************
                          songentryint.h  -  description
                             -------------------
    begin                : Sat Feb 9 2002
    copyright            : (C) 2002 by Oliver Nölle
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

#ifndef SONGENTRYINT_H
#define SONGENTRYINT_H

#include <songentry.h>

/**
  *@author Oliver Nölle
  */

class SongEntryInt : public SongEntry  {
public: 
	SongEntryInt(Song* s, int intInfo);
	~SongEntryInt();
	virtual int getBase() { return 1; };
	virtual int compare(int column, SongEntry* other);	
	virtual QString getKey(int column);
	virtual QString getColumn(int no);
	int intInfo;
};

#endif
