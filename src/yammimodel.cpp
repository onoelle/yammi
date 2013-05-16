/***************************************************************************
                          yammimodel.cpp  -  description
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

#include <QProgressDialog>
#include <QCheckBox>
#include <QDebug>
#include <QDomDocument>
#include <QMessageBox>
#include <QTextStream>
// TODO: avoid the gui-stuff includes in yammimodel

#include "yammimodel.h"
#include "yammigui.h"
#include "applytoalldialog.h"
#include "options.h"
#include "mediaplayer.h"
#include "ConsistencyCheckParameter.h"
#include "song.h"
#include "songentry.h"
#include "songentryint.h"
#include "songentrystring.h"
#include "songentrytimestamp.h"
#include "songinfo.h"
#include "fuzzsrch.h"
#include "prefs.h"
#include "mylist.h"
#include "mydatetime.h"
#include "util.h"


YammiModel::YammiModel( YammiGui *y ) {
    m_yammi = y;
}

YammiModel::~YammiModel() {}

Prefs* YammiModel::config()
{
    return &m_config;
}



/**
 * Reads from an xml file all existing songs into a given folder.
 * Not existing songs will be forgotten.
 * Creates a new category, if no folder given.
 */
bool YammiModel::readList(MyList* list, QString filename)
{
    QFile f(filename);
    if ( !f.open( QIODevice::ReadOnly ) ) {
        qCritical() << "Could not open file for reading:" << filename;
        return false;
    }
    QString errorMsg;
    int errorLine;
    int errorColumn;
    QDomDocument doc;
    if( !doc.setContent(&f, false, &errorMsg, &errorLine, &errorColumn) ) {
        QString msg = QString(tr("Error reading categories file:\n%1\n(Error: %2, line %3, column %4)") ).arg(f.name()).arg(errorMsg).arg(errorLine).arg(errorColumn);
        qCritical() << "Error reading file contents: " << filename << endl << msg;
        f.close();
        return false;
    }
    f.close();
    // get root element
    QDomElement e = doc.documentElement();
    if(e.tagName()!="category") {
        qCritical() << "Does not seem to be a category xml file: " << filename;
        return false;
    }

    QString name = e.attribute("name");

    if(list == 0) {
		// add category to database
    	list = new MyList;
    	allCategories.append(list);
    	categoryNames.append(name);
	}

    // add all songs contained
    e = e.firstChild().toElement( );
    int index=1;
    while( !e.isNull() ) {
        QString artist = e.attribute("artist");
        QString title  = e.attribute("title");
        QString album  = e.attribute("album");
        // search for item in allSongs
        Song* s = allSongs.getSongByKey(artist, title, album);
        if(!s) {
            qWarning() << "Song not found in database : " << artist << "/" << title << "/" << album;
        } else {
            SongEntryInt* toAdd = new SongEntryInt(s, index);
            list->append(toAdd);
            index++;
        }
        e = e.nextSibling().toElement();
    }
	return true;
}


/**
 * Reads in all xml-files found in the database directory.
 * Each xml file should represent one category.
 */
void YammiModel::readCategories() {
	QString categoryDir(config()->databaseDir + "categories");
    qDebug() << "Reading categories from " << categoryDir;

    int count=0;

	QDir d(categoryDir);
	if(!d.exists()) {
        qDebug() << "could not read categories: directory not existing: " << categoryDir;
		categoriesChanged(false);
		return;
	}
	QString filter("*.xml");
	QStringList cats = d.entryList(filter, QDir::Files, QDir::DirsFirst);

    int total=cats.count();
    QProgressDialog dia;
    dia.setModal(true);
    dia.setLabelText(tr("Loading categories"));
    dia.setRange(0, total);
    dia.setCancelButton(NULL);
    dia.setMinimumDuration(0);

    int progressCount=0;
    for( QStringList::Iterator it = cats.begin(); it!=cats.end(); ++it, progressCount++ ) {
        dia.setValue(progressCount);
        QString filename(d.absPath()+ "/" + (*it));
        qDebug() << "category found: " << filename;
		if(readList(0, filename)) {
			count++;
		}
    }
        qDebug() << count << " categories read";
    categoriesChanged(false);
}


