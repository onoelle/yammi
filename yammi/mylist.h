/***************************************************************************
                          mylist.h  -  description
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

#ifndef MYLIST_H
#define MYLIST_H

#include <QList>

#include "songentry.h"

class MyList;
class Song;


/**
 * This class extends QList (with fixed type <SongEntry*>).
 * It can be used everytime when you need to deal with a collection of songs/song entries.
 * - enables sorting
 * - enables adding/removing/searching SongEntry or Song types...
 * Otherwise, use exactly as QList<SongEntry*>.
*/
class MyList : public QList<SongEntry*> {

public: 
    enum enumSortBy {
        ByTitle = 1,
        ByArtist = 2,
        ByAlbum = 3,
        ByFilename = 4,
        ByAddedTo = 5,
        ByTrack = 6,
        ByYear = 7,
        ByGenre = 8,
        ByPath = 9,
        ByLastPlayed = 10,
        ByKey = ByArtist + 16*ByTitle + 256*ByAlbum,
    };

	MyList();
  MyList(MyList* listToCopy);
	~MyList();
	
	Song*			firstSong();

	void			appendSong(Song* s);					// appends <s> as simple SongEntry
	void			appendList(MyList* list);			// appends a whole list of song entrys
	void			removeSong(Song* toDelete);		// removes all occurences of SongEntry with pointer to <toDelete>
    int containsSong(Song* lookup); // returns if this list contains the song
  int       containsSelection(MyList* selection);
	Song*			getSongByKey(QString artist, QString title, QString album);

    int				lessThan(SongEntry* item1, SongEntry* item2);
	int				myCompare(Song* song1, Song* song2, int sortBy);
  int       getSortOrder() { return sortOrder; }
  void			setSortOrderAndSort(int newSortOrder, bool sortAnyway=false);
  void      reverse();
  void      shuffle();
	
	
	bool			dirty;
	
protected:
	int sortOrder;
};

#endif
