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

#include "yammigui.h"
#include "ApplyToAllBase.h"
#include "options.h"
#include "mediaplayer.h"
#include "ConsistencyCheckParameter.h"
#include "folder.h"
#include "foldergroups.h"
#include "foldercategories.h"
#include "foldermedia.h"
#include "foldersorted.h"

//FIXME -<clean
// // these are the includes I wanted to avoid (gui-stuff)
#include <qcheckbox.h>
// #include <qregexp.h>
// #include <qdir.h>
#include <qdom.h>
// #include <stdlib.h>
// #include <qstring.h>
// #include <qobject.h>
// #include <qdatetime.h>
// #include <qlist.h>
// #include <qvector.h>
// #include <qtimer.h>
// #include <qevent.h>

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


YammiModel::YammiModel( YammiGui *y )
{
	m_yammi = y;
}

YammiModel::~YammiModel()
{
}

Prefs YammiModel::config( )
{ 
	return m_yammi->config( );
}


// Reads the category from the found xml files in playlist directory.
void YammiModel::readCategories()
{
	// read in all xml-files found in a given directory that represent a category
	kdDebug()<<"Reading categories..." <<endl;
	
	QDomDocument doc("category");
	int count=0;

	QStringList cats = KGlobal::dirs()->findAllResources( "appdata","categories/*.xml", true );
	for( QStringList::Iterator it = cats.begin(); it!=cats.end(); ++it )
	{
		kdDebug()<<"cat resource found: "<<*it<<endl;
		QFile f( *it );
		if ( !f.open( IO_ReadOnly ) )
		{
			kdError()<<"Could not open file for reading:"<<*it<<endl;
			continue;
		}
		if ( !doc.setContent( &f ) ) 
		{
			kdError()<<"Error reading file contents"<<*it<<endl;
			f.close();
			continue;
		}
		f.close();
		// get root element
		QDomElement e = doc.documentElement();
		if(e.tagName()!="category") continue;
		
		QString name = e.attribute("name");
		
		// add category to database
		count++;
		MyList* category = new MyList;
		allCategories.append(category);
		categoryNames.append(name);

		// add all songs contained
		e = e.firstChild().toElement( );
		int index=1;
		while( !e.isNull() ) 
		{
			QString artist = e.attribute("artist");
			QString title  = e.attribute("title");
			QString album  = e.attribute("album");
			// search for item in allSongs
			Song* s = allSongs.getSongByKey(artist, title, album);
			if(!s)
			{
				kdWarning()<<"Song not found in database : "<<artist<<"/"<<title<<"/"<<album<<endl;
			}
			else 
			{
				SongEntryInt* toAdd = new SongEntryInt(s, index);
				category->append(toAdd);
				index++;
			}
			e = e.nextSibling().toElement();
		}
	}
	kdDebug()<<"done (" << count << " categories)"<<endl;
}


// Reads song history
void YammiModel::readHistory()
{
	kdDebug()<<"reading song history..."<<endl;

	// new version, history as xml-file
	QDomDocument doc( "history" );
	QString file = KGlobal::dirs()->findResource( "appdata","history.xml");
	kdDebug()<<"appdata hist"<<file<<endl;
	
	QFile f( file );
	if ( !f.open( IO_ReadOnly ) ) 
	{
		kdError()<<"could not open history file : "<<file<<endl;
		return;
	}
	if ( !doc.setContent( &f ) ) 
	{
		kdError()<<"could not parse history file, incorrect xml format? "<<file<<endl;
		f.close();
		return;
	}
	f.close();

	QDomElement docElem = doc.documentElement();
	QDomElement e = docElem.firstChild().toElement();
	int i = 0; //FIXME replace for progress bar indicator?
	while(!e.isNull())
	{
		if(i % 100==0)
			cout<< "." <<flush;
		QString artist = e.attribute("artist", "unknown");
		QString title  = e.attribute("title", "unknown");
		QString album  = e.attribute("album", "");
		QString timestamp = e.attribute("timestamp", "");

		// search for item in allSongs
		Song* s = allSongs.getSongByKey(artist, title, album);
		if(!s)
		{
			kdWarning()<<"history item "<<artist<<"/"<<title<<"/"<<album<<" not found in song database"<<endl;
		}
		else
		{
 			MyDateTime played;
 			played.readFromString(timestamp);
 			s->lastPlayed=played;
  			SongEntryTimestamp* hEntry=new SongEntryTimestamp(s, &played);
 			songHistory.append(hEntry);
		}
 		
 		e = e.nextSibling().toElement();
		i++;
 	}
	kdDebug()<<"done (" << songHistory.count() << " songs in history)"<<endl;
}

