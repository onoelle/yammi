/***************************************************************************
                          song.cpp  -  description
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

#include <id3/tag.h>

#include "song.h"
#include "mp3tag.h"
#include "mp3_layer.h"
#include "yammigui.h"



extern YammiGui* gYammiGui;

/** constructs a song object with the given parameters
 */
Song::Song(QString artist, QString title, QString album, QString filename, QString path, int length, int bitrate, MyDateTime addedTo, int year, QString comment, int trackNr, int genreNr)
{
	classified=false;
	tagsDirty=false;
	filenameDirty=false;
	this->artist=artist;
	this->title=title;
	this->album=album;
	this->filename=filename;
	this->path=path;
	this->length=length;
	this->bitrate=bitrate;
	this->addedTo=addedTo;
	this->comment=comment;
	this->trackNr=trackNr;
	this->genreNr=genreNr;
	this->year=year;
	this->lastPlayed.setDate(QDate(1900,1,1));
	this->lastPlayed.setTime(QTime(0,0,0));

	
	this->artistSure=true;
	this->titleSure=true;
	this->corrupted=false;
}



/** construct a new song object from a given filename
 */
Song::Song(const QString location, const QString mediaName)
{
	classified=false;
	tagsDirty=false;
	filenameDirty=false;
	corrupted=true;
	artistSure=true;
	titleSure=true;
	
	this->artist="";
	this->title="";
	this->album="";
	this->length=0;
	this->bitrate=-1;
	MyDateTime now=now.currentDateTime();
	this->addedTo=now;
	this->comment="";
	this->year=0;
	this->trackNr=0;
	this->genreNr=0;
	this->lastPlayed.setDate(QDate(1900,1,1));
	this->lastPlayed.setTime(QTime(0,0,0));

	QFileInfo* fi=new QFileInfo(location);
	QString saveFilename(fi->fileName());
	if (!fi->exists()) {
		cout << "trying to construct song, but file " << location << " does not exist!\n";
		return;
	}
	if(mediaName==0) {			// standard: song is on harddisk
		this->filename=fi->fileName();
		this->path=fi->dirPath(TRUE);
	}
	else {									// new: scanning removable media
		this->filename="";
		this->path="";
		QString mountPath=gYammiGui->getModel()->config.mediaDir;
		QString locationOnMedia=location;
		if(locationOnMedia.left(mountPath.length())!=mountPath)
			cout << "not on the mount path\n";
		locationOnMedia=locationOnMedia.right(locationOnMedia.length()-mountPath.length());
		cout << "mediaName: " << mediaName << ", locationOnMedia: " << locationOnMedia << "\n";
		addMediaLocation(mediaName, locationOnMedia);
	}
	delete fi;
	
	// open file
	QFile f(location);
	if(!f.open(IO_ReadOnly)) {
		cout << "error opening file " << location << "\n";
		return;
	}
	
	// get mp3layer info
	MP3Layer layer;
	if(!layer.scan(&f, location)) {
		cout << "could not get mp3 layer info from file " << location << "\n";
		f.close();
		return;
	}
	
	this->bitrate=layer.bitrate();
	this->filesize=layer.getFileSize();
	this->length=layer.length();
	//this->stereo=layer.stereo();				// we're not interested so far

	
	// guess artist/title from filename (if no id3 tags or in case of overlength)
	QString ffArtist, ffTitle;
	QString guessBase=saveFilename.left(saveFilename.length()-4);					// remove .mp3 suffix
	guessBase=guessBase.replace( QRegExp("_"), " " );												// replace "_" with " "
	
	int pos=guessBase.find('-');
	if(pos!=-1) {
		ffArtist=guessBase.left(pos);
		ffArtist=ffArtist.simplifyWhiteSpace();
		ffTitle=guessBase.right(guessBase.length()-pos-1);
		ffTitle=ffTitle.simplifyWhiteSpace();
	}
	else
	{
		ffArtist="unknown";
		ffTitle=guessBase;
		ffTitle=ffTitle.simplifyWhiteSpace();
	}

			
	// get ID3 tags
	MP3Tag tag;
	if(!tag.scan(&f)) {										// no id3 tags => guess values
		cout << "could not get mp3 tags from file " << location << ", try guessing values...\n";
		if(ffArtist=="unknown" || ffTitle.find('-')!=-1) {
			cout << "WARNING: not exactly one ""-"" separator... don't know whether my guess will be right...\n";
			artistSure=false;
			titleSure=false;
		}
		this->artist=ffArtist;
		this->title=ffTitle;
	}
	else																					// id3 tags existing
	{
		this->artist=tag.artist;
		this->artist=this->artist.simplifyWhiteSpace();
		this->title=tag.title;
		this->title=this->title.simplifyWhiteSpace();
		this->album=tag.album;
		this->album=this->album.simplifyWhiteSpace();
		this->comment=tag.comment;
		this->comment=this->comment.simplifyWhiteSpace();
		this->trackNr=tag.trackNr;
		this->genreNr=tag.gennum;
		QString strYear;
		strYear=tag.year;
		sscanf(strYear, "%d", &(this->year));
		
		// check whether artist/title exceeding 30 characters of id3 tags
		if(ffArtist.upper()!=this->artist.upper()) {
			if(ffArtist.length()>30 && ffArtist.left(29)==this->artist.left(29)) {
				cout << "artist exceeding 30 characters, taking full artist from filename\n";
				this->artist=ffArtist;
			}
			else
				artistSure=false;
		}
		if(ffTitle.upper()!=this->title.upper()) {
			if(ffTitle.length()>30 && ffTitle.left(29)==this->title.left(29)) {
				cout << "title exceeding 30 characters, taking full title from filename\n";
				this->title=ffTitle;
			}
			else
				titleSure=false;
		}
		
		// in case the id3 tags are empty => trust filename info
		if(this->title=="" && this->artist=="") {
			title=ffTitle;
			artist=ffArtist;
		}
	}
	f.close();
	
 	// remove trailing mp3 in title
	if(title.right(4).upper()==".MP3")
		title=title.left(title.length()-4);
			
	// simplify whitespaces
	artist=artist.simplifyWhiteSpace();
	title=title.simplifyWhiteSpace();
	
	// capitalize after spaces (any exceptions?)
	album=capitalize(album);
	artist=capitalize(artist);
	title=capitalize(title);
	
	corrupted=false;
	checkConsistency(gYammiGui->getModel()->config.tagsConsistent, gYammiGui->getModel()->config.filenamesConsistent);
}
	