// Reads song history
void YammiModel::readHistory() {
    QString filename = config()->databaseDir + "history.xml";
    qDebug() << "reading song history from " << filename;

    QFile f( filename );
    if ( !f.open( QIODevice::ReadOnly ) ) {
        qCritical() << "could not open history file : " << filename;
        return;
    }
    QDomDocument doc;
    if ( !doc.setContent( &f ) ) {
        qCritical() << "could not parse history file, incorrect xml format? " << filename;
        f.close();
        return;
    }
    f.close();

    QDomElement root = doc.documentElement();
    int total = root.attribute("count", "0").toInt( );
    QDomElement e = root.firstChild().toElement();

    QProgressDialog dia;
    dia.setModal(true);
    dia.setLabelText(tr("Loading song history"));
    dia.setRange(0, total);
    dia.setCancelButton(NULL);
    dia.setMinimumDuration(0);

    for(int i=0; !e.isNull(); i++) {
        if(i % 100==0) {
            dia.setValue(i);
        }
        QString artist = e.attribute("artist", "unknown");
        QString title  = e.attribute("title", "unknown");
        QString album  = e.attribute("album", "");
        QString timestamp = e.attribute("timestamp", "");

        // search for item in allSongs
        Song* s = allSongs.getSongByKey(artist, title, album);
        if(!s) {
            qWarning()<<"history item "<<artist<<"/"<<title<<"/"<<album<<" not found in song database";
        } else {
            MyDateTime played;
            played.readFromString(timestamp);
            s->lastPlayed=played;
            SongEntryTimestamp* hEntry=new SongEntryTimestamp(s, &played);
            songHistory.append(hEntry);
        }

        e = e.nextSibling().toElement();
    }
    qDebug()<<"done (" << songHistory.count() << " songs in history)";
}

/// saves the song history
void YammiModel::saveHistory() {
    QString path = config()->databaseDir;
    qDebug() << "Saving history file in directory" << path;

    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );    
    QDomElement root = doc.createElement("history");
    doc.appendChild(root);

    // iterate through songs in history AND songsPlayed folder
    // => save each song as a xml song element
    int count=0;
    for(SongEntry* entry=songHistory.first(); entry; entry=songHistory.next()) {
        QDomElement elem = doc.createElement( "song" );
        elem.setAttribute( "artist", entry->song()->artist );
        elem.setAttribute( "title", entry->song()->title );
        elem.setAttribute( "album", entry->song()->album );
        elem.setAttribute( "timestamp", ((SongEntryTimestamp*)entry)->timestamp.writeToString() );
        root.appendChild( elem );
        count++;
    }
    for(SongEntry* entry=songsPlayed.first(); entry; entry=songsPlayed.next()) {
        QDomElement elem = doc.createElement( "song" );
        elem.setAttribute( "artist", entry->song()->artist );
        elem.setAttribute( "title", entry->song()->title );
        elem.setAttribute( "album", entry->song()->album );
        elem.setAttribute( "timestamp", ((SongEntryTimestamp*)entry)->timestamp.writeToString() );
        root.appendChild( elem );
        count++;
    }
    root.setAttribute("count",count);

    // save history to file... (but first we make a backup of old file)
    QDir dir(path);
    if(dir.exists("history.xml") && dir.rename( path + "history.xml", path +"history_backup.xml")) {
        qDebug() << "backup of history saved in \"history_backup.xml\"";
    }
    

    QString filename(path + "history.xml");
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly)) {
        qCritical() << "could not open file for writing:" << filename;
        return;
    }
    QTextStream str(&file);
    str.setEncoding(QTextStream::UnicodeUTF8);
    doc.save(str, 2);
    file.close();
    qDebug() << "done";
}


