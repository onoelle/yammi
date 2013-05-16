/***************************************************************************
                          songentrytimestamp.cpp  -  description
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

#include "songentrytimestamp.h"

SongEntryTimestamp::SongEntryTimestamp(Song* s, MyDateTime* t)
	:SongEntry(s)
{
	QString str=t->writeToString();
	timestamp.readFromString(str);
}

SongEntryTimestamp::SongEntryTimestamp(Song* s)
	:SongEntry(s)
{
}

SongEntryTimestamp::~SongEntryTimestamp()
{
}

int SongEntryTimestamp::compare(int, SongEntry* other)
{
	return timestamp.secsTo(((SongEntryTimestamp*)other)->timestamp);
}

QString SongEntryTimestamp::getKey(int)
{
	return QString("%1").arg(999999999+ QDateTime( QDate(2222, 1, 1), QTime(0,0,0) ).secsTo(timestamp), 10);
}


QString SongEntryTimestamp::getColumn(int no)
{
	if(no==0)
		return timestamp.writeToString();
	else
		return QString("");
}
