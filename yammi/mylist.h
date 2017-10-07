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
	
	static const int ByTitle=1;
	static const int ByArtist=2;
	static const int ByAlbum=3;
	static const int ByFilename=4;
	static const int ByAddedTo=5;
	static const int ByTrack=6;
	static const int ByYear=7;
	static const int ByGenre=8;
	static const int ByPath=9;
	static const int ByLastPlayed=10;
	static const int ByKey=ByArtist + 16*ByTitle + 256*ByAlbum;
	
	
	
	bool			dirty;
	
protected:
	int sortOrder;
};

#endif
