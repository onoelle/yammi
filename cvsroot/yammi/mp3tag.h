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

static const char* ID3v1_Genre[] = {
     "Blues",
     "Classic Rock",
     "Country",
     "Dance",
     "Disco",
     "Funk",
     "Grunge",
     "Hip-Hop",
     "Jazz",
     "Metal",
     "New Age",
     "Oldies",
     "Other",
     "Pop",
     "R&B",
     "Rap",
     "Reggae",
     "Rock",
     "Techno",
     "Industrial",
     "Alternative",
     "Ska",
     "Death Metal",
     "Pranks",
     "Soundtrack",
     "Euro-Techno",
     "Ambient",
     "Trip-Hop",
     "Vocal",
     "Jazz+Funk",
     "Fusion",
     "Trance",
     "Classical",
     "Instrumental",
     "Acid",
     "House",
     "Game",
     "Sound Clip",
     "Gospel",
     "Noise",
     "AlternRock",
     "Bass",
     "Soul",
     "Punk",
     "Space",
     "Meditative",
     "Instrumental Pop",
     "Instrumental Rock",
     "Ethnic",
     "Gothic",
     "Darkwave",
     "Techno-Industrial",
     "Electronic",
     "Pop-Folk",
     "Eurodance",
     "Dream",
     "Southern Rock",
     "Comedy",
     "Cult",
     "Gangsta",
     "Top 40",
     "Christian Rap",
     "Pop/Funk",
     "Jungle",
     "Native American",
     "Cabaret",
     "New Wave",
     "Psychadelic",
     "Rave",
     "Showtunes",
     "Trailer",
     "Lo-Fi",
     "Tribal",
     "Acid Punk",
     "Acid Jazz",
     "Polka",
     "Retro",
     "Musical",
     "Rock & Roll",
     "Hard Rock",
     "Folk",
     "Folk-Rock",
     "National Folk",
     "Swing",
     "Fast Fusion",
     "Bebob",
     "Latin",
     "Revival",
     "Celtic",
     "Bluegrass",
     "Avantgarde",
     "Gothic Rock",
     "Progressive Rock",
     "Psychedelic Rock",
     "Symphonic Rock",
     "Slow Rock",
     "Big Band",
     "Chorus",
     "Easy Listening",
     "Acoustic",
     "Humour",
     "Speech",
     "Chanson",
     "Opera",
     "Chamber Music",
     "Sonata",
     "Symphony",
     "Booty Bass",
     "Primus",
     "Porn Groove",
     "Satire",
     "Slow Jam",
     "Club",
     "Tango",
     "Samba",
     "Folklore",
     "Ballad",
     "Power Ballad",
     "Rhythmic Soul",
     "Freestyle",
     "Duet",
     "Punk Rock",
     "Drum Solo",
     "Acapella",
     "Euro-House",
     "Dance Hall",
 };
static const int ID3v1_MaxGenreNr=125;


class MP3Tag {
public:
  char title[31];
  char artist[31];
  char album[31];
  char year[5];
  char comment[31];
//  char genre[31];
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

/*
from 		http://www.id3.org/id3v2-00.txt	
			0.Blues
      1.Classic Rock
      2.Country
      3.Dance
      4.Disco
      5.Funk
      6.Grunge
      7.Hip-Hop
      8.Jazz
      9.Metal
     10.New Age
     11.Oldies
     12.Other
     13.Pop
     14.R&B
     15.Rap
     16.Reggae
     17.Rock
     18.Techno
     19.Industrial
     20.Alternative
     21.Ska
     22.Death Metal
     23.Pranks
     24.Soundtrack
     25.Euro-Techno
     26.Ambient
     27.Trip-Hop
     28.Vocal
     29.Jazz+Funk
     30.Fusion
     31.Trance
     32.Classical
     33.Instrumental
     34.Acid
     35.House
     36.Game
     37.Sound Clip
     38.Gospel
     39.Noise
     40.AlternRock
     41.Bass
     42.Soul
     43.Punk
     44.Space
     45.Meditative
     46.Instrumental Pop
     47.Instrumental Rock
     48.Ethnic
     49.Gothic
     50.Darkwave
     51.Techno-Industrial
     52.Electronic
     53.Pop-Folk
     54.Eurodance
     55.Dream
     56.Southern Rock
     57.Comedy
     58.Cult
     59.Gangsta
     60.Top 40
     61.Christian Rap
     62.Pop/Funk
     63.Jungle
     64.Native American
     65.Cabaret
     66.New Wave
     67.Psychadelic
     68.Rave
     69.Showtunes
     70.Trailer
     71.Lo-Fi
     72.Tribal
     73.Acid Punk
     74.Acid Jazz
     75.Polka
     76.Retro
     77.Musical
     78.Rock & Roll
     79.Hard Rock

   The following genres are Winamp extensions

     80.Folk
     81.Folk-Rock
     82.National Folk
     83.Swing
     84.Fast Fusion
     85.Bebob
     86.Latin
     87.Revival
     88.Celtic
     89.Bluegrass
     90.Avantgarde
     91.Gothic Rock
     92.Progressive Rock
     93.Psychedelic Rock
     94.Symphonic Rock
     95.Slow Rock
     96.Big Band
     97.Chorus
     98.Easy Listening
     99.Acoustic
    100.Humour
    101.Speech
    102.Chanson
    103.Opera
    104.Chamber Music
    105.Sonata
    106.Symphony
    107.Booty Bass
    108.Primus
    109.Porn Groove
    110.Satire
    111.Slow Jam
    112.Club
    113.Tango
    114.Samba
    115.Folklore
    116.Ballad
    117.Power Ballad
    118.Rhythmic Soul
    119.Freestyle
    120.Duet
    121.Punk Rock
    122.Drum Solo
    123.A capella
    124.Euro-House
    125.Dance Hall

*/

#endif