/// saves the song history
void YammiModel::saveHistory()
{
	kdDebug()<<"saving history..."<<endl;

	QDomDocument doc( "history" );
	QDomElement root = doc.createElement("history");
	doc.appendChild(root);
	
	// iterate through songs in history AND songsPlayed folder
	// => save each song as a xml song element
	for(SongEntry* entry=songHistory.first(); entry; entry=songHistory.next()) 
	{
		QDomElement elem = doc.createElement( "song" );
		elem.setAttribute( "artist", entry->song()->artist );
		elem.setAttribute( "title", entry->song()->title );
		elem.setAttribute( "album", entry->song()->album );
		elem.setAttribute( "timestamp", ((SongEntryTimestamp*)entry)->timestamp.writeToString() );
		root.appendChild( elem );
	}
	for(SongEntry* entry=songsPlayed.first(); entry; entry=songsPlayed.next()) 
	{
		QDomElement elem = doc.createElement( "song" );
		elem.setAttribute( "artist", entry->song()->artist );
		elem.setAttribute( "title", entry->song()->title );
		elem.setAttribute( "album", entry->song()->album );
		elem.setAttribute( "timestamp", ((SongEntryTimestamp*)entry)->timestamp.writeToString() );
		root.appendChild( elem );
	}

	// save history to file... (but first we make a backup of old file)
	QString path = KGlobal::dirs()->saveLocation("appdata");
	kdDebug()<<"Saving history file in directory"<<path<<endl;
	QDir dir;
	if(dir.rename( path + "history.xml", path +"history_backup.xml"))
		kdDebug()<<"backup of history saved in \"history_backup.xml\""<<endl;
	QString contents = doc.toString();
	QFile f( path + "history.xml");
	if(!f.open(IO_WriteOnly)) 
	{
		kdError()<<"could not open history file for writing:"<<path<<"history.xml"<<endl;
		return;
	}
	f.writeBlock(contents, contents.length());
	f.close();
	kdDebug()<<"done"<<endl;
}


/// save categories (if changed) to xml-files
void YammiModel::saveCategories()
{
	kdDebug()<<"Saving categories..."<<endl;
	QString path = KGlobal::dirs()->saveLocation("appdata","categories/",true);
	QString name;
	kdDebug()<<"Saving categories in directory "<<path;

	// save all categoreis marked as dirty
	for( QListViewItem* f=m_yammi->folderCategories->firstChild(); f; f=f->nextSibling() ) 
	{
		Folder* folder=(Folder*)f;
		if(!folder->songlist().dirty)
			continue;
		name = folder->folderName();
		kdDebug()<<"Saving category: "<<name<<endl;

		QDomDocument doc( "category" );
		QDomElement root = doc.createElement("category");
		root.setAttribute("name", name);
		doc.appendChild(root);
		
		for(Song* s = folder->firstSong(); s; s=folder->nextSong()) 
		{
			QDomElement elem = doc.createElement( "song" );
			elem.setAttribute( "artist", s->artist );
			elem.setAttribute( "title", s->title );
			elem.setAttribute( "album", s->album );
			root.appendChild( elem );
		}
		// save to file...
		QString contents = doc.toString();
		QFile f( path + name + ".xml" );
		if ( !f.open( IO_WriteOnly  ) )
		{
			kdError()<<"Could not save category to file: "<<path<<name<<".xml"<<endl;
			continue;
		}
		f.writeBlock ( contents, contents.length() );
		f.close();
		folder->songlist().dirty=false;
	}

	categoriesChanged(false);
}



