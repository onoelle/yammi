/***************************************************************************
                          songentrystring.cpp  -  description
                             -------------------
    begin                : Sun Feb 10 2002
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

#include "songentrystring.h"

SongEntryString::SongEntryString(Song* s, QString stringInfo)
 :SongEntry(s)
{
	this->stringInfo=stringInfo;
}

SongEntryString::~SongEntryString()
{
}

int SongEntryString::compare(int column, SongEntry* other)
{
	return getKey(column).compare(other->getKey(column));
}
