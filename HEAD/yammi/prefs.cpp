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

/// sets preferences to the default values
Prefs::Prefs(){
// general	
	yammiVersion="0.7";
	trashDir="/mp3/trash/";
	scanDir="/mp3/inbox/";
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
	keepInXmms=3;
	groupThreshold=5;
  lazyGrouping=false;
	searchThreshold=20;
	searchMaximumNoResults=200;
	
// plugins	
	grabAndEncodeCmd="yammiGrabAndEncode";
	shutdownScript="yammiShutdownScript";

	pluginSongMenuEntry = new QStringList();
	pluginSongCmd = new QStringList();
	pluginPlaylistMenuEntry = new QStringList();
	pluginPlaylistCmd = new QStringList();
	pluginPlaylistCustomList = new QStringList();

// define default song plugins
//	pluginSongMenuEntry->append("copy to");
//	pluginSongCmd->append("cp \"%f\" \"/tmp/delme_%F\"");
	
// define default playlist plugins
	pluginPlaylistMenuEntry->append("burn audio cd");
	pluginPlaylistCmd->append("mp3burn -c ATIP %f &");
	pluginPlaylistCustomList->append("");
	pluginPlaylistMenuEntry->append("create cd label");
	pluginPlaylistCmd->append("cdlabelgen -c \"Title\" -s \"Subtitle\" -b -w -i \"%l\" > /tmp/cover.ps");
	pluginPlaylistCustomList->append("%i. %a - %t (%l)%");

// jukebox functions
	mediaDir="/dev/cdrom/";
	swapDir="/tmp/";
	swapSize=200;
	mountMediaDir=true;
}

Prefs::~Prefs(){
}
