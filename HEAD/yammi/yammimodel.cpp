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
#include "yammigui.h"
#include "ApplyToAllBase.h"
#include "options.h"


using namespace std;

extern YammiGui* gYammiGui;


YammiModel::YammiModel()
{
}

YammiModel::~YammiModel()
{
}


/// read preferences from xml-file
void YammiModel::readPreferences(QString baseDir)
{
	config.loadConfig(baseDir);
	noPrefsFound = !config.getPrefsFound();
}


/// save preferences (if changed) to disk
void YammiModel::savePreferences()
{
	config.saveConfig();
}



/**
 * Reads the category from the found xml files in playlist directory.
 */
void YammiModel::readCategories()
{
	// read in all xml-files found in a given directory that represent a category
	cout << "reading categories..." << flush;
	
  categoriesChanged(false);

  QDir d(config.yammiBaseDir+"/categories");
  if(!d.exists()) {
    cout << "\ncould not read categories\n";
    return;
  }
	d.setFilter( QDir::Files);
	d.setSorting( QDir::DirsFirst );
	const QFileInfoList *list = d.entryInfoList();
	QFileInfoListIterator it( *list );      // create list iterator
	QFileInfo *fi;                          // pointer for traversing
	QDomDocument doc("category");
	int count=0;

	// for all categories found...
	for ( ;(fi=it.current()); ++it ) {           // for each file
		// skip non-xml-files
		if (fi->extension(FALSE)!="xml") continue;
		
		QFile f( fi->filePath() );
		if ( !f.open( IO_ReadOnly ) )
			continue;
		if ( !doc.setContent( &f ) ) {
			f.close();
			continue;
		}
		f.close();

		// get root element
		QDomElement docElem = doc.documentElement();
		if(docElem.tagName()!="category") continue;
		cout << "." << flush;		
		QDomNode n = docElem.firstChild();
   	QString categoryName(docElem.attribute("name"));
		
		// add category to database
		count++;
		MyList* ptr=new MyList;
		allCategories.append(ptr);
		categoryNames.append(categoryName);

		// add all songs contained
		int index=1;
		while( !n.isNull() ) {
			QDomElement e = n.toElement();						// try to convert the node to an element.
			if( !e.isNull() ) { 											// the node is really an element.
				QString artist=e.attribute("artist");
				QString title=e.attribute("title");
				QString album=e.attribute("album");
				// search for item in allSongs
				Song* found=allSongs.getSongByKey(artist, title, album);
				if(found==0) {
//					cout << "\ncategory item " << artist << " - " << title << " (" << album << ") not found in song database\n";
				}
				else {
					SongEntryInt* toAdd=new SongEntryInt(found, index);
					ptr->append(toAdd);
					index++;
				}
			}
			n = n.nextSibling();
		}
  }
	
	cout << "\n..done (" << count << " categories)\n";
}

/**
 * Reads song history
 */
void YammiModel::readHistory()
{
 	// read in history of songs from logfile
 	cout << "reading song history..." << flush;
	
	// new version, history as xml-file
	QDomDocument doc( "history" );
	QFile f( config.yammiBaseDir+"/history.xml" );
	if ( !f.open( IO_ReadOnly ) ) {
		cout << "\ncould not open history file... => no songs in history\n";
		return;
	}
	if ( !doc.setContent( &f ) ) {
		cout << "\ncould not parse history file, incorrect xml format?\n";
		f.close();
		return;
	}
	f.close();

	QDomElement docElem = doc.documentElement();
	QDomNode n = docElem.firstChild();
	for(int i=0; !n.isNull(); i++) {
		QDomElement e = n.toElement();						// try to convert the node to an element.
		if( !e.isNull() ) { 											// the node is really an element.
			if(i % 100==0)
				cout << "." << flush;
			QString artist=e.attribute("artist", "unknown");
			QString title=e.attribute("title", "unknown");
			QString album=e.attribute("album", "");
			QString timestamp=e.attribute("timestamp", "");

			// search for item in allSongs
			Song* found=allSongs.getSongByKey(artist, title, album);
			if(found==0) {
				cout << "\nhistory item " << artist << " - " << title << " (" << album << ") not found in song database\n";
			}
			else {
 				MyDateTime played;
 				played.readFromString(timestamp);
 				found->lastPlayed=played;
  			SongEntryTimestamp* hEntry=new SongEntryTimestamp(found, &played);
 				songHistory.append(hEntry);
			}
 		}
 		n = n.nextSibling();
 	}
	cout << "\n..done (" << songHistory.count() << " songs in history)\n";
}

