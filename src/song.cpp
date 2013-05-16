/***************************************************************************
                          song.cpp  -  description
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

#include "song.h"

#include <taglib/audioproperties.h>
#include <taglib/fileref.h>
#include <taglib/id3v1genres.h>    //used to load genre list
#include <taglib/tag.h>
#include <taglib/tstring.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "ConsistencyCheckParameter.h"
#include "prefs.h"
#include "util.h"
#include "yammigui.h"

using namespace std;

extern YammiGui* gYammiGui;

#define MAX_FILENAME_LENGTH 1000

/**
 * creates a song object with default values
 */
Song::Song() {
    classified = false;
    corrupted = true;

    artist = "";
    title = "";
    album = "";
    length = 0;
    bitrate = -1;
    MyDateTime now = now.currentDateTime();
    addedTo = now;
    comment = "";
    year = 0;
    trackNr = 0;
    genre = "";
    lastPlayed.setDate(QDate(1900,1,1));
    lastPlayed.setTime(QTime(0,0,0));
}


void Song::updateReadableFields(Song* s) {
    updateWritableFields(s);
    this->lastPlayed=s->lastPlayed;
    this->noPlayed=s->noPlayed;
    this->addedTo=s->addedTo;
    this->filename=s->filename;
    this->path=s->path;
    this->filesize=s->filesize;
    this->bitrate=s->bitrate;
}

/**
 * Updates all writable fields according to the given Song.
 */
void Song::updateWritableFields(Song* s) {
    this->addedTo=s->addedTo;
    this->album=s->album;
    this->artist=s->artist;
    this->comment=s->comment;
    this->title=s->title;
    this->year=s->year;
    this->trackNr=s->trackNr;
    this->genre=s->genre;
}

/** constructs a song object with the given parameters
 */
Song::Song(QString artist, QString title, QString album, QString filename, QString path, int length, int bitrate, MyDateTime addedTo, int year, QString comment, int trackNr, QString genre) {
    classified = false;
    lastPlayed.setDate(QDate(1900,1,1));
    lastPlayed.setTime(QTime(0,0,0));

    corrupted = false;

    this->artist = artist;
    this->title = title;
    this->album = album;
    this->filename = filename;
    this->path = path;
    this->length = length;
    this->bitrate = bitrate;
    this->addedTo = addedTo;
    this->comment = comment;
    this->trackNr = trackNr;
    this->genre = genre;
    this->year = year;
}



/**
 * Try to create a new song object from a given filename.
 * @return    true if successful
 *            false on error (file not found)
 */
bool Song::create(const QString location, bool capitalizeTags) {
    // step 1:
    // - check for existence and access
    // - get filename, path and size

    QFileInfo* fi = new QFileInfo(location);
    if(!fi->exists()) {
        qDebug() << "trying to construct song, but file " << location << " does not exist!";
        return false;
    }
    if(!fi->isReadable()) {
        qDebug() << "trying to construct song, but file " << location << " is not accessible!";
        return false;
    }
    filename = fi->fileName();
    path = fi->dirPath(TRUE);
    filesize=fi->size();
    QString saveFilename(fi->fileName());
    QString savePath(fi->dirPath(TRUE));
    delete fi;
    fi = 0;

    if(!readTags(location)) {
        guessTagsFromFilename(saveFilename, savePath, &artist, &title, &album);
        qDebug() << "guessed: artist: " << artist << ", title: " << title << ", album: " << album;
    }
    if(!readLayerInfo(location)) {
        length = 0;
        bitrate = 0;
    }
    

    // simplify whitespaces
    artist=artist.simplifyWhiteSpace();
    title=title.simplifyWhiteSpace();

    if(capitalizeTags) {
        // capitalize after spaces (any exceptions?)
        album=capitalize(album);
        artist=capitalize(artist);
        title=capitalize(title);
    }

    corrupted=false;
    return true;
}


/**
 * Try to read tags from the file.
 *
 * @param filename absolute filename of file
 * @return true if at least one tag could be read and was not empty, false otherwise.
 */
