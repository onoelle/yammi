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
#include "yammigui.h"
#include "prefs.h"

#include "CMP3Info.h"

#ifdef ENABLE_OGGLIBS
#include "vcedit.h"
#include "glib.h"
#endif

using namespace std;

extern YammiGui* gYammiGui;

/**
 * creates a song object with default values
 */
Song::Song()
{
	classified = false;
	tagsDirty = false;
	filenameDirty = false;
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
	this->trackNr = 0;
	this->genreNr = 0;
	this->lastPlayed.setDate(QDate(1900,1,1));
	this->lastPlayed.setTime(QTime(0,0,0));
}

/** constructs a song object with the given parameters
 */
Song::Song(QString artist, QString title, QString album, QString filename, QString path, int length, int bitrate, MyDateTime addedTo, int year, QString comment, int trackNr, int genreNr)
{
	classified = false;
	tagsDirty = false;
	filenameDirty = false;
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
	this->genreNr = genreNr;
	this->year = year;
}



/**
 * Try to create a new song object from a given filename.
 * \return    0 if successful
 *            1 on error (file not found)
 */
int Song::create(const QString location, const QString mediaName, bool capitalizeTags)
{
  // step 1:
  // - check for existence and access
  // - get filename, path and size

  QFileInfo* fi=new QFileInfo(location);
	if(!fi->exists()) {
		cout << "trying to construct song, but file " << location << " does not exist!\n";
		return 1;
	}
  if(!fi->isReadable()) {
		cout << "trying to construct song, but file " << location << " is not accessible!\n";
		return 1;
  }
	if(mediaName==0) {			// song is on harddisk
		this->filename=fi->fileName();
		this->path=fi->dirPath(TRUE);
	}
	else {									// scanning removable media
		this->filename="";
		this->path="";
		QString mountPath=gYammiGui->getModel()->config.mediaDir;
		QString locationOnMedia=location;
		if(locationOnMedia.left(mountPath.length())!=mountPath)
			cout << "warning: song file is not on the mount path\n";
		locationOnMedia=locationOnMedia.right(locationOnMedia.length()-mountPath.length());
		cout << "mediaName: " << mediaName << ", locationOnMedia: " << locationOnMedia << "\n";
		addMediaLocation(mediaName, locationOnMedia);
	}
  this->filesize=fi->size();
	QString saveFilename(fi->fileName());
	QString savePath(fi->dirPath(TRUE));
	delete fi;




  // step 2:
  // get info about song object (bitrate, length, tags, ...)

  // guess artist/title from filename (in case no tags can be read)
	QString ffArtist, ffTitle, ffAlbum;
  guessTagsFromFilename(saveFilename, savePath, &ffArtist, &ffTitle, &ffAlbum);
  bool treated=false;


  // mp3 object
  if(location.right(4).upper()==".MP3") {
    // get mp3 layer info
    if(!getMp3LayerInfo(location)) {
      cout << "could not read layer information from mp3 file \"" << location << "\"\n";
    }

#ifdef ENABLE_ID3LIB
    // get id3 tags
    if(getMp3Tags(location)) {

      // now perform some consistency checks on the read tags...

      // check whether artist/title exceeding 30 characters of id3 tags
      // (due to the fact that upto id3 v1.1, the length was restricted to 30 characters)
      // if so, we might be able to retrieve the complete artist/title from the filename
      if(ffArtist.upper()!=this->artist.upper()) {
        if(ffArtist.length()>30 && ffArtist.left(29)==this->artist.left(29) && artist.length()<=30) {
          cout << "artist exceeding 30 characters, taking full artist from filename\n";
          artist=ffArtist;
        }
      }
      if(ffTitle.upper()!=this->title.upper()) {
        if(ffTitle.length()>30 && ffTitle.left(29)==this->title.left(29) && title.length()<=30) {
          cout << "title exceeding 30 characters, taking full title from filename\n";
          title=ffTitle;
        }
      }
      // in case the id3 tags are empty => better trust filename info
      if(title=="" && artist=="") {
        title=ffTitle;
        artist=ffArtist;
      }      
    }
    else {
      cout << "could not read tag information from mp3 file \"" << location << "\", guessing values from filename\n";
			title=ffTitle;
			artist=ffArtist;
      album=ffAlbum;
    }

    // just in case: remove trailing mp3 in title
    if(title.right(4).upper()==".MP3")
      title=title.left(title.length()-4);


#else
    // we have no id3lib support => we have to get info from filename
    title=ffTitle;
		artist=ffArtist;
    album=ffAlbum;
#endif // ENABLE_ID3LIB

    treated=true;
  }


  if(location.right(4).upper()==".TST") {
    title=ffTitle;
		artist=ffArtist;
    album=ffAlbum;
    treated=true;
  }
  
  // ogg object
  if(location.right(4).upper()==".OGG") {
    // get ogg info

#ifdef ENABLE_OGGLIBS

    if(!getOggInfo(location)) {
      cout << "could not read tag information from ogg file \"" << location << "\"\n";
    }

		// in case the ogg tags are empty => better trust filename info
		if(title=="" && artist=="") {
      cout << "ogg tags empty, taking guessed info from filename...\n";
			title=ffTitle;
			artist=ffArtist;
      album=ffAlbum;
		}
    // just in case: remove trailing ogg in title
    if(title.right(4).upper()==".OGG") {
      title=title.left(title.length()-4);
    }
#else
    artist=ffArtist;
    title=ffTitle;
    album=ffAlbum;
#endif // ENABLE_OGGLIBS

    treated=true;
  }



  // wav object

  if(location.right(4).upper()==".WAV") {
    char loc[1000];
    strcpy(loc, location);
    if(!getWavInfo(loc)) {
      cout << "could not read wav header information from wav file \"" << location << "\"\n";
    }
    artist=ffArtist;
    title=ffTitle;
    album=ffAlbum;
    treated=true;
  }


  if(!treated) {
    cout << location << ": unknown suffix, no special handling available...\n";
    cout << "  => cannot read information such as bitrate, length and tags\n";
    bitrate=0;
    length=0;
    artist="";
    album="";
    title=saveFilename;
  }


	// simplify whitespaces
	artist=artist.simplifyWhiteSpace();
	title=title.simplifyWhiteSpace();

  if(treated && capitalizeTags) {
    // capitalize after spaces (any exceptions?)
    album=capitalize(album);
    artist=capitalize(artist);
    title=capitalize(title);
  }
  
	corrupted=false;
  return 0;
}