// check filename (if given)
bool Song::checkFilename()
{
	if(filename=="")
 		return true;
	else
		return (constructFilename()==filename);
}

/// check id3 tags (if songfile available)
/// true: tags correctly set
/// false: differences between database info and file
bool Song::checkTags()
{
	if(filename=="")		// song not on local harddisk => we can't check
		return true;
	// get id3 tags
	QString location=this->path+"/"+this->filename;
	QFile f(location);
	if (! f.open(IO_ReadOnly) ) {
		cout << "ERROR: error opening file " << location << "\n";
		return false;
	}
	bool same=false;

  // id3lib (experimental => disabled)
  //**********************
/*  ID3_Tag myTag;
  myTag.Link(this->location());

  ID3_Frame* myFrame = myTag.Find(ID3FID_TITLE);
  if (NULL != myFrame)
  {
    ID3_Field* myField = myFrame->GetField(ID3FN_TEXT);
    if (NULL != myField)
    {
      // do something with myField
      // for ascii strings
      char title[1024];
      myField->Get(title, 1024); // copies up to 1024 bytes of the field data into str1
      cout << "title tag found: " << title << "|\n";
    }
    else {
      cout << "could not find field\n";
    }
  }
  else {
    cout << "could not find frame\n";
  }
*/


	MP3Tag tag;
	if(tag.scan(&f)) {										// tags exist => compare to our fields
		same=true;
		// beware that artist/title can exceed 30 characters (by extra info in filename)
		if(strcmp(this->artist.left(30).stripWhiteSpace().latin1(),  tag.artist) !=0) {same=false; cout << "(artist)"; }
		if(strcmp(this->title.left(30).stripWhiteSpace().latin1(),   tag.title)  !=0) {same=false; cout << "(title)"; }
    cout << "old title tag: " << tag.title << "|\n";
		if(strcmp(this->album.latin1(),   tag.album)  !=0)
		  {same=false; cout << "(album)"; }
		// häh??? why does application of left (on an empty string) change the string???
		if(strcmp(this->comment.latin1(), tag.comment)!=0) {same=false;cout << "(comment)"; }
		if(this->trackNr!=0 && this->trackNr!=tag.trackNr) {same=false; cout << "(trackNr)"; }
		if(this->genreNr!=tag.gennum)  {same=false; cout << "(genreNr)"; }
		QString strYear;
		int intYear;
		strYear=tag.year;
		sscanf(strYear, "%d", &intYear);
		if(this->year!=0 && this->year!=intYear) {same=false; cout << "(year)"; }
	}
	f.close();
	return same;
}