/// save categories (if changed) to xml-files
void YammiModel::saveCategories() {
    QString path = config()->databaseDir + "categories/";
	QDir dir(path);
	if(!dir.exists()) {
        qDebug() << "creating directory " << path;
        dir.mkpath(path);
	}
    qDebug() << "saving dirty categories to directory " << path;

    // save all categories marked as dirty
    QString categoryName;    
    int index=0;
    for(MyList* category=allCategories.first(); category; category=allCategories.next(), index++) {
        categoryName = categoryNames[index];
        qDebug() << "category " << categoryName;
        if(!category->dirty) {
                qDebug() << "...clean";
		continue;
	}
        qDebug() << "...dirty, saving...";
	saveList(category, path, categoryName);
        category->dirty=false;
    }
    categoriesChanged(false);
}


bool YammiModel::saveList(MyList* list, QString path, QString filename)
{
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
    QDomElement root = doc.createElement("category");
    root.setAttribute("name", filename);
    doc.appendChild(root);

    for(Song* s = list->firstSong(); s; s=list->nextSong()) {
        QDomElement elem = doc.createElement( "song" );
        elem.setAttribute( "artist", s->artist );
        elem.setAttribute( "title", s->title );
        elem.setAttribute( "album", s->album );
        root.appendChild( elem );
    }
    // save to file...
    QString absoluteFilename(path + "/" + filename + ".xml");
    QFile file(absoluteFilename);
    if(!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Could not save folder " << absoluteFilename;
        return false;
    }
    QTextStream str(&file);
    str.setEncoding(QTextStream::UnicodeUTF8);
    doc.save(str, 2);
    file.close();
    return true;
}



// reads the yammi database (an xml-file with all song information)
void YammiModel::readSongDatabase(  ) {
    QString filename(config()->databaseDir + "songdb.xml");
        qDebug() << "reading song database from: " << filename;
    QFile f(filename);
    if( !f.open(QIODevice::ReadOnly) ) {
                qWarning() << "could not read song database from " << filename;
        return;
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if( !doc.setContent(&f, false, &errorMsg, &errorLine, &errorColumn) ) {
        f.close();
        QString msg = QString(tr("Error reading database file:\n%1\n(Error: %2, line %3, column %4)") ).arg(f.name()).arg(errorMsg).arg(errorLine).arg(errorColumn);
        QMessageBox::warning(NULL, tr("Error reading database"), msg, QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }
    f.close();
    QDomElement root = doc.documentElement( );
    QString version = root.attribute("yammiVersion", "no version");
    if(version=="0.5.2" || version=="0.5.1" || version=="0.5" || version=="no version") {
        QString msg( tr("Your Song Database seems to be very old.\n You might need to create \
                          a new Database and scan your harddisk for songs") );
        QMessageBox::warning(NULL, tr("Unknown Song Database version"), msg, QMessageBox::Ok, QMessageBox::NoButton);
        //try to continue anyway...?
    }

    int total = root.attribute("count", "0").toInt( );
    QDomElement e = root.firstChild().toElement();


    QProgressDialog dia;
    dia.setModal(true);
    dia.setLabelText(tr("Loading database"));
    dia.setRange(0, total);
    dia.setCancelButton(NULL);
    dia.setMinimumDuration(0);

    int count = 0;
    while( !e.isNull( ) ) {
        if(count % 100==0) {
            dia.setValue( count );
        }

        QString artist   = e.attribute("artist", "unknown");
        QString title    = e.attribute("title", "unknown");
        QString album    = e.attribute("album", "");
        QString filename = e.attribute("filename", "");
        QString path     = e.attribute("path", "");
        QString comment  = e.attribute("comment", "");
        QString genre    = e.attribute("genre", "");

        unsigned long length = e.attribute("length","0").toULong( );
        int bitrate = e.attribute("bitrate", "0").toInt( );
        int year = e.attribute("year", "0").toInt( );
        int trackNr = e.attribute("trackNr", "0").toInt( );
        
        // read date as "dd/mm/yyyy, hh:mm:ss"
        MyDateTime addedTo;
        addedTo.readFromString(e.attribute("addedTo", "1/1/1970/00:00:00"));

        Song* s = new Song(artist, title, album, filename, path, length, bitrate, addedTo, year, comment, trackNr, genre);
        allSongs.appendSong( s );

        e = e.nextSibling().toElement();
        count++;
    }
    qDebug() << "Read " << count << " songs from database";
    allSongsChanged( false );
}

/// saves the songs in allSongs into an xml-file
void YammiModel::saveSongDatabase() {
    qDebug() << "saving database...";
    QDomDocument doc;
    doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
    QDomElement root = doc.createElement("songs");

    root.setAttribute("yammiVersion", m_yammi->config()->yammiVersion);
    doc.appendChild(root);

    // iterate through songs and save each song as a xml song element
    int count=0;
    for(Song* s = allSongs.firstSong(); s; s = allSongs.nextSong()) {
        count++;
        QDomElement elem = doc.createElement( "song" );
        if(s->artist!="unknown")
            elem.setAttribute( "artist", s->artist );
        if(s->title!="unknown")
            elem.setAttribute( "title", s->title );
        if(s->album!="")
            elem.setAttribute( "album", s->album);
        if(s->bitrate!=0)
            elem.setAttribute( "bitrate", QString("%1").arg(s->bitrate));
        if(s->comment!="")
            elem.setAttribute( "comment", s->comment );
        if(s->filename!="")
            elem.setAttribute( "filename", s->filename );
        if(s->length!=0)
            elem.setAttribute( "length", QString("%1").arg(s->length));
        if(s->path!="")
            elem.setAttribute( "path", s->path);
        if(s->year!=0)
            elem.setAttribute( "year", QString("%1").arg(s->year) );
        if(s->trackNr!=0)
            elem.setAttribute( "trackNr", QString("%1").arg(s->trackNr));
        if(s->genre!="")
            elem.setAttribute( "genre", QString("%1").arg(s->genre));

        elem.setAttribute( "addedTo", s->addedTo.writeToString());
        root.appendChild( elem );
    }
    root.setAttribute("count",count);

    // save songdb to file... (but first we make a backup of old file)
	QString filename = config()->databaseDir + "songdb.xml";
    qDebug() << "Saving song database to file " << filename;
    QDir dir(config()->databaseDir);
    if(dir.exists("songdb.xml") && !dir.rename(filename, filename + "_backup")) {
        qDebug() << "could not make backup of file " << filename;
    }

    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly)) {
        qCritical() << "could not open file for writing:" << filename;
        return;
    }
    QTextStream str(&file);
    str.setEncoding(QTextStream::UnicodeUTF8);
    doc.save(str, 2);
    file.close();

    qDebug() << "saving database: done";
    allSongsChanged(false);
}