/**
 * Re-reads the tags from filename,
 * overwriting the info in yammi's database.
 * (only has an effect for mp3 or ogg files so far)
 * Returns true, if tags have been read.
 */
bool Song::rereadTags()
{
  if(location().right(4).upper()==".MP3") {
#ifdef ENABLE_ID3LIB
    if(!getMp3Tags(location())) {
      cout << "could not read tag information from mp3 file \"" << location() << "\"\n";
    }
    else {
		  tagsDirty=true;
      return true;
    }
#endif // ENABLE_ID3LIB
  }

  if(location().right(4).upper()==".OGG") {
#ifdef ENABLE_OGGLIBS
    if(!getOggInfo(location())) {
      cout << "could not read tag information from ogg file \"" << location() << "\"\n";
    }
    else {
		  tagsDirty=true;
      return true;
    }
#endif // ENABLE_OGGLIBS
  }
  return false;
}



/**
 * check filename (if given)
 * returns true if it is consistent with the configured pattern
 */
bool Song::checkFilename(bool ignoreCase)
{
	if(filename=="") {
    return true;
  }
	else {
    if(ignoreCase) {
      return ((constructFilename().upper())==(filename.upper()));
    }
    else {
      return (constructFilename()==filename);
    }
  }
}

/**
 * check directory (if given)
 * returns true if it is consistent with the configured pattern
 */
bool Song::checkDirectory(bool ignoreCase)
{
	if(path=="") {
    return true;
  }
	else {
    if(ignoreCase) {
      return ((constructPath().upper())==(path.upper()));
    }
    else {
      return (constructPath()==path);
    }
  }
}


/** Tries to guess artist and title from filename.
 * So far, only a simple and an advanced pattern available for guessing
 * \todo: add more sophisticated pattern (leading trackNr?, directory with album?)
 * (or add a configurable pattern description)
 */
void Song::guessTagsFromFilename(QString filename, QString path, QString* artist, QString* title, QString* album)
{
  *album="";
  QString guessBase=filename;
  // remove suffix, if it looks like we have a suffix
  if(guessBase.at(guessBase.length()-4)=='.')
    guessBase=guessBase.left(guessBase.length()-4);
  guessBase=guessBase.replace( QRegExp("_"), " " );							// replace "_" with " "


  if(gYammiGui->getModel()->config.guessingMode==0) { //gYammiGui->getModel()->config.GUESSING_MODE_SIMPLE) {

    int pos=guessBase.find('-');
    if(pos!=-1) {
      *artist=guessBase.left(pos);
      *artist=artist->simplifyWhiteSpace();
      *title=guessBase.right(guessBase.length()-pos-1);
      *title=title->simplifyWhiteSpace();
    }
    else
    {
      *artist="unknown";
      *title=guessBase;
      *title=title->simplifyWhiteSpace();
    }
  }

  if(gYammiGui->getModel()->config.guessingMode==1) { //gYammiGui->getModel()->config.GUESSING_MODE_ADVANCED) {
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
    }
    else
    {
      *title=guessBase;
    }
  }
}




