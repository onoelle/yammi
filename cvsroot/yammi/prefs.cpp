/***************************************************************************
                          prefs.cpp  -  description
                             -------------------
    begin                : Sun Sep 9 2001
    copyright            : (C) 2001 by Brian O.Nlle
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

#include "prefs.h"

/// sets prefernces to the default values
Prefs::Prefs(){
	trashDir="/mp3/trash/";
	scanDir="/mp3/inbox/";
	doubleClickAction=None;
	middleClickAction=None;
	cutShort=false;
	logging=false;
	childSafe=false;
	tagsConsistent=false;
	filenamesConsistent=false;
	criticalSize=700;
	secondSoundDevice="";
	keepInXmms=3;
	searchThreshold=20;
	searchMaximumNoResults=200;
	grabAndEncodeCmd="grabAndEncode";
}

Prefs::~Prefs(){
}
