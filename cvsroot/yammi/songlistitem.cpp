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
#include "yammigui.h"
#include "mp3tag.h"

extern YammiGui* gYammiGui;

SongListItem::SongListItem( QListView *parent, SongEntry *entry, SongListItem* after )
    : QListViewItem( parent, after )
{
	songEntry=entry;
	Song* s=entry->song();
	int base=entry->getBase();
	for(int i=0; i<base; i++) {
		setText( i, entry->getColumn(i));
	}
  setText( base+0, s->artist );
  setText( base+1, s->title );
  setText( base+2, s->album );
	if(s->length!=0) {
		QString lengthStr=QString("%1").arg(s->length % 60);
		if (lengthStr.length()==1)
	  	lengthStr="0"+lengthStr;
		setText( base+3, QString("%1:%2").arg((s->length) / 60).arg(lengthStr));
	}
	if(s->year!=0)				setText( base+4, QString("%1").arg(s->year));
  if(s->trackNr!=0)			setText( base+5, QString("%1").arg(s->trackNr));
	int index=s->genreNr;
	if(index>ID3v1_MaxGenreNr)
		index=-1;
  if(index!=-1)					setText( base+6, QString("%1 (%2)").arg(ID3v1_Genre[index]).arg(s->genreNr));
	setText( base+7, s->addedTo.writeToString());
  if(s->bitrate!=0)			setText( base+8, QString("%1").arg(s->bitrate));
  if(s->filename!="")		setText( base+9, s->filename );
  if(s->path!="")				setText( base+10, s->path );
  if(s->comment!="")		setText( base+11, s->comment );
	MyDateTime never;
	never.setDate(QDate(1900,1,1));
	never.setTime(QTime(0,0,0));
  if(s->lastPlayed!=never)
												setText( base+12, s->lastPlayed.writeToString() );
	else
												setText( base+12, "never" );
}

void SongListItem::paintCell( QPainter *p, const QColorGroup &cg,
				 int column, int width, int alignment )
{
	QColorGroup _cg( cg );
  QColor c = _cg.text();

  if ( song()->artist=="{wish}" )													// wishes in grey
		_cg.setColor( QColorGroup::Text, Qt::lightGray );

	if(gYammiGui->getModel()->songsPlayed.containsSong(song()))	// already played in green
		_cg.setColor( QColorGroup::Text, Qt::darkGreen );
	
	if(gYammiGui->getModel()->songsToPlay.containsSong(song()))	// enqueued songs in blue
		_cg.setColor( QColorGroup::Text, Qt::blue );

	if(gYammiGui->currentSong==song())											// current song in red
		_cg.setColor( QColorGroup::Text, Qt::red );
	
  QListViewItem::paintCell( p, _cg, column, width, alignment );
  _cg.setColor( QColorGroup::Text, c );
}

/**
 * sorting for the listview
 * (tries to sort as reasonable as possible)
 */
QString SongListItem::key(int column, bool ascending) const
{
	int base=songEntry->getBase();
	const Song* s=song();
	if(column<base) {
		return songEntry->getKey(column);
	}
	
	if(column==base+0)
		if(s->artist=="")
			return " "+s->title;
		else
			return s->artist+s->title;
	if(column==base+2)
		if(s->album=="")
			return " "+s->title;
		else
			return s->album+s->title;
	if(column==base+3)
		return QString("%1").arg(s->length, 10);
	if(column==base+4)
		return QString("%1").arg(s->year, 10);
	if(column==base+5)
		return QString("%1").arg(s->trackNr, 10);
	if(column==base+6)
		return QString("%1").arg(999999999+ QDateTime( QDate(2222, 1, 1), QTime(0,0,0) ).secsTo(s->addedTo), 10);
	if(column==base+7)
		return QString("%1").arg(s->bitrate, 10);
	if(column==base+11)
		return QString("%1").arg(999999999+ QDateTime( QDate(2222, 1, 1), QTime(0,0,0) ).secsTo(s->lastPlayed), 10);
	return text(column);
}
