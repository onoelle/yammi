/***************************************************************************
                          SongListItem.h  -  description
                             -------------------
    begin                : Tue Oct 2 2001
    copyright            : (C) 2001 by Brian O.Nlle
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

#ifndef SONGLISTITEM_H
#define SONGLISTITEM_H

#include "song.h"
#include "songentry.h"
#include "qlistview.h"

class SongListItem : public QListViewItem
{
public:
  SongListItem( QListView *parent, SongEntry *s, SongListItem* after=0 );

  virtual void paintCell( QPainter *p, const QColorGroup &cg, int column, int width, int alignment );
	QString key(int column, bool ascending) const;
	Song* song() const	{ return songEntry->song(); }

protected:
	SongEntry* songEntry;

};

#endif