////////////////////////////////////
// special handling of mp3 files

#ifdef ENABLE_ID3LIB

/** get id3 tags from the specified mp3 file (using id3lib)
 */
bool Song::getMp3Tags(QString filename)
{
  ID3_Tag tag;
  tag.Link(filename, ID3TT_ALL);

  // experimental mp3 layer info retrieval
  // disabled => cannot handle VBR and is not documented in id3lib... (maybe later?)
  /*
  const Mp3_Headerinfo* mp3info;
  mp3info = tag.GetMp3HeaderInfo();
  switch (mp3info->version) {
    case MPEGVERSION_2_5:
    cout << "MPEG2.5/";
    break;
  case MPEGVERSION_2:
    cout << "MPEG2/";
    break;
  case MPEGVERSION_1:
    cout << "MPEG1/";
    break;
  default:
    cout << "unknown MPEGVERSION/";
    break;
  }

  switch (mp3info->layer)
  {
  case MPEGLAYER_III:
    cout << "layer III\n";
    break;
  case MPEGLAYER_II:
    cout << "layer II\n";
    break;
  case MPEGLAYER_I:
    cout << "layer I\n";
    break;
  default:
    cout << "unknown layer\n";
    break;
  }
  cout << "Bitrate: " << mp3info->bitrate/1000 << "KBps\n";
  cout << "Frequency: " << mp3info->frequency/1000 << "KHz\n";

  // end experimental
  */

  bool foundSomething=false;
  QString str;

  // title
  this->title="";
  if(getId3Tag(&tag, ID3FID_TITLE, ID3FN_TEXT, &str)) {
    this->title=str;
    foundSomething=true;
  }

  // artist
  this->artist="";
  if(getId3Tag(&tag, ID3FID_LEADARTIST, ID3FN_TEXT, &str)) {
    this->artist=str;
    foundSomething=true;
  }

  // album
  this->album="";
  if(getId3Tag(&tag, ID3FID_ALBUM, ID3FN_TEXT, &str)) {
    this->album=str;
    foundSomething=true;
  }

  // year
  this->year=0;
  if(getId3Tag(&tag, ID3FID_YEAR, ID3FN_TEXT, &str)) {
		sscanf(str, "%d", &(this->year));
    foundSomething=true;
  }

  // comment
  this->comment="";
  if(getId3Tag(&tag, ID3FID_COMMENT, ID3FN_TEXT, &str)) {
    this->comment=str;
    foundSomething=true;
  }

  // tracknum
  this->trackNr=0;
  if(getId3Tag(&tag, ID3FID_TRACKNUM, ID3FN_TEXT, &str)) {
		sscanf(str, "%d", &(this->trackNr));
    foundSomething=true;
  }

  // genre
  this->genreNr=-1;
  if(getId3Tag(&tag, ID3FID_CONTENTTYPE, ID3FN_TEXT, &str)) {
//    cout << "genre found: " << str << "\n";
    if(str.left(1)=="(") {
      // pattern: "(<genreNr>)[genreDesc]"
      int pos=str.find(')');
      if(pos==-1) {
        // genre broken?
        this->genreNr=CMP3Info::getGenreIndex(str);        
      }
      else {
        QString genreNrStr=str.mid(1, pos-1);
//        QString genreDesc=str.mid(pos+1);
//        TODO: support non-standard genres (change datatype from int to string...)
//      cout << "genreNr: " << genreNr << ", genreDesc: " << genreDesc << "\n";
        sscanf(genreNrStr, "%d", &(this->genreNr));
      }
    }
    else {
      // pattern: just a string, no brackets
      this->genreNr=CMP3Info::getGenreIndex(str);
      if(this->genreNr==-1) {
        // description did not match, maybe it's just a number without brackets?
        sscanf(str, "%d", &(this->genreNr));
      }
//      cout << "genreDesc: " << str << ", index: " << this->genreNr << "\n";
    }
    foundSomething=true;
  }

  if(foundSomething)
    return true;
  else
    return false;
}


/** accesses a particular frame content using id3lib.
 */
bool Song::getId3Tag(ID3_Tag* tag, ID3_FrameID frame, ID3_FieldID field, QString* content)
{
  // get frame
  ID3_Frame* theFrame = tag->Find(frame);
  if (theFrame == NULL) {
    return false;
  }

  // get field
// apparently the following line only works with id3lib3.8.x
//  ID3_Field* theField = theFrame->GetField(field);
  ID3_Field* theField = &(theFrame->Field(field));
  if (theField == NULL) {
    return false;
  }

  // convert to ascii string
  char theContent[1024];
  theField->Get(theContent, 1024); // copies up to 1024 bytes of the field data into char array
  *content=theContent;
  return true;
}



