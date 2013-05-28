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


FolderCategories::FolderCategories( QTreeWidget* parent, QString title)
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
    while (childCount()) {
        //qDebug() << "deleting item";
        QTreeWidgetItem* toDelete = QTreeWidgetItem::child(0);
		delete(toDelete);
	}
	
    folderPopup = new QMenu(tr("Folder ..."));
    folderPopup->addAction(QObject::tr("New Category ..."), this, SLOT(newCategory()));
	
	int count=0;
    for (QList<MyList*>::iterator it = allCategories.begin(); it != allCategories.end(); it++, count++) {
		FolderSorted *f = new FolderSorted( this, categoryNames[count] );
        //qDebug() << "calling f->update()";
        f->update(**it);
        f->folderPopup = new QMenu(tr("Folder ..."));
        f->folderPopup->addAction(tr("Remove Category ..."), this, SLOT(removeCategory()));
        f->folderPopup->addAction(tr("New Category ..."), this, SLOT(newCategory()));
        f->folderPopup->addAction(tr("Rename Category ..."), this, SLOT(renameCategory()));
        f->folderPopup->addAction(tr("Load .m3u into Category"), this, SLOT(loadM3uIntoCategory()));
        if (m_actionAutoPlayAction)
            m_actionAutoPlayAction->setChecked(true);
	}
	setText(0, fName+QString(" (%1)").arg(count));
    //qDebug() << "calling sortChildItems()";
    sortChildren(0, Qt::AscendingOrder);
    //qDebug() << "...done";
}

