/***************************************************************************
                          songentryint.cpp  -  description
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

#include "songentryint.h"

SongEntryInt::SongEntryInt(Song* s, int intInfo)
 :SongEntry(s)
{
	this->intInfo=intInfo;
}
SongEntryInt::~SongEntryInt(){
}

QString SongEntryInt::getKey(int column)
{
	return QString("%1").arg(intInfo, 5);
}

int SongEntryInt::compare(int column, SongEntry* other)
{
	return intInfo - ((SongEntryInt*)other)->intInfo;
}

QString SongEntryInt::getColumn(int no)
{
	return QString("%1").arg(intInfo, 5);
}
