#include "CMP3Info.h"

#include <iostream>
using namespace std;
#include <qfile.h>
#include <qdatastream.h>


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
