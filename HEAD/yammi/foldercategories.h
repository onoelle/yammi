/***************************************************************************
                          foldercategories.h  -  description
                             -------------------
    begin                : Tue Feb 27 2001
    copyright            : (C) 2001 by Brian O.N�lle
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

#include <qptrlist.h>
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
  *@author Brian O.N�lle
  */

class FolderCategories : public Folder
{
	Q_OBJECT
public:
	FolderCategories( QListView* parent, QString title );
	~FolderCategories();
	void update(QPtrList<MyList> allCategories, QStringList categoryNames);
  void popup(QPoint point, QPopupMenu* contentMenu);  
public slots:
	void removeCategory() { emit CategoryRemoved(); }
	void newCategory()		{ emit CategoryNew(); }
	void renameCategory() { emit CategoryRenamed(); }
	void autoplayCategory() { emit CategoryAutoplay(); }
signals:
	void CategoryRemoved();
	void CategoryNew();
	void CategoryRenamed();
	void CategoryAutoplay();
};

#endif
