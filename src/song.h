/***************************************************************************
                          song.h  -  description
                             -------------------
    begin                : Sat Feb 10 2001
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

#ifndef SONG_H
#define SONG_H

#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
#include <qregexp.h>
#include <qfileinfo.h>
#include <qdir.h>

#include "mydatetime.h"
#include "options.h"


/* Wave File header structure */
typedef struct {
    char riff[4];
    long filesize;
    char rifftype[4];
    char formatChunkId[4];
    long formatChunkSize;
    short wFormatTag;
    short nChannels;
    long nSamplesPerSec;
    long nAvgBytesPerSec;
    short nBlockAlign;
    short wBitsPerSample;
    char dataChunkId[4];
    long dataChunkSize;
    // Data follows here.
}
WaveHeader;








/**
 * This class represents all information related to a single song.
 * Stores all info, including location and tags...
 * todo: make _some_ more things protected...
 */

#define MAX_SONG_ACTION 15

class Song {
public:
    // all possible actions for a single or a selection of songs
    // caution: corresponds to the static songAction field in class Song (song.cpp)
    enum action {	None, Enqueue, EnqueueAsNext, PlayNow, SongInfo,
                  Delete, DeleteFile, DeleteEntry,
                  CheckConsistency, MoveTo,
                  Dequeue
                };

    /// default constructor, just assigns default values
    Song();

    /// constructs a song object from the given parameters
    Song(QString artist, QString title, QString album, QString filename, QString path, int length, int bitrate, MyDateTime addedTo, int year, QString comment, int trackNr, QString genre);

    /// constructs a song object from a given file
    bool create(const QString filename, bool capitalizeTags);

    /// check consistency
    QString checkConsistency(bool requireConsistentTags, bool requireConsistentFilename, bool ignoreCaseInFilename, bool requireConsistentDirectory);

    /// rereads the id3 or ogg tags from the file
    bool rereadTags();

    bool readTags(QString filename);
    bool readLayerInfo(QString filename);
    
    // specific to wav objects
    //************************
    bool getWavInfo(QString filename);


    // general info (not specific to file format)
    //*************
    void guessTagsFromFilename(QString filename, QString path, QString* artist, QString* title, QString* album);

    // checking methods
    bool checkTags();
    bool checkFilename(bool ignoreCase);
    bool checkDirectory(bool ignoreCase);
    bool checkReadability();

    // saving methods
    bool saveTags();
    bool correctFilename();
    bool correctPath();



    /// compare primary key
    bool sameAs(Song* s);
    bool sameAs(QString _artist, QString _title, QString _album);

    /// replace placeholders in a string with info from this song
    QString replacePlaceholders(QString input, int index);

    void setTo(Song* s);					///< set values to other song object
    void updateWritableFields(Song* s);
    void updateReadableFields(Song* s);

    QString displayName() {
        return this->artist+" - "+this->title;
    }
    QString location() 		{
        return this->path+"/"+this->filename;
    }

    /**
      * Constructs a filename according to the configured filename pattern.
     * Takes care of special characters not allowed in filenames.
     */
    QString constructFilename();

    /**
      * Constructs a path according to the configured directory pattern.
     * Takes care of special characters not allowed in filenames.
     */
    QString constructPath();

    QString getSuffix();

    /**
     * Replaces invalid characters (for a filesystem) in a string.
     */
    QString makeValidFilename(QString filename, bool file);


    void deleteFile(QString trashDir);
    void moveTo(QString dir);


    // this is all info that we want to save belonging to a song object:
    // id3 info
    QString artist;            // artist
    QString title;             // title
    QString album;             // album
    QString comment;           // comment
    int trackNr;               // trackNr on CD
    int year;                  // year of song
    QString genre;             // genre as string

    // file info
    QString filename;          // filename, excluding path
    QString path;              // path to file, excluding filename, no trailing slash
    unsigned long filesize;    // filesize in bytes
    int length;                // length of song in seconds
    int bitrate;               // bitrate in kbps

    // additional database info
    MyDateTime addedTo;        // when song was added to my database
    MyDateTime lastPlayed;     // last time song was played
    int noPlayed;              // how often song was played since added

    bool classified;
    bool corrupted;

    static QString getSongAction(int index);
    static int getMaxSongAction()     {
        return MAX_SONG_ACTION;
    }
    static QString getReplacementsDescription();


protected:
    QString capitalize(QString str); ///< capitalize all words
};

#endif