/** save id3 tags to the specified mp3 file (using id3lib)
 */
bool Song::setMp3Tags(QString filename)
{
  ID3_Tag tag;
  tag.Link(filename, ID3TT_ALL);

  // title
  if(title!="") {
    ID3_Frame titleFrame;
    if(!setId3Tag(&tag, ID3FID_TITLE, ID3FN_TEXT, this->title, &titleFrame))
      cout << "could not set tag (title)\n";
  }

  // artist
  if(artist!="") {
    ID3_Frame artistFrame;
    if(!setId3Tag(&tag, ID3FID_LEADARTIST, ID3FN_TEXT, this->artist, &artistFrame))
      cout << "could not set tag (title)\n";
  }

  // album
  if(album!="") {
    ID3_Frame albumFrame;
    if(!setId3Tag(&tag, ID3FID_ALBUM, ID3FN_TEXT, this->album, &albumFrame))
      cout << "could not set tag (title)\n";
  }

  // comment
  if(comment!="") {
    ID3_Frame commentFrame;
    if(!setId3Tag(&tag, ID3FID_COMMENT, ID3FN_TEXT, this->comment, &commentFrame))
      cout << "could not set tag (title)\n";
  }

  // year
  if(year!=0) {
    ID3_Frame yearFrame;
    if(!setId3Tag(&tag, ID3FID_YEAR, ID3FN_TEXT, QString("%1").arg(this->year), &yearFrame))
      cout << "could not set tag (year)\n";
  }

  // trackNr
  if(trackNr!=0) {
    ID3_Frame trackNrFrame;
    if(!setId3Tag(&tag, ID3FID_TRACKNUM, ID3FN_TEXT, QString("%1").arg(this->trackNr), &trackNrFrame))
      cout << "could not set tag (year)\n";
  }

  // genreNr
  if(genreNr!=-1) {
    ID3_Frame genreNrFrame;
    if(!setId3Tag(&tag, ID3FID_CONTENTTYPE, ID3FN_TEXT, QString("(%1)").arg(this->genreNr), &genreNrFrame))
      cout << "could not set tag (year)\n";
  }

  tag.Update();
  return true;
}




/** sets a particular frame content using id3lib.
 */
bool Song::setId3Tag(ID3_Tag* tag, ID3_FrameID frame, ID3_FieldID field, QString content, ID3_Frame* newFrame)
{
  // get frame
  ID3_Frame* theFrame = tag->Find(frame);
  if (theFrame == NULL) {
    cout << "could not find frame " << frame << ", creating it...\n";
    newFrame->SetID(frame);

// apparently the following line only works with id3lib3.8.x
//    newFrame->GetField(field)->Set(content.latin1());
    newFrame->Field(field).Set(content.latin1());
    tag->AddFrame(newFrame);
    return true;
  }

  // get field
// apparently the following line only works with id3lib3.8.x
//  ID3_Field* theField = theFrame->GetField(field);
  ID3_Field* theField = &(theFrame->Field(field));
  if (theField == NULL) {
    cout << "could not find field " << field << "\n";
    return false;
  }

//  cout << "trying to set field to: " << content.latin1() << "\n";
  theField->Set(content.latin1());
  return true;
}


#endif // ENABLE_ID3LIB

/** Gets the mp3 layer info (ie. for our purpose: bitrate and length) from the file.
 * For VBR songs: retrieves the average bitrate.
 */
bool Song::getMp3LayerInfo(QString filename)
{
  CMP3Info* mp3Info = new CMP3Info();
  char _filename[1000];
  strcpy(_filename, filename);
  int loadstate = mp3Info->loadInfo(_filename);
  if (loadstate!=0) {
    cout << "error on reading mp3 layer info: " << loadstate << "\n";
    return false;
  }
  this->bitrate=mp3Info->getBitrate();
  this->length=mp3Info->getLengthInSeconds();
//  cout << "frequency: " << mp3Info->getFrequency() << " Hz\n";
  delete mp3Info;
  return true;
}



// end of special handling of mp3 files
///////////////////////////////////////




////////////////////////////////
// special handling of ogg files

#ifdef ENABLE_OGGLIBS



