#include "CMP3Info.h"

#include <iostream>
using namespace std;
#include <qfile.h>
#include <qdatastream.h>

#include "../options.h"

#ifdef ENABLE_ID3LIB
// to have the genres from id3lib available
#include <id3/globals.h>
#endif

#ifndef ENABLE_ID3LIB
// if we don't have id3lib...we have to use this table
#define ID3_NR_OF_V1_GENRES 148
static char* ID3_v1_genre_description[ID3_NR_OF_V1_GENRES] =
{
  "Blues",             //0
  "Classic Rock",      //1
  "Country",           //2
  "Dance",             //3
  "Disco",             //4
  "Funk",              //5
  "Grunge",            //6
  "Hip-Hop",           //7
  "Jazz",              //8
  "Metal",             //9
  "New Age",           //10
  "Oldies",            //11
  "Other",             //12
  "Pop",               //13
  "R&B",               //14
  "Rap",               //15
  "Reggae",            //16
  "Rock",              //17
  "Techno",            //18
  "Industrial",        //19
  "Alternative",       //20
  "Ska",               //21
  "Death Metal",       //22
  "Pranks",            //23
  "Soundtrack",        //24
  "Euro-Techno",       //25
  "Ambient",           //26
  "Trip-Hop",          //27
  "Vocal",             //28
  "Jazz+Funk",         //29
  "Fusion",            //30
  "Trance",            //31
  "Classical",         //32
  "Instrumental",      //33
  "Acid",              //34
  "House",             //35
  "Game",              //36
  "Sound Clip",        //37
  "Gospel",            //38
  "Noise",             //39
  "AlternRock",        //40
  "Bass",              //41
  "Soul",              //42
  "Punk",              //43
  "Space",             //44
  "Meditative",        //45
  "Instrumental Pop",  //46
  "Instrumental Rock", //47
  "Ethnic",            //48
  "Gothic",            //49
  "Darkwave",          //50
  "Techno-Industrial", //51
  "Electronic",        //52
  "Pop-Folk",          //53
  "Eurodance",         //54
  "Dream",             //55
  "Southern Rock",     //56
  "Comedy",            //57
  "Cult",              //58
  "Gangsta",           //59
  "Top 40",            //60
  "Christian Rap",     //61
  "Pop/Funk",          //62
  "Jungle",            //63
  "Native American",   //64
  "Cabaret",           //65
  "New Wave",          //66
  "Psychadelic",       //67
  "Rave",              //68
  "Showtunes",         //69
  "Trailer",           //70
  "Lo-Fi",             //71
  "Tribal",            //72
  "Acid Punk",         //73
  "Acid Jazz",         //74
  "Polka",             //75
  "Retro",             //76
  "Musical",           //77
  "Rock & Roll",       //78
  "Hard Rock",         //79
// following are winamp extentions
  "Folk",                  //80
  "Folk-Rock",             //81
  "National Folk",         //82
  "Swing",                 //83
  "Fast Fusion",           //84
  "Bebob",                 //85
  "Latin",                 //86
  "Revival",               //87
  "Celtic",                //88
  "Bluegrass",             //89
  "Avantgarde",            //90
  "Gothic Rock",           //91
  "Progressive Rock",      //92
  "Psychedelic Rock",      //93
  "Symphonic Rock",        //94
  "Slow Rock",             //95
  "Big Band",              //96
  "Chorus",                //97
  "Easy Listening",        //98
  "Acoustic",              //99
  "Humour",                //100
  "Speech",                //101
  "Chanson",               //102
  "Opera",                 //103
  "Chamber Music",         //104
  "Sonata",                //105
  "Symphony",              //106
  "Booty Bass",            //107
  "Primus",                //108
  "Porn Groove",           //109
  "Satire",                //110
  "Slow Jam",              //111
  "Club",                  //112
  "Tango",                 //113
  "Samba",                 //114
  "Folklore",              //115
  "Ballad",                //116
  "Power Ballad",          //117
  "Rhythmic Soul",         //118
  "Freestyle",             //119
  "Duet",                  //120
  "Punk Rock",             //121
  "Drum Solo",             //122
  "A capella",             //123
  "Euro-House",            //124
  "Dance Hall",            //125
  "Goa",                   //126
  "Drum & Bass",           //127
  "Club-House",            //128
  "Hardcore",              //129
  "Terror",                //130
  "Indie",                 //131
  "Britpop",               //132
  "Negerpunk",             //133
  "Polsk Punk",            //134
  "Beat",                  //135
  "Christian Gangsta Rap", //136
  "Heavy Metal",           //137
  "Black Metal",           //138
  "Crossover",             //139
  "Contemporary Christian",//140
  "Christian Rock ",       //141
  "Merengue",              //142
  "Salsa",                 //143
  "Trash Metal",           //144
  "Anime",                 //145
  "JPop",                  //146
  "Synthpop"               //147
};
#endif


