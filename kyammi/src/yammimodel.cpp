/***************************************************************************
                          yammimodel.cpp  -  description
                             -------------------
    begin                : Sun Oct 7 2001
    copyright            : (C) 2001 by Brian O.Nlle
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

#include "yammimodel.h"

#include <kapplication.h>
#include <kstatusbar.h>
#include <kprogress.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>


#include <qdom.h>

// TODO: avoid the following includes in yammimodel (because it is gui-stuff)
#include <qcheckbox.h>
#include <qeventloop.h>


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
    if ( !f.open( IO_ReadOnly ) ) {
        kdError() << "Could not open file for reading:" << filename << endl;
        return false;
    }
    QString errorMsg;
    int errorLine;
    int errorColumn;
    QDomDocument doc;
    if( !doc.setContent(&f, false, &errorMsg, &errorLine, &errorColumn) ) {
        QString msg = QString(i18n("Error reading categories file:\n%1\n(Error: %2, line %3, column %4)") ).arg(f.name()).arg(errorMsg).arg(errorLine).arg(errorColumn);
        kdError() << "Error reading file contents: " << filename << endl << msg << endl;
        f.close();
        return false;
    }
    f.close();
    // get root element
    QDomElement e = doc.documentElement();
    if(e.tagName()!="category") {
        kdError() << "Does not seem to be a category xml file: " << filename << endl;
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
            kdWarning() << "Song not found in database : " << artist << "/" << title << "/" << album << endl;
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
    kdDebug() << "Reading categories from " << categoryDir << endl;

    int count=0;

	QDir d(categoryDir);
	if(!d.exists()) {
    	kdDebug() << "could not read categories: directory not existing: " << categoryDir << endl;
		categoriesChanged(false);
		return;
	}
	QString filter("*.xml");
	QStringList cats = d.entryList(filter, QDir::Files, QDir::DirsFirst);

	int total=cats.count();
    KProgressDialog dia(0,0,i18n("Loading categories"),i18n("Reading categories..."), true);
    dia.setAllowCancel(false);
    dia.setMinimumDuration(0);
    KProgress *p = dia.progressBar();
    p->setTotalSteps( total );
    int progressCount=0;
    for( QStringList::Iterator it = cats.begin(); it!=cats.end(); ++it, progressCount++ ) {
        p->setProgress( progressCount );
        kapp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
		QString filename(d.absPath()+ "/" + (*it));
        kdDebug() << "category found: " << filename << endl;
		if(readList(0, filename)) {
			count++;
		}
    }
	kdDebug() << count << " categories read" << endl;
    categoriesChanged(false);
}


// Reads song history
void YammiModel::readHistory() {
    QString filename = config()->databaseDir + "history.xml";
    kdDebug() << "reading song history from " << filename << endl;

    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) ) {
        kdError() << "could not open history file : " << filename << endl;
        return;
    }
    QDomDocument doc;
    if ( !doc.setContent( &f ) ) {
        kdError() << "could not parse history file, incorrect xml format? " << filename << endl;
        f.close();
        return;
    }
    f.close();

    QDomElement root = doc.documentElement();
    int total = root.attribute("count", "0").toInt( );
    QDomElement e = root.firstChild().toElement();

    KProgressDialog dia(0,0,i18n("Loading song history"),i18n("Reading song history..."), true);
    dia.setAllowCancel(false);
    dia.setMinimumDuration(0);
    KProgress *p = dia.progressBar();
    p->setTotalSteps( total );
    for(int i=0; !e.isNull(); i++) {
        if(i % 100==0) {
            p->setProgress( i );
            kapp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
        }
        QString artist = e.attribute("artist", "unknown");
        QString title  = e.attribute("title", "unknown");
        QString album  = e.attribute("album", "");
        QString timestamp = e.attribute("timestamp", "");

        // search for item in allSongs
        Song* s = allSongs.getSongByKey(artist, title, album);
        if(!s) {
            kdWarning()<<"history item "<<artist<<"/"<<title<<"/"<<album<<" not found in song database"<<endl;
        } else {
            MyDateTime played;
            played.readFromString(timestamp);
            s->lastPlayed=played;
            SongEntryTimestamp* hEntry=new SongEntryTimestamp(s, &played);
            songHistory.append(hEntry);
        }

        e = e.nextSibling().toElement();
    }
    kdDebug()<<"done (" << songHistory.count() << " songs in history)"<<endl;
}

/// saves the song history
void YammiModel::saveHistory() {
    QString path = config()->databaseDir;
    kdDebug() << "Saving history file in directory" << path << endl;

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
        kdDebug() << "backup of history saved in \"history_backup.xml\"" << endl;
    }
    

    QString filename(path + "history.xml");
    QFile file(filename);
    if(!file.open(IO_WriteOnly)) {
        kdError() << "could not open file for writing:" << filename << endl;
        return;
    }
    QTextStream str(&file);
    str.setEncoding(QTextStream::UnicodeUTF8);
    doc.save(str, 2);
    file.close();
    kdDebug() << "done" << endl;
}


/// save categories (if changed) to xml-files
void YammiModel::saveCategories() {
    QString path = config()->databaseDir + "categories/";
	QDir dir(path);
	if(!dir.exists()) {
		kdDebug() << "creating directory " << path << endl;
		dir.mkdir(path, true);
	}
    kdDebug() << "saving dirty categories to directory " << path << endl;

    // save all categories marked as dirty
    QString categoryName;    
    int index=0;
    for(MyList* category=allCategories.first(); category; category=allCategories.next(), index++) {
        categoryName = categoryNames[index];
        kdDebug() << "category " << categoryName << endl;
        if(!category->dirty) {
        	kdDebug() << "...clean\n";
		continue;
	}
        kdDebug() << "...dirty, saving..." << endl;        
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
    if(!file.open(IO_WriteOnly)) {
        kdError() << "Could not save folder " << absoluteFilename << endl;
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
	kdDebug() << "reading song database from: " << filename << endl;
    QFile f(filename);
    if( !f.open(IO_ReadOnly) ) {
		kdWarning() << "could not read song database from " << filename << endl;
        return;
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if( !doc.setContent(&f, false, &errorMsg, &errorLine, &errorColumn) ) {
        f.close();
        QString msg = QString(i18n("Error reading database file:\n%1\n(Error: %2, line %3, column %4)") ).arg(f.name()).arg(errorMsg).arg(errorLine).arg(errorColumn);
        KMessageBox::error(0L,msg,i18n("Error reading database"));
        return;
    }
    f.close();
    QDomElement root = doc.documentElement( );
    QString version = root.attribute("yammiVersion", "no version");
    if(version=="0.5.2" || version=="0.5.1" || version=="0.5" || version=="no version") {
        QString msg( i18n("Your Song Database seems to be very old.\n You might need to create \
                          a new Database and scan your harddisk for songs") );
        KMessageBox::sorry( 0L, msg, i18n("Unknown Song Database version") );
        //try to continue anyway...?
    }

    int total = root.attribute("count", "0").toInt( );
    QDomElement e = root.firstChild().toElement();

    KProgressDialog dia(0,0,i18n("Loading database"),i18n("Reading Song Database file"), true);
    dia.setAllowCancel(false);
    dia.setMinimumDuration(0);
    KProgress *p = dia.progressBar();
    p->setTotalSteps( total );
    int count = 0;
    while( !e.isNull( ) ) {
        if(count % 100==0) {
            p->setProgress( count );
            kapp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
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

        QDomNode child = e.firstChild();
        while( !child.isNull() ) {
            QDomElement mediaElem = child.toElement(); // try to convert the node to an element.
            if( !mediaElem.isNull() ) {
                QString mediaName=mediaElem.attribute("mediaName", "unspecified media");
                QString mediaLocation=mediaElem.attribute("mediaLocation", "unspecified location");
                s->addMediaLocation(mediaName, mediaLocation);
            }
            child = child.nextSibling();
        }

        e = e.nextSibling().toElement();
        count++;
    }
    kdDebug() << "Read " << count << " songs from database" << endl;
    allSongsChanged( false );
}

/// saves the songs in allSongs into an xml-file
void YammiModel::saveSongDatabase() {
    kdDebug() << "saving database..." << endl;
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
        for(unsigned int i=0; i<s->mediaName.count(); i++) {
            QDomElement media = doc.createElement( "media" );
            media.setAttribute( "mediaName", s->mediaName[i]);
            media.setAttribute( "mediaLocation", s->mediaLocation[i]);
            elem.appendChild(media);
        }
        root.appendChild( elem );
    }
    root.setAttribute("count",count);

    // save songdb to file... (but first we make a backup of old file)
	QString filename = config()->databaseDir + "songdb.xml";
    kdDebug() << "Saving song database to file " << filename << endl;
    QDir dir(config()->databaseDir);
    if(dir.exists("songdb.xml") && !dir.rename(filename, filename + "_backup")) {
        kdDebug() << "could not make backup of file " << filename << endl;
    }

    QFile file(filename);
    if(!file.open(IO_WriteOnly)) {
        kdError() << "could not open file for writing:" << filename << endl;
        return;
    }
    QTextStream str(&file);
    str.setEncoding(QTextStream::UnicodeUTF8);
    doc.save(str, 2);
    file.close();

    kdDebug() << "saving database: done" << endl;
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
void YammiModel::updateSongDatabase(QString scanDir, bool followSymLinks, QString filePattern, QString mediaName, KProgressDialog* progress) {
    entriesAdded=0;
    corruptSongs=0;
    if(m_yammi->config()->childSafe) {
        kdWarning()<<"updateSongDatabase : childSafe --> request denied"<<endl;
        return;
    }
    problematicSongs.clear();
    if(mediaName==0) {
        kdDebug()<<"scanning harddisk for new songs..."<<endl;
        // check that scanDir is an existing directory
        QDir d(scanDir);
        if(!d.exists()) {
            QString msg( i18n("The base directory for scanning does not exist!\n\
                              Set value \"scanDir\" to an existing directory!"));
            KMessageBox::error( m_yammi, msg);
            return;
        } else {
            traverse(scanDir, followSymLinks, filePattern, progress);
            kdDebug() << "finished scanning!" << endl;
        }
    } else {
        kdDebug() << "scanning removable media for new songs..." << endl;

        // mount media dir
        if(m_yammi->config()->mountMediaDir) {
            // linux specific
            QString cmd;
            cmd=QString("mount %1").arg(scanDir);
            system(cmd);
        }

        // check that mediaDir is an existing directory
        QDir d(scanDir);
        if(!d.exists()) {
            QString msg(i18n("The specified directory for removable media:\n %1 \n\
                             does not exist or is not redable!\n Set value \"mediaDir\" in the preferences to an existing directory!\n\
                             (if necessary, enable the \"mount media\" option in the preferences)"));
            msg.arg(scanDir);
            KMessageBox::error( m_yammi, msg);
        } else {
            traverse(scanDir, followSymLinks, filePattern, progress, mediaName);
            kdDebug() << "finished scanning!" << endl;
        }
        // umount media dir
        if(m_yammi->config()->mountMediaDir) {
            // linux specific
            QString cmd;
            cmd=QString("umount %1").arg(scanDir);
            system(cmd);
        }
    }
}

void YammiModel::updateSongDatabase(QStringList list) {
    entriesAdded=0;
    corruptSongs=0;
    if(m_yammi->config()->childSafe) {
        kdWarning()<<"updateSongDatabase : childSafe --> request denied"<<endl;
        return;
    }
    problematicSongs.clear();
    // iterate over list of songs to add
    for( QStringList::Iterator it = list.begin(); it!= list.end(); ++it ) {
        QString filename(*it);
        if(filename.endsWith(".m3u")) {
            QStringList* playlist = readM3uFile(filename);
            for(QStringList::Iterator it2 = playlist->begin();it2 != playlist->end();++it) {
                addSongToDatabase(QString(*it2), 0);
            }
            delete playlist;
        } else {
            addSongToDatabase(filename, 0);
        }
    }
}



/// traverses a directory recursively and processes all mp3 files
/// returns false, if scanning was cancelled
bool YammiModel::traverse(QString path, bool followSymLinks, QString filePattern, KProgressDialog* progress, QString mediaName) {
    // leave out the following directories
    if(path+"/"==m_yammi->config()->trashDir || path+"/"==m_yammi->config()->swapDir) {
        kdWarning() << "skipping trash or swap directory: " << path << endl;
        return true;
    }
    kdDebug() << "scanning directory " << path<<endl;
    progress->setLabel(QString(i18n("scanning directory %1 ...")).arg(path));
    progress->progressBar()->setProgress(0);
    kapp->processEvents();

    QDir d(path);

    // step 1: scan files
    d.setFilter(QDir::Files);
    d.setNameFilter(filePattern);
    d.setSorting( QDir::Name );
    const QFileInfoList* list = d.entryInfoList();
    if (!list) {
        kdWarning() << "Warning: Skipping unreadable directory: " << path << "\n";
        return true;
    }
    int filesToScan=list->count();
    progress->progressBar()->setTotalSteps(filesToScan);
    QFileInfoListIterator it( *list );
    int filesScanned=0;
    for(QFileInfo *fi; (fi=it.current()); ++it ) {
        filesScanned++;
        progress->progressBar()->setProgress(filesScanned);
        if(progress->wasCancelled()) {
            return false;
        }
        // okay, we have a file to scan, try to add to database
        addSongToDatabase(fi->filePath(), mediaName);
    }

    // step 2: recursively scan subdirectories
    QDir d2(path);
    d2.setFilter(QDir::Dirs | QDir::Readable);
    d2.setSorting( QDir::Name );
    const QFileInfoList* list2 = d2.entryInfoList();
    if (!list2) {
        kdWarning() << "Warning: Skipping unreadable child directory: " << path << "\n";
        return true;
    }
    QFileInfoListIterator it2( *list2 );
    for(QFileInfo *fi2; (fi2=it2.current()); ++it2 ) {
        if(fi2->fileName()=="." || fi2->fileName()=="..") {
            continue;
        }
        if(followSymLinks == false) {
            if(fi2->isSymLink()) {
                kdDebug() << "skipping symlink " << fi2->filePath() << endl;
                continue;
            }
        }
        if(traverse(fi2->filePath(), followSymLinks, filePattern, progress, mediaName) == false) {
            return false;
        }
    }
    return true;
}

/**
 * fix incorrect genres by re-reading them from whole song database
 */
