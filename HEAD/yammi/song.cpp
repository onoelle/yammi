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


#include "song.h"
#include "yammigui.h"

#include "mp3info/CMP3Info.h"       // used to retrieve mp3 layer info

#ifdef ENABLE_OGGLIBS
#include "vcedit.h"
#include "glib.h"
#endif

using namespace std;

extern YammiGui* gYammiGui;

/** creates a song object with default values
 */
Song::Song()
{
	classified=false;
	tagsDirty=false;
	filenameDirty=false;
	corrupted=true;

	artist="";
	title="";
	album="";
	length=0;
	bitrate=-1;
	MyDateTime now=now.currentDateTime();
	addedTo=now;
	comment="";
	year=0;
	this->trackNr=0;
	this->genreNr=0;
	this->lastPlayed.setDate(QDate(1900,1,1));
	this->lastPlayed.setTime(QTime(0,0,0));
}

/** constructs a song object with the given parameters
 */
Song::Song(QString artist, QString title, QString album, QString filename, QString path, int length, int bitrate, MyDateTime addedTo, int year, QString comment, int trackNr, int genreNr)
{
	classified=false;
	tagsDirty=false;
	filenameDirty=false;
	lastPlayed.setDate(QDate(1900,1,1));
	lastPlayed.setTime(QTime(0,0,0));

  corrupted=false;

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
}



/** try to create a new song object from a given filename
 * \return    0 if successful
 *            1 on error (file not found)
 */
int Song::create(const QString location, const QString mediaName)
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
	delete fi;




  // step 2:
  // get info about song object (bitrate, length, tags, ...)

  // guess artist/title from filename (in case no tags can be read)
	QString ffArtist, ffTitle;
  guessTagsFromFilename(saveFilename, &ffArtist, &ffTitle);
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
    }

    // just in case: remove trailing mp3 in title
    if(title.right(4).upper()==".MP3")
      title=title.left(title.length()-4);


#else
    // we have no id3lib support => we have to get info from filename
    title=ffTitle;
		artist=ffArtist;
#endif // ENABLE_ID3LIB

    treated=true;
  }




#ifdef ENABLE_OGGLIBS

  // ogg object
  if(location.right(4).upper()==".OGG") {
    // get ogg info
    if(!getOggInfo(location)) {
      cout << "could not read tag information from ogg file \"" << location << "\"\n";
    }

		// in case the ogg tags are empty => better trust filename info
		if(title=="" && artist=="") {
      cout << "ogg tags empty, taking guessed info from filename...\n";
			title=ffTitle;
			artist=ffArtist;
		}
    // just in case: remove trailing ogg in title
    if(title.right(4).upper()==".OGG") {
      title=title.left(title.length()-4);
    }

    treated=true;
  }

#endif // ENABLE_OGGLIBS


  // wav object

  if(location.right(4).upper()==".WAV") {
    char loc[1000];
    strcpy(loc, location);
    if(!getWavInfo(loc)) {
      cout << "could not read wav header information from wav file \"" << location << "\"\n";
    }
    artist=ffArtist;
    title=ffTitle;
    treated=true;
  }


  if(!treated) {
    cout << location << ": no special handling (such as for mp3 or ogg files) available (or disabled)...\n";
    cout << "  => cannot read information such as bitrate, length and tags\n";
    cout << "  => Yammi tries to guess artist and title from filename (using a simple \"artist - title\" pattern\n";
    bitrate=0;
    length=0;
    artist=ffArtist;
    title=ffTitle;
  }


	// simplify whitespaces
	artist=artist.simplifyWhiteSpace();
	title=title.simplifyWhiteSpace();

	// capitalize after spaces (any exceptions?)
	album=capitalize(album);
	artist=capitalize(artist);
	title=capitalize(title);

	corrupted=false;
  // TODO: maybe perform this check only if not scanning song from media?
  checkConsistency(gYammiGui->getModel()->config.tagsConsistent, gYammiGui->getModel()->config.filenamesConsistent);
  return 0;
}