/// saves the song history
void YammiModel::saveHistory()
{
	cout << "saving history...\n";
	
	// create xml-file
	QDomDocument doc( "history" );
	QString empty("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<history>\n</history>\n");
	if ( !doc.setContent( empty ) ) {
		cout << "ERROR: could not create xml document for history file\n";
		return;
	}
	QDomElement rootElem = doc.documentElement();
	
	// iterate through songs in history AND songsPlayed folder
	// => save each song as a xml song element
	for(SongEntry* entry=songHistory.first(); entry; entry=songHistory.next()) {
		QDomElement elem = doc.createElement( "song" );
		elem.setAttribute( "artist", entry->song()->artist.utf8() );
		elem.setAttribute( "title", entry->song()->title.utf8() );
		elem.setAttribute( "album", entry->song()->album.utf8() );
		elem.setAttribute( "timestamp", ((SongEntryTimestamp*)entry)->timestamp.writeToString().utf8() );
		rootElem.appendChild( elem );		
	}
	for(SongEntry* entry=songsPlayed.first(); entry; entry=songsPlayed.next()) {
		QDomElement elem = doc.createElement( "song" );
		elem.setAttribute( "artist", entry->song()->artist.utf8() );
		elem.setAttribute( "title", entry->song()->title.utf8() );
		elem.setAttribute( "album", entry->song()->album.utf8() );
		elem.setAttribute( "timestamp", ((SongEntryTimestamp*)entry)->timestamp.writeToString().utf8() );
		rootElem.appendChild( elem );		
	}

	// save history to file... (but first we make a backup of old file)
	QDir dir;
	if(dir.rename(config.yammiBaseDir+"/history.xml", config.yammiBaseDir+"/history_backup.xml"))
		cout << "backup of history saved in \"history_backup.xml\"\n";
	QString save=doc.toString();
	QFile f2( config.yammiBaseDir+"/history.xml");
	if(!f2.open(IO_WriteOnly)) {
		cout << "ERROR: could not save history\n";
		return;
	}
	f2.writeBlock(save, save.length());
	f2.close();
	cout << " ...done\n";
}


/// save categories (if changed) to xml-files
void YammiModel::saveCategories()
{
	// save all categories to xml-files
	cout << "saving categories...\n";
	
	// for all categories, check whether dirty: if yes => save
	for( QListViewItem* f=gYammiGui->folderCategories->firstChild(); f; f=f->nextSibling() ) {
		Folder* folder=(Folder*)f;
		if(!folder->songList->dirty)
			continue;
		QString categoryName=folder->folderName();
		cout << "folder " << categoryName << " (or songs therein) was modified, saving\n";
		
		// create xml-file
		QDomDocument doc( "category" );
		QString empty("<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE songdb>\n<category>\n</category>\n");
		if ( !doc.setContent( empty ) ) {
			cout << "ERROR: could not create xml file for category " << categoryName << "\n";
			continue;
		}
		
		QDomElement rootElem = doc.documentElement();
		rootElem.setAttribute("name", categoryName);
		
		// for all songs contained in that category...
		Song* s=folder->firstSong();
		for(; s; s=folder->nextSong()) {
			QDomElement elem = doc.createElement( "song" );
			elem.setAttribute( "artist", s->artist.utf8() );
			elem.setAttribute( "title", s->title.utf8() );
			elem.setAttribute( "album", s->album.utf8() );
			rootElem.appendChild( elem );
		}
		// save category to file...
		QString save=doc.toString();
		QString categoryFilename=QString("%1.xml").arg(categoryName);
		QFile file( config.yammiBaseDir+"/categories/"+categoryFilename );
		if ( !file.open( IO_WriteOnly  ) )
			return;
		file.writeBlock ( save, save.length() );
		file.close();
		folder->songList->dirty=false;
	}
	cout << "..done\n";
	categoriesChanged(false);
}



/** reads the yammi database (an xml-file with all song information)
 */