void YammiModel::allSongsChanged(bool changed) {
    _allSongsChanged = changed;
    //  if(m_yammi && m_yammi->tbSaveDatabase)
    //    m_yammi->tbSaveDatabase->setEnabled(_allSongsChanged || _categoriesChanged);
}

bool YammiModel::allSongsChanged() {
    return _allSongsChanged;
}

void YammiModel::categoriesChanged(bool changed) {
    _categoriesChanged = changed;
    //  if(m_yammi && m_yammi->tbSaveDatabase)
    //  	m_yammi->tbSaveDatabase->setEnabled(_allSongsChanged || _categoriesChanged);
}

bool YammiModel::categoriesChanged() {
    return _categoriesChanged;
}



//   Updates the xml-database by scanning harddisk
void YammiModel::updateSongDatabase(QString scanDir, bool followSymLinks, QString filePattern, QProgressDialog* progress) {
    entriesAdded=0;
    corruptSongs=0;
    if(m_yammi->config()->childSafe) {
        qWarning()<<"updateSongDatabase : childSafe --> request denied";
        return;
    }
    problematicSongs.clear();
    qDebug()<<"scanning harddisk for new songs...";
    // check that scanDir is an existing directory
    QDir d(scanDir);
    if(!d.exists()) {
        QString msg( tr("The base directory for scanning does not exist!\n\
                          Set value \"scanDir\" to an existing directory!"));
        QMessageBox::warning(NULL, "", msg, QMessageBox::Ok, Qt::NoButton);
        return;
    } else {
        traverse(scanDir, followSymLinks, filePattern, progress);
        qDebug() << "finished scanning!";
    }
}

