/***************************************************************************
                          foldergroups.cpp  -  description
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

#include "foldergroups.h"


FolderGroups::FolderGroups( QListView* parent, QString title)
		: Folder( parent, title )
{
}	
	
void FolderGroups::update(MyList* allSongs, int sortBy)
{	
	// we have to delete all existing items first!
//	clearSongs();
	while(firstChild()) {
		QListViewItem* toDelete=firstChild();
		delete(toDelete);
	}
	
	// init folder Groups/Artists
	allSongs->setSortOrder(sortBy);
	allSongs->sort();
	
	Song* s=allSongs->first();
	QString last("xxx");
	int count=0;
	int groupCount=0;
	bool same=false;
	for(; s; s=allSongs->next()) {
		if(sortBy==MyList::ByArtist)
			same=(last==s->artist);
		if(sortBy==MyList::ByAlbum)
			same=(last==s->album);
		
		if(same) {
		  count++;
		} else {
			if(count>=5) {
	   		// go back to first song of that artist/album
	   		groupCount++;
	   		bool sameArtist=true;
	   		s=allSongs->prev();
	   		QString last2=s->artist;
	   		for(int m=1; m<count; m++) {
	   			s=allSongs->prev();
	   			if(s->artist!=last2)
	   				sameArtist=false;
	   		}

				QString folderName("");
				if(sortBy==MyList::ByArtist)								// name folder by artist
					folderName=s->artist;
				if(sortBy==MyList::ByAlbum) {								// name folder by artist (if only one) + album
					if(sameArtist)
						folderName=s->artist+" - "+s->album;
					else
						folderName=s->album;					
				}
				Folder *f2 = new Folder( this, folderName );
				f2->folderPopup = new QPopupMenu( 0 );
				f2->folderPopup->insertItem( "Enqueue", this, SLOT(enqueueFolder()));
				f2->folderPopup->insertItem( "Burn folder...", this, SLOT(burnFolder()));

   			for(int m=0; m<count; m++) {
					f2->addSong(s);
					s->classified=true;
	   			s=allSongs->next();
	   		}
				f2->setText(0, folderName+QString(" (%1)").arg(count));
			}
			if(sortBy==MyList::ByArtist)
				last=s->artist;
			if(sortBy==MyList::ByAlbum)
				last=s->album;
			if(last=="")
				last="xxx";
			count=1;
		}
	}
	setText(0, fName+QString(" (%1)").arg(groupCount));
}



FolderGroups::~FolderGroups()
{
}