void YammiModel::readSongDatabase()
{
  cout << "reading song database..." << flush;
	// read in our xml-file
	noDatabaseFound=true;
	allSongsChanged(false);

  QDomDocument doc( "songdb" );
	QFile f( config.yammiBaseDir+"/songdb.xml" );
	if ( !f.open( IO_ReadOnly ) ) {
		cout << "\ncould not open song database file (first time started?)... => no songs in database\n";
		cout << "edit the base directory in the preferences and perform a database update to scan for songs...\n";
		return;
	}
	if ( !doc.setContent( &f ) ) {
		f.close();
		return;
	}
	f.close();

	noDatabaseFound=false;
	// get root element
	QDomElement docElem = doc.documentElement();
	QString version=docElem.attribute("yammiVersion", "no version");
	if(version!=config.yammiVersion) {
		QString msg("");
		msg+="Your song database is from version "+version+" of Yammi.\n";
		msg+="This Yammi version: "+config.yammiVersion+"\n\n";
		if(version=="0.5.3" || version=="0.6" || version=="0.6.1" || version=="0.7" || version=="0.7.1" || version=="0.8.0beta" || version=="0.8.0" || version=="0.8.1") {
			msg+="However, the database format did not change since then, so no worries!\n\n";
      msg+="(The next time your database will be saved, it will be marked with the new version)";
		}
    else {  
      if(version=="0.5.2" || version=="0.5.1" || version=="0.5" || version=="no version") {
        msg+="Yammi can only read a song database saved with version 0.5.3 or later,\n";
        msg+="sorry.............\n";
        msg+="You can either get version 0.5.3 of Yammi\n";
        msg+="and save your database with it,\n";
        msg+="or you start with an empty database\n";
        msg+="and scan your harddisk for your songs\n";
        msg+="(categories and history will be lost, too!)\n\n";
        msg+="I'm trying to read it anyway...";
      }
      else {
        msg+="Your database is probably from a future version of Yammi!\n\n";
        msg+="I cannot guarantee whether that might cause any problems...\n";
        msg+="...so better be careful...";
      }
    }
    
		QMessageBox::information(gYammiGui, "Yammi", msg, "OK");
    allSongsChanged(true);
	}
	QDomNode n = docElem.firstChild();
	for(int i=0; !n.isNull(); i++) {
		QDomElement e = n.toElement();						// try to convert the node to an element.
		if( !e.isNull() ) { 											// the node is really an element.
			if(i % 100==0)
				cout << "." << flush;
			QString artist=e.attribute("artist", "unknown");
			QString title=e.attribute("title", "unknown");
			QString album=e.attribute("album", "");
			QString filename=e.attribute("filename", "");
			QString path=e.attribute("path", "");
			QString comment=e.attribute("comment", "");
			QString lengthStr=e.attribute("length", "0");
			unsigned long length=atol(lengthStr);
			QString bitrateStr=e.attribute("bitrate", "0");
			int bitrate=atoi(bitrateStr);
			QString yearStr=e.attribute("year", "0");
			int year=atoi(yearStr);
			QString trackNrStr=e.attribute("trackNr", "0");
			int trackNr=atoi(trackNrStr);
			QString genreNrStr=e.attribute("genreNr", "0");
			int genreNr=atoi(genreNrStr);
			
			QString addedToStr=e.attribute("addedTo", "1/1/1970/00:00:00");
			// read date as "dd/mm/yyyy, hh:mm:ss"
			MyDateTime addedTo;
			addedTo.readFromString(addedToStr);
			
			
			Song* newSong=new Song(artist, title, album, filename, path, length, bitrate, addedTo, year, comment, trackNr, genreNr);
			allSongs.appendSong( newSong );

			QDomNode child = e.firstChild();
		  while( !child.isNull() ) {
  	 		QDomElement mediaElem = child.toElement(); // try to convert the node to an element.
				if( !mediaElem.isNull() ) { // the node was really an element.
					QString mediaName=mediaElem.attribute("mediaName", "unspecified media");
					QString mediaLocation=mediaElem.attribute("mediaLocation", "unspecified location");
   				newSong->addMediaLocation(mediaName, mediaLocation);
  	    }
	      child = child.nextSibling();
			}
		}
		n = n.nextSibling();
	}
	cout << "\n..done (" << allSongs.count() << " songs)\n";
}

