/***************************************************************************
                          yammimodel.h  -  description
                             -------------------
    begin                : Sun Oct 7 2001
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

#ifndef YAMMIMODEL_H
#define YAMMIMODEL_H

#include <QObject>
#include <QStringList>

#include "mylist.h"

class QProgressDialog;

class YammiGui;
class ConsistencyCheckParameter;
class Prefs;


/**
 * this is the model of Yammi
 *	@author Brian O.Nölle
 */
//This is actually Yammi's Database....
class YammiModel : public QObject {
    Q_OBJECT
public:
    /**
     * Constructor
     */
    YammiModel( YammiGui *y );
    /**
     * Destructor
     */
    ~YammiModel();

    Prefs* config();

    // data representation
    QString currentSongFilenameAtStartPlay;
    MyList allSongs; // all songs in database
    MyList problematicSongs; // problematic songs (in consistency check)
    MyList songHistory; // saved history of played songs
    MyList songsToPlay; // yammi playlist
    MyList songsPlayed; // songs played in this session
    MyList unclassifiedSongs; // songs not in any category/album/group
    MyList recentSongs; // songs recently added

    QList<MyList*> allCategories; // categories
    QStringList	categoryNames; // names of categories

    int entriesAdded;
    int corruptSongs;
    bool _allSongsChanged;
    bool _categoriesChanged;


    bool traverse(QString path, bool followSymLinks, QString filePattern, QProgressDialog* progress);
    Song* getSongFromFilename(QString filename);
    QString checkAvailability(Song* s);
    bool skipUnplayableSongs(bool firstTwo=false);    

    bool allSongsChanged();
    void allSongsChanged(bool changed);

    void readCategories();
    bool readList(MyList* list, QString filename);
    void saveCategories();
    bool saveList(MyList* list, QString path, QString filename);

    void newCategory(QString categoryName);
    void removeCategory(QString categoryName);
    void renameCategory(QString oldCategoryName, QString newCategoryName);

    bool categoriesChanged();
    void categoriesChanged(bool changed);

    void readSongDatabase(  );
    void saveSongDatabase();

    void readHistory();
    void saveHistory();

    void markPlaylists(Song* s);
    QStringList* readM3uFile(QString filename);


public slots:
    void save();
    void saveAll();
    /**
    * Updates the database by scanning harddisk
    *  - if specified, checks existence of files in databse and updates/deletes entries
    *  - scans recursively, starting from specified scanDir
    *  - constructs song objects from all files matching the filePattern
    *  - checks whether already existing, whether modified, if not => inserts into database */
    void updateSongDatabase(QString scanDir, bool followSymLinks, QString filePattern, QProgressDialog* progress);
    void updateSongDatabase(QStringList list);
    void addSongToDatabase(QString filename);
    void fixGenres(QProgressDialog* progress);

protected:
    YammiGui *m_yammi;
    Prefs* m_config;
};

#endif
