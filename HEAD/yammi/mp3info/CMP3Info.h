#ifndef CMP3INFO_H
#define CMP3INFO_H

#include "CFrameHeader.h"
#include "CVBitRate.h"
#include <qstring.h>

#ifdef ENABLE_ID3LIB
// to have the genres from id3lib available
#include <id3/globals.h>
#else
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

   9-2002: slight modifications by Oliver Nölle:
   - removed all tag handling methods (I'm using id3lib) (except genre methods)
   - made it work under linux
   ---------------------------------------------------------- */

class CMP3Info {

    public:

    // function to load a file into this structure
    // the argument passed is the path to a MP3 file
    int   loadInfo(QString srcMP3);

    // functions used to get information about the "file"
    int   getFileSize() { return fileSize; };
//    void  getFileName(char* input);

    // information that is avaliable in FrameHeader & VBR header
//    void  getVersion(char* input);
    int   getBitrate();
    int   getFrequency() { return header.getFrequency(); };
    QString getMode();
    
    int   getNumberOfFrames();

    // functions to calculate the length of the song
    // and to present it nicely
    int   getLengthInSeconds();
//    void  getFormattedLength(char* input);

    // just to know what kind of file it is.
    bool  isVBitRate() { return VBitRate; };

    // id3 genre methods
    static int getMaxGenreNr()        { return ID3_NR_OF_V1_GENRES-1; }
    static QString getGenre(int index);
    static int getGenreIndex(QString genre);
    
private:

    // these are the "sub-classes"
    CFrameHeader header;
    CVBitRate    vbr;

    // just to know what kind of file it is
    bool VBitRate;

    // the file information can not be found elsewhere
//    char fileName[256];
    int fileSize;

};

#endif
