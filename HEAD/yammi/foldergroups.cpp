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
#include "yammigui.h"
#include "mp3info/CMP3Info.h"

extern YammiGui* gYammiGui;

FolderGroups::FolderGroups( QListView* parent, QString title)
		: Folder( parent, title )
{
}	
	
void FolderGroups::update(MyList* allSongs, int sortBy)
{	
	// we have to delete all existing items first!
	while(firstChild()) {
		QListViewItem* toDelete=firstChild();
		// change to Folder* toDelete=(Folder*)firstChild();
		delete(toDelete);
	}
	
	// set sort order for grouping
	allSongs->setSortOrderAndSort(sortBy, true);
	
	QString last("xxxyyy");
	int count=0;
	int groupCount=0;
	bool same=false;
  int index=0;
	for(Song* s=allSongs->firstSong(); s; s=allSongs->nextSong(), index++) {
    QString next;
		if(sortBy==MyList::ByArtist) {
      next=s->artist;
    }
		if(sortBy==MyList::ByAlbum)
			next=s->album;
		if(sortBy==MyList::ByGenre)
			next=CMP3Info::getGenre(s->genreNr);
		
    if(gYammiGui->getModel()->config.lazyGrouping) {
      // lazy grouping is not guaranteed to work, as the sorting might be different
      // (we sort once, and scan in linear time)
      QString last2=last.upper();
      last2=last2.replace(QRegExp("_"), " ");
      QString next2=next.upper();
      next2=next2.replace(QRegExp("_"), " ");
      same=(last2==next2);
    }
    else
      same=(last==next);

		if(same) {
		  count++;
		}
    else {
			if(count>=gYammiGui->getModel()->config.groupThreshold) {
	   		// go back to first song of that artist/album/genre
	   		groupCount++;
	   		bool sameArtist=true;
	   		s=allSongs->prevSong();
	   		QString last2=s->artist;
	   		for(int m=1; m<count; m++) {
	   			s=allSongs->prevSong();
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
				if(sortBy==MyList::ByGenre) {
					folderName=CMP3Info::getGenre(s->genreNr);
				}
			
				Folder *f2 = new Folder( this, folderName );

   			for(int m=0; m<count; m++) {
					f2->addEntry(new SongEntry(s));
					if(fName!="Genre")
						s->classified=true;					// do not set on genre???
	   			s=allSongs->nextSong();
	   		}
				f2->setText(0, folderName+QString(" (%1)").arg(count));
			}
			if(sortBy==MyList::ByArtist)
				last=s->artist;
			if(sortBy==MyList::ByAlbum)
				last=s->album;
			if(sortBy==MyList::ByGenre)
				last=CMP3Info::getGenre(s->genreNr);
			if(last=="")
				last="xxxyyy";
			count=1;
		}
	}
	setText(0, fName+QString(" (%1)").arg(groupCount));
  sortChildItems(0, true);
	allSongs->setSortOrderAndSort(MyList::ByArtist);
}



FolderGroups::~FolderGroups()
{
}

