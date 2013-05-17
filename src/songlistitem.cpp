/***************************************************************************
                          SongListItem.cpp  -  description
                             -------------------
    begin                : Tue Oct 2 2001
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

#include "songlistitem.h"

#include <taglib/id3v1genres.h>

#include "folder.h"
#include "foldersorted.h"
#include "yammigui.h"
#include "yammimodel.h"


extern YammiGui* gYammiGui;

SongListItem::SongListItem( Q3ListView* parent, SongEntry* entry, SongListItem* after )
    : Q3ListViewItem( parent, after )
{
	setColumns(entry);
}

bool SongListItem::columnIsVisible(int column)
{
  return gYammiGui->columnIsVisible(column);
}

void SongListItem::setColumns(SongEntry* entry)
{
	songEntry=entry;
	Song* s=entry->song();
	int base=entry->getBase();
	for(int i=0; i<base; i++) {
		setText( i, entry->getColumn(i));
	}
  int current=base;
  if(columnIsVisible(gYammiGui->COLUMN_ARTIST)) {
    setText( current, s->artist );
    current++;
  }

  if(columnIsVisible(gYammiGui->COLUMN_TITLE)) {
    setText( current, s->title );
    current++;
  }
  if(columnIsVisible(gYammiGui->COLUMN_ALBUM)) {
    setText( current, s->album );
    current++;
  }
  if(columnIsVisible(gYammiGui->COLUMN_LENGTH)) {
    if(s->length!=0) {
      QString lengthStr=QString("%1").arg(s->length % 60);
      if (lengthStr.length()==1) {
        lengthStr="0"+lengthStr;
      }
      setText( current, QString("%1:%2").arg((s->length) / 60).arg(lengthStr));
    }
    current++;
  }
  if(columnIsVisible(gYammiGui->COLUMN_YEAR)) {
    if(s->year!=0) {
      setText( current, QString("%1").arg(s->year)); 
    }
    current++;
  }
  if(columnIsVisible(gYammiGui->COLUMN_TRACKNR)) {
    if(s->trackNr!=0)	{
    	setText( current, QString("%1").arg(s->trackNr));
    }	
    current++;
  }
  if(columnIsVisible(gYammiGui->COLUMN_GENRE)) {
    setText( current, s->genre);
    current++;
  }
  if(columnIsVisible(gYammiGui->COLUMN_ADDED_TO)) {
    setText( current, s->addedTo.writeToString());
    current++;
  }
  if(columnIsVisible(gYammiGui->COLUMN_BITRATE)) {
    if(s->bitrate!=0) {
  		setText( current, QString("%1").arg(s->bitrate));
    }
    current++;
  }
  if(columnIsVisible(gYammiGui->COLUMN_FILENAME)) {
    setText( current, s->filename );
    current++;
  }
  if(columnIsVisible(gYammiGui->COLUMN_PATH)) {
    setText( current, s->path );
    current++;
  }
  if(columnIsVisible(gYammiGui->COLUMN_COMMENT)) {
    if(s->comment!="") {
      setText( current, s->comment );
    }
    current++;
  }
  if(columnIsVisible(gYammiGui->COLUMN_LAST_PLAYED)) {
    MyDateTime never;
    never.setDate(QDate(1900,1,1));
    never.setTime(QTime(0,0,0));
    if(s->lastPlayed!=never) {
      setText( current, s->lastPlayed.writeToString() );
    }
    else {
      setText( current, QObject::tr("never") );
    }
    current++;
  }
  
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
	
  Q3ListViewItem::paintCell( p, _cg, column, width, alignment );
  _cg.setColor( QColorGroup::Text, c );
}


/**
 * This method cares about all non-character data (length, trackNr, ...)
 * character data is handled by the key() function
 * visibleColumn is the original column number
 */
int SongListItem::compare( Q3ListViewItem *i, int visibleColumn, bool ascending ) const
{
	SongListItem* other=(SongListItem*)i;
	int base=songEntry->getBase();
	if(visibleColumn<base) {
		return songEntry->compare(visibleColumn, other->songEntry);
	}
	const Song* s=song();
	const Song* s2=other->song();
    int column=gYammiGui->mapToRealColumn(visibleColumn-base);
  // 0:artist, 1:title, 2:album
	if(column==gYammiGui->COLUMN_LENGTH)
		return s->length - s2->length;
	if(column==gYammiGui->COLUMN_YEAR)
		return s->year - s2->year;
	if(column==gYammiGui->COLUMN_TRACKNR)
		return s->trackNr - s2->trackNr;
	if(column==gYammiGui->COLUMN_ADDED_TO)
        return s->addedTo.secsTo(s2->addedTo);
	if(column==gYammiGui->COLUMN_BITRATE)
		return s->bitrate - s2->bitrate;
	// 9:filename, 10:path, 11:comment
	if(column==gYammiGui->COLUMN_LAST_PLAYED) {
    MyDateTime never;
    never.setDate(QDate(1900,1,1));
    never.setTime(QTime(0,0,0));
    if(s->lastPlayed==never)
      return 1;
    if(s2->lastPlayed==never)
      return -1;
		return s->lastPlayed.secsTo(s2->lastPlayed);
  }
	
	// all other cases: call the key() method
	return key(visibleColumn, ascending).compare(other->key(visibleColumn, ascending) );
}

/**
 * associates a string to a column
 * (needed for sorting the listview)
 * tries to sort as reasonable as possible
 */
QString SongListItem::key(int visibleColumn, bool) const
{
    int base=songEntry->getBase();
    if(visibleColumn<base) {
        return songEntry->getKey(visibleColumn);
    }

    int column=gYammiGui->mapToRealColumn(visibleColumn-base);
    const Song* s=song();

    if(column==gYammiGui->COLUMN_ARTIST) {
        if(s->artist=="") {
            return " "+s->title;
        } else{
            return s->artist+s->title;
        }
    }
    if(column==gYammiGui->COLUMN_TITLE) {
        return s->title;
    }
    if(column==gYammiGui->COLUMN_ALBUM) {
        if(s->album=="") {
            return " "+s->title;
        } else {
            return s->album+QString("%1").arg(s->trackNr, 2);
        }
    }
    if(column==gYammiGui->COLUMN_GENRE) {
        return s->genre;
    }
    if(column==gYammiGui->COLUMN_PATH) {
        return s->path+s->filename;
    }
    return text(column);
}

Song* SongListItem::song() const
{
    return songEntry->song();
}
