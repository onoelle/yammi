/***************************************************************************
                          mp3tag.h  -  description
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
#ifndef MP3_TAG_H
#define MP3_TAG_H


#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <qfile.h>
#include <unistd.h>
#include <iostream.h>



class MP3Tag {
public:
  char title[31];
  char artist[31];
  char album[31];
  char year[5];
  char comment[31];
  char genre[31];
  int gennum;
  int trackNr;

  MP3Tag();
  void clear();
  bool scan(QFile* f);
  bool saveTags(QFile* f);

//  void setGenreName(char *genreName);
//  void setGenreNumber(int genreNumber);

//  static const int maxGenres;
//  static const char* getGenre(int genreNumber);
//  static int getGenre(const char*  genreName);

 	void spacecopy(char *, const char *, int );

private:
//  static const char * genres[];

  void safecopy(char* , char* , int );

  /* ID3 TAG Format, ATTENTION: Space-filled  */
  typedef struct {
    char tag[3];
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[30];
    unsigned char genre;
  } tag;
};


#endif