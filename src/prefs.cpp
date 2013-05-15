/***************************************************************************
                          prefs.cpp  -  description
                             -------------------
    begin                : Sun Sep 9 2001
    copyright            : (C) 2001 by Brian O.Nölle
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
#include "config.h"

#include <kdebug.h>



/// sets preferences to the default values
Prefs::Prefs() {
    setDefaultValues();
}

Prefs::~Prefs() {}

void Prefs::setDefaultValues(void) {
    // general
    mediaPlayer = MEDIA_PLAYER_XINEENGINE;
    yammiVersion = VERSION;
    databaseDir = KGlobal::dirs()->findResourceDir("appdata","songdb.xml");
    trashDir = QDir::homeDirPath() + "/Desktop/Trash";
    scanDir = "/mp3/inbox/";
    scanPattern = "*.mp3 *.ogg *.wav *.wma *.m4a *.flac";
    followSymLinks = false;
    guessingMode = GUESSING_MODE_SIMPLE;

    doubleClickAction = Song::None;
    middleClickAction = Song::None;
    logging = false;
    childSafe = false;
    tagsConsistent = false;
    filenamesConsistent = false;
    directoriesConsistent = false;
    capitalizeTags = true;
    criticalSize = 700;
    
    prelistenMp3Command = "mpg123|-a /dev/dsp1|--skip {skipFrames}|'{absoluteFilename}'";
    prelistenOggCommand = "ogg123|-d oss|-odsp:/dev/dsp1|--skip {skipSeconds}|'{absoluteFilename}'";
    prelistenWavCommand = "play|-d /dev/dsp1|'{absoluteFilename}'|trim {skipSamples}";
    prelistenFlacCommand = "mplayer|-ss {skipSeconds}|-ao oss:/dev/dsp1|'{absoluteFilename}'";
    prelistenOtherCommand ="";
    
    groupThreshold = 5;
    lazyGrouping = false;
    searchThreshold = 20;

    // plugins
    grabAndEncodeCmd = "yammiGrabAndEncode";
    shutdownScript = "dcop ksmserver ksmserver \"logout\" 0 2 0";

    // jukebox functions
    mediaDir = "/dev/cdrom/";
    swapDir = "/tmp/";
    swapSize = 200;
    mountMediaDir = true;
    consistencyPara.setDefaults();
    
    playqueueTemplate = "{scope:none}\
<html>\
<head></head>\
<body background=\"{yammiPicsDir}/playlistbackground.jpg\" style=\"font-size:10pt;font-family:helvetica\">\
<p>\
<nobr><span style=\"font-size:14pt;font-weight:bold;font-style:italic;color:#888888\">&nbsp; Playqueue</span></nobr><br />\
<span style=\"font-size:10pt;color:#00aa00\">\
{scope:-3}<nobr>-3. {artist} - {title} ({length})</nobr><br />\
{scope:-2}<nobr>-2. {artist} - {title} ({length})</nobr><br />\
{scope:-1}<nobr>-1. {artist} - {title} ({length})</nobr><br />\
</span>\
{scope:0}<span style=\"font-size:10pt;color:#ff0000\">\
<nobr><img align=\"left\" src=\"file:{yammiPicsDir}/nowplaying.png\"/>\
<b>&nbsp;{artist} - {title} ({length})</nobr></b><br />\
<nobr>&nbsp;Album: {album}, Track: {trackNr}</nobr><br />\
</span>\
<span style=\"font-size:10pt;color:#0000ff\">\
{scope:1}<nobr>1. &nbsp;{artist} - {title} ({length})</nobr><br/>\
{scope:2}<nobr>2. &nbsp;{artist} - {title} ({length})</nobr><br/>\
{scope:3}<nobr>3. &nbsp;{artist} - {title} ({length})</nobr><br/>\
{scope:4}<nobr>4. &nbsp;{artist} - {title} ({length})</nobr><br/>\
{scope:5}<nobr>5. &nbsp;{artist} - {title} ({length})</nobr><br/>\
{scope:none}\
</span>\
<span style=\"font-size:10pt;color:#888888\">\
<nobr>&nbsp;&nbsp;...{noSongsToPlay} more songs to play ({timeToPlay} hh:mm)</nobr><br/>\
</span>\
</p>\
</body></html>";
}


/**
 * baseDir must be an existing and readable directory
 */