void YammiModel::updateSongDatabase(QStringList list) {
    entriesAdded=0;
    corruptSongs=0;
    if(m_yammi->config()->childSafe) {
        qWarning()<<"updateSongDatabase : childSafe --> request denied";
        return;
    }
    problematicSongs.clear();
    // iterate over list of songs to add
    for( QStringList::Iterator it = list.begin(); it!= list.end(); ++it ) {
        QString filename(*it);
        if(filename.endsWith(".m3u")) {
            QStringList* playlist = readM3uFile(filename);
            for(QStringList::Iterator it2 = playlist->begin();it2 != playlist->end();++it) {
                addSongToDatabase(QString(*it2));
            }
            delete playlist;
        } else {
            addSongToDatabase(filename);
        }
    }
}



/// traverses a directory recursively and processes all mp3 files
/// returns false, if scanning was cancelled
bool YammiModel::traverse(QString path, bool followSymLinks, QString filePattern, QProgressDialog* progress) {
    // leave out the following directories
    if(path+"/"==m_yammi->config()->trashDir) {
        qWarning() << "skipping trash directory: " << path;
        return true;
    }
    qDebug() << "scanning directory " << path;
    progress->setLabelText(QString(tr("scanning directory %1 ...")).arg(path));
    progress->setValue(0);

    QDir d(path);

    // step 1: scan files
    d.setFilter(QDir::Files | QDir::Hidden);
    d.setNameFilters(filePattern.split(QChar(' ')));
    d.setSorting( QDir::Name );
    QFileInfoList list = d.entryInfoList();
    progress->setRange(0, list.count());
    int filesScanned=0;
    for(QFileInfoListIterator it = list.begin(); it != list.end(); ++it ) {
        filesScanned++;
        progress->setValue(filesScanned);
        if(progress->wasCanceled()) {
            return false;
        }
        // okay, we have a file to scan, try to add to database
        addSongToDatabase(it->filePath());
    }

    // step 2: recursively scan subdirectories
    QDir d2(path);
    d2.setFilter(QDir::Dirs | QDir::Readable  | QDir::Hidden);
    d2.setSorting( QDir::Name );
    QFileInfoList list2 = d2.entryInfoList();
    for(QFileInfoListIterator it2 = list2.begin(); it2 != list2.end(); ++it2) {
        if(it2->fileName()=="." || it2->fileName()=="..") {
            continue;
        }
        if(followSymLinks == false) {
            if(it2->isSymLink()) {
                qDebug() << "skipping symlink " << it2->filePath();
                continue;
            }
        }
        if(traverse(it2->filePath(), followSymLinks, filePattern, progress) == false) {
            return false;
        }
    }
    return true;
}

/**
 * fix incorrect genres by re-reading them from whole song database
 */
void YammiModel::fixGenres(QProgressDialog* progress) {
    int i = 0;
    for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong(), i++) {
        progress->setValue(i);
        if(progress->wasCanceled()) {
            qDebug() << "fixing genres aborted...";
            return;
        }
        
        qDebug() << "trying to fix genre in file " << s->location();
        Song* fixSong=new Song();
        bool success = fixSong->create(s->location(), config()->capitalizeTags);
        if(success) {
            qDebug() << "old genre saved in yammi database: " << s->genre << ", fixed genre: " << fixSong->genre;
            s->genre = fixSong->genre;
            allSongsChanged(true);
        }
        delete(fixSong);
    }
}