/** Gets bitrate, length and tags from an ogg file (using libvorbis)
 * Thanks to Philip Scott! <scotty@philipscott.freeserve.co.uk>
 *
 * The following list is copied from the easytag project (http://easytag.sourceforge.net)
 *
 * OGG fields names :
 *  - TITLE        : Track name
 *  - VERSION      : The version field may be used to differentiate multiple version of the same track title in a single collection. (e.g. remix info)
 *  - ALBUM        : The collection name to which this track belongs
 *  - TRACKNUMBER  : The track number of this piece if part of a specific larger collection or album
 *  - ARTIST       : Track performer
 *  - ORGANIZATION : Name of the organization producing the track (i.e. the 'record label')
 *  - DESCRIPTION  : A short text description of the contents
 *  - GENRE        : A short text indication of music genre
 *  - DATE         : Date the track was recorded
 *  - LOCATION     : Location where track was recorded
 *  - COPYRIGHT    : Copyright information
 *  - ISRC         : ISRC number for the track; see the ISRC intro page for more information on ISRC numbers.
 *
 * Field names should not be 'internationalized'; this is a concession to simplicity
 * not an attempt to exclude the majority of the world that doesn't speak English.
 * Field *contents*, however, are represented in UTF-8 to allow easy representation
 * of any language.
 */
bool Song::getOggInfo(QString filename)
{
  OggVorbis_File oggfile;
	FILE* ourfile;

	ourfile=fopen(filename, "r");
  if(ourfile==0)
    return false;
	int succ=ov_open(ourfile, &oggfile, NULL, 0);
  if(succ!=0) {
    cout << "error in opening ogg file (" << filename << "), return value of ov_open: " << succ << "\n";
    fclose(ourfile);
  }

  title   = getOggComment(&oggfile, "title");
  artist  = getOggComment(&oggfile, "artist");
	album   = getOggComment(&oggfile, "album");
	comment = getOggComment(&oggfile, "comment");
  QString trackNrStr=getOggComment(&oggfile, "tracknumber");
  trackNr = atoi(trackNrStr);
  QString yearStr=getOggComment(&oggfile, "date");
  year    = atoi(yearStr);
  QString genreStr=getOggComment(&oggfile, "genre");
  if(genreStr!="") {
    genreNr=CMP3Info::getGenreIndex(genreStr);
    cout << "genre found, str: " << genreStr << ", index: " << genreNr << "\n";
  }

  length  = (int)ov_time_total(&oggfile, -1);
	bitrate = ov_bitrate(&oggfile, -1)/1000;

  succ=ov_clear(&oggfile);
  if(succ!=0)
    cout << "error when closing ogg file?\n";
  return true;
}


/** Gets a specific ogg comment.
 */
QString Song::getOggComment(OggVorbis_File* oggfile, QString commentName)
{
	vorbis_comment* ourComment = ov_comment(oggfile, -1);

	for(int i=0; i < (*ourComment).comments; i++)	{
		QString curstr((*ourComment).user_comments[i]);
		if( curstr.left(commentName.length()).upper() == commentName.upper()) {
			return curstr.right(curstr.length() - commentName.length() - 1);
		}
	}
  // nothing found => return empty string
	return "";
}

/**
 * Sets a single vorbis comment
 */
void Song::setOggComment(vorbis_comment* vc, QString key, QString value)
{
  QString qstr=QString("%1=%2").arg(key).arg(value);
  char* string=(char*) qstr.latin1();
  vorbis_comment_add(vc, string);
  // any useful return value?
}


/**
 * Sets all ogg tags.
 */
bool Song::setOggTags(QString filename)
{
  vcedit_state* state;
  state = vcedit_new_state();    // Allocate memory for 'state'

  // Test to know if we can write into the file
  FILE* file_in;
  if( (file_in=fopen(filename,"rb"))==NULL ) {
    cout << "ERROR (saving ogg tags) while opening file: " << filename << "\n";
    return false;
  }

  if( vcedit_open(state, file_in) < 0 ) {
    cout << "ERROR (saving ogg tags), failed to open file: " << filename << vcedit_error(state) << "\n";
    fclose(file_in);
    return false;
  }

  vorbis_comment* vc;
  // Get data from tag
  vc = vcedit_comments(state);
  vorbis_comment_clear(vc);
  vorbis_comment_init(vc);


  if( title!="" )       { setOggComment(vc, "title", title); }
  if( artist!="" )      { setOggComment(vc, "artist", artist); }
  if( album!="" )       { setOggComment(vc, "album", album); }
  if( comment!="" )     { setOggComment(vc, "comment", comment); }
  if( trackNr!=0 )      { setOggComment(vc, "tracknumber", QString("%1").arg(trackNr)); }
  if( this->year!=0 )   { setOggComment(vc, "date", QString("%1").arg(year)); }
  if( genreNr!=-1)      { setOggComment(vc, "genre", CMP3Info::getGenre(genreNr)); }

  // open temp file for writing to
  FILE* file_out;
  QString newFilename=filename+".new";
  if ( (file_out=fopen(newFilename,"w"))==NULL ) {
    cout << "ERROR (saving ogg tags) while opening file " << newFilename << "\n";
    return false;
  }

  int succ=vcedit_write(state, file_out);
  fclose(file_out);
  if(succ != 0) {
    cout << "ERROR writing new ogg-file " << newFilename << "\n";
  }
  else {
  	QDir dir;
		// linux specific
		QString cmd=QString("chmod --reference \"%1\" \"%2\"").arg(filename).arg(newFilename);
		system(cmd);
    // delete original file...
    dir.remove(filename);
    // ...and rename temp file to original filename
  	dir.rename(newFilename, filename);
  }
                                               
  return true;
}


