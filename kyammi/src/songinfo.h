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

class Song;

/**
  *@author Brian O.Nlle
  */

class SongInfo : public SongInfoDialog  {
   Q_OBJECT
public: 
	SongInfo(QWidget *parent, Song* editSong);
	~SongInfo();

protected slots:
    void updateProposedFilename();

protected:
	Song* editSong;
};

#endif
