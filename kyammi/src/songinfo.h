/***************************************************************************
                          songinfo.h  -  description
                             -------------------
    begin                : Fri Aug 10 2001
    copyright            : (C) 2001 by Brian O.Nlle
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

#ifndef SONGINFO_H
#define SONGINFO_H

#include <qwidget.h>
#include <SongInfoDialog.h>

/**
  *@author Brian O.Nlle
  */

class SongInfo : public SongInfoDialog  {
   Q_OBJECT
public: 
	SongInfo(QWidget *parent=0, const char *name=0, bool modal=true);
	~SongInfo();
};

#endif
