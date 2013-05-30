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
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    databaseDir = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[0] + "/";
#else
    databaseDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/";
#endif
    trashDir = QDir::homePath() + "/Desktop/Trash";
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
    trashDir                     = cfg.value("trashDir", trashDir ).toString();
    scanDir                      = cfg.value("scanDir", scanDir).toString();
    followSymLinks               = cfg.value("followSymLinks", followSymLinks).toBool();
    guessingMode                 = cfg.value("guessingMode", guessingMode).toInt();
    scanPattern                  = cfg.value("scanPattern", scanPattern).toString();
    doubleClickAction            = (Song::action) cfg.value("doubleClickAction", doubleClickAction).toInt();
    middleClickAction            = (Song::action) cfg.value("middleClickAction", middleClickAction).toInt();
    logging                      = cfg.value("logging", logging).toBool();
    childSafe                    = cfg.value("childSafe", childSafe).toBool();
    capitalizeTags               = cfg.value("capitalizeTags", capitalizeTags).toBool();
    prelistenMp3Command          = cfg.value("prelistenMp3Command", prelistenMp3Command).toString();
    prelistenOggCommand          = cfg.value("prelistenOggCommand", prelistenOggCommand).toString();
    prelistenWavCommand          = cfg.value("prelistenWavCommand", prelistenWavCommand).toString();
    prelistenFlacCommand         = cfg.value("prelistenFlacCommand", prelistenFlacCommand).toString();
    prelistenOtherCommand        = cfg.value("prelistenOtherCommand", prelistenOtherCommand).toString();
    groupThreshold               = cfg.value("groupThreshold", groupThreshold).toInt();
    if(groupThreshold < 1) {
        groupThreshold = 1;
    }
    lazyGrouping                 = cfg.value("lazyGrouping", lazyGrouping).toBool();
    searchThreshold              = cfg.value("searchThreshold", searchThreshold).toInt();
    mediaPlayer                  = cfg.value("mediaPlayer", mediaPlayer).toInt();
    QString tmp                  = cfg.value("playqueueTemplate", playqueueTemplate).toString();
    if (tmp.length() > 0) playqueueTemplate = tmp; // to be able to reset it to the default
    cfg.endGroup();

    cfg.beginGroup("Plugins");
    pluginCommand                = cfg.value("pluginCommand").toStringList();
    pluginMenuEntry              = cfg.value("pluginMenuEntry").toStringList();
    pluginCustomList             = cfg.value("pluginCustomList").toStringList();
    pluginConfirm                = cfg.value("pluginConfirm").toStringList();
    pluginMode                   = cfg.value("pluginMode").toStringList();
    cfg.endGroup();

    cfg.beginGroup("ConsistencyCheck");
    tagsConsistent          = cfg.value("tagsConsistent", tagsConsistent).toBool();
    filenamesConsistent	    = cfg.value("filenamesConsistent", filenamesConsistent).toBool();
    directoriesConsistent	= cfg.value("directoriesConsistent", directoriesConsistent).toBool();
    consistencyPara->checkDirectories	= cfg.value("checkDirectories", consistencyPara->checkDirectories).toBool();
    consistencyPara->checkDoubles		= cfg.value("checkDoubles", consistencyPara->checkDoubles).toBool();
    consistencyPara->ignoreCaseInFilenames = cfg.value("ignoreCaseInFilenames", consistencyPara->ignoreCaseInFilenames).toBool();
    consistencyPara->checkFilenames		= cfg.value("checkFilenames", consistencyPara->checkFilenames).toBool();
    consistencyPara->checkForExistence	= cfg.value("checkForExistence", consistencyPara->checkForExistence).toBool();
    consistencyPara->checkTags			= cfg.value("checkTags", consistencyPara->checkTags).toBool();
    consistencyPara->correctDirectories	= cfg.value("correctDirectories", consistencyPara->correctDirectories).toBool();
    consistencyPara->correctFilenames	= cfg.value("correctFilenames", consistencyPara->correctFilenames).toBool();
    consistencyPara->correctTags			= cfg.value("correctTags", consistencyPara->correctTags).toBool();
    consistencyPara->correctTagsDirection= cfg.value("correctTagsDirection", consistencyPara->correctTagsDirection).toBool();
    consistencyPara->deleteEmptyDirectories= cfg.value("deleteEmptyDirectories", consistencyPara->deleteEmptyDirectories).toBool();
    consistencyPara->directoryPattern	= cfg.value("directoryPattern", consistencyPara->directoryPattern).toString();
    consistencyPara->filenamePattern		= cfg.value("filenamePattern", consistencyPara->filenamePattern).toString();
    consistencyPara->ignoreCaseInFilenames= cfg.value("ignoreCaseInFilenames", consistencyPara->ignoreCaseInFilenames).toBool();
    consistencyPara->updateNonExisting= cfg.value("updateNonExisting", consistencyPara->updateNonExisting).toBool();
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