void YammiModel::fixGenres(KProgressDialog* progress) {
    int i = 0;
    for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong(), i++) {
        progress->progressBar()->setProgress(i);
        kapp->processEvents();
        kapp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
        if(progress->wasCancelled()) {
            kdDebug() << "fixing genres aborted..." << endl;
            return;
        }
        
        kdDebug() << "trying to fix genre in file " << s->location() << endl;
        Song* fixSong=new Song();
        bool success = fixSong->create(s->location(), 0, config()->capitalizeTags);
        if(success) {
            kdDebug() << "old genre saved in yammi database: " << s->genre << ", fixed genre: " << fixSong->genre << endl;
            s->genre = fixSong->genre;
            allSongsChanged(true);
        }
        delete(fixSong);
    }
}

// adds a single songfile to the database
void YammiModel::addSongToDatabase(QString filename, QString mediaName=0) {
    kdDebug() << "scanning file '" << filename << "'..." << endl;
    bool found=false;
    for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong()) {
        // this check might fail when filename has strange characters?
        if(filename == s->location()) {
            found=true;
            /*
            // here we can fix/update our database with additional info...
            // eg: fix genre like this:
            if(s->genre == "") {
                kdDebug() << "trying to fix genre in file " << filename << endl;
                Song* fixSong=new Song();
                bool success = fixSong->create(filename, "", config()->capitalizeTags);
                if(success) {
                    kdDebug() << "old genre: " << s->genre << ", fixed genre: " << fixSong->genre << endl;
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
    newSong->create(filename, mediaName, m_yammi->config()->capitalizeTags);
    if(newSong->corrupted) {
        kdError() << "new song file " << filename << " is corrupt (not readable for yammi), skipping" << endl;
        corruptSongs++;
        return;
    }
    // check whether other version of this song existing (with same primary key)
    Song* s = allSongs.getSongByKey(newSong->artist, newSong->title, newSong->album);
    if(s!=0) {
        // yes, song with this key already existing!
        if(newSong->length==s->length && newSong->bitrate==s->bitrate && newSong->album==s->album) {
            // a) okay, we assume it is exactly the same song...
            if(mediaName==0) {
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
                        kdWarning() << "looks like file " << newSong->filename << " has been moved from " << s->path << " to " << newSong->path << ", correcting path info" << endl;
                        s->setTo(newSong);
                        allSongsChanged(true);
                        delete(newSong);
                        return;
                    }
                    // song is available, we don't need two songs => skip
                    kdWarning() << "file " << newSong->filename << " already available at " << s->location() << ", skipping" << endl;
                    delete(newSong);
                    return;
                }
            } else {
                // case 2: scanning removable media => add media, if not already added
                bool exists=false;
                for(unsigned int i=0; i<s->mediaName.count(); i++) {
                    if(s->mediaName[i]==mediaName)
                        exists=true;
                }
                if(!exists) {
                    kdDebug() << "adding media " << mediaName << " to mediaList in song " << s->displayName() << endl;
                    QString locationOnMedia=filename;
                    if(locationOnMedia.left(m_yammi->config()->mediaDir.length())!=m_yammi->config()->mediaDir)
                        kdError() << "strange error, scanning media, but file not on media" << endl;
                    locationOnMedia=locationOnMedia.right(locationOnMedia.length()-m_yammi->config()->mediaDir.length());
                    s->addMediaLocation(mediaName, locationOnMedia);
                    allSongsChanged(true);
                } else {
                    kdDebug() << "song " << s->location() << " is already known to be on this media" << endl;
                }
                return;
            }
        } else {
            // b) not exactly the same => add as new (change title to make it unique)
            // here we do not have to distinguish between harddisk and media
            kdWarning() << "seems like new song '" << newSong->artist << " - " << newSong->title << "' already existing..." << endl << "(Yammi does not allow two songs with the same artist/title/album identification)" << endl;
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
    kdDebug() << "Song added to database: " << newSong->displayName() << endl;
    entriesAdded++;
    allSongsChanged(true);
}

/**
 * Read an m3u file, returning a list of strings
 */
QStringList* YammiModel::readM3uFile(QString filename) {
    QStringList* list=new QStringList();
    QFile file(filename);
    if (!file.open( IO_ReadOnly  ) )
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
    kdDebug() << "remove category: " << categoryName << endl;
    QString name=categoryNames.first();
    int i=0;
    for(MyList* ptr=allCategories.first(); ptr; ptr=allCategories.next(), i++) {
        QString name=categoryNames[i];
        if(name==categoryName) {
            kdDebug() << "category found, deleting..." << endl;
            allCategories.remove();
            categoryNames.remove(categoryNames.at(i));
            QString file = config()->databaseDir + "categories/" + name + ".xml";
            kdDebug() << "file to delete:" << file << endl;
            QDir d;
			if(!d.remove(file)) {
				kdWarning() << "could not delete a category file: " << file << endl;
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
            kdDebug() << "renaming category.." << endl;
            categoryNames[i]=newCategoryName;
            ptr->dirty=true;
            categoriesChanged(true);
            QDir dir;
            QString path = config()->databaseDir + "categories/";
            if(!dir.rename(path+oldCategoryName+".xml", path+newCategoryName+".xml"))
                kdError() << "could not rename category file:" << path << oldCategoryName << ".xml" << endl;
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
    kdDebug() << "model save()" << endl;
    KApplication::setOverrideCursor( Qt::waitCursor );
    // save dirty categories
    saveCategories();
    if(allSongsChanged()) {
        saveSongDatabase();
    }
    if(allSongsChanged() || m_yammi->config()->logging) {
        saveHistory();
    }
    KApplication::restoreOverrideCursor();
}



/* removing a media, ie. removing the media entry in all songs that are on it
 */
void YammiModel::removeMedia(QString mediaToDelete) {
    kdDebug()<<"removing media: "<<mediaToDelete<<endl;
    for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong()) {
        QStringList::Iterator it2 = s->mediaLocation.begin();
        for ( QStringList::Iterator it = s->mediaName.begin(); it != s->mediaName.end(); ++it ) {
            if(mediaToDelete==(*it)) {
                it=s->mediaName.remove(it);
                --it;
                it2=s->mediaLocation.remove(it2);
                --it2;
            }
            ++it2;
        }
    }
    // now delete the directory (if existing)
    // linux specific
    kdWarning()<<"Remove media disabled"<<endl;
    // 	QString cmd=QString("rm -r \"%1/media/%2\"").arg(m_yammi->config()->yammiBaseDir).arg(mediaToDelete);
    // 	system(cmd);
    allSongsChanged(true);
}

void YammiModel::renameMedia(QString oldMediaName, QString newMediaName) {
    for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong()) {
        s->renameMedia(oldMediaName, newMediaName);
    }
    // now move the directory (if existing)
    QDir dir;
    QString path = config()->databaseDir + "media/";
    if(!dir.rename(path+oldMediaName, path+newMediaName)) {
        kdDebug() << "could not rename media dir!:" << path + oldMediaName << endl;
    }
    allSongsChanged(true);
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
    QString path=filename.left(pos+1);
    QString lookFor=filename.right(filename.length()-pos-1);

    if(path==m_yammi->config()->swapDir) {
        for(SongEntry* entry=allSongs.first(); entry; entry=allSongs.next()) {
            if(entry->song()->filename=="" && entry->song()->constructFilename()==lookFor)
                return entry->song();
        }
    } else {
        for(SongEntry* entry=allSongs.first(); entry; entry=allSongs.next()) {
            if(entry->song()->filename==lookFor)
                return entry->song();
        }
    }
    return 0;
}


/**
 * Checks whether a song is available on the local harddisk
 * or needs to be retrieved from a removable media.
 * If song is available, returns the absolute path+filename to the songfile.
 * (if song is in swap dir and touch is set to true,
 * the file will be touched to implement a kind of LRU strategy)
 * If not yet available, returns "".
 * If song will never be available because it is not contained on any media, returns "never".
 */
QString YammiModel::checkAvailability(Song* s, bool touch) {
    if(s->location()!="/") {
        QFileInfo fi(s->location());
        if(fi.exists() && fi.isReadable()) {
            return s->location();
        }
        cout << "warning: song " << s->displayName() << "has location given, but file does not exist or is not readable!\n";
    }
    // no location given, check whether already existing in swap dir
    QString dir=m_yammi->config()->swapDir;
    QString filename=s->constructFilename();
    QFileInfo fi(dir+filename);
    if(fi.exists() && fi.isReadable()) {
        if(touch) {
            // linux specific
            QString cmd;
            cmd=QString("touch \"%1\"").arg(dir+filename);
            system(cmd);
            /*		does not work: touching a file using QT classes...
            			QFile touchFile(dir+filename);
            			if(!touchFile.open(IO_ReadWrite))
            				cout << "could not touch songfile (for LRU method)\n";
            			else {
            				touchFile.flush();
            				touchFile.close();
            			}
            */
        }
        return dir+filename;
    }

    // not available, need to load it from media
    if(s->mediaLocation.count()!=0) {
        return "";
    } else {
        return "never";
    }
}

/**
 * Remove all those thongs from top of playlist that are unplayable:
 * - swapped songs not on harddisk and not loaded from removable media
 * - those that match unplayable file mask ("*.txt")
 * Does NOT emit the playlistChanged() signal!
 */
bool YammiModel::skipUnplayableSongs(bool firstTwo) {
    kdDebug() << "skipUnplayableSongs()\n";
    QString location;
    bool songsRemoved = false;
    while( songsToPlay.at(0) ) {
        location = checkAvailability( songsToPlay.at(0)->song() );
        if( location == "" || location == "never" ) {
            kdDebug() << "Song " << songsToPlay.at(0)->song()->displayName() << "not available, skipping\n";
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
                kdDebug() << "Song " << songsToPlay.at(1)->song()->displayName() << "not available, skipping\n";
                songsToPlay.remove(1);
                songsRemoved = true;
            } else {
                break;
            }
        }
    }    
    return songsRemoved;
}