bool Song::readTags(QString filename) {
    TagLib::FileRef f(QFile::encodeName(filename));
    if(f.isNull()) {
        qDebug() << "reading tags: could not create TagLib::FileRef for file " << filename;
        return false;
    }
    // read tags
    TagLib::Tag* tag;
    tag = f.tag();
    if(tag == 0) {
        qDebug() << "reading tags: tag == 0 in file " << filename;
        return false;
    }
    if(tag->isEmpty()) {
        qDebug() << "reading tags: tags are empty in file " << filename;
        return false;
    }
    album = TStringToQString( tag->album() ).stripWhiteSpace();
    artist = TStringToQString( tag->artist() ).stripWhiteSpace();
    title = TStringToQString( tag->title() ).stripWhiteSpace();
    comment = TStringToQString( tag->comment() ).stripWhiteSpace();
    year = (int) tag->year();
    trackNr = (int) tag->track();
    genre = TStringToQString ( tag->genre() ).stripWhiteSpace();
    return true;
}

/**
 * Read audio properties from the file: length and bitrate.
 *
 * @return true if properties could be read, false otherwise.
 */
bool Song::readLayerInfo(QString filename) {
    TagLib::FileRef f(QFile::encodeName(filename));
    if(f.isNull()) {
        qDebug() << "reading layer info: could not create TagLib::FileRef for file " << filename;
    }
    else {
        TagLib::AudioProperties* audioProperties;
        audioProperties = f.audioProperties();
        if(audioProperties == 0) {
            qDebug() << "could not read audio properties from file " << filename;
        }
        else {
            bitrate = audioProperties->bitrate();
            length = audioProperties->length();
            return true;
        }
    }

    // taglib was not successful
    if(filename.right(4).upper()==".WAV") {
        if(getWavInfo(filename)) {
            return true;
        }
        else {
            qDebug() << "reading layer info: could not read wav header information from wav file \"" << filename << "\"";
        }
    }
    qDebug() << "reading layer info: not successful => length and bitrate set to 0";
    return false;
}

/**
 * Re-reads the tags from filename,
 * overwriting the info in yammi's database.
 * Returns true, if tags have been read.
 */
bool Song::rereadTags() {
    if(!readTags(location())) {
        guessTagsFromFilename(filename, path, &artist, &title, &album);
        qDebug() << "guessed: artist: " << artist << ", title: " << title << ", album: " << album;
    }
    return true;
}



/**
 * check filename (if given)
 * returns true if it is consistent with the configured pattern
 */
bool Song::checkFilename(bool ignoreCase) {
    if(filename=="") {
        return true;
    } else {
        if(ignoreCase) {
            return ((constructFilename().upper())==(filename.upper()));
        } else {
            return (constructFilename()==filename);
        }
    }
}

/**
 * check directory (if given)
 * returns true if it is consistent with the configured pattern
 */
bool Song::checkDirectory(bool ignoreCase) {
    if(path=="") {
        return true;
    } else {
        if(ignoreCase) {
            return ((constructPath().upper())==(path.upper()));
        } else {
            return (constructPath()==path);
        }
    }
}


/** Tries to guess artist and title from filename.
 * So far, only a simple and an advanced pattern available for guessing
 * \todo: add more sophisticated pattern (leading trackNr?, directory with album?)
 * (or add a configurable pattern description)
 */
void Song::guessTagsFromFilename(QString filename, QString path, QString* artist, QString* title, QString* album) {
    if(filename == "") {
        return;
    }
    *album="";
    *artist="";
    *title="";
    QString guessBase=filename;
    int suffixPosition=guessBase.findRev('.');
    if(suffixPosition != -1) {
        // remove suffix if there is one
        guessBase=guessBase.left(suffixPosition);
    }
    guessBase=guessBase.replace( QRegExp("_"), " " );                           // replace "_" with " "

    if(gYammiGui->config()->guessingMode == Prefs::GUESSING_MODE_SIMPLE) {
        int pos=guessBase.find('-');
        if(pos!=-1) {
            *artist=guessBase.left(pos);
            *artist=artist->simplifyWhiteSpace();
            *title=guessBase.right(guessBase.length()-pos-1);
            *title=title->simplifyWhiteSpace();
        } else {
            *artist="unknown";
            *title=guessBase;
            *title=title->simplifyWhiteSpace();
        }
        return;
    }

    if(gYammiGui->config()->guessingMode == Prefs::GUESSING_MODE_ADVANCED) {
        // eg "/artist/album/01 - trackname.mp3"
        //    pos1  pos2

        int pos2=path.findRev('/');
        if(pos2>0) {
            int pos1=path.findRev('/', pos2-1);
            if(pos1>0) {
                *artist=path.mid(pos1+1, pos2-pos1-1);
            }
            *album=path.mid(pos2+1);
        }

        int pos=guessBase.find('-');
        if(pos!=-1) {
            *title=guessBase.mid(pos+1);
            *title=title->simplifyWhiteSpace();
            QString trackNrStr=guessBase.left(pos);
            trackNr=atoi(trackNrStr);
        } else {
            *title=guessBase;
        }
        return;
    }
    qDebug() << "ERROR: unknown guessing mode for guessing tags!";
}







