/***************************************************************************
                          yammimodel.h  -  description
                             -------------------
    begin                : Sun Oct 7 2001
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

#ifndef YAMMIMODEL_H
#define YAMMIMODEL_H


/**
	* this is the model of Yammi
  *	@author Brian O.Nlle
  */

// these are the includes I wanted to avoid (gui-stuff)
#include <qprogressdialog.h>

//#include <qapplication.h>
#include <qheader.h>
#include <qregexp.h>
#include <qdir.h>
#include <qdom.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <qstring.h>
#include <qobject.h>
#include <qdatetime.h>
#include <qlist.h>
#include <qvector.h>
#include <qtimer.h>
#include <qevent.h>

// xmms control
#include <xmmsctrl.h>

// my includes
#include "song.h"
#include "songentry.h"
#include "songentrystring.h"
#include "songentrytimestamp.h"

#include "songinfo.h"
#include "fuzzsrch.h"
#include "mp3tag.h"
#include "prefs.h"
#include "mylist.h"
#include "mydatetime.h"


class YammiModel : public QObject {
    Q_OBJECT
public:
	YammiModel();
	~YammiModel();

	// data representation
	MyList		allSongs;										// all songs in database
	MyList		problematicSongs;						// problematic songs (in consistency check)
	MyList		songHistory;								// saved history of played songs
	MyList 		songsToPlay;								// yammi playlist
	MyList		songsPlayed;								// songs played in this session
	MyList 		unclassifiedSongs;					// songs not in any category/album/group

	QPtrList<MyList> allCategories;					// categories
	QStringList	categoryNames;							// names of categories
	
	int				songsAdded;
	int				corruptSongs;
	bool			_allSongsChanged;
  bool			_categoriesChanged;

	// preferences
	Prefs config;
	bool			noPrefsFound;
	bool			noDatabaseFound;
	
	void 			traverse(QString path, QString filePattern, QProgressDialog* progress, QString mediaName=0);

	bool			allSongsChanged();
	void			allSongsChanged(bool changed);

	void 			readCategories();
	void 			saveCategories();
	void			newCategory(QString categoryName);
	void			removeCategory(QString categoryName);
	void			renameCategory(QString oldCategoryName, QString newCategoryName);
	
  bool			categoriesChanged();
  void			categoriesChanged(bool changed);

  void			removeMedia(QString mediaToDelete);
	void			renameMedia(QString oldMediaName, QString newMediaName);
		
	void 			readPreferences();
	void 			savePreferences();
		
	void 			readSongDatabase();
	void 			saveSongDatabase();
	
	void			readHistory();
	void			saveHistory();

  void			markPlaylists(Song* s);
	
	
public slots:
	void			save();
	void			saveAll();
	void			updateSongDatabase(bool checkExistence, QString scanDir, QString filePattern, QString mediaName, QProgressDialog* progress);
	void			addSongToDatabase(QString filename, QString mediaName);
	bool			checkConsistency(QProgressDialog* progress);

protected:
	QString		getProperty(const QDomDocument* doc, const QString propName, const QString propDefault="");	// remove default???
	int				getProperty(const QDomDocument* doc, const QString propName, const int propDefault);
	bool			getProperty(const QDomDocument* doc, const QString propName, const bool propDefault);
	QStringList* getProperty(const QDomDocument* doc, const QString propName, QStringList* propDefault);
	
	void			setProperty(QDomDocument* doc, const QString propName, const QString propValue);
	void			setProperty(QDomDocument* doc, const QString propName, const int propValue);
	void			setProperty(QDomDocument* doc, const QString propName, const bool propValue);
	void 			setProperty(QDomDocument* doc, const QString propName, const QStringList* propValue);

	bool			startFirstTime();
};

#endif