/// saves the songs in allSongs into an xml-file
void YammiModel::saveSongDatabase()
{
	cout << "saving database...\n";
	int sumDirtyTags=0;
	int sumDirtyFilenames=0;
	for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong()) {
		if(s->tagsDirty)
			sumDirtyTags++;
		if(s->filenameDirty)
			sumDirtyFilenames++;
	}
	if(config.tagsConsistent)
		cout << sumDirtyTags 			<< " dirty tags...\n";
	if(config.filenamesConsistent)
		cout << sumDirtyFilenames << " dirty filenames...\n";

	
	// create xml-file
	QDomDocument doc( "songdb" );
	QString empty("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<songs yammiVersion=\""+config.yammiVersion+"\">\n</songs>\n");
	if ( !doc.setContent( empty ) ) {
		cout << "ERROR: could not create xml file for song database\n";
		return;
	}
	QDomElement rootElem = doc.documentElement();
	
	// iterate through songs and save each song as a xml song element
	int count=0;
	for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong(), count++) {
		// lets append a new element to the end of our xml database
		QDomElement elem = doc.createElement( "song" );
		// consistencyMode: if song dirty, we make it consistent
		if(s->tagsDirty && config.tagsConsistent)
			s->saveTags();
		if(s->filenameDirty && config.filenamesConsistent)
			s->saveFilename();

		
		if(s->artist!="unknown")	elem.setAttribute( "artist", s->artist.utf8() );
		if(s->title!="unknown")		elem.setAttribute( "title", s->title.utf8() );
		if(s->album!="") 					elem.setAttribute( "album", s->album.utf8() );
		if(s->bitrate!=0)					elem.setAttribute( "bitrate", QString("%1").arg(s->bitrate).utf8() );
		if(s->comment!="")				elem.setAttribute( "comment", s->comment.utf8() );
		if(s->filename!="")				elem.setAttribute( "filename", s->filename.utf8() );
		if(s->length!=0)					elem.setAttribute( "length", QString("%1").arg(s->length).utf8() );
		if(s->path!="")						elem.setAttribute( "path", s->path.utf8() );
		if(s->year!=0)						elem.setAttribute( "year", QString("%1").arg(s->year).utf8() );
		if(s->trackNr!=0)					elem.setAttribute( "trackNr", QString("%1").arg(s->trackNr).utf8() );
		if(s->genreNr!=0)					elem.setAttribute( "genreNr", QString("%1").arg(s->genreNr).utf8() );
		if(true)									elem.setAttribute( "addedTo", s->addedTo.writeToString().utf8());
		
		
		for(unsigned int i=0; i<s->mediaName.count(); i++) {
			QDomElement media = doc.createElement( "media" );
			media.setAttribute( "mediaName", s->mediaName[i].utf8());
			media.setAttribute( "mediaLocation", s->mediaLocation[i].utf8());
			elem.appendChild(media);
		}
		rootElem.appendChild( elem );
		
	}

		
	// save songdb to file... (but first we make a backup of old file)
	QDir dir;
	if(dir.rename(config.yammiBaseDir+"/songdb.xml", config.yammiBaseDir+"/songdb_backup.xml"))
		cout << "backup of songdb saved in \"songdb_backup.xml\"\n";
	QString save=doc.toString();
	QFile f2( config.yammiBaseDir+"/songdb.xml");
	if(!f2.open(IO_WriteOnly)) {
		cout << "ERROR: could not write song database\n";
		return;
	}
	f2.writeBlock(save, save.length());
	f2.close();
	cout << " ...done\n";
	allSongsChanged(false);
}



void YammiModel::allSongsChanged(bool changed)
{
	_allSongsChanged=changed;
//  if(gYammiGui && gYammiGui->tbSaveDatabase)
//    gYammiGui->tbSaveDatabase->setEnabled(_allSongsChanged || _categoriesChanged);
}

bool YammiModel::allSongsChanged()
{
	return _allSongsChanged;
}

void YammiModel::categoriesChanged(bool changed)
{
	_categoriesChanged=changed;
//  if(gYammiGui && gYammiGui->tbSaveDatabase)
//  	gYammiGui->tbSaveDatabase->setEnabled(_allSongsChanged || _categoriesChanged);
}

bool YammiModel::categoriesChanged()
{
	return _categoriesChanged;
}


/**
 * Updates the xml-database by scanning harddisk
 * - if specified, checks existence of files in databse and updates/deletes entries
 * - scans recursively, starting from specified scanDir
 * - constructs song objects from all files matching the filePattern
 * - checks whether already existing, whether modified, if not => inserts into database
 */
