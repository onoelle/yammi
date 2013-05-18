/***************************************************************************
                          mylist.cpp  -  description
                             -------------------
    begin                : Mon Mar 19 2001
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

#include "mylist.h"

#include <taglib/id3v1genres.h>

#include <tr1/functional>

#include "song.h"


MyList::MyList()
{
	sortOrder=ByKey;
	dirty=false;
}

MyList::~MyList()
{
}


MyList::MyList(MyList* listToClone) {
    sortOrder = ByKey;
    dirty = false;
    for (iterator it = listToClone->begin(); it != listToClone->end(); it++) {
        append(*it);
    }
}

void MyList::appendList(MyList* list) {
    for (iterator it = list->begin(); it != list->end(); it++) {
        append(*it);
    }
}

Song* MyList::firstSong()
{
    SongEntry* entry = 0;
    if (!isEmpty()) {
        entry = first();
    }
	if(entry)
		return entry->song();
	else
		return 0;
}

void MyList::appendSong(Song* s)
{
	append(new SongEntry(s));
	dirty=true;
}

// removes all appearances of song <toDelete> from the list
void MyList::removeSong(Song* toDelete)
{
    QMutableListIterator<SongEntry*> it(*this);
    while (it.hasNext()) {
        SongEntry* entry = it.next();
		if(entry->song()==toDelete) {
            delete it.value();
            it.remove();
			dirty=true;
		}
	}
}


// returns the number of occurrences of this song
int MyList::containsSong(Song* lookup)
{
	int count=0;
    for (iterator it = begin(); it != end(); it++) {
        if ((*it)->song() == lookup)
			count++;
	}
	return count;
}


/**
 * returns
 * 0 if none
 * 1 if some
 * 2 if all
 * ...of the entries in the second list are contained in the first list
 */
int MyList::containsSelection(MyList* selection)
{
  bool allContained=true;
  bool noneContained=true;

  for (iterator it = selection->begin(); it != selection->end(); it++) {
    if(containsSong((*it)->song())) {
      noneContained=false;
    }
    else {
      allContained=false;
    }
  }
  if(noneContained)    return 0;
  if(allContained)     return 2;
  return 1;
}

// returns the song with the given key, or 0 if not existing
Song* MyList::getSongByKey(QString artist, QString title, QString album)
{
    for (iterator it = begin(); it != end(); it++) {
        if ((*it)->song()->sameAs(artist, title, album))
            return (*it)->song();
	}
	return 0;
}

// set the sort order and sort if sort order changed
void MyList::setSortOrderAndSort(int newSortOrder, bool sortAnyway)
{
	if(sortOrder!=newSortOrder || sortAnyway) {
		sortOrder=newSortOrder;
        qSort(begin(), end(), std::tr1::bind(std::tr1::mem_fn(&MyList::lessThan), this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));
	}
}

// compares on one or more attributes
int MyList::lessThan(SongEntry* item1, SongEntry* item2)
{
    Song* song1 = item1->song();
    Song* song2 = item2->song();
	int t=myCompare(song1, song2, sortOrder & 0xF);
	if(t==0) {
		t=myCompare(song1, song2, (sortOrder >> 4 )& 0xF);
		if(t==0) {
			t=myCompare(song1, song2, (sortOrder >> 8 )& 0xF);
		}
	}
    return (t < 0);
}
	
// compares on one attribute
int MyList::myCompare(Song* song1, Song* song2, int sortBy)
{
	if(sortBy==ByTitle) {
		return(QString::compare(song1->title, song2->title));
  }
	if(sortBy==ByArtist) {
    return(QString::compare(song1->artist, song2->artist));
  }
	if(sortBy==ByAlbum) {
    return(QString::compare(song1->album, song2->album));
  }
	if(sortBy==ByFilename) {
		return(QString::compare(song1->filename, song2->filename));
  }
	if(sortBy==ByPath) {
		return(QString::compare(song1->path, song2->path));
  }
  if(sortBy==ByLastPlayed) {
    if(song1->lastPlayed>song2->lastPlayed) return 1;
		if(song1->lastPlayed<song2->lastPlayed) return -1;
		return 0;
  }
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
	if(sortBy==ByGenre) {
        return(QString::compare(song1->genre, song2->genre));
	}
	return 0;
}

/**
 * Reverses the order of all entries in this list.
 */
void MyList::reverse()
{
  int noItems=count();
  for(int i=0; i<noItems; i++) {
    SongEntry* entry = takeAt(i);
    prepend(entry);
  }
}

/**
 * Shuffles the order of all entries in this list.
 */
void MyList::shuffle()
{
  // 1. copy all items to a temp list
  MyList tempList;
  while(!isEmpty()) {
    tempList.append(takeAt(0));
  }

  while(!tempList.isEmpty()) {
    int songsLeft=tempList.count();
    // create random number
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime xmas( QDate(2050,12,24), QTime(17,00) );
    int chosen=(dt.secsTo(xmas) + dt.time().msec()) % songsLeft;
    if(chosen<0) {
      chosen=-chosen;
    }
    append(tempList.takeAt(chosen));
  }
}

