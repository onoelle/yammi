/***************************************************************************
                          folderactual.h  -  description
                             -------------------
    begin                : Sat Sep 15 2001
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

#ifndef FOLDERACTUAL_H
#define FOLDERACTUAL_H

#include <folder.h>

/**folder containing the current song
(maybe the last played + the next n songs?)
  *@author Brian O.Nlle
  */

class FolderActual : public Folder  {
public: 
	FolderActual( QListView *parent, QString title);
	~FolderActual();
	void update(QList<Song> lastPlayed);
};

#endif
