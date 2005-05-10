/***************************************************************************
                          mylist.cpp  -  description
                             -------------------
    begin                : Mon Mar 19 2001
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

#include "mylist.h"
#include "song.h"

MyList::MyList()
{
	sortOrder=1;
}

MyList::~MyList()
{
}


Song* MyList::firstSong()
{
	SongEntry* entry=first();
	if(entry)
		return entry->song();
	else
		return 0;
}

Song* MyList::nextSong()
{
	SongEntry* entry=next();
	if(entry)
		return entry->song();
	else
		return 0;
}

Song* MyList::prevSong()
{
	SongEntry* entry=prev();
	if(entry)
		return entry->song();
	else
		return 0;
}

void MyList::appendSong(Song* s)
{
	append(new SongEntry(s));
}

// removes all appearances of song <toDelete> fromo the list
void MyList::removeSong(Song* toDelete)
{
	for(SongEntry* entry=first(); entry; ) {
		if(entry->song()==toDelete) {
			cout << "calling remove for entry " << entry->song()->displayName() << "\n";
			remove();
			if(entry)
				entry=next();					// shouldn't this be done by remove() ?
		}
		else {
			entry=next();
		}
	}
}

int MyList::containsSong(Song* lookup)
{
	int count=0;
	for(SongEntry* entry=first(); entry; entry=next()) {
		if(entry->song()==lookup)
			count++;
	}
	return count;
}

// compares on one or more attributes
int MyList::compareItems( QCollection::Item item1, QCollection::Item item2)
{
	SongEntry* entry1=(SongEntry*) item1;
	SongEntry* entry2=(SongEntry*) item2;
	
	Song* song1=entry1->song();
	Song* song2=entry2->song();
	int t=myCompare(song1, song2, sortOrder & 0xF);
	if(t==0) {
		t=myCompare(song1, song2, (sortOrder >> 4 )& 0xF);
		if(t==0) {
			t=myCompare(song1, song2, (sortOrder >> 8 )& 0xF);
		}
	}
	return t;
}
	
// compares on one attribute
int MyList::myCompare(Song* song1, Song* song2, int sortBy)
{	
	if(sortBy==ByTitle)
		return(QString::compare(song1->title, song2->title));
	if(sortBy==ByArtist)
		 return QString::compare(song1->artist, song2->artist);
	if(sortBy==ByAlbum)
		return(QString::compare(song1->album, song2->album));
	if(sortBy==ByFilename)
		return(QString::compare(song1->filename, song2->filename));
	if(sortBy==ByTrack) {
		if(song1->trackNr>song2->trackNr) return 1;
		if(song1->trackNr<song2->trackNr) return -1;
		return 0;
	}
	if(sortBy==ByYear) {
		if(song1->year>song2->year) return 1;
		if(song1->year<song2->year) return -1;
		return 0;
	}
	if(sortBy==ByAddedTo) {
		if(song1->addedTo>song2->addedTo) return -1;
		if(song1->addedTo<song2->addedTo) return 1;
		return 0;
	}
	return 0;
}