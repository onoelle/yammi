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
#include <qdom.h>


/** 
 * this class just holds all configuration values
 * @author Brian O.Nlle
 */
class Prefs {
public: 
	/**
	 * Constructor. Set configuration values to default.
	 */
	Prefs();

	/**
	 * Destructor.
	 */
	~Prefs();

// constants
	static const int MEDIA_PLAYER_XMMS = 0;
	static const int MEDIA_PLAYER_NOATUN = 1;
	static const int GUESSING_MODE_SIMPLE = 0;
	static const int GUESSING_MODE_ADVANCED = 1;
  
// general	
	/** player which is used for playing songs */
	int player;
	/** version of yammi */
	QString yammiVersion;
	/** Base directory for scanning */
	QString scanDir;
	/** Trash directory for deleted song files 
	 *  Trashcan for songs. Files will be moved to this dir on deleting
	 */
	QString trashDir; 
	/** Yammi base directory for configuration files */
	QString yammiBaseDir;
	/** Filename pattern for renaming files */
	QString filenamePattern;
	/** Guessing pattern if no tag is available */
	int guessingMode;
	
	/** Action for double clicks on left button */
	action doubleClickAction;
	/** Action for click on middle button */
	action middleClickAction;
	/** Action for middle button click with control key */
	action controlClickAction;
	/** Action for middle button click with shift key */
	action shiftClickAction;
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
	/** Second sound device */
	QString secondSoundDevice;
	/** Group threshold */
	int groupThreshold;
	/** Lazy grouping activated */
	bool lazyGrouping;
	/** Threshold for searching */
	int searchThreshold;
	/** Maximum results for searching */
	int searchMaximumNoResults;

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

public:
	/**
	 * Save configuration
	 */
	bool saveConfig(void);

	/**
	 * Load configuration. Base directory can be set with \c setBaseDir.
	 */
	bool loadConfig(QString baseDir);

	/**
	 * return if preferences were found after calling \c loadConfig
	 */
	bool getPrefsFound(void) {		return prefsFound;	}

private:
	QString baseDir; /**< Base directory for yammi's configuration files */
	bool prefsFound; /**< set if preferences were found after \c loadConfig */

	/** 
	 * Set configuration to default values
	 */
	void setDefaultValues(void);

	/**
	 * Add yammi's standard plugins
	 */
	void addStandardPlugins(void);

	/**
	 * Creates yammi's directories
	 */
	bool startFirstTime(QString& baseDir);

	/**
	 * Get a \c QString property from \c QDomDocument object
	 * \param doc XML document from who the \c propName tag should be read
	 * \param propName XML tag which should be read
	 * \param propDefault default value if tag isn't found
	 * \return value of the tag parsed or the default value
	 */
	QString getProperty(const QDomDocument& doc, const QString& propName, const QString& propDefault);

	/**
	 * This is an overloaded member function for \c int values, provided for convenience. It behaves
	 * essentially like the above function. 
	 */
	int getProperty(const QDomDocument& doc, const QString& propName, int propDefault);

	/**
	 * This is an overloaded member function for \c bool values, provided for convenience. It behaves
	 * essentially like the above function. 
	 */
	bool getProperty(const QDomDocument& doc, const QString& propName, bool propDefault);

	/**
	 * This is an overloaded member function for \c QStringList values, provided for convenience. It behaves
	 * essentially like the above function. 
	 */
	QStringList getProperty(const QDomDocument& doc, const QString& propName, const QStringList& propDefault);
	
	/**
	 * Set a \c QString into a \c QDomDocument object on the given tag.
	 * \param doc XML document which the tag should be written
	 * \param propName Tag which the value should be written
	 * \param propValue Value of the tag
	 */
	void setProperty(QDomDocument& doc, const QString& propName, const QString& propValue);

	/**
	 * This is an overloaded member function for \c int values, provided for convenience. It behaves
	 * essentially like the above function. 
	 */
	void setProperty(QDomDocument& doc, const QString& propName, int propValue);

	/**
	 * This is an overloaded member function for \c bool values, provided for convenience. It behaves
	 * essentially like the above function. 
	 */
	void setProperty(QDomDocument& doc, const QString& propName, bool propValue);

	/**
	 * This is an overloaded member function for \c QStringList values, provided for convenience. It behaves
	 * essentially like the above function. 
	 */
	void setProperty(QDomDocument& doc, const QString& propName, const QStringList& propValue);

};

#endif