void YammiModel::updateSongDatabase(QString scanDir, QString filePattern, QString mediaName, QProgressDialog* progress)
{
	entriesAdded=0;
	corruptSongs=0;
	if(config.childSafe)
		return;
  problematicSongs.clear();
	if(mediaName==0) {
		cout << "scanning harddisk for new songs... \n";
		// check that scanDir is an existing directory
		QDir d(scanDir);
		if(!d.exists()) {
      QString msg="The base directory for scanning does not exist!\n";
      msg+="Set value \"scanDir\" in preferences to an existing directory!";
      QMessageBox::information( gYammiGui, "Yammi", msg, "Good idea!" );
		}
		else {
			traverse(scanDir, filePattern, progress);
 			cout << "..finished scanning!\n";
	 	}
	}
	else {				
		cout << "scanning removable media for new songs... \n";
		
		// mount media dir
		if(config.mountMediaDir) {
			// linux specific
			QString cmd;
			cmd=QString("mount %1").arg(scanDir);
			system(cmd);
		}

		// check that mediaDir is an existing directory
		QDir d(config.mediaDir);
		if(!d.exists()) {
      QString msg="The directory for removable media does not exist or is not readable!\n";
      msg+="Set value \"mediaDir\" in preferences to an existing directory!\n";
      msg+="(if necessary, enable \"mount media\" in preferences)";
      QMessageBox::information( gYammiGui, "Yammi", msg, "Good idea!" );
		}
		else {
			traverse(scanDir, filePattern, progress, mediaName);
 			cout << "..finished scanning!\n";
	 	}
		// umount media dir
		if(config.mountMediaDir) {
			// linux specific
			QString cmd;
			cmd=QString("umount %1").arg(scanDir);
			system(cmd);
		}
	}
}




/// traverses a directory recursively and processes all mp3 files
/// returns false, if scanning was cancelled
bool YammiModel::traverse(QString path, QString filePattern, QProgressDialog* progress, QString mediaName)
{
	// leave out the following directories
	if(path+"/"==config.trashDir || path+"/"==config.swapDir) {
		cout << "skipping trash or swap directory: " << path << "\n";
		return true;
	}
 	
 	cout << "scanning directory " << path << "\n";
	progress->setLabelText("scanning directory "+path+"...");
  progress->setProgress(0);
//  qApp->processEvents();
	
	QDir d(path);

  // step 1: scan files
	
  d.setFilter(QDir::Files);
  d.setNameFilter(filePattern);
	d.setSorting( QDir::Name );
	const QFileInfoList* list = d.entryInfoList();
	int filesToScan=list->count();
	progress->setTotalSteps(filesToScan);
	QFileInfoListIterator it( *list );								      // create list iterator

	int filesScanned=0;
	for(QFileInfo *fi; (fi=it.current()); ++it ) {						// for each file/dir
		filesScanned++;
	  progress->setProgress(filesScanned);
//	  qApp->processEvents();
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
	QFileInfoListIterator it2( *list2 );								      // create list iterator

	for(QFileInfo *fi2; (fi2=it2.current()); ++it2 ) {						// for each file/dir
		if(fi2->fileName()=="." || fi2->fileName()=="..")
      continue;
		if(traverse(fi2->filePath(), filePattern, progress, mediaName)==false) {
      return false;
    }
	}
  return true;      // scanning was not cancelled
}


/** adds a single songfile to the database */
void YammiModel::addSongToDatabase(QString filename, QString mediaName=0)
{
 	cout << "scanning file: " << filename << "...\n";
 	bool found=false;
 	for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong()) {
 		// this check might fail when filename has strange characters?
 		if(filename==s->location()) {
 			found=true;
 			cout << "...existing\n";
 			// here we can fix/update our database with additional info...
				
 			/* eg:
 			// add genre number to Song info
 			Song* fixSong=new Song(fi->filePath());
 			s->genreNr=fixSong->genreNr;
 			delete(fixSong);
 			allSongsChanged(true);
 			*/
 			break;
 		}
 	}
 	if(found)
 		return;
		
 	// okay, new song (at least new filename/path) => construct song object
 	Song* newSong=new Song();
  newSong->create(filename, mediaName);
 	if(newSong->corrupted) {
 		cout << "new song file " << filename << " is corrupt (not readable for yammi), skipping\n";
 		corruptSongs++;
 		return;
 	}
						
 	// check whether other version of this song existing (with same primary key)
 	Song* s=allSongs.getSongByKey(newSong->artist, newSong->title, newSong->album);
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
 				}
 				else {
 					// case 1b: song has filename
 					QFileInfo fileInfo(s->location());
 					if(!fileInfo.exists()) {
 						// but still not available => probably has been moved
 						cout << "looks like file " << newSong->filename << " has been moved from " << s->path << " to " << newSong->path << ", correcting path info\n";
 						s->setTo(newSong);
 						allSongsChanged(true);
 						delete(newSong);
 						return;
 					}
 					// song is available, we don't need two songs => skip
 					cout << "file " << newSong->filename << " already available at " << s->location() << ", skipping\n";
 					delete(newSong);
 					return;
 				}
 			}
 			else {
 				// case 2: scanning removable media => add media, if not already added
 				bool exists=false;
 				for(unsigned int i=0; i<s->mediaName.count(); i++) {
 					if(s->mediaName[i]==mediaName)
 						exists=true;
 				}
 				if(!exists) {
 					cout << "adding media " << mediaName << " to mediaList in song " << s->displayName() << "\n";
 					QString locationOnMedia=filename;
 					if(locationOnMedia.left(config.mediaDir.length())!=config.mediaDir)
 						cout << "strange error, scanning media, but file not on media\n";
 					locationOnMedia=locationOnMedia.right(locationOnMedia.length()-config.mediaDir.length());							
 					s->addMediaLocation(mediaName, locationOnMedia);
 					allSongsChanged(true);
 				}
 				else {
 					cout << "song " << s->location() << " is already known to be on this media\n";
 				}
 				return;
 			}
 		}
 		else {
 			// b) not exactly the same => add as new (change title to make it unique)
 			// here we do not have to distinguish between harddisk and media
 			cout << "seems like new song <" << newSong->artist << " - " << newSong->title << "> already existing...\n";
 			cout << "(Yammi does not allow two songs with the same artist/title/album identification)\n";
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
 	cout << "Song added: " << newSong->displayName() << "\n";
	entriesAdded++;
 	allSongsChanged(true);
}
			