////////////////////////////////
// special handling of wav files


bool Song::getWavInfo(QString filename) {
    QFile wavFile(filename);
    if( wavFile.open( QIODevice::ReadOnly ) ) {
        QDataStream stream( &wavFile );
        WaveHeader header;
        stream.readRawBytes((char*)&header, sizeof(WaveHeader));
        wavFile.close();
        //    qDebug() << "header data: nChannels: " << header.nChannels << ", wBitsPerSample: " << header.wBitsPerSample;
        //    qDebug() << "nSamplesPerSec: " << header.nSamplesPerSec << ", filesize: " << header.filesize << ", avgBytesPerSec: " << header.nAvgBytesPerSec;
        //    qDebug() << "formatChunkSize: " << header.formatChunkSize << ", dataChunkSize: " << header.dataChunkSize;

        if(header.nChannels==0 || header.nSamplesPerSec==0 || header.wBitsPerSample==0) {
            qDebug() << "length calculation of file " << filename << " would have yielded division by zero, debug info:";
            qDebug() << "header data: nChannels: " << header.nChannels << ", wBitsPerSample: " << header.wBitsPerSample;
            qDebug() << "nSamplesPerSec: " << header.nSamplesPerSec << ", filesize: " << header.filesize << ", avgBytesPerSec: " << header.nAvgBytesPerSec;
            qDebug() << "formatChunkSize: " << header.formatChunkSize << ", dataChunkSize: " << header.dataChunkSize;
            qDebug() << "setting length to 1";
            length=1;
        } else {
            qDebug() << "calculating length...";
            this->length=((((header.dataChunkSize * 8) / header.nChannels) / header.nSamplesPerSec) / header.wBitsPerSample);
            qDebug() << "...and bitrate...";
            this->bitrate=header.nAvgBytesPerSec * 8 / 1000;
            qDebug() << "...done!";
            this->comment=QString("%1, %2 KHz, %3 bit").arg(header.nChannels==2 ? "stereo" : "mono").arg(header.nSamplesPerSec).arg(header.wBitsPerSample);
        }
        return true;
    } else {
        qDebug() << "Error in opening wav file for reading!";
        return false;
    }
}

// end of special handling of wav files
//////////////////////////////////




/**
 * check tags (if songfile available)
 * @return true if tags correctly set, false if differences between database info and tags in file
 */
bool Song::checkTags() {
    if(filename=="") {      // song not on local harddisk => we can't check
        return true;
    }
    qDebug() << "checking tags of " << displayName() << ", title: " << title;

    QString ffArtist, ffTitle, ffAlbum;
    guessTagsFromFilename(filename, path, &ffArtist, &ffTitle, &ffAlbum);

    bool same=false;

    QString _album=this->album;
    QString _artist=this->artist;
    QString _comment=this->comment;
    QString _title=this->title;
    QString _genre=this->genre;
    int _year=this->year;
    int _trackNr=this->trackNr;

    if(!readTags(location())) {
        guessTagsFromFilename(filename, path, &artist, &title, &album);
    }
    
    // tags exist => compare to our fields
    same=true;
    if(_album     != this->album)    {
        same=false;
        qDebug() << "(album)";
    }
    if(_artist    != this->artist)   {
        same=false;
        qDebug() << "(artist)";
    }
    if(_comment   != this->comment)  {
        same=false;
        qDebug() << "(comment)";
    }
    if(_title     != this->title)    {
        same=false;
        qDebug() << "(title)";
    }
    if(_year      != this->year)     {
        same=false;
        qDebug() << "(year)";
    }
    if(_trackNr   != this->trackNr)  {
        same=false;
        qDebug() << "(trackNr)";
    }
    if(_genre   != this->genre)  {
        same=false;
        qDebug() << "(genre: yammi: " << _genre << ", file: " << genre << ")";
    }

    if(same) {            // no differences
        return true;
    }
    qDebug() << "checked tags of " << displayName() << ", title: " << title;

    // restore original values
    this->album=_album;
    this->artist=_artist;
    this->comment=_comment;
    this->title=_title;
    this->year=_year;
    this->trackNr=_trackNr;
    this->genre=_genre;

    return false;
}


