/***************************************************************************
                          folderunclassified.h  -  description
                             -------------------
    begin                : Mon Oct 1 2001
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

#ifndef FOLDERUNCLASSIFIED_H
#define FOLDERUNCLASSIFIED_H

#include <folder.h>

/**this folder contains all the songs that are not in other folders/playlist
  *@author Brian O.Nlle
  */

class FolderUnclassified : public Folder  {
	Q_OBJECT
public:
	FolderUnclassified( QListView *parent, QString title);
	~FolderUnclassified();
	void update(MyList* allSongs);
};

#endif