/**
 * checks consistency of all songs
 * @returns true, if consistent, false, if problematic songs were found
 */
bool YammiModel::checkConsistency(QProgressDialog* progress, MyList* selection, ConsistencyCheckParameter* p)
{
	if(config.childSafe) {
    cout << "sorry, not allowed in child-safe mode...\n";
		return true;
  }
  
	cout << "checking consistency of database... \n";
	problematicSongs.clear();


	// 1. iterate through all songs in database
  if(p->checkForExistence || p->checkTags || p->checkFilenames) {
      
    progress->setLabelText("Step 1: checking all songs in database...");
    progress->setTotalSteps(selection->count());
    progress->setProgress(0);
    qApp->processEvents();
	
    p->dirtyTags=0;
    p->dirtyFilenames=0;
    p->doublesFound=0;
    p->filenamesCorrected=0;
    p->nonExisting=0;
    p->nonExistingDeleted=0;
    p->nonExistingUpdated=0;
    p->tagsCorrected=0;

    p->correctTagsConfirmed=-1;
    p->correctFilenamesConfirmed=-1;
    
    int i=0;
    for(Song* s=selection->firstSong(); s; s=selection->nextSong(), i++) {
      if(i%10==0) {
        progress->setProgress(i);
      }
      if(progress->wasCancelled())
        break;		
      QString diagnosis=s->checkConsistency(true, true);
      if(diagnosis=="") {
        continue;
      }

      // okay, some kind of problem...

      if(diagnosis=="file not readable" && p->checkForExistence) {
        p->nonExisting++;
        cout << "file not existing or readable: " << s->displayName();
        bool onMedia=s->mediaName.count()>0;
        if(p->updateNonExisting) {
          // if we update, there are two cases:
          if(onMedia) {
            // 1. update entry: set filename+path to ""
            s->filename="";
            s->path="";
            p->nonExistingUpdated++;
            problematicSongs.append(new SongEntryString(s, "file not existing on harddisk any more, filename cleared"));
          }
          else {
            // 2. delete entry in database
            cout << "deleting entry " << s->displayName();
            gYammiGui->forSong(s, DeleteEntry);
            p->nonExistingDeleted++;
          }
        }
        else {
          problematicSongs.append(new SongEntryString(s, "file not readable"));
        }
      }


      if(diagnosis.contains("tags not correct") && p->checkTags) {
        p->dirtyTags++;
        if(p->correctTags) {

          bool reallyCorrect;
          if(p->correctTagsConfirmed==-1) {
            // warning dialog!
            ApplyToAllBase confirm(progress, "confirmDialog", true);
            QString msg=QString("Correct tags in file\n\t%1\nto new values:\n").arg(s->filename);
            msg+=QString("artist: %1, title: %2\n").arg(s->artist).arg(s->title);
            msg+=QString("album: %1, comment: %2\n").arg(s->album).arg(s->comment);
            msg+=QString("year: %1, trackNr: %2, genreNr: %3").arg(s->year).arg(s->trackNr).arg(s->genreNr);
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
            if(s->saveTags()) {
              p->tagsCorrected++;
              problematicSongs.append(new SongEntryString(s, "tags corrected"));
            }
          }
          else {
            problematicSongs.append(new SongEntryString(s, "tags not correct"));
          }
        }
        else {
          problematicSongs.append(new SongEntryString(s, "tags not correct"));
        }
      }
      

      if(diagnosis.contains("filename not consistent") && p->checkFilenames) {
        p->dirtyFilenames++;
        if(p->correctFilenames) {
          bool reallyCorrect;
          if(p->correctFilenamesConfirmed==-1) {
            // warning dialog!
            ApplyToAllBase confirm(progress, "confirmDialog", true);
            QString msg=QString("Correct filename in file\n\t%1\n").arg(s->filename);
            msg+=QString("to\n\t%1").arg(s->constructFilename());
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
            if(s->saveFilename()) {
              p->filenamesCorrected++;
              problematicSongs.append(new SongEntryString(s, "filename corrected"));
            }
          }
          else {
            problematicSongs.append(new SongEntryString(s, "filename not consistent"));
          }
        }
        else {
          problematicSongs.append(new SongEntryString(s, "filename not consistent"));
        }
      }
    }
	}

  
	// 2. check for songs contained twice
  if(!progress->wasCancelled() && p->checkDoubles) {
    progress->setLabelText("Step 2: check for song entries pointing to same file");
    progress->setTotalSteps(allSongs.count());
    progress->setProgress(0);
    qApp->processEvents();
	
    allSongs.setSortOrderAndSort(MyList::ByFilename + 16*(MyList::ByPath));
    Song* last=allSongs.firstSong();
    int i=0;
    // TODO: perform this check also only on the selected songs?
    for(Song* s=allSongs.nextSong(); s; s=allSongs.nextSong(), i++) {
      if(i % 20==0)
        progress->setProgress(i);
      if(progress->wasCancelled())
        break;		
      if(s->artist=="{wish}")			// ignore wishes
        continue;
      if(s->path=="" && s->filename=="")		// ignore songs not on local harddisk
        continue;
		
      // check for songs contained twice in database (but pointing to same file+path)
      if(last->location()==s->location()) {
        cout << "two database entries pointing to same file: " << s->filename << ", deleting one\n";
        allSongs.remove();		// problem, coz we are iterating through this list???
        allSongsChanged(true);
        p->doublesFound++;
        continue;
      }
		
      last=s;
    }

    // 3. check for two songs with identical primary key
    progress->setLabelText("Step 3: check for two songs with identical primary keys");
    progress->setTotalSteps(allSongs.count());
    progress->setProgress(0);
    qApp->processEvents();
	
    allSongs.setSortOrderAndSort(MyList::ByKey);
    last=allSongs.firstSong();
    i=0;
    for(Song* s=allSongs.nextSong(); s; s=allSongs.nextSong(), i++) {
      if(i % 20==0)
        progress->setProgress(i);
      if(progress->wasCancelled())
        break;		
      if(s->artist=="{wish}")
        continue;
	
      if(s->sameAs(last)) {
        cout << "!!!   song contained twice: " << s->filename << ", check in problematic songs!\n";
        problematicSongs.append(new SongEntryString(last, "contained twice(1)"));
        problematicSongs.append(new SongEntryString(s, "contained twice(2)"));
        p->doublesFound++;
      }
    }
  }



  // reset sortOrder
	allSongs.setSortOrderAndSort(MyList::ByKey);

	if(progress->wasCancelled())
		cout << "..consistency check aborted\n";
	cout << "..consistency checked\n";

	if(problematicSongs.count()==0) {
		cout << "your yammi database is nice and clean!\n";
		return true;
	}
	else {
		allSongsChanged(true);
		return false;
	}
}


