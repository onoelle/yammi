/***************************************************************************
                          folder.cpp  -  description
                             -------------------
    begin                : Fri Feb 23 2001
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

#include "folder.h"

// constructs a top-level folder
Folder::Folder( QListView *parent, const QString &name )
			: QListViewItem( parent )
{
	setText( 0, name );
	folderPopup=0;
	fName=name;
}

// construct a folder (not top-level)
Folder::Folder( QListViewItem* parent, const QString &name )
			: QListViewItem( parent )
{
	setText( 0, name );
	folderPopup=0;
	fName=name;
/*
	folderPopup = new QPopupMenu();
	folderPopup->insertItem( "Folder...", 113 );
	folderPopup->insertItem("Content...", contentMenu);
	folderPopup->insertItem( "For content..", 114 );
	*/
}


/*
void Folder::insertSubFolders( const QObjectList *lst )
{
	Folder *f;
	for ( f = ( Folder* )( ( QObjectList* )lst )->first(); f; f = ( Folder* )( ( QObjectList* )lst )->next() )
		(void)new Folder( (QListViewItem*)this, "sub" );
}
*/

// update the view (after changes in songlist)
void Folder::update(MyList* allSongs)
{
	clearSongs();
	for(Song* s=allSongs->first(); s; s=allSongs->next()) {
		addSong(s);
	}
}


// insert content menu...
void Folder::popup(QPoint point, QPopupMenu* contentMenu)
{
	allPopup=new QPopupMenu();
	if (folderPopup)
		allPopup->insertItem("Folder...", folderPopup);
	if (contentMenu)
		allPopup->insertItem("Content...", contentMenu);
	if(folderPopup || contentMenu)
		allPopup->popup(point);
}

void Folder::enqueueFolder()
{
	emit EnqueueFolder();
}

void Folder::burnFolder()
{
	emit BurnFolder();
}

