/***************************************************************************
                          prefs.h  -  description
                             -------------------
    begin                : Sun Sep 9 2001
    copyright            : (C) 2001 by Brian O.Nlle
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

#include "song.h"
#include "ConsistencyCheckParameter.h"


/**
 * this class just holds all configuration values
 * @author Brian O.Nlle
 */
class Prefs {
public:
    // constants
    static const int MEDIA_PLAYER_XMMS = 0;
    static const int MEDIA_PLAYER_NOATUN = 1;
    static const int MEDIA_PLAYER_ARTSPLAYER = 2;
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

    /**
     * Add yammi's standard plugins
     */
    void addStandardPlugins(void);

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
    /** Trash directory for deleted song files
     *  Trashcan for songs. Files will be moved to this dir on deleting
     */
    QString trashDir;
    /** Yammi base directory for configuration files */
    // 	QString yammiBaseDir;
    /** Filename pattern for renaming files */
    QString filenamePattern;
    /** Filename pattern for renaming files */
    QString directoryPattern;
    /** Guessing pattern if no tag is available */
    int guessingMode;

    /** Action for double clicks on left button */
    Song::action doubleClickAction;
    /** Action for click on middle button */
    Song::action middleClickAction;
    /** Max size of media for burning */
    int criticalSize;
    /** Logging of played songs activated */
    bool logging;
    /** Child safe mode activated (no changes allowed) */
    bool childSafe;
    /** Keep tags consistent with DB */
    bool tagsConsistent;
    /** Keep filename consistent with DB */
    bool filenamesConsistent;
    /** Ignore case in filenames (if keeping them consistent */
    bool ignoreCaseInFilenames;
    /** capitalize title, artist and album */
    bool capitalizeTags;


    /** Second sound device */
    QString secondSoundDevice;
    /** Group threshold */
    int groupThreshold;
    /** Lazy grouping activated */
    bool lazyGrouping;
    /** Threshold for searching */
    int searchThreshold;

    // xmms specific
    /** Songs in playlist of xmms */
    int keepInXmms;

    // noatun specific
    /** Fading time for Noatun (0=disabled) */
    int fadeTime;
    /** Fade out in % */
    int fadeOutEnd;
    /** Fade in in % */
    int fadeInStart;

    // plugins
    /** Name of grab and encode script */
    QString grabAndEncodeCmd;
    /** Shutdown script */
    QString shutdownScript;

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

    // jukebox functions
    /** Maximum swap size */
    int swapSize;
    /** Should media been mounted and unmounted */
    bool mountMediaDir;

    /** Mountpoint for mounting external media */
    QString mediaDir;
    /** Directory for swapping */
    QString swapDir;

    /** All consistency check related parameter */
    ConsistencyCheckParameter consistencyPara;

private:

    /**
     * Set configuration to default values
     */
    void setDefaultValues(void);

};

#endif