// reads the yammi database (an xml-file with all song information)
void YammiModel::readSongDatabase(  )
{

	//FIXME - initialize model first!
	kdDebug()<<"reading song database : db=" << m_yammi->config( ).dbFile << endl;
	QFile f( m_yammi->config( ).dbFile );
	if( !f.open(IO_ReadOnly) )
	{
		kdDebug() << "Could not open data base file. Checking at old location..." << QDir::homeDirPath() << "/.yammi/songdb.xml" << endl;
		f.setName( QDir::homeDirPath() + "/.yammi/songdb.xml" );
		if( f.exists( ) && f.open(IO_ReadOnly) )
		{//move db to new location (kde)
			kdDebug()<<"reading DB from "<<f.name()<<endl;
		}
		else
		{
			QString msg( i18n( "The Song Database file could not be opened.\n\
Please edit the settings (Settings -> Configure Yammi ...) and set the path to your Song Database\
or perform a harddisk scan to create a new Database") );
			KMessageBox::error( 0L, msg, i18n("Error opening Song Database") );
			return;
		}
	}
	
	QDomDocument doc("songdb");
	QString errorMsg;
	int errorLine;
	int errorColumn;
	if( !doc.setContent(&f, false, &errorMsg, &errorLine, &errorColumn) )
	{
		f.close();
		QString msg = QString(i18n("Error reading database file:\n%1\n(Error: %2, line %3, column %4)") ).arg(f.name()).arg(errorMsg).arg(errorLine).arg(errorColumn);
		KMessageBox::error(0L,msg,i18n("Error reading database"));		
		return;
	}
	f.close();
	QDomElement root = doc.documentElement( );
	QString version = root.attribute("yammiVersion", "no version");
	if(version=="0.5.2" || version=="0.5.1" || version=="0.5" || version=="no version")
	{
		QString msg( i18n("Your Song Database seems to be very old.\n You might need to create \
a new Database and scan your harddisk for songs") );
		KMessageBox::sorry( 0L, msg, i18n("Unknown Song Database version") );
		//try to continue anyways...?
	}
	
	int total = root.attribute("count", "0").toInt( );
	QDomElement e = root.firstChild().toElement();
	
	//FIXME - change this to a progress in the status bar instead...
	KProgressDialog dia(0,0,i18n("Loading database"),i18n("Reading Song Database file"));
	dia.setAllowCancel(false);
	KProgress *p = dia.progressBar();
	p->setTotalSteps( total );
	int count = 0;
	while( !e.isNull( ) )
	{
		p->setProgress( count );
		
		QString artist   = e.attribute("artist", "unknown");
		QString title    = e.attribute("title", "unknown");
		QString album    = e.attribute("album", "");
		QString filename = e.attribute("filename", "");
		QString path     = e.attribute("path", "");
		QString comment  = e.attribute("comment", "");
		
		unsigned long length = e.attribute("length","0").toULong( );
		int bitrate = e.attribute("bitrate", "0").toInt( );
		int year = e.attribute("year", "0").toInt( );
		int trackNr = e.attribute("trackNr", "0").toInt( );
		int genreNr = e.attribute("genreNr", "0").toInt( );
		
		// read date as "dd/mm/yyyy, hh:mm:ss"
		MyDateTime addedTo;
		addedTo.readFromString(e.attribute("addedTo", "1/1/1970/00:00:00"));
		
		Song* s = new Song(artist, title, album, filename, path, length, bitrate, addedTo, year, comment, trackNr, genreNr);
		allSongs.appendSong( s );

		QDomNode child = e.firstChild();
		while( !child.isNull() )
		{
			QDomElement mediaElem = child.toElement(); // try to convert the node to an element.
			if( !mediaElem.isNull() ) 
			{
				QString mediaName=mediaElem.attribute("mediaName", "unspecified media");
				QString mediaLocation=mediaElem.attribute("mediaLocation", "unspecified location");
				s->addMediaLocation(mediaName, mediaLocation);
			}
			child = child.nextSibling();
		}
		
		e = e.nextSibling().toElement();
		count++;
	}
	kdDebug()<<"Read "<<count<<" songs from DB. ( allSongs = "<<allSongs.count()<<" )"<<endl;
	allSongsChanged( true );
}

/// saves the songs in allSongs into an xml-file
void YammiModel::saveSongDatabase()
{
	kdDebug()<<"saving database..."<<endl;
	int sumDirtyTags=0;
	int sumDirtyFilenames=0;
	for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong()) 
	{
		if(s->tagsDirty)
			sumDirtyTags++;
		if(s->filenameDirty)
			sumDirtyFilenames++;
	}
	if(m_yammi->config( ).tagsConsistent) 
	{
		kdDebug()<<sumDirtyTags<< " dirty tags..."<<endl;
	}
	if(m_yammi->config( ).filenamesConsistent) 
	{
		kdDebug()<<sumDirtyFilenames<<" dirty filenames..."<<endl;
	}
  
	QDomDocument doc;
	doc.appendChild( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\"" ) );
	QDomElement root = doc.createElement("songs");
	
	root.setAttribute("yammiVersion", m_yammi->config( ).yammiVersion);
	doc.appendChild(root);
	
	// iterate through songs and save each song as a xml song element
	int count=0;
	bool haveToReloadPlaylist=false;
	bool currentSongRenamed=false;    // set to true, if currently played song's filename is changed
	for(Song* s = allSongs.firstSong(); s; s = allSongs.nextSong()) 
	{
		count++;
		QDomElement elem = doc.createElement( "song" );
		// consistencyMode: if song dirty, we make it consistent
		if(s->tagsDirty && m_yammi->config( ).tagsConsistent) 
		{
			s->saveTags();
		}
		if(s->filenameDirty && m_yammi->config( ).filenamesConsistent) 
		{
			s->correctFilename();
			if(songsToPlay.containsSong(s)) 
			{
				haveToReloadPlaylist=true;
				if(songsToPlay.firstSong()==s) 
				{
					currentSongRenamed=true;
				}
			}
		}
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
		if(s->genreNr!=0)
			elem.setAttribute( "genreNr", QString("%1").arg(s->genreNr));
			
		elem.setAttribute( "addedTo", s->addedTo.writeToString());
		for(unsigned int i=0; i<s->mediaName.count(); i++) 
		{
			QDomElement media = doc.createElement( "media" );
			media.setAttribute( "mediaName", s->mediaName[i]);
			media.setAttribute( "mediaLocation", s->mediaLocation[i]);
			elem.appendChild(media);
		}
		root.appendChild( elem );
	}
	root.setAttribute("count",count);
	
	//save to the same file we opened...