#endif // ENABLE_OGGLIBS

// end of special handling of ogg files
//////////////////////////////////



////////////////////////////////
// special handling of wav files


bool Song::getWavInfo(QString filename)
{
  QFile wavFile(filename);
  if( wavFile.open( IO_ReadOnly ) ) {
    QDataStream stream( &wavFile );
    WaveHeader header;
    stream.readRawBytes((char*)&header, sizeof(WaveHeader));
    wavFile.close();
//    cout << "header data: nChannels: " << header.nChannels << ", wBitsPerSample: " << header.wBitsPerSample << "\n";
//    cout << "nSamplesPerSec: " << header.nSamplesPerSec << ", filesize: " << header.filesize << ", avgBytesPerSec: " << header.nAvgBytesPerSec << "\n";
//    cout << "formatChunkSize: " << header.formatChunkSize << ", dataChunkSize: " << header.dataChunkSize << "\n";
    
    if(header.nChannels==0 || header.nSamplesPerSec==0 || header.wBitsPerSample==0) {
      cout << "length calculation of file " << filename << " would have yielded division by zero, debug info:\n";
      cout << "header data: nChannels: " << header.nChannels << ", wBitsPerSample: " << header.wBitsPerSample << "\n";
      cout << "nSamplesPerSec: " << header.nSamplesPerSec << ", filesize: " << header.filesize << ", avgBytesPerSec: " << header.nAvgBytesPerSec << "\n";
      cout << "formatChunkSize: " << header.formatChunkSize << ", dataChunkSize: " << header.dataChunkSize << "\n";
      cout << "setting length to 1\n";
      length=1;
    }
    else {
      cout << "calculating length...\n";
      this->length=((((header.dataChunkSize * 8) / header.nChannels) / header.nSamplesPerSec) / header.wBitsPerSample);
      cout << "...and bitrate...\n";
      this->bitrate=header.nAvgBytesPerSec * 8 / 1000;
      cout << "...done!\n";
      this->comment=QString("%1, %2 KHz, %3 bit").arg(header.nChannels==2 ? "stereo" : "mono").arg(header.nSamplesPerSec).arg(header.wBitsPerSample);
    }
    return true;
  }
  else {
    cout << "Error in opening wav file for reading!\n";
    return false;
  }
}

// end of special handling of wav files
//////////////////////////////////




/**
 * check tags (if songfile available)
 * true: tags correctly set
 * false: differences between database info and tags in file
 */
bool Song::checkTags()
{
	if(filename=="")		// song not on local harddisk => we can't check
		return true;

	bool same=false;

  QString _album=this->album;
  QString _artist=this->artist;
  QString _comment=this->comment;
  QString _title=this->title;
  int _year=this->year;
  int _trackNr=this->trackNr;
  int _genreNr=this->genreNr;

  bool treated=false;

#ifdef ENABLE_ID3LIB
  if(filename.right(4).upper()==".MP3") {
    if(!this->getMp3Tags(location()))
      return false;
    treated=true;
  }
#endif

#ifdef ENABLE_OGGLIBS
  if(filename.right(4).upper()==".OGG") {
    if(!this->getOggInfo(location()))
      return false;
    treated=true;
  }
#endif

  if(!treated) {
    // we can't read any tags, so we can't check them... => return true
    return true;
  }

  // tags exist => compare to our fields
  same=true;
  if(_album     != this->album)    {same=false; cout << "(album)"; }
  if(_artist    != this->artist)   {same=false; cout << "(artist)"; }
  if(_comment   != this->comment)  {same=false; cout << "(comment)"; }
  if(_title     != this->title)    {same=false; cout << "(title)"; }
  if(_year      != this->year)     {same=false; cout << "(year)"; }
  if(_trackNr   != this->trackNr)  {same=false; cout << "(trackNr)"; }
  if(_genreNr   != this->genreNr)  {same=false; cout << "(genreNr)"; }

  if(same)            // no differences
    return true;

  this->album=_album;
  this->artist=_artist;
  this->comment=_comment;
  this->title=_title;
  this->year=_year;
  this->trackNr=_trackNr;
  this->genreNr=_genreNr;

  return false;
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
		cout << "writing tags: file " << filename << " not writable, skipping\n";
		tagsDirty=false;
		return true;
	}

  bool treated=false;