// check filename (if given)
bool Song::checkFilename()
{
	if(filename=="")
 		return true;
	else
		return (constructFilename()==filename);
}

/** Tries to guess artist and title from filename.
 * So far, assumes a pattern of "artist - title.mp3"
 * \todo: add more sophisticated pattern (leading trackNr?, directory with album?)
 */
void Song::guessTagsFromFilename(QString filename, QString* artist, QString* title)
{
  QString guessBase=filename;

  // remove suffix, if it looks like we have a suffix
  if(guessBase.at(guessBase.length()-4)=='.')
    guessBase=guessBase.left(guessBase.length()-4);

  guessBase=guessBase.replace( QRegExp("_"), " " );							// replace "_" with " "

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
//  cout << "guessed artist: " << *artist << ", title: " << *title << "\n";
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
//    cout << "found tag: (title)" << str << "\n";
    this->title=str;
    foundSomething=true;
  }

  // artist
  this->artist="";
  if(getId3Tag(&tag, ID3FID_LEADARTIST, ID3FN_TEXT, &str)) {
//    cout << "found tag: (artist)" << str << "\n";
    this->artist=str;
    foundSomething=true;
  }

  // album
  this->album="";
  if(getId3Tag(&tag, ID3FID_ALBUM, ID3FN_TEXT, &str)) {
//    cout << "found tag: (album)" << str << "\n";
    this->album=str;
    foundSomething=true;
  }

  // year
  this->year=0;
  if(getId3Tag(&tag, ID3FID_YEAR, ID3FN_TEXT, &str)) {
//    cout << "found tag: (year)" << str << "\n";
		sscanf(str, "%d", &(this->year));
    foundSomething=true;
  }

  // comment
  this->comment="";
  if(getId3Tag(&tag, ID3FID_COMMENT, ID3FN_TEXT, &str)) {
//    cout << "found tag: (comment)" << str << "\n";
    this->comment=str;
    foundSomething=true;
  }

  // tracknum
  this->trackNr=0;
  if(getId3Tag(&tag, ID3FID_TRACKNUM, ID3FN_TEXT, &str)) {
//    cout << "found tag: (tracknum)" << str << "\n";
		sscanf(str, "%d", &(this->trackNr));
    foundSomething=true;
  }

  // genre
  this->genreNr=-1;
  if(getId3Tag(&tag, ID3FID_CONTENTTYPE, ID3FN_TEXT, &str)) {
//    cout << "found tag: (genre)" << str << "\n";
    if(str.left(1)=="(")
      str=str.right(str.length()-1);
    if(str.right(1)==")")
      str=str.left(str.length()-1);
//    cout << "stripped: " << str << "|\n";
		sscanf(str, "%d", &(this->genreNr));
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
//    cout << "could not find frame " << frame << "\n";
    return false;
  }

  // get field
