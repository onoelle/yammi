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

/**
  * This class extends QList (with fixed type <Song>), to enable sorting
  * otherwise, use exactly as QList<Song>
  */

class MyList : public QList<Song>  {
public: 
	MyList();
	~MyList();
	
	int compareItems( QCollection::Item item1, QCollection::Item item2);
	int myCompare(Song* song1, Song* song2, int sortBy);
	void setSortOrder(int orderBy);
	
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