bool Prefs::loadConfig( ) {
    kdDebug() << "loading Configuration..." << endl;

    KConfig* cfg = kapp->config();

    cfg->setGroup("General Options");
    trashDir                     = cfg->readEntry("trashDir", trashDir );
    scanDir                      = cfg->readEntry("scanDir", scanDir);
    followSymLinks               = cfg->readBoolEntry("followSymLinks", followSymLinks);
    guessingMode                 = cfg->readNumEntry("guessingMode", guessingMode);
    scanPattern                  = cfg->readEntry("scanPattern", scanPattern);
    doubleClickAction            = (Song::action) cfg->readNumEntry("doubleClickAction", doubleClickAction);
    middleClickAction            = (Song::action) cfg->readNumEntry("middleClickAction", middleClickAction);
    logging                      = cfg->readBoolEntry("logging", logging);
    childSafe                    = cfg->readBoolEntry("childSafe", childSafe);
    capitalizeTags               = cfg->readBoolEntry("capitalizeTags", capitalizeTags);
    criticalSize                 = cfg->readNumEntry("criticalSize", criticalSize);
    prelistenMp3Command          = cfg->readEntry("prelistenMp3Command", prelistenMp3Command);
    prelistenOggCommand          = cfg->readEntry("prelistenOggCommand", prelistenOggCommand);
    prelistenWavCommand          = cfg->readEntry("prelistenWavCommand", prelistenWavCommand);
    prelistenFlacCommand          = cfg->readEntry("prelistenFlacCommand", prelistenFlacCommand);
    prelistenOtherCommand          = cfg->readEntry("prelistenOtherCommand", prelistenOtherCommand);
    groupThreshold               = cfg->readNumEntry("groupThreshold", groupThreshold);
    if(groupThreshold < 1) {
        groupThreshold = 1;
    }
    lazyGrouping                 = cfg->readBoolEntry("lazyGrouping", lazyGrouping);
    searchThreshold              = cfg->readNumEntry("searchThreshold", searchThreshold);
    mediaPlayer                  = cfg->readNumEntry("mediaPlayer", mediaPlayer);
    playqueueTemplate            = cfg->readEntry("playqueueTemplate", playqueueTemplate);

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


    cfg->setGroup("ConsistencyCheck");
    tagsConsistent          = cfg->readBoolEntry("tagsConsistent", tagsConsistent);
    filenamesConsistent	    = cfg->readBoolEntry("filenamesConsistent", filenamesConsistent);
    directoriesConsistent	= cfg->readBoolEntry("directoriesConsistent", directoriesConsistent);
    consistencyPara.checkDirectories	= cfg->readBoolEntry("checkDirectories", consistencyPara.checkDirectories);
    consistencyPara.checkDoubles		= cfg->readBoolEntry("checkDoubles", consistencyPara.checkDoubles);
    consistencyPara.ignoreCaseInFilenames = cfg->readBoolEntry("ignoreCaseInFilenames", consistencyPara.ignoreCaseInFilenames);
    consistencyPara.checkFilenames		= cfg->readBoolEntry("checkFilenames", consistencyPara.checkFilenames);
    consistencyPara.checkForExistence	= cfg->readBoolEntry("checkForExistence", consistencyPara.checkForExistence);
    consistencyPara.checkTags			= cfg->readBoolEntry("checkTags", consistencyPara.checkTags);
    consistencyPara.correctDirectories	= cfg->readBoolEntry("correctDirectories", consistencyPara.correctDirectories);
    consistencyPara.correctFilenames	= cfg->readBoolEntry("correctFilenames", consistencyPara.correctFilenames);
    consistencyPara.correctTags			= cfg->readBoolEntry("correctTags", consistencyPara.correctTags);
    consistencyPara.correctTagsDirection= cfg->readBoolEntry("correctTagsDirection", consistencyPara.correctTagsDirection);
    consistencyPara.deleteEmptyDirectories= cfg->readBoolEntry("deleteEmptyDirectories", consistencyPara.deleteEmptyDirectories);
    consistencyPara.directoryPattern	= cfg->readEntry("directoryPattern", consistencyPara.directoryPattern);
    consistencyPara.filenamePattern		= cfg->readEntry("filenamePattern", consistencyPara.filenamePattern);
    consistencyPara.ignoreCaseInFilenames= cfg->readBoolEntry("ignoreCaseInFilenames", consistencyPara.ignoreCaseInFilenames);
    consistencyPara.updateNonExisting= cfg->readBoolEntry("updateNonExisting", consistencyPara.updateNonExisting);

    kdDebug() << "Config loaded" << endl;
    return true;
}

