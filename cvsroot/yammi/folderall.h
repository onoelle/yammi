/***************************************************************************
                          folderall.h  -  description
                             -------------------
    begin                : Tue Feb 27 2001
    copyright            : (C) 2001 by Brian O.Nölle
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

#ifndef FOLDERALL_H
#define FOLDERALL_H

#include <qlist.h>
#include <qlistview.h>
#include <qdir.h>
#include <qdatetime.h>

#include "folder.h"
#include "song.h"

/**
  *@author Brian O.Nölle
  *@version after cvs-add
  */

class FolderAll : public Folder {
	Q_OBJECT
public:
	FolderAll( QListView *parent, QString title);
	~FolderAll();
	void update(MyList* allSongs);
};

#endif
