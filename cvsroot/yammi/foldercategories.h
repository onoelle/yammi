/***************************************************************************
                          foldercategories.h  -  description
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

#ifndef FOLDERCATGEGORIES_H
#define FOLDERCATEGORIES_H

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

class FolderCategories : public Folder
{
	Q_OBJECT
public:
	FolderCategories( QListView* parent, QString title );
	~FolderCategories();
	void update(QList<MyList> allCategories, QStrList categoryNames);
public slots:
	void removeCategory();
signals:
	void CategoryRemoved();
};

#endif