/// save id3 tags to file
bool Song::saveTags()
{
	if(filename=="") {
		tagsDirty=false;
		return true;
	}
	QString filename=location();
	QFileInfo fi(filename);
	if(!fi.isWritable()) {
		cout << "wirting tags: file " << filename << " not writable, skipping\n";
		tagsDirty=false;
		return true;
	}
	
	// set ID3 tags according to info in song
	MP3Tag tag;
	tag.spacecopy(tag.artist, this->artist.latin1(), 30);
	tag.spacecopy(tag.title, this->title.latin1(), 30);
	tag.spacecopy(tag.album, this->album.latin1(), 30);
	QCString tmpYear;
	if(this->year!=-1) {
		tmpYear.sprintf("%i", this->year);
	}
	else {
		tmpYear="";
	}
	tag.spacecopy(tag.year, tmpYear.data(), 4);
	tag.spacecopy(tag.comment, this->comment.latin1(), 30);
	tag.trackNr=this->trackNr;
	tag.gennum=this->genreNr;
	
	// open file
	QString location=this->path+"/"+this->filename;
	QFile f(location);
	if (! f.open(IO_ReadWrite) ) {
		cout << "ERROR: error opening file " << location << "\n";
		return false;
	}
	bool success=tag.saveTags(&f);
	f.close();
	if(!success) {
		cout << "ERROR: could not save mp3 tags to file " << location << "\n";
		return false;
	}
	cout << "id3 tags corrected in file " << this->filename << "\n";
	tagsDirty=false;
	return true;
}

/* correct filename to convention (ie. "artist - title")
 */
bool Song::saveFilename()
{
	if(filename=="") {
		filenameDirty=false;
		return true;
	}
	QString oldname=location();
	QFileInfo fi0(oldname);
	if(!fi0.isWritable()) {
		cout << "renaming file: file " << oldname << " not writable, skipping\n";
		filenameDirty=false;
		return true;
	}
	QString newFilename=constructFilename();
	QString newname=QString("%3/%4").arg(this->path).arg(newFilename);

	// we first check whether we can create a file with that name (overwriting anything?)
	QFileInfo fi1(newname);
	if(fi1.exists()) {
		cout << "WARNING: renaming: new Filename already existing: " << newFilename << "\n";
		return false;			// return if we don't want to overwrite anything
//		cout << "overwriting old file...\n";
	}
	
	QFile touchFile(newname);
	if(!touchFile.open(IO_WriteOnly)) {
		cout << "ERROR renaming: could not touch file\n";
		return false;
	}
	QString dummy="test";
	touchFile.writeBlock(dummy, dummy.length());
	touchFile.close();

	QFileInfo fi(newname);
	if(!fi.exists()) {
		cout << "ERROR renaming: Filename not allowed: " << newFilename << "\n";
		return false;
	}
	// okay, successful
	QDir currentDir=QDir("/");
	
	// bug in windows-filesystem?
	// renaming fails if new and old are just different in case
	if(newname.upper()==oldname.upper()) {
		currentDir.rename(oldname, oldname+".xxx");
		oldname=oldname+".xxx";
	}
		
	if(!currentDir.rename(oldname, newname)) {
		cout << "ERROR: renaming from " << oldname << " to " << newname << "failed!\n";
		return false;
	}
	this->filename=newFilename;
	cout << "filename corrected to " << newFilename << "\n";
	filenameDirty=false;
	return true;
}


/** returns true if primary key (artist, title, album) is the same
 */
bool Song::sameAs(Song* s)
{
	return(artist==s->artist && title==s->title && album==s->album);
}

bool Song::sameAs(QString _artist, QString _title, QString _album)
{
	return(artist==_artist && title==_title && album==_album);
}

// sets value of this to new values of s
// does not affect: addedTo, lastPlayed and noPlayed!
void Song::setTo(Song* s)
{
	this->album=s->album;
	this->artist=s->artist;
	this->bitrate=s->bitrate;
	this->comment=s->comment;
	this->filename=s->filename;
	this->length=s->length;
	this->path=s->path;
	this->title=s->title;
	this->year=s->year;
	this->artistSure=s->artistSure;
	this->titleSure=s->titleSure;
	this->trackNr=s->trackNr;
	this->genreNr=s->genreNr;
}

/** check songfile (exists and readable)
 * @returns true, if songfile is accessible
 */
bool Song::checkReadability()
{
	if(filename=="")
		return false;
	QFileInfo fileInfo(location());
 	if(!fileInfo.exists() || !fileInfo.isReadable()) {
		cout << "file " << location() << " does not exist or unreadable\n";
 		return false;
 	}
 	return true;
}
	


/** check consistency of song
 * - file exists and readable
 * - id3 tags
 * - consistency with filename
 * @returns "" on wish or song okay
 * diagnosis string on song unreadable, tags dirty, filename dirty
 */
