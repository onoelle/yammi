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

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QSettings>

#include "config.h"
#include "ConsistencyCheckParameter.h"


/// sets preferences to the default values
Prefs::Prefs()
{
    consistencyPara = new ConsistencyCheckParameter();
    setDefaultValues();
}

Prefs::~Prefs()
{
    delete consistencyPara;
}

void Prefs::setDefaultValues(void) {
    // general
    mediaPlayer = MEDIA_PLAYER_XINEENGINE;
    yammiVersion = VERSION;
    databaseDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/";
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
    
    prelistenMp3Command = "mpg123|-a|/dev/dsp1|--skip|{skipFrames}|'{absoluteFilename}'";
    prelistenOggCommand = "ogg123|-d|oss|-odsp:/dev/dsp1|--skip|{skipSeconds}|'{absoluteFilename}'";
    prelistenWavCommand = "play|-d|/dev/dsp1|'{absoluteFilename}'|trim|{skipSamples}";
    prelistenFlacCommand = "mplayer|-ss|{skipSeconds}|-ao|oss:/dev/dsp1|'{absoluteFilename}'";
    prelistenOtherCommand ="";
    
    groupThreshold = 5;
    lazyGrouping = false;
    searchThreshold = 20;

    consistencyPara->setDefaults();
    
    playqueueTemplate = "{scope:none}\
<html>\
<head></head>\
<body background=\"icons:playlistbackground.jpg\" style=\"font-size:10pt;font-family:helvetica\">\
<p>\
<nobr><span style=\"font-size:14pt;font-weight:bold;font-style:italic;color:#888888\">&nbsp; Playqueue</span></nobr><br />\
<span style=\"font-size:10pt;color:#00aa00\">\
{scope:-3}<nobr>-3. {artist} - {title} ({length})</nobr><br />\
{scope:-2}<nobr>-2. {artist} - {title} ({length})</nobr><br />\
{scope:-1}<nobr>-1. {artist} - {title} ({length})</nobr>\
</span>\
{scope:0}<span style=\"font-size:10pt;color:#ff0000\">\
<div><table border=\"0\"><tr>\
<td align=\"left\"><img src=\"icons:nowplaying.png\"/></td>\
<td align=\"left\"><b>{artist} - {title} ({length})</b><br>Album: {album}, Track: {trackNr}</td>\
</tr></table></div>\
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
</p><img src=\"icons:playlistbackground.jpg\"/>\
</body></html>";
}
/* needed to view playlistbackground in text also; without it QTextEdit in my Qt version would not show it */


/**
 * baseDir must be an existing and readable directory
 */
bool Prefs::loadConfig( ) {
    qDebug() << "loading Configuration...";

    QSettings cfg;

    cfg.beginGroup("General Options");
    trashDir                     = cfg.readEntry("trashDir", trashDir );
    scanDir                      = cfg.readEntry("scanDir", scanDir);
    followSymLinks               = cfg.readBoolEntry("followSymLinks", followSymLinks);
    guessingMode                 = cfg.readNumEntry("guessingMode", guessingMode);
    scanPattern                  = cfg.readEntry("scanPattern", scanPattern);
    doubleClickAction            = (Song::action) cfg.readNumEntry("doubleClickAction", doubleClickAction);
    middleClickAction            = (Song::action) cfg.readNumEntry("middleClickAction", middleClickAction);
    logging                      = cfg.readBoolEntry("logging", logging);
    childSafe                    = cfg.readBoolEntry("childSafe", childSafe);
    capitalizeTags               = cfg.readBoolEntry("capitalizeTags", capitalizeTags);
    prelistenMp3Command          = cfg.readEntry("prelistenMp3Command", prelistenMp3Command);
    prelistenOggCommand          = cfg.readEntry("prelistenOggCommand", prelistenOggCommand);
    prelistenWavCommand          = cfg.readEntry("prelistenWavCommand", prelistenWavCommand);
    prelistenFlacCommand         = cfg.readEntry("prelistenFlacCommand", prelistenFlacCommand);
    prelistenOtherCommand        = cfg.readEntry("prelistenOtherCommand", prelistenOtherCommand);
    groupThreshold               = cfg.readNumEntry("groupThreshold", groupThreshold);
    if(groupThreshold < 1) {
        groupThreshold = 1;
    }
    lazyGrouping                 = cfg.readBoolEntry("lazyGrouping", lazyGrouping);
    searchThreshold              = cfg.readNumEntry("searchThreshold", searchThreshold);
    mediaPlayer                  = cfg.readNumEntry("mediaPlayer", mediaPlayer);
    QString tmp                  = cfg.readEntry("playqueueTemplate", playqueueTemplate);
    if (tmp.length() > 0) playqueueTemplate = tmp; // to be able to reset it to the default
    cfg.endGroup();

    cfg.beginGroup("Plugins");
    pluginCommand                = cfg.readListEntry("pluginCommand");
    pluginMenuEntry              = cfg.readListEntry("pluginMenuEntry");
    pluginCustomList             = cfg.readListEntry("pluginCustomList");
    pluginConfirm                = cfg.readListEntry("pluginConfirm");
    pluginMode                   = cfg.readListEntry("pluginMode");
    cfg.endGroup();

    cfg.beginGroup("ConsistencyCheck");
    tagsConsistent          = cfg.readBoolEntry("tagsConsistent", tagsConsistent);
    filenamesConsistent	    = cfg.readBoolEntry("filenamesConsistent", filenamesConsistent);
    directoriesConsistent	= cfg.readBoolEntry("directoriesConsistent", directoriesConsistent);
    consistencyPara->checkDirectories	= cfg.readBoolEntry("checkDirectories", consistencyPara->checkDirectories);
    consistencyPara->checkDoubles		= cfg.readBoolEntry("checkDoubles", consistencyPara->checkDoubles);
    consistencyPara->ignoreCaseInFilenames = cfg.readBoolEntry("ignoreCaseInFilenames", consistencyPara->ignoreCaseInFilenames);
    consistencyPara->checkFilenames		= cfg.readBoolEntry("checkFilenames", consistencyPara->checkFilenames);
    consistencyPara->checkForExistence	= cfg.readBoolEntry("checkForExistence", consistencyPara->checkForExistence);
    consistencyPara->checkTags			= cfg.readBoolEntry("checkTags", consistencyPara->checkTags);
    consistencyPara->correctDirectories	= cfg.readBoolEntry("correctDirectories", consistencyPara->correctDirectories);
    consistencyPara->correctFilenames	= cfg.readBoolEntry("correctFilenames", consistencyPara->correctFilenames);
    consistencyPara->correctTags			= cfg.readBoolEntry("correctTags", consistencyPara->correctTags);
    consistencyPara->correctTagsDirection= cfg.readBoolEntry("correctTagsDirection", consistencyPara->correctTagsDirection);
    consistencyPara->deleteEmptyDirectories= cfg.readBoolEntry("deleteEmptyDirectories", consistencyPara->deleteEmptyDirectories);
    consistencyPara->directoryPattern	= cfg.readEntry("directoryPattern", consistencyPara->directoryPattern);
    consistencyPara->filenamePattern		= cfg.readEntry("filenamePattern", consistencyPara->filenamePattern);
    consistencyPara->ignoreCaseInFilenames= cfg.readBoolEntry("ignoreCaseInFilenames", consistencyPara->ignoreCaseInFilenames);
    consistencyPara->updateNonExisting= cfg.readBoolEntry("updateNonExisting", consistencyPara->updateNonExisting);
    cfg.endGroup();

    qDebug() << "Config loaded";
    return true;
}