// 	// save songdb to file... (but first we make a backup of old file)
// 	QString path = KGlobal::dirs()->saveLocation("appdata");
 	kdDebug() << "Saving song database to file " << m_yammi->config().dbFile << endl;
 	QDir dir;
 	if(!dir.rename(m_yammi->config().dbFile, m_yammi->config().dbFile+"_backup")) {
 		kdDebug() << "could not make backup of songdb..." << endl;
	}

	QFile file(m_yammi->config().dbFile);
	if(!file.open(IO_WriteOnly)) {
		kdError() << "could not open db file for writing:" << m_yammi->config().dbFile << endl;
		return;
	}
	QTextStream str(&file);
	str.setEncoding(QTextStream::UnicodeUTF8);
        doc.save(str, 2);
        file.close();
	
	if(haveToReloadPlaylist) 
	{//FIXME!! this is kinda ugly...
		kdDebug()<<"Reload playlist to player..."<<endl;
		m_yammi->player->clearPlaylist();
		Song* save=0;
		if(songsToPlay.count()>0) 
		{
			currentSongRenamed=(currentSongFilenameAtStartPlay!=songsToPlay.firstSong()->location());
		}
		if(currentSongRenamed) 
		{
			save=songsToPlay.firstSong();
			songsToPlay.removeSong(save);
		}
		m_yammi->player->syncYammi2Player(false);
		if(currentSongRenamed) 
		{
			songsToPlay.insert(0, new SongEntryInt(save, 0));
		}
	}
	kdDebug()<<"saving database: done"<<endl;
	allSongsChanged(false);
}



void YammiModel::allSongsChanged(bool changed)
{
	_allSongsChanged = changed;
//  if(m_yammi && m_yammi->tbSaveDatabase)
//    m_yammi->tbSaveDatabase->setEnabled(_allSongsChanged || _categoriesChanged);
}

bool YammiModel::allSongsChanged()
{
	return _allSongsChanged;
}

void YammiModel::categoriesChanged(bool changed)
{
	_categoriesChanged = changed;
//  if(m_yammi && m_yammi->tbSaveDatabase)
//  	m_yammi->tbSaveDatabase->setEnabled(_allSongsChanged || _categoriesChanged);
}

bool YammiModel::categoriesChanged()
{
	return _categoriesChanged;
}



//   Updates the xml-database by scanning harddisk
void YammiModel::updateSongDatabase(QString scanDir, QString filePattern, QString mediaName, KProgressDialog* progress)
{
	entriesAdded=0;
	corruptSongs=0;
	if(m_yammi->config( ).childSafe)
	{
		kdWarning()<<"updateSongDatabase : childSafe --> request denied"<<endl;
		return;
	}
	problematicSongs.clear();
	if(mediaName==0)
	{
		kdDebug()<<"scanning harddisk for new songs..."<<endl;
		// check that scanDir is an existing directory
		QDir d(scanDir);
		if(!d.exists())
		{
			QString msg( i18n("The base directory for scanning does not exist!\n\
Set value \"scanDir\" in the preferences to an existing directory!"));
			KMessageBox::error( m_yammi, i18n("Yammi"), msg );
		}
		else
		{
			traverse(scanDir, filePattern, progress);
			kdDebug()<<"finished scanning!"<<endl;
		}
	}
	else 
	{
		kdDebug()<<"scanning removable media for new songs..."<<endl;
		
		// mount media dir
		if(m_yammi->config( ).mountMediaDir) 
		{
			// linux specific
			QString cmd;
			cmd=QString("mount %1").arg(scanDir);
			system(cmd);
		}

		// check that mediaDir is an existing directory
		QDir d(scanDir);
		if(!d.exists())
		{
			QString msg(i18n("The specified directory for removable media:\n %1 \n\
does not exist or is not redable!\n Set value \"mediaDir\" in the preferences to an existing directory!\n\
(if necessary, enable the \"mount media\" option in the preferences)"));
			msg.arg(scanDir);
			KMessageBox::error( m_yammi, i18n("Yammi"), msg );
		}
		else 
		{
			traverse(scanDir, filePattern, progress, mediaName);
			kdDebug()<<"finished scanning!"<<endl;
		}
		// umount media dir
		if(m_yammi->config( ).mountMediaDir) 
		{
			// linux specific
			QString cmd;
			cmd=QString("umount %1").arg(scanDir);
			system(cmd);
		}
	}
}

void YammiModel::updateSongDatabase(QStringList list)
{
	entriesAdded=0;
	corruptSongs=0;
	if(m_yammi->config( ).childSafe)
	{
		kdWarning()<<"updateSongDatabase : childSafe --> request denied"<<endl;
		return;
	}
	problematicSongs.clear();
	// iterate over list of songs to add
	for( QStringList::Iterator it = list.begin(); it!= list.end(); ++it )
	{
		QString filename(*it);
		if(filename.endsWith(".m3u"))
		{
			QStringList* playlist = readM3uFile(filename);
			for(QStringList::Iterator it2 = playlist->begin();it2 != playlist->end();++it)
			{
				addSongToDatabase(QString(*it2), 0);
			}
			delete playlist;
			}
		else 
		{
			addSongToDatabase(filename, 0);
		}
	}
}