/**
 * Saves tags from yammi song database to file.
 * @return true if tags have been successfully written.
 */
bool Song::saveTags() {
    qDebug() << "trying to save tags on file " << filename;
    QString filename = location();
    if(filename=="") {
        return false;
    }
    QFileInfo fi(filename);
    if(!fi.isWritable()) {
        qWarning() << "writing tags: file " << filename << " not writable, skipping";
        return false;
    }

    TagLib::FileRef f(QFile::encodeName(filename));
    if(f.isNull()) {
        qDebug() << "saving tags: could not create TagLib::FileRef for file " << filename;
        return false;
    }
    TagLib::Tag* tag;
    tag = f.tag();
    if(tag == 0) {
        qDebug() << "saving tags: tag == 0 in file " << filename;
        return false;
    }
    tag->setAlbum(QStringToTString(album));
    tag->setArtist(QStringToTString(artist));
    tag->setTitle(QStringToTString(title));
    tag->setComment(QStringToTString(comment));
    tag->setYear(year);
    tag->setTrack(trackNr);
    tag->setGenre(QStringToTString(genre));
    bool success = f.save();
    if(!success) {
        qDebug() << "saving tags: error on writing tags";
    }
    return success;
}

/**
 * correct filename according to filename pattern (eg. "artist - title")
 */
bool Song::correctFilename() {
    if(filename=="") {
        return true;
    }
    QString oldname=location();
    QFileInfo fi0(oldname);
    if(!fi0.isWritable()) {
        qDebug() << "renaming file: file " << oldname << " not writable, skipping";
        return true;
    }
    QString newFilename=constructFilename();
    QString newname=QString("%1/%2").arg(this->path).arg(newFilename);

    // bug in windows-filesystem?
    // renaming fails if new and old are just different in case
    QDir currentDir=QDir("/");
    if(newname.upper()==oldname.upper()) {
        if(!currentDir.rename(oldname, oldname+".xxx")) {
            qDebug() << "WARNING: renaming: new filename equals old filename (except case), and renaming failed (" << newFilename << ")";
            return false;
        }
        oldname=oldname+".xxx";
    }

    // we first check whether we can create a file with that name (overwriting anything?)
    QFileInfo fi1(newname);
    if(fi1.exists()) {
        qDebug() << "WARNING: renaming: new Filename already existing, aborting (" << newFilename << ")";
        return false;           // return if we don't want to overwrite anything
    }

    QFile touchFile(newname);
    if(!touchFile.open(QIODevice::WriteOnly)) {
        qDebug() << "ERROR renaming: could not touch file";
        return false;
    }
    QString dummy="test";
    touchFile.writeBlock(dummy, dummy.length());
    touchFile.close();

    QFileInfo fi(newname);
    if(!fi.exists()) {
        qDebug() << "ERROR renaming: Filename not allowed: " << newFilename;
        return false;
    }
    // okay, successful

    if(!currentDir.rename(oldname, newname)) {
        qDebug() << "ERROR: renaming from " << oldname << " to " << newname << "failed!";
        return false;
    }
    this->filename=newFilename;
    qDebug() << "filename corrected to " << newFilename;
    return true;
}


/**
 * Correct location of file
 */
bool Song::correctPath() {
    QString newPath=constructPath();
    if(!Util::ensurePathExists(newPath)) {
        qDebug() << "could not create path " << newPath;
        return false;
    }
    QDir d=QDir("/");
    if(!d.rename(location(), newPath + "/" + filename)) {
        qDebug() << "could not rename file from " << location() << " to " << newPath;
        return false;
    }
    path = newPath;
    qDebug() << "path corrected to " << newPath;
    return true;
}


/** returns true if primary key (artist, title, album) is the same
 */
bool Song::sameAs(Song* s) {
    return(artist==s->artist && title==s->title && album==s->album);
}

bool Song::sameAs(QString _artist, QString _title, QString _album) {
    return(artist==_artist && title==_title && album==_album);
}

// sets value of this to new values of s
// does not affect: addedTo, lastPlayed and noPlayed!
void Song::setTo(Song* s) {
    this->album=s->album;
    this->artist=s->artist;
    this->bitrate=s->bitrate;
    this->comment=s->comment;
    this->filename=s->filename;
    this->length=s->length;
    this->path=s->path;
    this->title=s->title;
    this->year=s->year;
    this->trackNr=s->trackNr;
    this->genre=s->genre;
}

