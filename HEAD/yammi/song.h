/***************************************************************************
                          song.h  -  description
                             -------------------
    begin                : Sat Feb 10 2001
    copyright            : (C) 2001 by Brian O.N�lle
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

#include <id3/tag.h>                // used to read id3 tags


// all possible actions for a single or a selection of songs
// caution: corresponds to the static songAction field in Song
enum action {	None, Enqueue, EnqueueAsNext, PlayNow, SongInfo,
							PrelistenStart, PrelistenMiddle, PrelistenEnd,
							Delete, DeleteFile, DeleteEntry,
							CheckConsistency, CopyTo, MoveTo, CopyAsWavTo,
							Dequeue, BurnToMedia
};

#define MAX_SONG_ACTION 17

/**
 * This class represents all information related to a single song.
 * Stores all info, including location and tags...
 * todo: make _some_ more things protected...
 */

class Song
{
public:

  /// default constructor, just assigns default values
  Song();
  
  /// constructs a song object from the given parameters
	Song(QString artist, QString title, QString album, QString filename, QString path, int length, int bitrate, MyDateTime addedTo, int year, QString comment, int trackNr, int genreNr);
	
	/// constructs a song object from a given file
	int create(const QString filename, const QString mediaName = 0);
	
	/// check consistency
	QString checkConsistency(bool requireConsistentTags, bool requireConsistentFilename);
	
  // id3 tag reading / guessing
  bool getTags(QString filename);
  bool setTags(QString filename);
  bool getId3Tag(ID3_Tag* tag, ID3_FrameID frame, ID3_FieldID field, QString* content);
  bool setId3Tag(ID3_Tag* tag, ID3_FrameID frame, ID3_FieldID field, QString content, ID3_Frame* newFrame);
  void guessTagsFromFilename(QString filename, QString* artist, QString* title);

  // mp3 layer info
  bool getLayerInfo(QString filename);

  // checking methods
	bool checkTags();
	bool checkFilename();
	bool checkReadability();
	
	// saving methods
	bool saveTags();
	bool saveFilename();
	
	/// compare primary key
	bool sameAs(Song* s);
	bool sameAs(QString _artist, QString _title, QString _album);
	
	void setTo(Song* s);					///< set values to other song object

	QString displayName() { return this->artist+" - "+this->title; }
	QString location() 		{ return this->path+"/"+this->filename; }

  /* Constructs a filename following the "artist-title.mp3" pattern.
   * (Should) Take care of special characters not allowed in filenames.
   */
  QString constructFilename();
	

	void addMediaLocation(QString mediaName, QString locationOnMedia);  ///< adds the location on a media to song info
	void renameMedia(QString oldMediaName, QString newMediaName);
	void deleteFile(QString trashDir);
	void moveTo(QString dir);
	void copyTo(QString dir);
	void copyAsWavTo(QString dir);
	

// this is all info that we want to save belonging to a song object:
	// id3 info
	QString artist;													// artist
	QString title;													// title
	QString album;													// album
	QString comment;												// comment
	int trackNr;														// trackNr on CD
	int year;																// year of song
	int bitrate;														// bitrate in kbps
	int genreNr;														// genre (index number)

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
	bool corrupted;
	bool tagsDirty;
	bool filenameDirty;

  static QString getSongAction(int index);
  static int getMaxSongAction()     { return MAX_SONG_ACTION; }	

protected:
	QString capitalize(QString str);				///< capitalize all words
};

#endif