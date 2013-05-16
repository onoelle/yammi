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
#include "foldersorted.h"
#include <kdebug.h>

FolderCategories::FolderCategories( QListView* parent, QString title)
		: Folder( parent, title )
{
}


FolderCategories::~FolderCategories()
{
}


// update all Categorys according to given structures
void FolderCategories::update(QPtrList<MyList> allCategories, QStringList categoryNames)
{
	// we have to delete all existing items first!
	while(firstChild()) {
        kdDebug() << "deleting item" << endl;   
		QListViewItem* toDelete=firstChild();
		delete(toDelete);
	}
	
	folderPopup = new QPopupMenu( 0 );
	folderPopup->insertItem( "New Category..", this, SLOT(newCategory()));
	
	int count=0;
	for(MyList* ptr=allCategories.first(); ptr; ptr=allCategories.next(), count++)
	{
		FolderSorted *f = new FolderSorted( this, categoryNames[count] );
        kdDebug() << "calling f->update()" << endl;      
		f->update(*ptr);
		f->folderPopup = new QPopupMenu( 0 );
		f->folderPopup->insertItem( "Remove Category", this, SLOT(removeCategory()));
		f->folderPopup->insertItem( "New Category..", this, SLOT(newCategory()));
		f->folderPopup->insertItem( "Rename Category..", this, SLOT(renameCategory()));
        f->folderPopup->insertItem( "Load .m3u into Category", this, SLOT(loadM3uIntoCategory()));
        f->folderPopup->setItemChecked(13, true);
	}
	setText(0, fName+QString(" (%1)").arg(count));
    kdDebug() << "calling sortChildItems()" << endl;      
    sortChildItems(0, true);
    kdDebug() << "...done" << endl;      
}