// adds a single songfile to the database
void YammiModel::addSongToDatabase(QString filename) {
    qDebug() << "scanning file '" << filename << "'...";
    bool found=false;
    for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong()) {
        // this check might fail when filename has strange characters?
        if(filename == s->location()) {
            found=true;
            /*
            // here we can fix/update our database with additional info...
            // eg: fix genre like this:
            if(s->genre == "") {
                qDebug() << "trying to fix genre in file " << filename;
                Song* fixSong=new Song();
                bool success = fixSong->create(filename, "", config()->capitalizeTags);
                if(success) {
                    qDebug() << "old genre: " << s->genre << ", fixed genre: " << fixSong->genre;
                    s->genre = fixSong->genre;
                    allSongsChanged(true);
                }
                delete(fixSong);
            }
            */
            break;
        }
    }
    if(found) {
        return;
    }

    // okay, new song (at least new filename/path) => construct song object
    Song* newSong = new Song();
    newSong->create(filename, m_yammi->config()->capitalizeTags);
    if(newSong->corrupted) {
        qCritical() << "new song file " << filename << " is corrupt (not readable for yammi), skipping";
        corruptSongs++;
        return;
    }
    // check whether other version of this song existing (with same primary key)
    Song* s = allSongs.getSongByKey(newSong->artist, newSong->title, newSong->album);
    if(s!=0) {
        // yes, song with this key already existing!
        if(newSong->length==s->length && newSong->bitrate==s->bitrate && newSong->album==s->album) {
            // a) okay, we assume it is exactly the same song...

            // case 1: scanning harddisk
            if(s->filename=="") {
                // case 1a: song has no filename = was not available on harddisk
                // => make it available (at new location)
                s->setTo(newSong);
                allSongsChanged(true);
                delete(newSong);
                return;
            } else {
                // case 1b: song has filename
                QFileInfo fileInfo(s->location());
                if(!fileInfo.exists()) {
                    // but still not available => probably has been moved
                    qWarning() << "looks like file " << newSong->filename << " has been moved from " << s->path << " to " << newSong->path << ", correcting path info";
                    s->setTo(newSong);
                    allSongsChanged(true);
                    delete(newSong);
                    return;
                }
                // song is available, we don't need two songs => skip
                qWarning() << "file " << newSong->filename << " already available at " << s->location() << ", skipping";
                delete(newSong);
                return;
            }
        } else {
            // b) not exactly the same => add as new (change title to make it unique)
            // here we do not have to distinguish between harddisk and media
            qWarning() << "seems like new song '" << newSong->artist << " - " << newSong->title << "' already existing..." << endl << "(Yammi does not allow two songs with the same artist/title/album identification)";
            int tryNo=2;
            QString extTitle;
            for(bool notUnique=true; notUnique; tryNo++) {
                extTitle=newSong->title+QString("(%1)").arg(tryNo);
                notUnique=(allSongs.getSongByKey(s->artist, extTitle, s->album)!=0);
            }

            newSong->title=extTitle;
            problematicSongs.append(new SongEntryString(newSong, QString("song existing, appended with (%1)").arg(tryNo-1)));
            problematicSongs.append(new SongEntryString(s, "existing song"));
        }
    }
    // new song, not in database yet
    allSongs.appendSong(newSong);
    qDebug() << "Song added to database: " << newSong->displayName();
    entriesAdded++;
    allSongsChanged(true);
}

/**
 * Read an m3u file, returning a list of strings
 */
QStringList* YammiModel::readM3uFile(QString filename) {
    QStringList* list=new QStringList();
    QFile file(filename);
    if (!file.open( QIODevice::ReadOnly  ) )
        return list;
    QTextStream stream(&file);
    while(!stream.atEnd()) {
        QString line=stream.readLine().stripWhiteSpace();
        if(line.startsWith("#")) {
            // skip lines starting with #
            continue;
        }
        list->append(line);
    }
    file.close();
    return list;
}


void YammiModel::removeCategory(QString categoryName) {
    qDebug() << "remove category: " << categoryName;
    QString name=categoryNames.first();
    int i=0;
    for(MyList* ptr=allCategories.first(); ptr; ptr=allCategories.next(), i++) {
        QString name=categoryNames[i];
        if(name==categoryName) {
            qDebug() << "category found, deleting...";
            allCategories.remove();
            categoryNames.remove(categoryNames.at(i));
            QString file = config()->databaseDir + "categories/" + name + ".xml";
            qDebug() << "file to delete:" << file;
            QDir d;
			if(!d.remove(file)) {
                                qWarning() << "could not delete a category file: " << file;
			}
            break;
        }
    }
}