/**
 * save preferences (if changed) to disk
 */
bool Prefs::saveConfig( ) {
    qDebug() << "saving Configuration...";

    QSettings cfg;

    cfg.beginGroup("General Options");

    cfg.setValue("scanDir", scanDir);
    cfg.setValue("followSymLinks", followSymLinks);
    cfg.setValue("guessingMode", guessingMode);
    cfg.setValue("scanPattern", scanPattern);

    cfg.setValue("trashDir", trashDir );
    cfg.setValue("doubleClickAction", (int)doubleClickAction);
    cfg.setValue("middleClickAction", (int)middleClickAction);
    cfg.setValue("logging", logging);
    cfg.setValue("childSafe", childSafe);
    cfg.setValue("capitalizeTags", capitalizeTags);
    cfg.setValue("prelistenMp3Command", prelistenMp3Command);
    cfg.setValue("prelistenOggCommand", prelistenOggCommand);
    cfg.setValue("prelistenWavCommand", prelistenWavCommand);
    cfg.setValue("prelistenFlacCommand", prelistenFlacCommand);
    cfg.setValue("prelistenOtherCommand", prelistenOtherCommand);
    cfg.setValue("groupThreshold", groupThreshold);
    cfg.setValue("lazyGrouping", lazyGrouping);
    cfg.setValue("searchThreshold", searchThreshold);
    cfg.setValue("mediaPlayer", mediaPlayer);
    cfg.setValue("playqueueTemplate", playqueueTemplate);
    cfg.endGroup();

    cfg.beginGroup("Plugins");
    cfg.setValue("pluginCommand", pluginCommand);
    cfg.setValue("pluginMenuEntry", pluginMenuEntry);
    cfg.setValue("pluginCustomList", pluginCustomList);
    cfg.setValue("pluginConfirm", pluginConfirm);
    cfg.setValue("pluginMode", pluginMode);
    cfg.endGroup();

    cfg.beginGroup("ConsistencyCheck");
    cfg.setValue("tagsConsistent", tagsConsistent);
    cfg.setValue("filenamesConsistent", filenamesConsistent);
    cfg.setValue("directoriesConsistent", directoriesConsistent);

    cfg.setValue("checkDirectories", consistencyPara->checkDirectories);
    cfg.setValue("checkDoubles", consistencyPara->checkDoubles);
    cfg.setValue("checkFilenames", consistencyPara->checkFilenames);
    cfg.setValue("checkForExistence", consistencyPara->checkForExistence);
    cfg.setValue("checkTags", consistencyPara->checkTags);
    cfg.setValue("correctDirectories", consistencyPara->correctDirectories);
    cfg.setValue("correctFilenames", consistencyPara->correctFilenames);
    cfg.setValue("correctTags", consistencyPara->correctTags);
    cfg.setValue("correctTagsDirection", consistencyPara->correctTagsDirection);
    cfg.setValue("deleteEmptyDirectories", consistencyPara->deleteEmptyDirectories);
    cfg.setValue("directoryPattern", consistencyPara->directoryPattern);
    cfg.setValue("filenamePattern", consistencyPara->filenamePattern);
    cfg.setValue("ignoreCaseInFilenames", consistencyPara->ignoreCaseInFilenames);
    cfg.setValue("updateNonExisting", consistencyPara->updateNonExisting);
    cfg.endGroup();
    qDebug() << "Config saved";
    return true;
}


