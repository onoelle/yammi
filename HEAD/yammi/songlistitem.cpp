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
#include "mp3info/CMP3Info.h"

extern YammiGui* gYammiGui;

SongListItem::SongListItem( QListView* parent, SongEntry* entry, SongListItem* after )
    : QListViewItem( parent, after )
{
	setColumns(entry);
}

void SongListItem::setColumns(SongEntry* entry)
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
	if(index>CMP3Info::getMaxGenreNr())
		index=-1;
  if(index!=-1)					setText( base+6, QString("%1 (%2)").arg(CMP3Info::getGenre(index)).arg(s->genreNr));
	setText( base+7, s->addedTo.writeToString());
  if(s->bitrate!=0)			setText( base+8, QString("%1").arg(s->bitrate));
	if(s->filename!="") {
  	setText( base+9, s->filename );
  	setText( base+10, s->path );
	}
	else {		// song not on harddisk
		QString mediaNameList("");
		for(unsigned int i=0; i<s->mediaName.count(); i++)
			mediaNameList+="<"+s->mediaName[i]+"> ";
		setText( base+10, mediaNameList);
	}
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

	if(gYammiGui->getModel()->songsPlayed.containsSong(song()))	// already played in green
		_cg.setColor( QColorGroup::Text, Qt::darkGreen );
	
	if(gYammiGui->getModel()->songsToPlay.containsSong(song()))	// enqueued songs in blue
		_cg.setColor( QColorGroup::Text, Qt::blue );

	// availability: in playlist we really check whether available (maybe in swap dir)
	// otherwise we just check filename (for performance)
	if(gYammiGui->chosenFolder==gYammiGui->folderActual) {
	  QString path=gYammiGui->getModel()->checkAvailability(song());
  	if (path=="" )										// not yet available in light grey
			_cg.setColor( QColorGroup::Text, Qt::lightGray );
	  else if (path=="never" )					// never available in yellow
			_cg.setColor( QColorGroup::Text, Qt::yellow );
		else
      if(song()->filename=="")															// swapped in dark grey
        _cg.setColor( QColorGroup::Text, Qt::darkGray );
      
	}
 	else {	
	 	if (song()->filename=="") {
			_cg.setColor( QColorGroup::Text, Qt::gray );
		}
	}

	if(gYammiGui->currentSong==song())											// current song in red
		_cg.setColor( QColorGroup::Text, Qt::red );
	
  QListViewItem::paintCell( p, _cg, column, width, alignment );
  _cg.setColor( QColorGroup::Text, c );
}


/**
 * This method cares about all non-character data (length, trackNr, ...)
 * character data is handled by the key() function
 */
int SongListItem::compare( QListViewItem *i, int column, bool ascending ) const
{
	SongListItem* other=(SongListItem*)i;
	int base=songEntry->getBase();
	if(column<base) {
		return songEntry->compare(column, other->songEntry);
	}
	const Song* s=song();
	const Song* s2=other->song();

  // 0:artist, 1:title, 2:album
	if(column==base+3)				// length
		return s->length - s2->length;
	if(column==base+4)				// year
		return s->year - s2->year;
	if(column==base+5)				// trackNr
		return s->trackNr - s2->trackNr;
	if(column==base+7)				// addedTo
		return s->addedTo.secsTo(s2->addedTo);
	if(column==base+8)				// bitrate
		return s->bitrate - s2->bitrate;
	// 9:filename, 10:path, 11:comment
	if(column==base+12)				// last played
		return s->lastPlayed.secsTo(s2->lastPlayed);
	
	// all other cases: call the key() method
	return key(column, ascending).compare(other->key(column, ascending) );
}

/**
 * associates a string to a column
 * (needed for sorting the listview)
 * tries to sort as reasonable as possible
 */
QString SongListItem::key(int column, bool ascending) const
{
	int base=songEntry->getBase();
	const Song* s=song();
	if(column<base) {
		return songEntry->getKey(column);
	}
	
	if(column==base+0)				// artist
		if(s->artist=="")
			return " "+s->title;
		else
			return s->artist+s->title;
	if(column==base+1)				// title
		return s->title;
	if(column==base+2)				// album
		if(s->album=="")
			return " "+s->title;
		else
			return s->album+QString("%1").arg(s->trackNr, 2);       //			return s->album+s->title;
	if(column==base+6)				// genre
		return QString("%1").arg(CMP3Info::getGenre(s->genreNr));
	return text(column);
}
