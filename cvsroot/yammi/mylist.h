/***************************************************************************
                          mylist.h  -  description
                             -------------------
    begin                : Mon Mar 19 2001
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

#ifndef MYLIST_H
#define MYLIST_H

#include <qlist.h>
#include <song.h>
#include <songentry.h>

/**
  * This class extends QList (with fixed type <SongEntry>)
  * enables sorting
  * enables adding/removing/searching SongEntry or Song types...
  * otherwise, use exactly as QList<SongEntry>
  */

class MyList : public QList<SongEntry>  {
public: 
	MyList();
	~MyList();
	
	Song*			firstSong();
	Song*			nextSong();
	Song*			prevSong();
	
	void			appendSong(Song* s);					// appends <s> as simple SongEntry
	void			removeSong(Song* toDelete);		// removes all occurences of SongEntry with pointer to <toDelete>
	int				containsSong(Song* lookup);		// returns the number of occurences of the song

	
	int				compareItems( QCollection::Item item1, QCollection::Item item2);
	int				myCompare(Song* song1, Song* song2, int sortBy);
	void			setSortOrder(int orderBy) {sortOrder=orderBy; };

	
	static const int ByTitle=1;
	static const int ByArtist=2;
	static const int ByAlbum=3;
	static const int ByFilename=4;
	static const int ByAddedTo=5;
	static const int ByTrack=6;
	static const int ByYear=7;
	
protected:
	int sortOrder;
};

#endif
