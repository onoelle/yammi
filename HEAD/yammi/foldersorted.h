/***************************************************************************
                          foldersorted.h  -  description
                             -------------------
    begin                : Thu Mar 28 2002
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

#ifndef FOLDERSORTED_H
#define FOLDERSORTED_H

#include "folder.h"
#include "mylistview.h"

/**
  *@author Oliver Nölle
  */

class FolderSorted : public Folder  {
public: 
	FolderSorted	(QListView *parent, const QString &name );						// top-level folder
	FolderSorted	(QListViewItem *parent, const QString &name );				// subfolder
	~FolderSorted();
	
	virtual void		update(MyList* songList);					// common update method
	virtual void		addSong(Song* s);
	virtual void		insertSong(Song* s, int index);
	virtual void		removeSong(Song* s);
	
	void						syncWithListView(MyListView* listView);
	void						correctOrder();
};

#endif

