/***************************************************************************
                          prefs.cpp  -  description
                             -------------------
    begin                : Sun Sep 9 2001
    copyright            : (C) 2001 by Brian O.Nlle
    email                : yammi-developer@lists.sourceforge.net
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

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>

#include "options.h"

#include <kdebug.h>



/// sets preferences to the default values
Prefs::Prefs(){
	setDefaultValues();
}

Prefs::~Prefs(){
}

void Prefs::setDefaultValues(void) {
	// general	
	// media player: 0=XMMS, 1=Noatun, 2=Arts
	mediaPlayer = MEDIA_PLAYER_ARTSPLAYER;

	yammiVersion = "1.2";
	dbFile = KGlobal::dirs()->findResource("appdata","songdb.xml");
	trashDir = QDir::homeDirPath() + "/Desktop/Trash";
	scanDir = "/mp3/inbox/";
	filenamePattern = "{artist} - {title}.{suffix}";
	directoryPattern = "{artist}/{album}";
	guessingMode = GUESSING_MODE_SIMPLE;
  
	doubleClickAction = Song::None;
	middleClickAction = Song::None;
	logging = false;
	childSafe = false;
	tagsConsistent = false;
	filenamesConsistent = false;
	ignoreCaseInFilenames = false;
	capitalizeTags = true;
	criticalSize = 700;
	secondSoundDevice="";
	groupThreshold = 5;
	lazyGrouping = false;
	searchThreshold = 20;

	keepInXmms = 3;

	fadeTime = 10000;
	fadeOutEnd = 50;
	fadeInStart = 70;
	
	// plugins	
	grabAndEncodeCmd = "yammiGrabAndEncode";
	shutdownScript = "dcop ksmserver ksmserver \"logout\" 0 2 0";

	// jukebox functions
	mediaDir = "/dev/cdrom/";
	swapDir = "/tmp/";
	swapSize = 200;
	mountMediaDir = true;
}


/**
 * baseDir must be an existing and readable directory
 */
bool Prefs::loadConfig( )
{
	kdDebug()<<"loading Configuration..."<<endl;
	
	KConfig *cfg = kapp->config();
	
	cfg->setGroup("General Options");
	trashDir                     = cfg->readEntry("trashDir", trashDir );
	scanDir                      = cfg->readEntry("scanDir", scanDir);
	filenamePattern              = cfg->readEntry("filenamePattern", filenamePattern);
	directoryPattern             = cfg->readEntry("directoryPattern", directoryPattern);
	guessingMode                 = cfg->readNumEntry("guessingMode", guessingMode);
	doubleClickAction            = (Song::action) cfg->readNumEntry("doubleClickAction", doubleClickAction);
	middleClickAction            = (Song::action) cfg->readNumEntry("middleClickAction", middleClickAction);
	logging                      = cfg->readBoolEntry("logging", logging);
	childSafe                    = cfg->readBoolEntry("childSafe", childSafe);
	tagsConsistent               = cfg->readBoolEntry("tagsConsistent", tagsConsistent);
	filenamesConsistent          = cfg->readBoolEntry("filenamesConsistent", filenamesConsistent);
	ignoreCaseInFilenames        = cfg->readBoolEntry("ignoreCaseInFilenames", ignoreCaseInFilenames);
	capitalizeTags               = cfg->readBoolEntry("capitalizeTags", capitalizeTags);
	criticalSize                 = cfg->readNumEntry("criticalSize", criticalSize);
	secondSoundDevice            = cfg->readEntry("secondSoundDevice", secondSoundDevice);
	groupThreshold               = cfg->readNumEntry("groupThreshold", groupThreshold);
	if(groupThreshold < 1) {
		groupThreshold = 1;
	}
	lazyGrouping                 = cfg->readBoolEntry("lazyGrouping", lazyGrouping);
	searchThreshold              = cfg->readNumEntry("searchThreshold", searchThreshold);
	mediaPlayer = cfg->readNumEntry("mediaPlayer", mediaPlayer);
	
	cfg->setGroup("Database");
	dbFile = cfg->readEntry( "dbFile", dbFile );
	kdDebug()<<"dbFile set to "<<dbFile<<endl;
  
	cfg->setGroup("Xmms");
	keepInXmms                   = cfg->readNumEntry("keepInXmms", keepInXmms);

	cfg->setGroup("Noatun");
	fadeTime                     = cfg->readNumEntry("fadeTime", fadeTime);
	fadeOutEnd                   = cfg->readNumEntry("fadeOutEnd", fadeOutEnd);
	fadeInStart                  = cfg->readNumEntry("fadeInStart", fadeInStart);

	cfg->setGroup("Plugins");
	grabAndEncodeCmd             = cfg->readEntry("grabAndEncodeCmd", grabAndEncodeCmd);
	shutdownScript               = cfg->readEntry("shutdownScript", shutdownScript);
	pluginCommand                = cfg->readListEntry("pluginCommand");
	pluginMenuEntry              = cfg->readListEntry("pluginMenuEntry");
	pluginCustomList             = cfg->readListEntry("pluginCustomList");
	pluginConfirm                = cfg->readListEntry("pluginConfirm");
	pluginMode                   = cfg->readListEntry("pluginMode");
	
	cfg->setGroup("Jukebox");
	mediaDir                     = cfg->readEntry("mediaDir", mediaDir);
	mountMediaDir                = cfg->readBoolEntry("mountMediaDir", mountMediaDir);
	swapDir                      = cfg->readEntry("swapDir", swapDir);
	swapSize                     = cfg->readNumEntry("swapSize", swapSize);

	kdDebug() << "Config loaded" << endl;
	return true;
}

