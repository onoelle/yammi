/***************************************************************************
                          mp3tag.cpp  -  description
                             -------------------
    begin                : Fri Feb 9 2001
    copyright            : (C) 2001 by Oliver Nölle
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

#include "mp3tag.h"


MP3Tag::MP3Tag() {
  title[0] = 0;
  artist[0] = 0;
  album[0] = 0;
  year[0] = 0;
  comment[0] = 0;
//  genre[0] = 0;
  gennum = -1;
  trackNr= -1;
}


void MP3Tag::clear(){
  *title   = '\0';
  *artist  = '\0';
  *album   = '\0';
  *year    = '\0';
  *comment = '\0';
//  *genre   = '\0';
  gennum = -1;
  trackNr= -1;
}


void MP3Tag::safecopy(char *to, char *from, int maxlen){
  int where;
  strncpy(to, from, maxlen);
  to[maxlen] = 0;
  for (where = maxlen - 1; ((where >= 0) && (to[where] == ' ')); where--) {
    to[where] = 0;
  }
}


void MP3Tag::spacecopy(char *to, const char *from, int maxlen){
	int where;
	strncpy(to, from, maxlen);
	for (where = maxlen - 1; ((where >= 0) && (to[where] == 0)); where--) {
		to[where] = ' ';
	}
}


bool MP3Tag::scan(QFile* f){
  tag song;
  if(!f->at(f->size()-128)){
    perror("fseek");
    return false;
  }
  if(f->readBlock((char*)&song, 128) < 1){
    perror("fread");
    return false;
  }
  if (!strncmp(song.tag, "TAG", 3)) {
    safecopy(title, song.title, 30);
    safecopy(artist, song.artist, 30);
    safecopy(album, song.album, 30);
    safecopy(year, song.year, 4);
		// oli: added v1.1 support? last byte in comment is for track nr?
		if(song.comment[28]==0 && song.comment[29]!=0) {
	    safecopy(comment, song.comment, 28);
	    comment[29]=0;
	    comment[30]=0;
	    comment[31]=0;
    	trackNr=(int)song.comment[29];
		}
    else {
	    safecopy(comment, song.comment, 30);
    	trackNr=0;
    }
    gennum = song.genre & 0xFF;


//    if ((gennum < 0) || (gennum >= maxGenres)) gennum = maxGenres;
//    strcpy(genre, genres[gennum]);
    return true;
  }
  return false;
}


// save id3 tags to given file
bool MP3Tag::saveTags(QFile* f){
  tag song;

  spacecopy(song.title, title, 30);
  spacecopy(song.artist, artist, 30);
  spacecopy(song.album, album, 30);
  spacecopy(song.year, year, 4);
  spacecopy(song.comment, comment, 30);
	if(trackNr!=0) {
	  song.comment[28]=0;
	  song.comment[29]=(unsigned char)trackNr;
	}
  song.genre = (unsigned char) gennum;

  if(!f->at(f->size()-128)){
		cout << "error saving tags (1)\n";
		return false;
	}
  if(f->readBlock((char*)&song, 3) < 1){
  	cout << "error saving tags(1b)\n";
  }

  if (!strncmp(song.tag, "TAG", 3)) {
		f->at(f->size()-128);
	}
  else {
  	f->at(f->size());
	}

  strncpy(song.tag, "TAG", 3);
	  if(f->writeBlock((char*)&song, 128) < 1) {
  	cout << "error saving tags (2)\n";
  	return false;
  }
//	cout << "tags successfully saved\n";
  return true;
}
