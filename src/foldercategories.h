/***************************************************************************
                          foldercategories.h  -  description
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

#ifndef FOLDERCATEGORIES_H
#define FOLDERCATEGORIES_H

#include <QDir>
#include <QString>

#include "folder.h"


/**
  *@author Brian O.Nölle
  */

class FolderCategories : public Folder
{
	Q_OBJECT
public:
	FolderCategories( Q3ListView* parent, QString title );
	~FolderCategories();
    void update(QList<MyList*> allCategories, QStringList categoryNames);
public slots:
	void removeCategory() { emit CategoryRemoved(); }
	void newCategory()		{ emit CategoryNew(); }
	void renameCategory() { emit CategoryRenamed(); }
	void loadM3uIntoCategory() { emit LoadM3uIntoCategory(); }
signals:
	void CategoryRemoved();
	void CategoryNew();
	void CategoryRenamed();
	void LoadM3uIntoCategory();
};

#endif
