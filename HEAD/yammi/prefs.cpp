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
  player=1;
#endif
#ifdef ENABLE_XMMS
  player=0;
#endif
	yammiVersion="0.8.0";
	trashDir="/mp3/trash/";
	scanDir="/mp3/inbox/";
  filenamePattern="%a - %t";
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


// define default plugins
	pluginMenuEntry->append("Noatun: Enqueue");
	pluginCommand->append("dcop noatun Noatun addFile \"%f\" 0");
	pluginCustomList->append("");
  pluginConfirm->append("false");
  pluginMode->append("single");

	pluginMenuEntry->append("Noatun: Play");
	pluginCommand->append("dcop noatun Noatun addFile \"%f\" 1");
	pluginCustomList->append("");
  pluginConfirm->append("false");
  pluginMode->append("single");

  pluginMenuEntry->append("create cd label");
	pluginCommand->append("cdlabelgen -c \"Title\" -s \"Subtitle\" -b -w -i \"%l\" > /tmp/cover.ps");
	pluginCustomList->append("%i. %a - %t (%l)%");
  pluginConfirm->append("true");
  pluginMode->append("group");

  pluginMenuEntry->append("export to playlist (m3u)");
	pluginCommand->append("echo -e \"%l\" > /tmp/playlist.m3u");
	pluginCustomList->append("%F%n");
  pluginConfirm->append("true");
  pluginMode->append("group");


// jukebox functions
	mediaDir="/dev/cdrom/";
	swapDir="/tmp/";
	swapSize=200;
	mountMediaDir=true;

}

Prefs::~Prefs(){
}