QString Song::checkConsistency(bool requireConsistentTags, bool requireConsistentFilename)
{
	//	if(location()="")
	//		return true; ??
	QString diagnosis="";
	
	if(artist=="{wish}")							// ignore wishes... 	
		return diagnosis;
	if(filename=="")									// ...and songs not on harddisk
		return diagnosis;
 	
	if(!checkReadability()) {
		diagnosis+="song file not readable. ";
		return diagnosis;
	}
	
	
	tagsDirty=false;
	if(requireConsistentTags) {
		// checking tags
	 	if(!checkTags()) {
			cout << "tags on file " << this->filename << " are not set correctly...\n";
			tagsDirty=true;
			diagnosis+="tags not set correctly. ";
		}
	}
	
	filenameDirty=false;
	if(requireConsistentFilename && filename!="") {
		// checking filename
		if(!checkFilename()) {
			cout << "file " << this->filename << " does not have correct filename\n";
			filenameDirty=true;
			diagnosis+="filename not consistent. ";
		}
	}
	
	return diagnosis;
}


QString Song::capitalize(QString str)
{
	// do nothing with the empty string
	if(str=="")
		return str;
	str.at(0)=str.at(0).upper();
	for(unsigned int pos=1; pos<str.length(); pos++) {
		if(str.at(pos-1)==' ')
			str.at(pos)=str.at(pos).upper();
	}
	return str;
}

	
/**
 * Constructs a filename following the "artist - title.mp3" pattern.
 * Should take care of special characters not allowed in filenames.
 */
QString Song::constructFilename()
{
	QString s=this->artist+" - "+this->title+".mp3";
//	QCString s2=QFile::encodeName(s);
//	QString s3=QFile::decodeName(s2);
//	cout << "raw name: " << s << ", encodedName: " << s2 << ", decodedName: " << s3 << "\n";
	
	// replace all forbidden characters for filenames with nothing
	// for windows and linux filesystems!
	s.replace(QRegExp("\""), "");													// "
	s.replace(QRegExp("'"), "");													// '
	s.replace(QRegExp("/"), "");													// /
	s.replace(QRegExp("&"), "");													// &
	s.replace(QRegExp("[?]"), "");													// ?
	s.replace(QRegExp(":"), "");													// :

	s.replace(QRegExp("ü"), "ue");													// umlaute
	s.replace(QRegExp("Ü"), "Ue");													//
	s.replace(QRegExp("ö"), "oe");													//
	s.replace(QRegExp("Ö"), "Oe");													//
	s.replace(QRegExp("ä"), "ae");													//
	s.replace(QRegExp("Ä"), "Ae");													//

//	cout << "replacedName: " << s << "\n";
	return s;
}


void Song::addMediaLocation(QString mediaName, QString locationOnMedia)
{
	this->mediaName.append(mediaName);
	this->mediaLocation.append(locationOnMedia);
}

void Song::renameMedia(QString oldMediaName, QString newMediaName)
{
	for(unsigned int i=0; i<mediaName.count(); i++) {
		if(mediaName[i]==oldMediaName)
			mediaName[i]=newMediaName;
	}
}

void Song::deleteFile(QString trashDir)						// move songfile to trash
{
	if(filename=="")
		return;
	moveTo(trashDir);
	filename="";
	path="";
}
		
void Song::copyTo(QString dir)						// copy songfile to other location
{
	if(filename=="")
		return;
	cout << "copying song " << displayName() << "\n";
//mmm	mainStatusBar->message(QString("copying song %1").arg(s->displayName()), 2000);
	QString cmd= QString("cp \"%1\" \"%3/%4\"").arg(location()).arg(dir).arg(filename);
	system(cmd.data());
//mmm	mainStatusBar->message(QString("song %1 copied").arg(s->displayName()), 2000);
}

void Song::copyAsWavTo(QString dir)
{
	if(filename=="")
		return;
	// copy song as wav to somewhere (think of padding before burning as audio track!)
	QString outName=filename;
	outName=outName.left(outName.length()-3) + "wav";
	QString cmd= QString("mpg123 -w \"%1/%2\" \"%3\"").arg(dir).arg(outName).arg(location());
	system(cmd.data());
}

// move file to another directory	
void Song::moveTo(QString dir)
{
	if(filename=="")
		return;
	QString newname=QString("%3/%4").arg(dir).arg(filename);
	QDir currentDir("/");
	if(!currentDir.rename(location(), newname)) {
		cout << "renaming failed! song: " << displayName() << "\n";
	}
	else {
		path=dir;
	}
}
