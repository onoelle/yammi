/***************************************************************************
                          songentrystring.h  -  description
                             -------------------
    begin                : Sun Feb 10 2002
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

#ifndef SONGENTRYSTRING_H
#define SONGENTRYSTRING_H

#include "songentry.h"


/** song entry enriched with a string
  *@author Oliver Nölle
  */

class SongEntryString : public SongEntry  {
public:
    SongEntryString(Song* s, QString stringInfo);
    ~SongEntryString();

    virtual int getBase() { return 1; };
    virtual int compare(int column, SongEntry* other);
    virtual QString getKey(int) { return stringInfo; };
    virtual QString getColumn(int) { return stringInfo; };
protected:
    QString stringInfo;
};

#endif
