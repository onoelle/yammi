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
#include "options.h"
#include "prefs.h"

/// sets preferences to the default values
Prefs::Prefs(){
// general	
  // media player: 0=XMMS, 1=Noatun
#ifdef ENABLE_NOATUN
  player=MEDIA_PLAYER_NOATUN;
#endif
#ifdef ENABLE_XMMS
  player=MEDIA_PLAYER_XMMS;
#endif
	yammiVersion="0.8.0";
	trashDir="/mp3/trash/";
	scanDir="/mp3/inbox/";
  filenamePattern="%a - %t";
  guessingMode=GUESSING_MODE_SIMPLE;
  
	doubleClickAction=None;
	middleClickAction=None;
	controlClickAction=None;
	shiftClickAction=None;
	logging=false;
	childSafe=false;
	tagsConsistent=false;
	filenamesConsistent=false;
	criticalSize=700;
	secondSoundDevice="";
	groupThreshold=5;
  lazyGrouping=false;
	searchThreshold=20;
	searchMaximumNoResults=200;

	keepInXmms=3;

  fadeTime=10000;
  fadeOutEnd=50;
  fadeInStart=70;
	
// plugins	
	grabAndEncodeCmd="yammiGrabAndEncode";
	shutdownScript="dcop ksmserver ksmserver \"logout\" 0 2 0";

	pluginMenuEntry = new QStringList();
	pluginCommand = new QStringList();
	pluginCustomList = new QStringList();
	pluginConfirm = new QStringList();
	pluginMode = new QStringList();

// jukebox functions
	mediaDir="/dev/cdrom/";
	swapDir="/tmp/";
	swapSize=200;
	mountMediaDir=true;

}

Prefs::~Prefs(){
}