/** check songfile (exists and readable)
 * @returns true, if songfile is accessible
 */
bool Song::checkReadability() {
    if(filename=="")
        return false;
    QFileInfo fileInfo(location());
    if(!fileInfo.exists() || !fileInfo.isReadable()) {
        qDebug() << "file " << location() << " does not exist or unreadable";
        return false;
    }
    return true;
}



/** check consistency of song
 * - file exists and readable
 * - id3 tags
 * - consistency with filename
 * @returns "" on song okay (or wish or filename==""), or on error:
 * "file not readable", "tags not set correctly", "filename not consistent"
 */
QString Song::checkConsistency(bool requireConsistentTags, bool requireConsistentFilename, bool ignoreCaseInFilenames, bool requireConsistentDirectory) {
    QString diagnosis="";

    if(artist=="{wish}") {                          // ignore wishes...
        return "";
    }
    if(filename=="") {                                  // ...and songs not on harddisk
        return "";
    }

    if(checkReadability()==false) {
        return "file not readable";
    }


    if(requireConsistentTags) {
        // checking tags
        if(!checkTags()) {
            qDebug() << "tags on file " << this->filename << " are not set correctly...";
            diagnosis+="tags not correct ";
        }
    }

    if(requireConsistentFilename && filename!="") {
        // checking filename
        if(!checkFilename(ignoreCaseInFilenames)) {
            qDebug() << "file " << this->filename << " does not have correct filename";
            diagnosis+="filename not consistent ";
        }
    }

    if(requireConsistentDirectory && path!="") {
        // checking directory
        if(!checkDirectory(ignoreCaseInFilenames)) {
            qDebug() << "file " << this->filename << " is not in correct directory";
            diagnosis+="directory not consistent ";
        }
    }
    return diagnosis;
}


QString Song::capitalize(QString str) {
    // do nothing with the empty string
    if(str=="")
        return str;
    str[0] = str.at(0).upper();
    for(int pos=1; pos<str.length(); pos++) {
        if(str.at(pos-1)==' ')
            str[pos]=str.at(pos).upper();
    }
    return str;
}


/**
 * Constructs a filename according to the configured filename pattern.
 * Takes care of special characters not allowed in filenames.
 */
QString Song::constructFilename() {
    QString pattern=gYammiGui->config()->consistencyPara->filenamePattern;
    QString filename=replacePlaceholders(pattern, 0);
    return makeValidFilename(filename, true);
}

/**
 * Constructs a path according to the configured directory pattern.
 * Takes care of special characters not allowed in filenames.
 * Returns the path without trailing slash.
 */
QString Song::constructPath() {
    QString pattern=gYammiGui->config()->consistencyPara->directoryPattern;
    QString path=replacePlaceholders(pattern, 0);
    // now remove "empty navigation steps" such as "Madonna//Holiday.mp3" (because no album given)
    while(path.contains("//")) {
        path.replace("//", "/");
    }
    while(path.endsWith("/")) {
        path=path.left(path.length()-1);
    }
    return makeValidFilename(path, false);
}


/**
 * Returns the suffix of the filename of this song
 */
QString Song::getSuffix() {
    QString base;
    base=filename;
    if(filename=="!") {
        return "!";
    }

    int suffixLength=base.findRev('.');
    if(suffixLength==-1) {
        return "";
    }
    QString suffix=base.mid(suffixLength+1);
    if(base.toInt()==0) {
        return "";
    }
    return suffix;
}


/**
 * replace all forbidden characters for filenames with nothing
 * for windows and linux filesystems!
 * TODO: make configurable (depending on filesystem?)
 */
QString Song::makeValidFilename(QString filename, bool file) {
    filename.replace(QRegExp("\""), "");                                                // "
    filename.replace(QRegExp("'"), "");                                                 // '
    if(file) {
        filename.replace(QRegExp("/"), "");                                                 // /
    }
    filename.replace(QRegExp("&"), "");                                                 // &
    filename.replace(QRegExp("[?]"), "");                                               // ?
    filename.replace(QRegExp(":"), "");                                                 // :

    filename.replace(QRegExp("ü"), "ue");                                                   // umlaute
    filename.replace(QRegExp("Ü"), "Ue");                                                   //
    filename.replace(QRegExp("ö"), "oe");                                                   //
    filename.replace(QRegExp("Ö"), "Oe");                                                   //
    filename.replace(QRegExp("ä"), "ae");                                                   //
    filename.replace(QRegExp("Ä"), "Ae");                                                   //
    return filename;
}


