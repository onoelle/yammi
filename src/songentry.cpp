/***************************************************************************
                          songentry.cpp  -  description
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

#include "songentry.h"


SongEntry::SongEntry(Song* s)
{
	songPtr=s;
}

SongEntry::~SongEntry()
{
}

QString SongEntry::getKey(int)
{
    return QString("");
}

QString SongEntry::getColumn(int)
{
    return QString("");
}