void YammiModel::removeCategory(QString categoryName)
{
	QString name=categoryNames.first();
	int i=0;
	for(MyList* ptr=allCategories.first(); ptr; ptr=allCategories.next(), i++) {
		QString name=categoryNames[i];
		if(name==categoryName) {
			cout << "found, deleting..\n";
			allCategories.remove();
			categoryNames.remove(categoryNames.at(i));
			QDir d("/");
			QString todel=QString("%1/categories/%2.xml").arg(config.yammiBaseDir).arg(categoryName);
			d.remove(todel);
			break;
		}
	}
}

void YammiModel::renameCategory(QString oldCategoryName, QString newCategoryName)
{
	int i=0;
	for(MyList* ptr=allCategories.first(); ptr; ptr=allCategories.next(), i++) {
		QString name=categoryNames[i];
		if(name==oldCategoryName) {
			cout << "found, renaming..\n";
			categoryNames[i]=newCategoryName;
			ptr->dirty=true;
			categoriesChanged(true);
			QDir dir;
			if(!dir.rename(	config.yammiBaseDir+"/categories/"+oldCategoryName+".xml",
									config.yammiBaseDir+"/categories/"+newCategoryName+".xml"))
				cout << "could not rename category file\n";
			break;
		}
	}
}

void YammiModel::newCategory(QString categoryName)
{
	MyList* newList=new MyList;
	allCategories.append(newList);
	categoryNames.append(categoryName);
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
{
	QApplication::setOverrideCursor( Qt::waitCursor );
	// save dirty categories
	saveCategories();
	if(allSongsChanged())
		saveSongDatabase();
	if(allSongsChanged() || config.logging)
		saveHistory();

	QApplication::restoreOverrideCursor();
}



/* removing a media, ie. removing the media entry in all songs that are on it
 */
void YammiModel::removeMedia(QString mediaToDelete)
{
	cout << "removing media: " << mediaToDelete << "\n";
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
  QString cmd=QString("rm -r \"%1/media/%2\"").arg(config.yammiBaseDir).arg(mediaToDelete);
	system(cmd);
	allSongsChanged(true);
}

void YammiModel::renameMedia(QString oldMediaName, QString newMediaName)
{
	for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong()) {
		s->renameMedia(oldMediaName, newMediaName);
	}
	// now move the directory (if existing)
	QDir dir;
	if(!dir.rename(	config.yammiBaseDir+"/media/"+oldMediaName,
									config.yammiBaseDir+"/media/"+newMediaName))
		cout << "could not rename media dir!\n";
	allSongsChanged(true);
}

