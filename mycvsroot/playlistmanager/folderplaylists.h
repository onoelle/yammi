/***************************************************************************
                          folderplaylists.h  -  description
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

#ifndef FOLDERPLAYLISTS_H
#define FOLDERPLAYLISTS_H

#include <qlist.h>
#include <qlistview.h>
#include <qdom.h>
#include <qdir.h>
#include <qstring.h>
#include <qinputdialog.h>
#include <qvector.h>
#include <qarray.h>
#include "folder.h"
#include "song.h"


/**
  *@author Brian O.Nölle
  */

class FolderPlayLists : public Folder
{
	Q_OBJECT
public:
	FolderPlayLists( QListView* parent, QString title );
	~FolderPlayLists();
	void update(QList<MyList> allPlaylists, QStrList playlistNames);
public slots:
	void removePlaylist();
signals:
	void PlaylistRemoved();
};

#endif
