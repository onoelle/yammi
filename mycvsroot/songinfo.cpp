/***************************************************************************
                          songinfo.cpp  -  description
                             -------------------
    begin                : Fri Aug 10 2001
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

#include "songinfo.h"

SongInfo::SongInfo(QWidget *parent, const char *name, bool modal ) : SongInfoDialog(parent,name,modal)
{
}
SongInfo::~SongInfo(){
}