void YammiModel::renameCategory(QString oldCategoryName, QString newCategoryName) {
    int i=0;
    for(MyList* ptr=allCategories.first(); ptr; ptr=allCategories.next(), i++) {
        QString name=categoryNames[i];
        if(name==oldCategoryName) {
            qDebug() << "renaming category..";
            categoryNames[i]=newCategoryName;
            ptr->dirty=true;
            categoriesChanged(true);
            QDir dir;
            QString path = config()->databaseDir + "categories/";
            if(!dir.rename(path+oldCategoryName+".xml", path+newCategoryName+".xml"))
                qCritical() << "could not rename category file:" << path << oldCategoryName << ".xml";
            break;
        }
    }
}

void YammiModel::newCategory(QString name) {
    MyList* newList = new MyList;
    allCategories.append(newList);
    categoryNames.append(name);
    newList->dirty=true;
    categoriesChanged(true);
}


/** save all
 * (needed for update / correcting xml-file format)
 * marks everything as dirty and invokes save()
 */
void YammiModel::saveAll() {
    categoriesChanged(true);
    allSongsChanged(true);
    int i=0;
    for(MyList* ptr=allCategories.first(); ptr; ptr=allCategories.next(), i++) {
        ptr->dirty=true;
    }
    save();
}

/**
 * saves changed information (categories + songDatabase)
 */
void YammiModel::save() {
    qDebug() << "model save()";
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    // save dirty categories
    saveCategories();
    if(allSongsChanged()) {
        saveSongDatabase();
    }
    if(allSongsChanged() || m_yammi->config()->logging) {
        saveHistory();
    }
    QApplication::restoreOverrideCursor();
}


/**
 * Marks those playlists as dirty that contain the given song
 */
void YammiModel::markPlaylists(Song* s) {
    for(MyList* category=allCategories.first(); category; category=allCategories.next()) {
        if(category->containsSong(s)) {
            category->dirty=true;
	}
    }
}


// finds out the corresponding song entry given a filename
// (now also takes care of songs in swap dir)
// returns 0 if no song entry found
Song* YammiModel::getSongFromFilename(QString filename) {
    // strip filename to relative name
    int pos=filename.findRev('/', -1);
    QString lookFor=filename.right(filename.length()-pos-1);

    for(SongEntry* entry=allSongs.first(); entry; entry=allSongs.next()) {
        if(entry->song()->filename==lookFor)
            return entry->song();
    }

    return 0;
}


/**
 * Checks whether a song is available on the local harddisk
 * If song is available, returns the absolute path+filename to the songfile.
 * If not yet available, returns "".
 * If song will never be available because it is not contained on any media, returns "never".
 */
QString YammiModel::checkAvailability(Song* s) {
    if(s->location()!="/") {
        QFileInfo fi(s->location());
        if(fi.exists() && fi.isReadable()) {
            return s->location();
        }
    }
    return "never";
}

/**
 * Remove all those thongs from top of playlist that are unplayable:
 * - swapped songs not on harddisk
 * - those that match unplayable file mask ("*.txt")
 * Does NOT emit the playlistChanged() signal!
 */
bool YammiModel::skipUnplayableSongs(bool firstTwo) {
    qDebug() << "skipUnplayableSongs()";
    QString location;
    bool songsRemoved = false;
    while( songsToPlay.at(0) ) {
        location = checkAvailability( songsToPlay.at(0)->song() );
        if( location == "" || location == "never" ) {
            qDebug() << "Song " << songsToPlay.at(0)->song()->displayName() << "not available, skipping";
            songsToPlay.removeFirst();
            songsRemoved = true;
        } else {
            break;
        }
    }
    if(firstTwo) {
        while( songsToPlay.at(1) ) {
            location = checkAvailability( songsToPlay.at(1)->song() );
            if( location == "" || location == "never" ) {
                qDebug() << "Song " << songsToPlay.at(1)->song()->displayName() << "not available, skipping";
                songsToPlay.remove(1);
                songsRemoved = true;
            } else {
                break;
            }
        }
    }    
    return songsRemoved;
}


