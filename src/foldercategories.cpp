/***************************************************************************
                          foldercategories.cpp  -  description
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

#include "foldercategories.h"

#include <QMenu>
#include <QDebug>

#include "foldersorted.h"


FolderCategories::FolderCategories( Q3ListView* parent, QString title)
		: Folder( parent, title )
{
}


FolderCategories::~FolderCategories()
{
}


// update all Categorys according to given structures
void FolderCategories::update(QList<MyList*> allCategories, QStringList categoryNames)
{
	// we have to delete all existing items first!
	while(firstChild()) {
        qDebug() << "deleting item";
		Q3ListViewItem* toDelete=firstChild();
		delete(toDelete);
	}
	
    folderPopup = new QMenu( 0 );
    folderPopup->insertItem(QObject::tr("New Category ..."), this, SLOT(newCategory()));
	
	int count=0;
    for (QList<MyList*>::iterator it = allCategories.begin(); it != allCategories.end(); it++, count++) {
		FolderSorted *f = new FolderSorted( this, categoryNames[count] );
        qDebug() << "calling f->update()";
        f->update(**it);
        f->folderPopup = new QMenu( 0 );
        f->folderPopup->insertItem(tr("Remove Category ..."), this, SLOT(removeCategory()));
        f->folderPopup->insertItem(tr("New Category ..."), this, SLOT(newCategory()));
        f->folderPopup->insertItem(tr("Rename Category ..."), this, SLOT(renameCategory()));
        f->folderPopup->insertItem(tr("Load .m3u into Category"), this, SLOT(loadM3uIntoCategory()));
        f->folderPopup->setItemChecked(13, true);
	}
	setText(0, fName+QString(" (%1)").arg(count));
    qDebug() << "calling sortChildItems()";
    sortChildItems(0, true);
    qDebug() << "...done";
}