// apparently the following line only works with id3lib3.8.x
//  ID3_Field* theField = theFrame->GetField(field);
  ID3_Field* theField = &(theFrame->Field(field));
  if (theField == NULL) {
//    cout << "could not find field " << field << "\n";
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
    if(!setId3Tag(&tag, ID3FID_CONTENTTYPE, ID3FN_TEXT, QString("%1").arg(this->genreNr), &genreNrFrame))
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

  this->title   = getOggComment(&oggfile, "title");
  this->artist  = getOggComment(&oggfile, "artist");
	this->album   = getOggComment(&oggfile, "album");
	this->comment = getOggComment(&oggfile, "comment");
  QString trackNrStr=getOggComment(&oggfile, "tracknumber");
  this->trackNr = atoi(trackNrStr);
  QString yearStr=getOggComment(&oggfile, "date");
  this->year    = atoi(yearStr);
  QString genreStr=getOggComment(&oggfile, "genre");
  if(genreStr!="") {
    cout << "genre found: " << genreStr << "\n";
    // TODO: convert to id3 genre number?
  }

  this->length  = (int)ov_time_total(&oggfile, -1);
	this->bitrate = ov_bitrate(&oggfile, -1)/1000;

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
//  cout << "looking for: " << commentName << "\n";

	for(int i=0; i < (*ourComment).comments; i++)	{
    cout << "i: " << i << "\n";
		QString curstr((*ourComment).user_comments[i]);
		if( curstr.left(commentName.length()) == commentName) {
//      cout << "match found for name: " << commentName << ", string: " << curstr << "name.length(): " << commentName.length() << ", curstr.length(): " << curstr.length() << "\n";
			return curstr.right(curstr.length() - commentName.length() - 1);
		}
	}
  // nothing found => return empty string
	return "";
}


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

  gchar *string; //, *string1;

  // title
  if( title!="" ) {
    cout << "writing title...\n";
    string  = g_strconcat("title=", title.latin1(), NULL);
//    convert_to_utf8(string);
     vorbis_comment_add(vc, string);
     g_free(string);
//     g_free(string1);
  }

  // artist
  if( artist!="" ) {
    cout << "writing artist...\n";
    string  = g_strconcat("artist=", artist.latin1(), NULL);
    vorbis_comment_add(vc, string);
    g_free(string);
  }

  // album
  if( album!="" ) {
    string  = g_strconcat("album=", "testing album", NULL);
    vorbis_comment_add(vc, string);
    g_free(string);
  }

  // comment
  if( comment!="" ) {
    string  = g_strconcat("comment=", comment.latin1(), NULL);
    vorbis_comment_add(vc, string);
    g_free(string);
  }

  // tracknumber
  if( trackNr!=0 ) {
    string  = g_strconcat("tracknumber=", QString("%1").arg(trackNr).latin1(), NULL);
    vorbis_comment_add(vc, string);
    g_free(string);
  }

  // date
  if( this->year!=0 ) {
    string  = g_strconcat("date=", QString("%1").arg(year).latin1(), NULL);
    vorbis_comment_add(vc, string);
    g_free(string);
  }

  if( genreNr!=-1) {
    // TODO: save genre nr as string  
  }

  // open temp file for writing to
  FILE* file_out;
  if ( (file_out=fopen(filename+".new","w"))==NULL ) {
    cout << "ERROR (saving ogg tags) while opening file " << filename << "\n";
    return false;
  }
  int succ=vcedit_write(state, file_out);

  // delete original file...
	QString cmd=QString("rm %1").arg(filename);
	system(cmd);      // linux-specific...
  // ...and rename temp file to original filename
	QDir dir;
	dir.rename(filename+".new", filename);

  cout << "succ (vcedit_write): " << succ << "\n";
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




/// check tags (if songfile available)
/// true: tags correctly set
/// false: differences between database info and tags in file
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
QString Song::checkConsistency(bool requireConsistentTags, bool requireConsistentFilename)
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
		if(!checkFilename()) {
			cout << "file " << this->filename << " does not have correct filename\n";
			filenameDirty=true;
			diagnosis+="filename not consistent ";
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
 * Constructs a filename following the "artist - title.suffix" pattern.
 * Should take care of special characters not allowed in filenames.
 */
QString Song::constructFilename()
{
	QString s=this->artist+" - "+this->title+this->filename.right(4);
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
		
/*
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
*/

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


QString Song::getSongAction(int index)
{
  const char* songAction[] = {"None", "Enqueue", "EnqueueAsNext", "PlayNow", "SongInfo",
							"PrelistenStart", "PrelistenMiddle", "PrelistenEnd",
							"Delete", "DeleteFile", "DeleteEntry",
							"CheckConsistency", "MoveTo",
							"Dequeue", "BurnToMedia", "AutoPlay" };
  if(index<=MAX_SONG_ACTION)
    return QString(songAction[index]);
  else
    return QString("no such action");
}