/***************************************************************************
                          prefs.h  -  description
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

#ifndef PREFS_H
#define PREFS_H



#include "song.h"


/**this class just holds all configuration values
  *@author Brian O.Nlle
  */

class Prefs {
public: 
	Prefs();
	~Prefs();
	
// general	
	QString yammiVersion;
	QString scanDir;
	QString trashDir;					// our trashcan for songs (files will be moved to this dir on deleting)
	QString yammiBaseDir;
	
	action doubleClickAction;
	action middleClickAction;
	action controlClickAction;
	action shiftClickAction;
	int criticalSize;					// critical media size in MB
	bool logging;
	bool childSafe;
	bool tagsConsistent;
	bool filenamesConsistent;
	QString secondSoundDevice;
	int keepInXmms;
	int groupThreshold;
  bool lazyGrouping;
	int searchThreshold;
	int searchMaximumNoResults;

// plugins
	QString grabAndEncodeCmd;
	QString shutdownScript;

	QStringList* pluginPlaylistCmd;
	QStringList* pluginPlaylistMenuEntry;
	QStringList* pluginPlaylistCustomList;
	QStringList* pluginSongCmd;
	QStringList* pluginSongMenuEntry;

// jukebox functions
	int swapSize;
	bool mountMediaDir;

	QString mediaDir;
	QString swapDir;

};

#endif
