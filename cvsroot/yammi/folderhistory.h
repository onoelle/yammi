/***************************************************************************
                          folderhistory.h  -  description
                             -------------------
    begin                : Sun Nov 4 2001
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

#ifndef FOLDERHISTORY_H
#define FOLDERHISTORY_H

#include <folder.h>

/**this folder contains all played songs with a time stamp
(recorded in logfile)

  *@author Brian O.Nlle
  */

class FolderHistory : public Folder  {
public: 
	FolderHistory( QListView* parent, QString title);
	~FolderHistory();
	void update(QList<Song> history);
};

#endif