/**
 * save preferences (if changed) to disk
 */
bool Prefs::saveConfig( )
{
	kdDebug() << "saving Configuration..." << endl;
	
	KConfig *cfg = kapp->config();
	
	cfg->setGroup("General Options");
	
	cfg->writeEntry("trashDir", trashDir );
	cfg->writeEntry("scanDir", scanDir);
	cfg->writeEntry("filenamePattern", filenamePattern);
	cfg->writeEntry("directoryPattern", directoryPattern);
	cfg->writeEntry("guessingMode", mediaPlayer);
	cfg->writeEntry("doubleClickAction", (int)doubleClickAction);
	cfg->writeEntry("middleClickAction", (int)middleClickAction);
	cfg->writeEntry("logging", logging);
	cfg->writeEntry("childSafe", childSafe);
	cfg->writeEntry("tagsConsistent", tagsConsistent);
	cfg->writeEntry("filenamesConsistent", filenamesConsistent);
	cfg->writeEntry("ignoreCaseInFilenames", ignoreCaseInFilenames);
	cfg->writeEntry("capitalizeTags", capitalizeTags);
	cfg->writeEntry("criticalSize", criticalSize);
	cfg->writeEntry("secondSoundDevice", secondSoundDevice);
	cfg->writeEntry("groupThreshold", groupThreshold);
	cfg->writeEntry("lazyGrouping", lazyGrouping);
	cfg->writeEntry("searchThreshold", searchThreshold);
	cfg->writeEntry("mediaPlayer", mediaPlayer );
  
	cfg->setGroup("Database");
	cfg->writeEntry( "dbFile", dbFile );
	
	cfg->setGroup("Xmms");
	cfg->writeEntry("keepInXmms", keepInXmms);

	cfg->setGroup("Noatun");
	cfg->writeEntry("fadeTime", fadeTime);
	cfg->writeEntry("fadeOutEnd", fadeOutEnd);
	cfg->writeEntry("fadeInStart", fadeInStart);

	cfg->setGroup("Plugins");
	cfg->writeEntry("grabAndEncodeCmd", grabAndEncodeCmd);
	cfg->writeEntry("shutdownScript", shutdownScript);
	cfg->writeEntry("pluginCommand", pluginCommand);
	cfg->writeEntry("pluginMenuEntry", pluginMenuEntry);
	cfg->writeEntry("pluginCustomList", pluginCustomList);
	cfg->writeEntry("pluginConfirm", pluginConfirm);
	cfg->writeEntry("pluginMode", pluginMode);
	
	cfg->setGroup("Jukebox");
	cfg->writeEntry("mediaDir", mediaDir);
	cfg->writeEntry("mountMediaDir", mountMediaDir);
	cfg->writeEntry("swapDir", swapDir);
	cfg->writeEntry("swriteize", swapSize);

	kdDebug() << "Config saved" << endl;
	return true;
}

/**
 * Add all the standard plugins (on switch to new version)
 */
void Prefs::addStandardPlugins()
{
	kdDebug() << "Prefs::addStandardPlugins() adding Yammi's standard plugins to the plugin list" << endl;;

	if(!pluginMenuEntry.contains("Create CD Label")) {
		pluginMenuEntry.append("Create CD Label");
		pluginCommand.append("cdlabelgen -c \"Title\" -s \"Subtitle\" -b -w -i \"{customList}\" > {fileDialog}");
		pluginCustomList.append("{index}. {artist} - {title} ({length})%");
		pluginConfirm.append("true");
		pluginMode.append("group");
	}
	
	if(!pluginMenuEntry.contains("Export to m3u Playlist")) {
		pluginMenuEntry.append("Export to m3u Playlist");
		pluginCommand.append("echo -e \"#EXTM3U\n{customList}\" > {fileDialog}");
		pluginCustomList.append("#EXTINF:{lengthInSeconds},{artist} - {title}{newline}{absoluteFilename}{newline}");
		pluginConfirm.append("true");
		pluginMode.append("group");
	}
	
	if(!pluginMenuEntry.contains("Burn with K3b(audio)")) {
		pluginMenuEntry.append("Burn with K3b(audio)");
		pluginCommand.append("echo -e \"#EXTM3U\n{customList}\" > /tmp/burnlist.m3u && k3b --audiocd /tmp/burnlist.m3u &");
		pluginCustomList.append("#EXTINF:{lengthInSeconds},{artist} - {title}{newline}{absoluteFilename}{newline}");
		pluginConfirm.append("true");
		pluginMode.append("group");
	}
	if(!pluginMenuEntry.contains("Burn with K3b(data)")) {
		pluginMenuEntry.append("Burn with K3b(data)");
		pluginCommand.append("k3b --datacd {customListViaFile} &");
		pluginCustomList.append("\"{absoluteFilename}\" ");
		pluginConfirm.append("true");
		pluginMode.append("group");
	}
	kdDebug() << "Prefs::addStandardPlugins() done" << endl;	
}


