/***************************************************************************
                          foldercategories.cpp  -  description
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

#include "foldercategories.h"

FolderCategories::FolderCategories( QListView* parent, QString title)
		: Folder( parent, title )
{
}


FolderCategories::~FolderCategories()
{
}


// update all Categorys according to given structures
void FolderCategories::update(QList<MyList> allCategories, QStrList categoryNames)
{
	// we have to delete all existing items first!
	while(firstChild()) {
		QListViewItem* toDelete=firstChild();
		delete(toDelete);
	}
	
	MyList* ptr=allCategories.first();
	QString name=categoryNames.first();
	for(; ptr; ptr=allCategories.next(), name=categoryNames.next())
	{
		Folder *f = new Folder( this, name );
		f->folderPopup = new QPopupMenu( 0 );
		f->folderPopup->insertItem( "Remove Category", this, SLOT(removeCategory()));
		f->folderPopup->insertItem( "Enqueue", this, SLOT(enqueueFolder()));
		f->folderPopup->insertItem( "Burn folder...", this, SLOT(burnFolder()));
		
		int count=0;
		for(Song* s=ptr->first(); s; s=ptr->next(), count++) {
			f->addSong(s);
			s->classified=true;
		}
		f->setText(0, name+QString(" (%1)").arg(count));
	}
//	setText(0, fname+QString(" (%1)").arg(count));
}

void FolderCategories::removeCategory()
{
	emit CategoryRemoved();
}