/**
 * save preferences (if changed) to disk
 */
bool Prefs::saveConfig( ) {
    kdDebug() << "saving Configuration..." << endl;

    KConfig *cfg = kapp->config();

    cfg->setGroup("General Options");

    cfg->writeEntry("scanDir", scanDir);
    cfg->writeEntry("followSymLinks", followSymLinks);
    cfg->writeEntry("guessingMode", guessingMode);
    cfg->writeEntry("scanPattern", scanPattern);

    cfg->writeEntry("trashDir", trashDir );
    cfg->writeEntry("doubleClickAction", (int)doubleClickAction);
    cfg->writeEntry("middleClickAction", (int)middleClickAction);
    cfg->writeEntry("logging", logging);
    cfg->writeEntry("childSafe", childSafe);
    cfg->writeEntry("capitalizeTags", capitalizeTags);
    cfg->writeEntry("criticalSize", criticalSize);
    cfg->writeEntry("prelistenMp3Command", prelistenMp3Command);
    cfg->writeEntry("prelistenOggCommand", prelistenOggCommand);
    cfg->writeEntry("prelistenWavCommand", prelistenWavCommand);
    cfg->writeEntry("prelistenFlacCommand", prelistenFlacCommand);
    cfg->writeEntry("prelistenOtherCommand", prelistenOtherCommand);
    cfg->writeEntry("groupThreshold", groupThreshold);
    cfg->writeEntry("lazyGrouping", lazyGrouping);
    cfg->writeEntry("searchThreshold", searchThreshold);
    cfg->writeEntry("mediaPlayer", mediaPlayer);    
    cfg->writeEntry("playqueueTemplate", playqueueTemplate);

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


    cfg->setGroup("ConsistencyCheck");
    cfg->writeEntry("tagsConsistent", tagsConsistent);
    cfg->writeEntry("filenamesConsistent", filenamesConsistent);
    cfg->writeEntry("directoriesConsistent", directoriesConsistent);

    cfg->writeEntry("checkDirectories", consistencyPara.checkDirectories);
    cfg->writeEntry("checkDoubles", consistencyPara.checkDoubles);
    cfg->writeEntry("checkFilenames", consistencyPara.checkFilenames);
    cfg->writeEntry("checkForExistence", consistencyPara.checkForExistence);
    cfg->writeEntry("checkTags", consistencyPara.checkTags);
    cfg->writeEntry("correctDirectories", consistencyPara.correctDirectories);
    cfg->writeEntry("correctFilenames", consistencyPara.correctFilenames);
    cfg->writeEntry("correctTags", consistencyPara.correctTags);
    cfg->writeEntry("correctTagsDirection", consistencyPara.correctTagsDirection);
    cfg->writeEntry("deleteEmptyDirectories", consistencyPara.deleteEmptyDirectories);
    cfg->writeEntry("directoryPattern", consistencyPara.directoryPattern);
    cfg->writeEntry("filenamePattern", consistencyPara.filenamePattern);
    cfg->writeEntry("ignoreCaseInFilenames", consistencyPara.ignoreCaseInFilenames);
    cfg->writeEntry("updateNonExisting", consistencyPara.updateNonExisting);
    kdDebug() << "Config saved" << endl;
    return true;
}


