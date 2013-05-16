/***************************************************************************
                          foldermedia.h  -  description
                             -------------------
    begin                : Thu Sep 6 2001
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

#ifndef FOLDERMEDIA_H
#define FOLDERMEDIA_H

#include "folder.h"

/**
  *@author Brian O.Nlle
  */

class FolderMedia : public Folder
{
	Q_OBJECT
public:
	FolderMedia( QListView* parent, QString title);
	~FolderMedia();
	void update(MyList* allSongs);
public slots:
	void removeMedia() 				{ emit RemoveMedia(); }
	void renameMedia() 				{ emit RenameMedia(); }
signals:
	void RemoveMedia();
	void RenameMedia();
};

#endif
