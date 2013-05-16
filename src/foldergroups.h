
/***************************************************************************
                          foldergroups.h  -  description
                             -------------------
    begin                : Tue Feb 27 2001
    copyright            : (C) 2001 by Brian O.Nölle
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

#ifndef FOLDERGROUPS_H
#define FOLDERGROUPS_H

#include "folder.h"


/**
  *@author Brian O.Nölle
  */
class FolderGroups : public Folder  {
	Q_OBJECT
public:
	FolderGroups( Q3ListView* parent, QString title);
	~FolderGroups();
	void update(MyList* allSongs, int sortBy);
private:
  void createGroupFolder(MyList* group, int sortBy);

};

#endif