int CMP3Info::getMaxGenreNr()
{
  return ID3_NR_OF_V1_GENRES-1;
}


/* ----------------------------------------------------------
   CMP3Info class is your complete guide to the 
   MP3 file format in the C++ language. It's a large class
   with three different, quite large "sub-classes" so it's
   a fair amount of code to look into.

   This code will be well commented, so that everyone can
   understand, as it's made for the public and not for
   private use, although private use is allowed. :)

   all functions specified both in the header and .cpp file
   will have explanations in both locations.

   everything here by: Gustav "Grim Reaper" Munkby
                       http://home.swipnet.se/grd/
                       grd@swipnet.se
   ---------------------------------------------------------- */

#define ERR_FILEOPEN    0x0001
#define ERR_NOSUCHFILE  0x0002
#define ERR_NOMP3FILE   0x0004
#define ERR_ID3TAG      0x0008

int CMP3Info::loadInfo( QString srcMP3 ) {
    
  // open input-file stream to the specified file, name
  QFile file(srcMP3);
  if( file.open( IO_ReadOnly ) ) {
    QDataStream stream( &file );
        
    
    // get file size, by setting the pointer in the end and tell the position
    fileSize=file.size();

    // get srcMP3 into fileName variable
    int pos = 0; // current position in file...


    /******************************************************/
    /* search and load the first frame-header in the file */
    /******************************************************/
        
    char headerchars[4]; // char variable used for header-loading

    stream.readRawBytes(headerchars, 4);
    pos+=4;
    bool headerFound=false;
    for(; !headerFound && pos<(1024*200) && !stream.atEnd(); pos++) {

      // convert four chars to CFrameHeader structure
      header.loadHeader(headerchars);
      if(header.isValidHeader()) {
        headerFound=true;
      }
      else {
        // read one more byte and try again
        headerchars[0]=headerchars[1];
        headerchars[1]=headerchars[2];
        headerchars[2]=headerchars[3];
        stream.readRawBytes(&headerchars[3], 1);
      }
    }

    if(!headerFound) {
      // if no header has been found after 200kB
      // or the end of the file has been reached
      // then there's probably no mp3-file
      return ERR_NOMP3FILE;
    }


        

    /******************************************************/
    /* check for a vbr-header, to ensure the info from a  */
    /* vbr-mp3 is correct                                 */
    /******************************************************/

    char vbrchars[12];
        
    // determine offset from first frame-header
    // it depends on two things, the mpeg-version
    // and the mode(stereo/mono)

    int skip=0;
    if( header.getVersionIndex()==3 ) {  // mpeg version 1

      if( header.getModeIndex()==3 ) skip = 17; // Single Channel
      else                           skip = 32;

    } else {                             // mpeg version 2 or 2.5

      if( header.getModeIndex()==3 ) skip =  9; // Single Channel
      else                           skip = 17;
    }

    char skipChars[64];
    stream.readRawBytes(skipChars, skip);
    pos+=skip;

    // read next twelve bits in
    stream.readRawBytes(vbrchars, 12);

    // turn 12 chars into a CVBitRate class structure
    VBitRate = vbr.loadHeader(vbrchars);        
  }
  else {
    return ERR_NOSUCHFILE;
  }

  file.close();
  return 0;
}



