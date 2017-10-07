/***************************************************************************
                          prefs.h  -  description
                             -------------------
    begin                : Sun Sep 9 2001
    copyright            : (C) 2001 by Brian O.Nölle
    email                :  yammi-developer@lists.sourceforge.net
 
    04/10/2003 Stefan Gmeiner (riddlebox@freesurf.ch)
               Class documented; Moved load and save function from 
	       yammiModel into this class; added some message boxes for
               errors on loading and saving
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

#include <QStringList>

#include "song.h"

class ConsistencyCheckParameter;


/**
 * this class just holds all configuration values
 * @author Brian O.Nölle
 */
class Prefs {
public:
    // constants
    static const int MEDIA_PLAYER_DUMMY = 0;
    static const int MEDIA_PLAYER_XINEENGINE = 1;
    // Unused 2
    static const int MEDIA_PLAYER_QMEDIAPLAYERENGINE = 3;
    static const int MEDIA_PLAYER_VLCENGINE = 4;
    static const int GUESSING_MODE_SIMPLE = 0;
    static const int GUESSING_MODE_ADVANCED = 1;

    /**
     * Constructor. Set configuration values to default.
     */
    Prefs();

    /**
     * Destructor.
     */
    ~Prefs();

    /**
     * Load configuration.
     */
    bool loadConfig( );

    /**
     * Save configuration
     */
    bool saveConfig( );

    QString getSoundDevice();
    QString getSoundDeviceParameter();
    QString getDBusService();
    QString getDBusPath();
    QString getDBusServiceOtherYammi();
    QString getDBusPathOtherYammi();

    ///////////////////////////////////////
    // directory where song database file and categories can be found
    QString databaseDir;

    // general
    /** player which is used for playing songs */
    int mediaPlayer;
    /** version of yammi */
    QString yammiVersion;
    /** Base directory for scanning */
    QString scanDir;
    /** flag whether to follow symlinks */
    bool followSymLinks;
    /** Trash directory for deleted song files
     *  Trashcan for songs. Files will be moved to this dir on deleting
     */
    QString trashDir;
    /** Filename pattern for scanning */
    QString scanPattern;
    /** Guessing pattern if no tag is available */
    int guessingMode;

    /** Action for double clicks on left button */
    Song::action doubleClickAction;
    /** Action for click on middle button */
    Song::action middleClickAction;
    /** Logging of played songs activated */
    bool logging;
    /** Child safe mode activated (no changes allowed) */
    bool childSafe;
    /** Keep tags consistent with DB */
    bool tagsConsistent;
    /** Keep filename consistent with DB */
    bool filenamesConsistent;
    /** Keep directories consistent with DB */
    bool directoriesConsistent;
    /** capitalize title, artist and album */
    bool capitalizeTags;


    /** prelistening configuration */
    QString prelistenMp3Command;
    QString prelistenOggCommand;
    QString prelistenWavCommand;
    QString prelistenFlacCommand;
    QString prelistenOtherCommand;

    QString firstYammiSoundDevice;
    QString secondYammiSoundDevice;
    bool thisIsSecondYammi;
    QString soundDeviceParameter;
    
    /** Group threshold */
    int groupThreshold;
    /** Lazy grouping activated */
    bool lazyGrouping;
    /** Threshold for searching */
    int searchThreshold;

    // plugins
    /** List of plugins */
    QStringList pluginMenuEntry;
    /** List of command of plugins */
    QStringList pluginCommand;
    /** List of custon list string of plugins */
    QStringList pluginCustomList;
    /** List of plugin mode */
    QStringList pluginMode;
    /** List of plugin confirmation */
    QStringList pluginConfirm;

    /** All consistency check related parameter */
    ConsistencyCheckParameter* consistencyPara;

    /** template for playqueue */
    QString playqueueTemplate;
    
private:

    /**
     * Set configuration to default values
     */
    void setDefaultValues(void);

};

#endif
