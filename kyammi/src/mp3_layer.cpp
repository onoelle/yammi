/*
    Copyright (C) 2000 Rainer Maximini
                       r_maximi@informatik.uni-kl.de

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* 

  Original code was created by SMF aka Antoine Laydier (laydier@usa.net)

*/

/*=============================================================================
  HEADERs
 =============================================================================*/

#include <qfile.h>
#include "mp3_layer.h"

/*=============================================================================
 Class : Layers
 =============================================================================*/

#define _ANALYSE_BUFFER_SIZE    4096


const int MP3Layer::MPG_MD_STEREO        = 0;
const int MP3Layer::MPG_MD_JOINT_STEREO  = 1;
const int MP3Layer::MPG_MD_DUAL_CHANNEL  = 2;
const int MP3Layer::MPG_MD_MONO          = 3;

const int MP3Layer::MPG_MD_LR_LR = 0;
const int MP3Layer::MPG_MD_LR_I  = 1;
const int MP3Layer::MPG_MD_MS_LR = 2;
const int MP3Layer::MPG_MD_MS_I  = 3;

const char *MP3Layer::mode_names[5] = {"Stereo", "Joint Stereo", "dual Channel",
				    "Single Channel", "Multi Channel"};
const char *MP3Layer::layer_names[3] = {"I", "II", "III"};
const char *MP3Layer::version_names[3] = {"MPEG-1", "MPEG-2 LSF", "MPEG-2.5"};
const char *MP3Layer::version_nums[3] = {"1", "2", "2.5"};
const unsigned int MP3Layer::bitrates[3][3][15] =
{
  {
    {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448},
    {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384},
    {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320}
  },
  {
    {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},
    {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
    {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}
  },
  {
    {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},
    {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
    {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}
  }
};

const unsigned int MP3Layer::s_freq[3][4] =
{
  {44100, 48000, 32000, 0},
  {22050, 24000, 16000, 0},
  {11025, 8000, 8000, 0}
};

const char * MP3Layer::mode_name(void) {  
  if ((mode >= 0) && (mode <= 4)) {
    return (MP3Layer::mode_names[mode]); 
  }
  return 0;
}


const char * MP3Layer::layer_name(void) {  
  if ((lay >=1) && (lay <=3)) {
    return (MP3Layer::layer_names[(lay-1)]); 
  }
  return 0;
}

int MP3Layer::layer_num(void) {  
  if ((lay >=1) && (lay <=3)) {
    return (lay); 
  }
  return 0;
}


const char * MP3Layer::version_name(void) {  
  if ((version >= 0) && (version <= 2)) {
    return (MP3Layer::version_names[version]); 
  }
  return 0;
}


const char * MP3Layer::version_num(void) {  
  if ((version >= 0) && (version <= 2)) {
    return (MP3Layer::version_nums[version]); 
  }
  return 0;
}


unsigned int MP3Layer::bitrate(void)  { 
  if (lXingVBR) {
    return 0;
  }
  if (!((version >= 0) && (version <= 2))) {
    return 0;
  }
  if (!((lay >=1) && (lay <=3))) {
    return 0;
  }
  if (!((bitrate_index >= 0) && (bitrate_index <= 14))) {
    return 0;
  }

  return (MP3Layer::bitrates[version][(lay-1)][bitrate_index]); 
}


unsigned int MP3Layer::sfreq(void)  { 
  if (!((version >= 0) && (version <= 2))) {
    return 0;
  }
  if (!((sampling_frequency>=0) && (sampling_frequency <= 2))) {
    return 0;
  }
  return (MP3Layer::s_freq[version][sampling_frequency]);
}


unsigned int  MP3Layer::pcmPerFrame(void) {   return pcm; }
unsigned long MP3Layer::getFileSize(void) {   return fileSize; }
int           MP3Layer::isXingVBR(){ return lXingVBR; }
//XHEADDATA*    MP3Layer::getXHeadData() { return xHeadData; }


unsigned int MP3Layer::getFrameSize() {
  int framesize=0;
  if(lay==1)  {
    int frequency=sfreq();
    if (frequency != 0) {
      framesize=(12000*bitrate())/frequency;
    }
    if(frequency==44100 && padding)framesize++;
    framesize<<=2;
  } 
  else {
    int frequency=sfreq();
    int div=frequency<<version;
    if (div != 0) {
      framesize=(144000*bitrate())/div;
    }
    if(padding)framesize++;
  }
  return framesize;
}


unsigned long MP3Layer::length(void) { 
  int back=0;
  
  if (lXingVBR) {
/*    float totalframes=xHeadData->frames;
    float wavfilesize=(totalframes*pcm);
    float frequence=(float)sfreq();
    if (frequence != 0) {
      back=(int)(wavfilesize/frequence);
    }
    return back;
    */
  }
  float totalframes=getTotalframes();
  float wavfilesize=(totalframes*pcm);

  float frequence=(float)sfreq();
  
  if (frequence != 0) {
    back=(int)(wavfilesize/frequence);
  }
  return back;
}
    

int MP3Layer::getTotalframes() {
  int pcm=getFrameSize();
  int back=0;
  if (pcm > 0) {
    back=fileSize/pcm;
  }
  return back;
}


bool MP3Layer::scan(QFile* f, QString filename){
//  unsigned char* buff = new unsigned char[_ANALYSE_BUFFER_SIZE];
  unsigned char* buff = new unsigned char[_ANALYSE_BUFFER_SIZE];
//  unsigned char *buffer;
  unsigned char *buffer;
  size_t readsize;
  struct stat fi;

  stat(filename.data(), &fi);
  fileSize = (unsigned long)fi.st_size;
  if (fileSize < _ANALYSE_BUFFER_SIZE) {
    //    fclose(file);
    return false;
  }
  /* Theoretically reading 1024 instead of just 4 means a performance hit
   * if we transfer over net filesystems... However, no filesystem I know
   * of uses block sizes under 1024 bytes.
   */
//  f->at(0, SEEK_SET);
  f->at(0);
  readsize = f->readBlock((char*)buff, _ANALYSE_BUFFER_SIZE);
  readsize -= 4;
  //  fclose(file);
  
  if (readsize <= 0) {
    delete buff;
    return (false);
  }
  lXingVBR=false;
//  if (GetXingHeader(xHeadData,buff)) {
//    lXingVBR=true;
//		QMessageBox::information( this, "Yammi", "Xing Header!!!", "Ups!" );
//  }


  // Synchronize
  int flag=false;
  buffer=buff-1;

  do  {
    if (buffer==buff+readsize) {
      break;
    }
    buffer++;
    if (buffer==buff+readsize) {
      break;
    }    
    if((*buffer)==0xff)
      while(!flag)
      {
	buffer++;
	if (buffer==buff+readsize) {
	  break;
	}
	if(((*buffer)&0xf0)==0xf0)
	{
	  flag=true;
	  break;
	}
	else if((*buffer)!=0xff)break;
      }
  }while(!flag);
  

  
  if (flag==false) {
    delete buff;
    return (false);
  } 
  else {
    switch ((buffer[0]>> 3) & 0x3 ) {
    case 3:
      version = 0;
      break;
    case 2:
      version = 1;
      break;
    case 0:
      version = 2;
      break;
    default:
      delete buff;
      return (false);
    }
    lay = (4 - ((buffer[0] >> 1)) &0x3 );
    error_protection = !(buffer[0] & 0x1);
    bitrate_index = (buffer[1] >> 4) & 0x0F;
    sampling_frequency = (buffer[1] >> 2) & 0x3;
    padding = (buffer[1] >> 1) & 0x01;
    extension = buffer[1] & 0x01;
    mode = (buffer[2] >> 6) & 0x3;
    mode_ext = (buffer[2] >> 4) & 0x03;
    copyright = (buffer[2] >> 3) & 0x01;
    original = (buffer[2] >> 2) & 0x1;
    emphasis = (buffer[2]) & 0x3;
    stereo = (mode == MP3Layer::MPG_MD_MONO) ? 1 : 2;

    if((bitrate_index <0) || (bitrate_index >14)){
      QString message;
//      message.sprintf("Wrong MP3 Layer, Bitrate is to high\nBitrateIndex: %d  Version: %d   Layer: %d\n", bitrate_index,version,lay);
//      throw MP3FileException(message, -1);
    }

    // Added by Cp
    pcm= 32;
    if( lay == 3 ){
      pcm*= 18;
      if( version == 0 )
        pcm*= 2;
    }
    else{
      pcm*= 12;
      if( lay == 2 )
        pcm*= 3;
    }

    delete buff;
    return (true);
  }

}


MP3Layer::MP3Layer() {
  lXingVBR=false;
//  xHeadData=new XHEADDATA();
//  xHeadData->toc=new unsigned char[101];
}


MP3Layer::~MP3Layer() {
//  delete (xHeadData->toc);
//  delete xHeadData;
}


