/***************************************************************************
                          songentryint2.cpp  -  description
                             -------------------
    begin                : Sat Mar 23 2002
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

#include "songentryint2.h"

SongEntryInt2::SongEntryInt2(Song* s, int intInfo)
 :SongEntry(s)
{
	this->intInfo=intInfo;
}
SongEntryInt2::~SongEntryInt2(){
}

QString SongEntryInt2::getKey(int)
{
	return QString("%1").arg(1000-intInfo, 4);
}

int SongEntryInt2::compare(int, SongEntry* other)
{
	return - intInfo + ((SongEntryInt2*)other)->intInfo;
}

QString SongEntryInt2::getColumn(int)
{
	return QString("%1\%").arg(intInfo/10, 3);
}

