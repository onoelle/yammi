/***************************************************************************
                          options.h  -  description
                             -------------------
    begin                : Mon Sep 9 2002
    copyright            : (C) 2002 by Oliver Nölle
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

// disabling does not work yet: the linker still requires the libraries to be present
// (I don't know how to make it configurable - anyone wants to explain it briefly?)
 
// put a comment in front of the following line, if you don't want special mp3 support
// advantages:
// - doesn't need id3lib to compile and run
// disadvantages:
// - no reading of mp3 id3 tags
// - no reading of layer info
#define MP3_SUPPORT

// put a comment in front of the following line, if you don't want special ogg support
// advantages:
// - doesn't need libvorbis and libvorbisfile to compile and run
// disadvantages:
// - no reading of ogg tags
// - no reading of layer info
#define OGG_SUPPORT