#ifdef ENABLE_ID3LIB
  if(filename.right(4).upper()==".MP3") {
    cout << "writing tags to file " << this->location() << "\n";
    setMp3Tags(location());
    cout << "id3 tags corrected in file " << this->filename << "\n";
    treated=true;
  }
#endif

#ifdef ENABLE_OGGLIBS
  if(filename.right(4).upper()==".OGG") {
    setOggTags(location());    
    cout << "ogg tags corrected in file " << this->filename << "\n";
    treated=true;
  }
#endif

  if(!treated) {
    cout << "cannot set tags on file " << this->filename << " (only possible for mp3 and ogg files)\n";    
    return true;
  }

	tagsDirty=false;
	return true;
}

/* correct filename to convention (ie. "artist - title")
 */
bool Song::correctFilename()
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
	QString newname=QString("%1/%2").arg(this->path).arg(newFilename);

  // bug in windows-filesystem?
	// renaming fails if new and old are just different in case
	QDir currentDir=QDir("/");
	if(newname.upper()==oldname.upper()) {
		if(!currentDir.rename(oldname, oldname+".xxx")) {
      cout << "WARNING: renaming: new filename equals old filename (except case), and renaming failed (" << newFilename << ")\n";
      return false;
    }
		oldname=oldname+".xxx";
	}

  // we first check whether we can create a file with that name (overwriting anything?)
	QFileInfo fi1(newname);
	if(fi1.exists()) {
		cout << "WARNING: renaming: new Filename already existing, aborting (" << newFilename << ")\n";
		return false;			// return if we don't want to overwrite anything
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
	
	if(!currentDir.rename(oldname, newname)) {
		cout << "ERROR: renaming from " << oldname << " to " << newname << "failed!\n";
		return false;
	}
	this->filename=newFilename;
	cout << "filename corrected to " << newFilename << "\n";
	filenameDirty=false;
	return true;
}


/**
 * Correct location of file
 */
