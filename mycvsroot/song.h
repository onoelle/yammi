/***************************************************************************
                          song.h  -  description
                             -------------------
    begin                : Sat Feb 10 2001
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

#ifndef SONG_H
#define SONG_H

#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <qregexp.h>
#include <qfileinfo.h>
#include <qdir.h>

#include "mydatetime.h"


// possible actions for selected/current songs
enum action {	None, Enqueue, EnqueueAsNext, PlayNow, SongInfo,
							Delete, DeleteFile, DeleteEntry,
							CheckConsistency, CopyTo, MoveTo, CopyAsWavTo
};

/**
 * This class represents all information related to a single song.
 * Stores all info, including location and tags...
 * todo: make _some_ more things protected...
 */

class Song
{
public:
	/// default constructor
	Song(QString artist, QString title, QString album, QString filename, QString path, int length, int bitrate, MyDateTime addedTo, int year, QString comment, int trackNr);
	
	/// constructs a song object from a given file
	Song(const QString filename);
	
	/// check consistency (and make consistent)
	bool checkConsistency();
	
	// checking methods
	bool checkTags();
	bool checkFilename() { return (constructFilename()==filename); };
	bool checkReadability();
	
	// saving methods
	bool saveTags();
	bool saveFilename();
	
	bool sameAs(Song* s);					///< compare if same as other song
	void setTo(Song* s);					///< set values to other song object

	QString displayName() { return this->artist+" - "+this->title; }
	QString location() { return this->path+"/"+this->filename; }

  /* Constructs a filename following the "artist-title.mp3" pattern.
   * (Should) Take care of special characters not allowed in filenames.
   */
  QString constructFilename();
	QString capitalize(QString str);				///< capitalize all words
	

	void addMediaLocation(QString mediaName, QString locationOnMedia);  ///< adds the location on a media to song info
	void deleteFile(QString trashDir);
	void moveTo(QString dir);
	void copyTo(QString dir);
	void copyAsWavTo(QString dir);
	

// this is all info that we want to save belonging to a song object:
	// id3 info
	QString artist;
	QString title;
	QString album;
	QString comment;												// comment
	int trackNr;														// trackNr on CD
	int year;																// year of song
	int bitrate;														// bitrate in kbps
	// file and mp3-layer info
	QString filename;												// filename, excluding path
	QString path;														// path to file, excluding filename, no trailing slash
	unsigned long filesize;									// filesize in bytes
	int length;															// length of song in seconds
	
	// additional database info
	MyDateTime addedTo;											// when song was added to my database
	MyDateTime lastPlayed;									// last time song was played
	int noPlayed;														// how often song was played since added
	QStringList mediaName;
	QStringList mediaLocation;
	
	bool classified;
	bool artistSure;
	bool titleSure;
	bool corrupted;
	bool tagsDirty;
	bool filenameDirty;
	
//	int genre;														// sorry, I'm not using genre (so far)

};

#endif