/***************************************************************************
                          yammimodel.h  -  description
                             -------------------
    begin                : Sun Oct 7 2001
    copyright            : (C) 2001 by Brian O.Nlle
    email                : yammi-developer@lists.sourceforge.net

    04/10/2003 Stefan Gmeiner (riddlebox@freesurf.ch)
               load and save function for preferences removed (they are now
               in class prefs)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef YAMMIMODEL_H
#define YAMMIMODEL_H



// these are the includes I wanted to avoid (gui-stuff)
#include <qprogressdialog.h>

//#include <qapplication.h>
#include <qheader.h>
#include <qregexp.h>
#include <qdir.h>
#include <qdom.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace std;
#include <qstring.h>
#include <qobject.h>
#include <qdatetime.h>
#include <qlist.h>
#include <qvector.h>
#include <qtimer.h>
#include <qevent.h>

// my includes
#include "song.h"
#include "songentry.h"
#include "songentrystring.h"
#include "songentrytimestamp.h"

#include "songinfo.h"
#include "fuzzsrch.h"
#include "prefs.h"
#include "mylist.h"
#include "mydatetime.h"
#include "ConsistencyCheckParameter.h"


/**
 * this is the model of Yammi
 *	@author Brian O.Nlle
 */
class YammiModel : public QObject {
    Q_OBJECT
public:
	/**
	 * Constructor
   */
	YammiModel();
	/**
	 * Destructor
	 */
	~YammiModel();

	// data representation
	MyList allSongs; // all songs in database
	MyList problematicSongs; // problematic songs (in consistency check)
	MyList songHistory; // saved history of played songs
	MyList songsToPlay; // yammi playlist
	MyList songsPlayed; // songs played in this session
	MyList unclassifiedSongs; // songs not in any category/album/group

	QPtrList<MyList> allCategories; // categories
	QStringList	categoryNames; // names of categories
	
	int entriesAdded;
	int corruptSongs;
	bool _allSongsChanged;
  bool _categoriesChanged;

	// preferences
	Prefs config;
	bool noDatabaseFound;
	bool noPrefsFound;
	
	bool traverse(QString path, QString filePattern, QProgressDialog* progress, QString mediaName=0);
	Song* getSongFromFilename(QString filename);
	QString checkAvailability(Song* s, bool touch=false);

	bool allSongsChanged();
	void allSongsChanged(bool changed);

	void readCategories();
	void saveCategories();
	void newCategory(QString categoryName);
	void removeCategory(QString categoryName);
	void renameCategory(QString oldCategoryName, QString newCategoryName);
	
	bool categoriesChanged();
	void categoriesChanged(bool changed);

	void removeMedia(QString mediaToDelete);
	void renameMedia(QString oldMediaName, QString newMediaName);
		
	void readPreferences(QString baseDir);
	void savePreferences();
		
	void readSongDatabase();
	void saveSongDatabase();
	
	void readHistory();
	void saveHistory();

  void markPlaylists(Song* s);
	
	
public slots:
	void save();
	void saveAll();
	void updateSongDatabase(QString scanDir, QString filePattern, QString mediaName, QProgressDialog* progress);
	void addSongToDatabase(QString filename, QString mediaName);
	bool checkConsistency(QProgressDialog* progress, MyList* selection, ConsistencyCheckParameter* p);

protected:
  
};

#endif