int CMP3Info::getBitrate() {

    if (VBitRate) {

        // get average frame size by deviding fileSize by the number of frames
        float medFrameSize = (float)fileSize / (float)getNumberOfFrames();
        
        /* Now using the formula for FrameSizes which looks different,
           depending on which mpeg version we're using, for mpeg v1:
        
           FrameSize = 12 * BitRate / SampleRate + Padding (if there is padding)

           for mpeg v2 the same thing is:

           FrameSize = 144 * BitRate / SampleRate + Padding (if there is padding)

           remember that bitrate is in kbps and sample rate in Hz, so we need to
           multiply our BitRate with 1000.

           For our purpose, just getting the average frame size, will make the
           padding obsolete, so our formula looks like:

           FrameSize = (mpeg1?12:144) * 1000 * BitRate / SampleRate;
        */

        return (int)( 
                     ( medFrameSize * (float)header.getFrequency() ) / 
                     ( 1000.0 * ( (header.getLayerIndex()==3) ? 12.0 : 144.0))
                    );

    }
    else return header.getBitrate();

}

int CMP3Info::getLengthInSeconds() {

    // kiloBitFileSize to match kiloBitPerSecond in bitrate...
    int kiloBitFileSize = (8 * fileSize) / 1000;
    
    return (int)(kiloBitFileSize/getBitrate());

}


int CMP3Info::getNumberOfFrames() {

    if (!VBitRate) {

        /* Now using the formula for FrameSizes which looks different,
           depending on which mpeg version we're using, for layer 1:
        
           FrameSize = 12 * BitRate / SampleRate + Padding (if there is padding)

           for layer 2 & 3 the same thing is:

           FrameSize = 144 * BitRate / SampleRate + Padding (if there is padding)

           remember that bitrate is in kbps and sample rate in Hz, so we need to
           multiply our BitRate with 1000.

           For our purpose, just getting the average frame size, will make the
           padding obsolete, so our formula looks like:



           FrameSize = (layer1?12:144) * 1000 * BitRate / SampleRate;
        */
           

        float medFrameSize = (float)( 
                                     ( (header.getLayerIndex()==3) ? 12 : 144 ) *
                                     (
                                      (1000.0 * (float)header.getBitrate() ) /
                                      (float)header.getFrequency()
                                     )
                                    );
        
        return (int)(fileSize/medFrameSize);

    }
    else return vbr.getNumberOfFrames();

}

/*
void CMP3Info::getVersion(char* input) {

    char versionchar[32]; // temporary string
    char tempchar2[4]; // layer

    // call CFrameHeader member function
    float ver = header.getVersion();

    // create the layer information with the amounts of I
    int i;
    for( i=0; i<header.getLayer(); i++ ) tempchar2[i] = 'I';
    tempchar2[i] = '\0';


    // combine strings
    sprintf(versionchar,"MPEG %g Layer %s", (double)ver, tempchar2);

    // copy result into inputstring
    strcpy(input, versionchar);

}
*/

QString CMP3Info::getMode() {
    // call CFrameHeader member function
    return header.getMode();
}


/** returns the genre of a given index as a string, or "not supported" if index too high
 */
QString CMP3Info::getGenre(int index)
{
  if(index>=ID3_NR_OF_V1_GENRES || index<0)
    return QString("not supported");

  return QString(ID3_v1_genre_description[index]);
}

// returns the index of a given genre string, or -1 if not found
int CMP3Info::getGenreIndex(QString genre)
{
  for(int i=0; i<ID3_NR_OF_V1_GENRES; i++) {
    if(QString(ID3_v1_genre_description[i])==genre)
      return i;
  }
  cout << "genre not found: " << genre << "\n";
  return -1;
}
