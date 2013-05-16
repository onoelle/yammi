/***************************************************************************
                          songentrytimestamp.h  -  description
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

#ifndef SONGENTRYTIMESTAMP_H
#define SONGENTRYTIMESTAMP_H

#include <songentry.h>
#include <mydatetime.h>


/**song info extended with a timestamp
  *@author Oliver Nölle
  */

class SongEntryTimestamp : public SongEntry  {
public: 
	SongEntryTimestamp(Song* s);
	SongEntryTimestamp(Song* s, MyDateTime* datetime);
	virtual ~SongEntryTimestamp();
	
	virtual int getBase() { return 1; };
	virtual int compare(int column, SongEntry* other);
	virtual QString getKey(int no);
	virtual QString getColumn(int no);
	MyDateTime timestamp;
protected:
};

#endif