void Song::deleteFile(QString trashDir)                     // move songfile to trash
{
    if(filename=="")
        return;
    moveTo(trashDir);
    filename="";
    path="";
}


// move file to another directory
void Song::moveTo(QString dir) {
    if(filename=="") {
        return;
    }
    if(dir.right(1)=="/") {
        // strip trailing slash
        dir=dir.left(dir.length()-1);
    }

    QString newLocation=QString("%3/%4").arg(dir).arg(filename);
    moveFile(location(), newLocation);
}


QString Song::getSongAction(int index) {
    const char* songAction[] = {"None", "Enqueue", "EnqueueAsNext", "PlayNow", "SongInfo",
                                "PrelistenStart", "PrelistenMiddle", "PrelistenEnd",
                                "Delete", "DeleteFile", "DeleteEntry",
                                "CheckConsistency", "MoveTo",
                                "Dequeue"
                               };
    if(index<=MAX_SONG_ACTION-1) {
        return QString(songAction[index]);
    }
    else {
        return QString("no such action");
    }
}


/**
 * Return a description of available replacements for a song.
 */
QString Song::getReplacementsDescription() {
    QString msg;
    msg+=QObject::tr("{filename} (without path)\n");
    msg+=QObject::tr("{absoluteFilename} (including path)\n");
    msg+=QObject::tr("{filenameWithoutSuffix} (without path, without suffix)\n");
    msg+=QObject::tr("{suffix} (without leading dot)\n");
    msg+=QObject::tr("{path} (without filename)\n");
    msg+=QObject::tr("{artist}, {title}, {album}, {comment} (corresponding to the tags)\n");
    msg+=QObject::tr("{bitrate} (in kbps)\n");
    msg+=QObject::tr("{length} (length in format mm:ss)\n");
    msg+=QObject::tr("{lengthInSeconds} (length in seconds)\n");
    msg+=QObject::tr("{trackNr} (Track number)\n");
    msg+=QObject::tr("{trackNr2Digit} (as above, but padded with zero if necessary)\n");
    return msg;
}

/**
 * replace placeholders in a string with info from this song
 *
 */
QString Song::replacePlaceholders(QString input, int index) {
    // 1. prepare strings
    QString lengthStr;

    // filename without suffix
    QString filenameWithoutSuffix;
    int suffixPos = filename.findRev('.');
    if(suffixPos == -1) {
        filenameWithoutSuffix = filename;
    }
    else {
        filenameWithoutSuffix = filename.left(suffixPos);
    }

    // trackNr
    QString trackNrStr;
    if(trackNr==0) {
        trackNrStr="";
    } else {
        trackNrStr=QString("%1").arg(trackNr);
    }

    // replace
    input.replace(QRegExp("'\\{absoluteFilename\\}'"), location()); // With the QProcess in Qt4 the '' would be given to the program - which would therefore not find the file.
    input.replace(QRegExp("\\{absoluteFilename\\}"), location());
    input.replace(QRegExp("\\{filename\\}"), filename);
    input.replace(QRegExp("\\{filenameWithoutSuffix\\}"), filenameWithoutSuffix);
    input.replace(QRegExp("\\{suffix\\}"), getSuffix());
    input.replace(QRegExp("\\{path\\}"), path);
    input.replace(QRegExp("\\{artist\\}"), artist);
    input.replace(QRegExp("\\{title\\}"), title);
    input.replace(QRegExp("\\{album\\}"), album);
    input.replace(QRegExp("\\{comment\\}"), comment);
    input.replace(QRegExp("\\{bitrate\\}"), QString("%1").arg(bitrate));
    input.replace(QRegExp("\\{index\\}"), QString("%1").arg(index));
    input.replace(QRegExp("\\{length\\}"), lengthStr.sprintf("%2d:%02d", length/60, length % 60));
    input.replace(QRegExp("\\{lengthInSeconds\\}"), QString("%1").arg(length));
    input.replace(QRegExp("\\{newline\\}"), "\n");
    input.replace(QRegExp("\\{trackNr\\}"), trackNrStr);
    input.replace(QRegExp("\\{trackNr2Digit\\}"), trackNrStr.rightJustify(2,'0'));
    return input;
}