/// traverses a directory recursively and processes all mp3 files
/// returns false, if scanning was cancelled
bool YammiModel::traverse(QString path, QString filePattern, KProgressDialog* progress, QString mediaName)
{
	// leave out the following directories
	if(path+"/"==m_yammi->config( ).trashDir || path+"/"==m_yammi->config( ).swapDir)
	{
		kdWarning()<<"skipping trash or swap directory: "<<path<<endl;
		return true;
	}
	kdDebug()<<"scanning directory "<<path<<endl;
	progress->setLabel(QString(i18n("scanning directory %1 ...")).arg(path));
	progress->progressBar()->setProgress(0);
//  qApp->processEvents();
	
	QDir d(path);

  // step 1: scan files
	d.setFilter(QDir::Files);
	d.setNameFilter(filePattern);
	d.setSorting( QDir::Name );
	const QFileInfoList* list = d.entryInfoList();
	int filesToScan=list->count();
	progress->progressBar()->setTotalSteps(filesToScan);
	QFileInfoListIterator it( *list );
	int filesScanned=0;
	for(QFileInfo *fi; (fi=it.current()); ++it )
	{
		filesScanned++;
		progress->progressBar()->setProgress(filesScanned);
//	  qApp->processEvents();
		if(progress->wasCancelled())
		{
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
	QFileInfoListIterator it2( *list2 );

	for(QFileInfo *fi2; (fi2=it2.current()); ++it2 )
	{
		if(fi2->fileName()=="." || fi2->fileName()=="..") {
			continue;
    }
    if(fi2->isSymLink()) {
	    kdDebug()<<"skipping symlink " << fi2->filePath() << endl;
      continue;
    }
		if(traverse(fi2->filePath(), filePattern, progress, mediaName) == false)
		{
			return false;
		}
	}
	return true;// scanning was not cancelled
}


// adds a single songfile to the database
void YammiModel::addSongToDatabase(QString filename, QString mediaName=0)
{
	kdDebug()<<"adding file to db"<<filename<<endl;
	bool found=false;
	for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong())
	{
 		// this check might fail when filename has strange characters?
 		if(filename == s->location())
		{
 			found=true;
 			// here we can fix/update our database with additional info...
//  			eg:
//  			// add genre number to Song info
//  			Song* fixSong=new Song(fi->filePath());
//  			s->genreNr=fixSong->genreNr;
//  			delete(fixSong);
//  			allSongsChanged(true);
			break;
		}
	}
	if(found)
		return;
		
	// okay, new song (at least new filename/path) => construct song object
	Song* newSong = new Song();
	newSong->create(filename, mediaName, m_yammi->config( ).capitalizeTags);
	if(newSong->corrupted)
	{
		kdError()<<"new song file " <<filename<< " is corrupt (not readable for yammi), skipping"<<endl;
		corruptSongs++;
		return;
	}
	// check whether other version of this song existing (with same primary key)
	Song* s = allSongs.getSongByKey(newSong->artist, newSong->title, newSong->album);
	if(s!=0)
	{
		// yes, song with this key already existing!
		if(newSong->length==s->length && newSong->bitrate==s->bitrate && newSong->album==s->album)
		{
			// a) okay, we assume it is exactly the same song...
			if(mediaName==0)
			{
				// case 1: scanning harddisk
				if(s->filename=="")
				{
					// case 1a: song has no filename = was not available on harddisk
					// => make it available (at new location)
					s->setTo(newSong);
					allSongsChanged(true);
					delete(newSong);
					return;
				}
				else
				{
 					// case 1b: song has filename
 					QFileInfo fileInfo(s->location());
 					if(!fileInfo.exists())
					{
 						// but still not available => probably has been moved
 						kdWarning()<<"looks like file " <<newSong->filename<< " has been moved from " <<s->path<< " to " <<newSong->path<< ", correcting path info"<<endl;
 						s->setTo(newSong);
 						allSongsChanged(true);
 						delete(newSong);
 						return;
 					}
 					// song is available, we don't need two songs => skip
 					kdWarning()<<"file "<<newSong->filename<<" already available at " << s->location() << ", skipping"<<endl;
 					delete(newSong);
 					return;
 				}
 			}
 			else
			{
 				// case 2: scanning removable media => add media, if not already added
 				bool exists=false;
 				for(unsigned int i=0; i<s->mediaName.count(); i++) {
 					if(s->mediaName[i]==mediaName)
 						exists=true;
 				}
 				if(!exists)
				{
 					kdDebug()<<"adding media "<<mediaName <<" to mediaList in song "<<s->displayName()<<endl;
 					QString locationOnMedia=filename;
 					if(locationOnMedia.left(m_yammi->config( ).mediaDir.length())!=m_yammi->config( ).mediaDir)
 						kdError()<<"strange error, scanning media, but file not on media"<<endl;
 					locationOnMedia=locationOnMedia.right(locationOnMedia.length()-m_yammi->config( ).mediaDir.length());							
 					s->addMediaLocation(mediaName, locationOnMedia);
 					allSongsChanged(true);
 				}
 				else
				{
 					kdDebug()<<"song " << s->location() << " is already known to be on this media"<<endl;
 				}
 				return;
 			}
 		}
 		else
		{
 			// b) not exactly the same => add as new (change title to make it unique)
 			// here we do not have to distinguish between harddisk and media
 			kdWarning()<< "seems like new song <" << newSong->artist << " - " << newSong->title << "> already existing..."<<endl
 				   << "(Yammi does not allow two songs with the same artist/title/album identification)"<<endl;
 			int tryNo=2;
 			QString extTitle;
 			for(bool notUnique=true; notUnique; tryNo++)
			{
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
	kdDebug()<<"Song added: "<<newSong->displayName()<<endl;
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


/**
 * checks consistency of all songs
 * @returns true, if consistent, false, if problematic songs were found
 */
bool YammiModel::checkConsistency(KProgressDialog* progress, MyList* selection, ConsistencyCheckParameter* p)
{
	if(m_yammi->config( ).childSafe)
	{
		kdWarning()<<"checkConsistency : childSafe --> request denied"<<endl;
		return false;
	}
  
	kdDebug()<<"checking consistency of database..."<<endl;
	problematicSongs.clear();

	// 1. iterate through all songs in database
  if(p->checkForExistence || p->checkTags || p->checkFilenames) {
      
    progress->setLabel("Step 1: checking all songs in database...");
    progress->progressBar()->setTotalSteps(selection->count());
    progress->progressBar()->setProgress(0);
    kapp->processEvents();
	
    // TODO: move this to constructor of p?
    p->dirtyTags=0;
    p->dirtyFilenames=0;
    p->dirtyDirectories=0;
    p->doublesFound=0;
    p->filenamesCorrected=0;
    p->directoriesCorrected=0;
    p->nonExisting=0;
    p->nonExistingDeleted=0;
    p->nonExistingUpdated=0;
    p->tagsCorrected=0;

    p->correctTagsConfirmed=-1;
    p->correctFilenamesConfirmed=-1;
    p->correctDirectoriesConfirmed=-1;
    
    int i=0;
    for(Song* s=selection->firstSong(); s; s=selection->nextSong(), i++) {
      if(i%10==0) {
        progress->progressBar()->setProgress(i);
      }
      if(progress->wasCancelled())
        break;		
      QString diagnosis=s->checkConsistency(true, true, p->ignoreCaseInFilenames, true);
      if(diagnosis=="") {
        continue;
      }

      // okay, some kind of problem...

      if(diagnosis=="file not readable" && p->checkForExistence) {
        p->nonExisting++;
        kdError()<< "file not existing or readable: " << s->displayName()<<endl;
        bool onMedia=s->mediaName.count()>0;
        if(p->updateNonExisting) {
          // if we update, there are two cases:
          if(onMedia) {
            // 1. update entry: set filename+path to ""
            s->filename="";
            s->path="";
            p->nonExistingUpdated++;
            problematicSongs.append(new SongEntryString(s, "File not existing on harddisk any more, filename cleared"));
          }
          else {
            // 2. delete entry in database
            kdWarning()<< "deleting entry " << s->displayName()<<endl;
            m_yammi->deleteEntry(s);
            p->nonExistingDeleted++;
          }
        }
        else {
          problematicSongs.append(new SongEntryString(s, "File not readable"));
        }
      }


      if(diagnosis.contains("tags not correct") && p->checkTags) {
        p->dirtyTags++;
        if(p->correctTags) {

          bool reallyCorrect;
          if(p->correctTagsConfirmed==-1) {
            // warning dialog!
            ApplyToAllBase confirm(progress, "confirmDialog", true);
            QString msg=QString("Correct tags in file\n\n\t%1?\n\n").arg(s->filename);
            if(p->correctTagsDirection==p->YAMMI2TAGS) {
              msg+=QString("(Write yammi info to file tags:\n");
              msg+=QString("artist: %1, title: %2\n").arg(s->artist).arg(s->title);
              msg+=QString("album: %1, comment: %2\n").arg(s->album).arg(s->comment);
              msg+=QString("year: %1, trackNr: %2, genreNr: %3)").arg(s->year).arg(s->trackNr).arg(s->genreNr);              
            }
            if(p->correctTagsDirection==p->TAGS2YAMMI) {
              msg+=QString("(Reread tags from filename and update Yammi info)");
            }
            confirm.TextLabel->setText(msg);
            // show dialog
            int result=confirm.exec();
            if(result==QDialog::Accepted) {
              reallyCorrect=true;
              if(confirm.CheckBoxApply->isChecked()) {
                p->correctTagsConfirmed=1;
              }
            }
            else {
              reallyCorrect=false;
              if(confirm.CheckBoxApply->isChecked()) {
                p->correctTagsConfirmed=0;
              }
            }
          }
          else {
            reallyCorrect=(p->correctTagsConfirmed==1);
          }
          if(reallyCorrect) {
            if(p->correctTagsDirection==p->YAMMI2TAGS) {
              if(s->saveTags()) {
                p->tagsCorrected++;
                problematicSongs.append(new SongEntryString(s, "Yammi info written to file tags"));
              }              
            }
            if(p->correctTagsDirection==p->TAGS2YAMMI) {
              if(s->rereadTags()) {
                p->tagsCorrected++;
                problematicSongs.append(new SongEntryString(s, "Tags reread from file and Yammi info updated"));
              }
            }              
          }
          else {
            problematicSongs.append(new SongEntryString(s, "Yammi info and file tags not consistent"));
          }
        }
        else {
          problematicSongs.append(new SongEntryString(s, "Yammi info and file tags not consistent"));
        }
      }

      

      if(diagnosis.contains("filename not consistent") && p->checkFilenames) {
        p->dirtyFilenames++;
        if(p->correctFilenames) {
          bool reallyCorrect;
          if(p->correctFilenamesConfirmed==-1) {
            // warning dialog!
            ApplyToAllBase confirm(progress, "confirmDialog", true);
            QString msg=QString("Correct filename from\n\t%1\n").arg(s->filename);
            msg+=QString("to\n\t%1?").arg(s->constructFilename());
            confirm.TextLabel->setText(msg);
            // show dialog
            int result=confirm.exec();
            if(result==QDialog::Accepted) {
              reallyCorrect=true;
              if(confirm.CheckBoxApply->isChecked()) {
                p->correctFilenamesConfirmed=1;
              }
            }
            else {
              reallyCorrect=false;
              if(confirm.CheckBoxApply->isChecked()) {
                p->correctFilenamesConfirmed=0;
              }
            }
          }
          else {
            reallyCorrect=(p->correctFilenamesConfirmed==1);
          }
          if(reallyCorrect) {
            if(s->correctFilename()) {
              p->filenamesCorrected++;
              problematicSongs.append(new SongEntryString(s, "Filename corrected"));
            }
          }
          else {
            problematicSongs.append(new SongEntryString(s, "Filename not consistent with Yammi info"));
          }
        }
        else {
          problematicSongs.append(new SongEntryString(s, "Filename not consistent with Yammi info"));
        }
      }

      if(diagnosis.contains("directory not consistent") && p->checkDirectories) {
        p->dirtyDirectories++;
        if(p->correctDirectories) {
          bool reallyCorrect;
          if(p->correctDirectoriesConfirmed==-1) {
            // warning dialog!
            ApplyToAllBase confirm(progress, "confirmDialog", true);
            QString msg=QString("Correct path from\n\t%1\n").arg(s->path);
            msg+=QString("to\n\t%1?").arg(s->constructPath());
            confirm.TextLabel->setText(msg);
            // show dialog
            int result=confirm.exec();
            if(result==QDialog::Accepted) {
              reallyCorrect=true;
              if(confirm.CheckBoxApply->isChecked()) {
                p->correctDirectoriesConfirmed=1;
              }
            }
            else {
              reallyCorrect=false;
              if(confirm.CheckBoxApply->isChecked()) {
                p->correctDirectoriesConfirmed=0;
              }
            }
          }
          else {
            reallyCorrect=(p->correctDirectoriesConfirmed==1);
          }
          if(reallyCorrect) {
            if(s->correctPath()) {
              p->directoriesCorrected++;
              problematicSongs.append(new SongEntryString(s, "Directory corrected"));
            }
          }
          else {
            problematicSongs.append(new SongEntryString(s, "Directory not consistent with Yammi info"));
          }
        }
        else {
          problematicSongs.append(new SongEntryString(s, "Directory not consistent with Yammi info"));
        }
      }
                  
    }
	}

  
	// 2. check for songs contained twice
  if(!progress->wasCancelled() && p->checkDoubles) {
    progress->setLabel("Step 2: check for song entries pointing to same file");
    progress->progressBar()->setTotalSteps(allSongs.count());
    progress->progressBar()->setProgress(0);
    kapp->processEvents();
	
    allSongs.setSortOrderAndSort(MyList::ByFilename + 16*(MyList::ByPath));
    Song* last=allSongs.firstSong();
    int i=0;
    // TODO: perform this check also only on the selected songs?
    for(Song* s=allSongs.nextSong(); s; s=allSongs.nextSong(), i++) {
      if(i % 20==0)
        progress->progressBar()->setProgress(i);
      if(progress->wasCancelled())
        break;		
      if(s->artist=="{wish}")			// ignore wishes
        continue;
      if(s->path=="" && s->filename=="")		// ignore songs not on local harddisk
        continue;
		
      // check for songs contained twice in database (but pointing to same file+path)
      if(last->location()==s->location()) {
        kdWarning()<<"two database entries pointing to same file: " << s->filename << ", deleting one"<<endl;
        allSongs.remove();// problem, coz we are iterating through this list???
        allSongsChanged(true);
        p->doublesFound++;
        continue;
      }
      last=s;
    }

    // 3. check for two songs with identical primary key
    progress->setLabel("Step 3: check for two songs with identical primary keys");
    progress->progressBar()->setTotalSteps(allSongs.count());
    progress->progressBar()->setProgress(0);
    kapp->processEvents();
	
    allSongs.setSortOrderAndSort(MyList::ByKey);
    last=allSongs.firstSong();
    i=0;
    for(Song* s=allSongs.nextSong(); s; s=allSongs.nextSong(), i++) {
      if(i % 20==0)
        progress->progressBar()->setProgress(i);
      if(progress->wasCancelled())
        break;		
      if(s->artist=="{wish}")
        continue;
	
      if(s->sameAs(last)) {
        kdError()<<"!!!   song contained twice: " << s->filename << ", check in problematic songs!"<<endl;
        problematicSongs.append(new SongEntryString(last, "contained twice(1)"));
        problematicSongs.append(new SongEntryString(s, "contained twice(2)"));
        p->doublesFound++;
      }
    }
  }
  // reset sortOrder
	allSongs.setSortOrderAndSort(MyList::ByKey);

	if(progress->wasCancelled())
		kdDebug()<<"consistency check aborted"<<endl;
	else
		kdDebug()<<"consistency checked"<<endl;    

	if(problematicSongs.count()==0) {
		kdDebug()<< "Your Yammi database is nice and clean!"<<endl;
		return true;
	}
	else {
		allSongsChanged(true);
		return false;
	}
}


void YammiModel::removeCategory(QString categoryName)
{
	kdDebug()<<"remove category: "<<categoryName<<endl;
	QString name=categoryNames.first();
	int i=0;
	for(MyList* ptr=allCategories.first(); ptr; ptr=allCategories.next(), i++) 
	{
		QString name=categoryNames[i];
		if(name==categoryName) 
		{
			kdDebug()<<"category found, deleting..."<<endl;
			allCategories.remove();
			categoryNames.remove(categoryNames.at(i));
			QString file = KGlobal::dirs()->findResource("appdata","categories/"+name+".xml");
			kdDebug()<<"file to delete:"<<file<<endl;
			QDir d;
			d.remove(file);
			break;
		}
	}
}

void YammiModel::renameCategory(QString oldCategoryName, QString newCategoryName)
{
	int i=0;
	for(MyList* ptr=allCategories.first(); ptr; ptr=allCategories.next(), i++)
	{
		QString name=categoryNames[i];
		if(name==oldCategoryName)
		{
			kdDebug()<<"renaming category.."<<endl;
			categoryNames[i]=newCategoryName;
			ptr->dirty=true;
			categoriesChanged(true);
			QDir dir;
			QString path = KGlobal::dirs()->saveLocation("appdata","categories/",false);
			if(!dir.rename(path+oldCategoryName+".xml", path+newCategoryName+".xml"))
				kdError()<<"could not rename category file:"<<path<<oldCategoryName<<".xml"<<endl;
			break;
		}
	}
}

void YammiModel::newCategory(QString name)
{
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
void YammiModel::saveAll()
{
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
void YammiModel::save()
{kdDebug()<<"model save()"<<endl;
	KApplication::setOverrideCursor( Qt::waitCursor );
	// save dirty categories
	saveCategories();
	if(allSongsChanged())
		saveSongDatabase();
	if(allSongsChanged() || m_yammi->config( ).logging)
		saveHistory();

	KApplication::restoreOverrideCursor();
}



/* removing a media, ie. removing the media entry in all songs that are on it
 */
void YammiModel::removeMedia(QString mediaToDelete)
{
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
// 	QString cmd=QString("rm -r \"%1/media/%2\"").arg(m_yammi->config( ).yammiBaseDir).arg(mediaToDelete);
// 	system(cmd);
	allSongsChanged(true);
}

void YammiModel::renameMedia(QString oldMediaName, QString newMediaName)
{
	for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong()) {
		s->renameMedia(oldMediaName, newMediaName);
	}
	// now move the directory (if existing)
	QDir dir;
	QString path = KGlobal::dirs()->saveLocation("appdata","media/",false);
	if(!dir.rename(path+oldMediaName, path+newMediaName))
		kdDebug()<<"could not rename media dir!:"<<path+oldMediaName<<endl;
	allSongsChanged(true);
}

/** marks those playlists as dirty that contain the given song
 */
void YammiModel::markPlaylists(Song* s)
{	
	// for all categories, check whether they contain the song, if yes => mark as dirty
	for( QListViewItem* f=m_yammi->folderCategories->firstChild(); f; f=f->nextSibling() ) {
		Folder* folder=(Folder*)f;
		if(folder->songlist().containsSong(s))
			folder->songlist().dirty=true;
	}
}


// finds out the corresponding song entry given a filename
// (now also takes care of songs in swap dir)
// returns 0 if no song entry found
Song* YammiModel::getSongFromFilename(QString filename)
{
	// strip filename to relative name
	int pos=filename.findRev('/', -1);
	QString path=filename.left(pos+1);
	QString lookFor=filename.right(filename.length()-pos-1);

	if(path==m_yammi->config( ).swapDir) {
		for(SongEntry* entry=allSongs.first(); entry; entry=allSongs.next()) {
			if(entry->song()->filename=="" && entry->song()->constructFilename()==lookFor)
				return entry->song();
		}
	}
	else {
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
QString YammiModel::checkAvailability(Song* s, bool touch)
{
	if(s->location()!="/") {
		QFileInfo fi(s->location());
		if(fi.exists() && fi.isReadable()) {
			return s->location();
		}
		cout << "warning: song " << s->displayName() << "has location given, but file does not exist or is not readable!\n";
	}
	// no location given, check whether already existing in swap dir
	QString dir=m_yammi->config().swapDir;
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
  }
	else {
		return "never";
  }
}