bool Song::correctPath()
{
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
 * @returns "" on song okay (or wish or filename==""), or on error:
 * "file not readable", "tags not set correctly", "filename not consistent"
 */
QString Song::checkConsistency(bool requireConsistentTags, bool requireConsistentFilename, bool ignoreCaseInFilenames, bool requireConsistentDirectory)
{
	QString diagnosis="";
	
	if(artist=="{wish}")							// ignore wishes... 	
		return "";
	if(filename=="")									// ...and songs not on harddisk
		return "";
 	
	if(checkReadability()==false)
		return "file not readable";
	
	
	tagsDirty=false;
	if(requireConsistentTags) {
		// checking tags
	 	if(!checkTags()) {
			cout << "tags on file " << this->filename << " are not set correctly...\n";
			tagsDirty=true;
			diagnosis+="tags not correct ";
		}
	}
	
	filenameDirty=false;
	if(requireConsistentFilename && filename!="") {
		// checking filename
		if(!checkFilename(ignoreCaseInFilenames)) {
			cout << "file " << this->filename << " does not have correct filename\n";
			filenameDirty=true;
			diagnosis+="filename not consistent ";
		}
	}

  directoryDirty=false;
	if(requireConsistentDirectory && path!="") {
		// checking directory
		if(!checkDirectory(ignoreCaseInFilenames)) {
			cout << "file " << this->filename << " is not in correct directory\n";
			directoryDirty=true;
			diagnosis+="directory not consistent ";
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
 * Constructs a filename according to the configured filename pattern.
 * Takes care of special characters not allowed in filenames.
 */
QString Song::constructFilename()
{
  QString pattern=gYammiGui->getModel()->config.filenamePattern;
  QString filename=replacePlaceholders(pattern, 0);
  return makeValidFilename(filename, true);
}

/**
 * Constructs a path according to the configured directory pattern.
 * Takes care of special characters not allowed in filenames.
 */
QString Song::constructPath()
{
  QString pattern=gYammiGui->getModel()->config.directoryPattern;
  QString path=replacePlaceholders(pattern, 0);
  // now remove "empty navigation steps" such as "Madonna//Holiday.mp3" (because no album given)
  path.replace("//", "/");
  if(path.endsWith("/")) {
    path=path.left(path.length()-1);
  }
  return makeValidFilename(path, false);
}
	

/**
 * Returns the suffix of the filename of this song
 */
QString Song::getSuffix()
{
  QString base;
  if(filename=="") {
    // this is the case for swapped songs: filename is empty
    // => we have to look it up in the stored media location
    if(mediaLocation.count()==0) {
      cout << "warning: no suffix found, assuming \".mp3\"...\n";  // TODO
      return ".mp3";
    }
    else {
      base=mediaLocation[0];
    }
  }
  else {
    base=filename;
  }

  int suffixLength=base.findRev('.');
  if(suffixLength==-1) {
    return "";
  }
  QString suffix=base.mid(suffixLength+1);
  if(base==0) {
    return "";
  }
  return suffix;
}


/**
 * replace all forbidden characters for filenames with nothing
 * for windows and linux filesystems!
 * TODO: make configurable (depending on filesystem?)
 */
QString Song::makeValidFilename(QString filename, bool file)
{
  filename.replace(QRegExp("\""), "");												// "
	filename.replace(QRegExp("'"), "");													// '
  if(file) {
    filename.replace(QRegExp("/"), "");													// /
  }
	filename.replace(QRegExp("&"), "");													// &
	filename.replace(QRegExp("[?]"), "");												// ?
	filename.replace(QRegExp(":"), "");													// :

	filename.replace(QRegExp("ü"), "ue");													// umlaute
	filename.replace(QRegExp("Ü"), "Ue");													//
	filename.replace(QRegExp("ö"), "oe");													//
	filename.replace(QRegExp("Ö"), "Oe");													//
	filename.replace(QRegExp("ä"), "ae");													//
	filename.replace(QRegExp("Ä"), "Ae");													//
	return filename;
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
		

// move file to another directory	
void Song::moveTo(QString dir)
{
	if(filename=="") {
		return;
  }
  if(dir.right(1)=="/") {
    // strip trailing slash
    dir=dir.left(dir.length()-1);
  }
    
  QString newLocation=QString("%3/%4").arg(dir).arg(filename);
  qDebug("newLocation: %s", newLocation.latin1());
	QDir currentDir("/");
	if(!currentDir.rename(location(), newLocation)) {
		cout << "renaming failed! song: " << displayName() << ", new location: " << newLocation << "\n";
	}
	else {
		path=dir;
	}
}


QString Song::getSongAction(int index)
{
  const char* songAction[] = {"None", "Enqueue", "EnqueueRandom", "EnqueueAsNext", "EnqueueAsNextRandom", "PlayNow", "SongInfo",
							"PrelistenStart", "PrelistenMiddle", "PrelistenEnd",
							"Delete", "DeleteFile", "DeleteEntry",
							"CheckConsistency", "MoveTo",
							"Dequeue", "BurnToMedia" };
  if(index<=MAX_SONG_ACTION)
    return QString(songAction[index]);
  else
    return QString("no such action");
}

/**
 * replace placeholders in a string with info from this song
 *
 */
QString Song::replacePlaceholders(QString input, int index)
{
  // 1. prepare strings
  // length (mm:ss)  TODO: (hh:mm:ss if necessary)
  QString secondsStr=QString("%1").arg(length % 60);
  secondsStr.rightJustify(2,'0');
  QString lengthStr=QString("%1:%2").arg((length) / 60).arg(secondsStr);

  // medialist
  QString mediaList="";
	for(unsigned int i=0; i<mediaName.count(); i++) {
		if(i!=0) {
			mediaList+=", ";
    }
		mediaList+=mediaName[i];
	}

  // filename without suffix
  int suffixPos = filename.findRev('.');
  QString filenameWithoutSuffix = filename.left(suffixPos);

  // trackNr
  QString trackNrStr;
  if(trackNr==0) {
    trackNrStr="";
  }
  else {
    trackNrStr=QString("%1").arg(trackNr);
  }

  // replace
  input.replace(QRegExp("\\{absoluteFilename\\}"), location());
	input.replace(QRegExp("\\{filename\\}"), filename);
	input.replace(QRegExp("\\{filenameWithoutSuffix\\}"), filenameWithoutSuffix);
  input.replace(QRegExp("\\{suffix\\}"), getSuffix());
	input.replace(QRegExp("\\{path\\}"), path);
	input.replace(QRegExp("\\{artist\\}"), artist);
	input.replace(QRegExp("\\{title\\}"), title);
	input.replace(QRegExp("\\{album\\}"), album);
	input.replace(QRegExp("\\{bitrate\\}"), QString("%1").arg(bitrate));
	input.replace(QRegExp("\\{index\\}"), QString("%1").arg(index));
	input.replace(QRegExp("\\{length\\}"), lengthStr);
	input.replace(QRegExp("\\{lengthInSeconds\\}"), QString("%1").arg(length));
  input.replace(QRegExp("\\{newline\\}"), "\n");
	input.replace(QRegExp("\\{mediaList\\}"), mediaList);
  input.replace(QRegExp("\\{trackNr\\}"), trackNrStr);
  input.replace(QRegExp("\\{trackNr2Digit\\}"), trackNrStr.rightJustify(2,'0'));
  return input;
}

