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
#include "foldersorted.h"


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
		QListViewItem* toDelete=firstChild();
		delete(toDelete);
	}
	
	folderPopup = new QPopupMenu( 0 );
	folderPopup->insertItem( "New Category..", this, SLOT(newCategory()));
	
	int count=0;
	for(MyList* ptr=allCategories.first(); ptr; ptr=allCategories.next(), count++)
	{
		FolderSorted *f = new FolderSorted( this, categoryNames[count] );
		f->update(ptr);
		f->folderPopup = new QPopupMenu( 0 );
		f->folderPopup->insertItem( "Remove Category", this, SLOT(removeCategory()));
		f->folderPopup->insertItem( "New Category..", this, SLOT(newCategory()));
		f->folderPopup->insertItem( "Rename Category..", this, SLOT(renameCategory()));
		f->folderPopup->insertItem( "Autoplay", this, SLOT(autoplayCategory()));
	}
	setText(0, fName+QString(" (%1)").arg(count));
  sortChildItems(0, true);
}



// insert content menu...
void FolderCategories::popup(QPoint point, QPopupMenu* contentMenu)
{
	allPopup=new QPopupMenu();
	if (folderPopup) {
		allPopup->insertItem("Folder...", folderPopup);
// TODO:
//    cout << "checking..\n";
//    if(gYammiGui->folderAutoplay==this)
//      cout << "it this one\n";
  }
	if (contentMenu)
		allPopup->insertItem("Content...", contentMenu);
	if(folderPopup || contentMenu)
		allPopup->popup(point);
}
