/***************************************************************************
                          songentryint2.h  -  description
                             -------------------
    begin                : Sat Mar 23 2002
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

#ifndef SONGENTRYINT2_H
#define SONGENTRYINT2_H

#include <songentry.h>

/**
  *@author Oliver Nölle
  */

class SongEntryInt2 : public SongEntry  {
public:
	SongEntryInt2(Song* s, int intInfo);
	~SongEntryInt2();
	virtual int getBase() { return 1; };
	virtual int compare(int column, SongEntry* other);
	virtual QString getKey(int column);
	virtual QString getColumn(int no);
protected:
	int intInfo;
};

#endif
