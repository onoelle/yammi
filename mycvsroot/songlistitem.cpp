/***************************************************************************
                          SongListItem.cpp  -  description
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

#include "songlistitem.h"


SongListItem::SongListItem( QListView *parent, Song *s, SongListItem* after )
    : QListViewItem( parent, after )
{
	mySong = s;
  setText( 0, mySong->artist );
  setText( 1, mySong->title );
  setText( 2, mySong->album );
	if(mySong->length!=0) {
		QString lengthStr=QString("%1").arg(mySong->length % 60);
		if (lengthStr.length()==1)
	  	lengthStr="0"+lengthStr;
		setText( 3, QString("%1:%2").arg((mySong->length) / 60).arg(lengthStr));
	}
	if(mySong->year!=0)					setText( 4, QString("%1").arg(mySong->year));
  if(mySong->trackNr!=0)			setText( 5, QString("%1").arg(mySong->trackNr));
	setText( 6, mySong->addedTo.writeToString());
  if(mySong->bitrate!=0)			setText( 7, QString("%1").arg(mySong->bitrate));
  if(mySong->filename!="")		setText( 8, mySong->filename );
  if(mySong->path!="")				setText( 9, mySong->path );
  if(mySong->comment!="")			setText(10, mySong->comment );
}

void SongListItem::paintCell( QPainter *p, const QColorGroup &cg,
				 int column, int width, int alignment )
{
	QColorGroup _cg( cg );
  QColor c = _cg.text();

  if ( mySong->artist=="{wish}" )				// show wishes in grey
		_cg.setColor( QColorGroup::Text, Qt::lightGray );

  QListViewItem::paintCell( p, _cg, column, width, alignment );
  _cg.setColor( QColorGroup::Text, c );
}

/**
 * sorting for the listview
 * (tries to sort as reasonable as possible)
 */
QString SongListItem::key(int column, bool ascending) const
{
	const Song* s=song();
	switch(column)
	{
	case 0:
		if(s->artist=="") return " "+s->title;
		return s->artist+s->title;
	case 2:
		if(s->album=="") return " "+s->title;
		return s->album+s->title;
	case 3:
		return QString("%1").arg(s->length, 10);
	case 4:
		return QString("%1").arg(s->year, 10);
	case 5:
		return QString("%1").arg(s->trackNr, 10);
	case 7:
		return QString("%1").arg(s->bitrate, 10);
	case 6:		// my personal Y2K+222-bug, might cause problems in 2222...
		return QString("%1").arg( s->addedTo.secsTo( QDateTime(QDate(2222, 1, 1), QTime(0,0,0)) ), 10);
	default:
		return text(column);
	}
}