/** marks those playlists as dirty that contain the given song
 */
void YammiModel::markPlaylists(Song* s)
{	
	// for all categories, check whether they contain the song, if yes => mark as dirty
	for( QListViewItem* f=gYammiGui->folderCategories->firstChild(); f; f=f->nextSibling() ) {
		Folder* folder=(Folder*)f;
		if(folder->songList->containsSong(s))
			folder->songList->dirty=true;
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

	if(path==config.swapDir) {
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


/** checks whether a song is available on the local harddisk
 * or needs to be retrieved from a removable media
 * if song available, returns the complete path+filename to the songfile
 * (if in swap dir, the file will be touched to implement the LRU strategy)
 * if not yet available, returns ""
 * if never available, returns "never"
 */
QString YammiModel::checkAvailability(Song* s, bool touch)
{
	if(s->location()!="/") {
		QFileInfo fi(s->location());
		if(fi.exists() && fi.isReadable()) {
			return s->location();
		}
//		cout << "song " << s->displayName() << "has location given, but file does not exist or is not readable!\n";
	}
	// no location given, check whether already existing in swap dir
	QString dir=config.swapDir;
	QString filename=s->constructFilename();
	QFileInfo fi(dir+filename);
	if(fi.exists() && fi.isReadable()) {
		if(touch) {
			// linux specific
			QString cmd;
			cmd=QString("touch \"%1\"").arg(dir+filename);
			system(cmd);
/*		does not work: touching a file
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
	if(s->mediaLocation.count()!=0)
		return "";
	else
		return "never";
}

