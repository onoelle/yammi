/***************************************************************************
                          yammimodel.cpp  -  description
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

#include "yammimodel.h"

YammiModel::YammiModel(){
}
YammiModel::~YammiModel(){
}


/// read preferences from xml-file
void YammiModel::readPreferences()
{
	cout << "reading preferences...\n";
	QDir d = QDir::home();  // now points to home directory
	if(!d.cd(".yammi")) {
		startFirstTime();			// return on false?
		d.cd(".yammi");
	}

	config.yammiBaseDir=d.absPath();
	QDomDocument doc( "prefs" );
	QFile f( config.yammiBaseDir+"/prefs.xml" );
	if ( !f.open( IO_ReadOnly ) ) {
		cout << "no preferences found (first time started?)... using defaults\n";
		return;
	}	
	if ( !doc.setContent( &f ) ) {
		cout << "error in parsing preferences-file (xml-file corrupt?)... => using defaults\n";
		f.close();
		return;
	}
	f.close();

	// 1: get prefs from file
	config.baseDir					=	getProperty(&doc, "baseDir", config.baseDir);
	config.scanDir					=	getProperty(&doc, "scanDir", config.scanDir);
	config.doubleClickAction=	(action) getProperty(&doc, "doubleClickAction", config.doubleClickAction);
	config.middleClickAction=	(action) getProperty(&doc, "middleClickAction", config.middleClickAction);
	config.cutShort					=	getProperty(&doc, "cutShort", config.cutShort);
	config.logging					=	getProperty(&doc, "logging", config.logging);
	config.childSafe				=	getProperty(&doc, "childSafe", config.childSafe);
	config.tagsConsistent		=	getProperty(&doc, "tagsConsistent", config.tagsConsistent);
	config.filenamesConsistent =	getProperty(&doc, "filenamesConsistent", config.filenamesConsistent);
	config.criticalSize			=	getProperty(&doc, "criticalSize", config.criticalSize);
	config.secondSoundDevice=	getProperty(&doc, "secondSoundDevice", config.secondSoundDevice);
	cout << "..done\n";
}


/// save preferences (if changed) to disk
void YammiModel::savePreferences()
{
	cout << "saving preferences...\n";
	
	// create xml-file
	QDomDocument doc( "prefs" );
	QString empty("<?xml version=\"1.0\" encoding=\"UTF-16\"?><!DOCTYPE prefs>\n<prefs>\n</prefs>\n");
	if ( !doc.setContent( empty ) ) {
		cout << "ERROR: saving preferences failed!\n";
		return;
	}
	
	// iterate through properties and save each property as an element
	setProperty(&doc, "baseDir", 						config.baseDir);
	setProperty(&doc, "scanDir", 						config.scanDir);
	setProperty(&doc, "doubleClickAction",	config.doubleClickAction);
	setProperty(&doc, "middleClickAction",	config.middleClickAction);
	setProperty(&doc, "cutShort", 					config.cutShort);
	setProperty(&doc, "logging", 						config.logging);
	setProperty(&doc, "childSafe", 					config.childSafe);
	setProperty(&doc, "tagsConsistent", 		config.tagsConsistent);
	setProperty(&doc, "filenamesConsistent",config.filenamesConsistent);
	setProperty(&doc, "criticalSize", 			config.criticalSize);
	setProperty(&doc, "secondSoundDevice",		config.secondSoundDevice);
	
	// save to file...
	QString save=doc.toString();
	QFile f2( config.yammiBaseDir+"/prefs.xml" );
	if ( !f2.open( IO_WriteOnly  ) )
		return;
	f2.writeBlock ( save, save.length() );
	f2.close();
	cout << " ...done\n";
}




/// get int property
int YammiModel::getProperty(const QDomDocument* doc, const QString propName, const int propDefault)
{
	return atoi(getProperty(doc, propName, QString("%1").arg(propDefault)));
}

/// get bool property
bool YammiModel::getProperty(const QDomDocument* doc, const QString propName, const bool propDefault)
{
	QString str=getProperty(doc, propName, QString("%1").arg(propDefault));
	return (str=="1");
}

/// read a property from the given xml document or set it to the default value
QString YammiModel::getProperty(const QDomDocument* doc, const QString propName, const QString propDefault="")
{	
	QDomNodeList list=doc->elementsByTagName ( propName );
	QDomNode node=list.item(0);					// we only retrieve first item
	if(node.isNull() || !node.isElement()) {
		cout << "setting " << propName << " to default value: " << propDefault << "\n";
		return propDefault;
	}
	QDomElement elem=node.toElement();
	return elem.text();
}


/// set an int property
void YammiModel::setProperty(QDomDocument* doc, const QString propName, const int propValue)
{
	setProperty(doc, propName, QString("%1").arg(propValue));
}

/// set a bool property
void YammiModel::setProperty(QDomDocument* doc, const QString propName, const bool propValue)
{
	setProperty(doc, propName, QString("%1").arg(propValue));
}
/// set a string property
void YammiModel::setProperty(QDomDocument* doc, const QString propName, const QString propValue)
{
	QDomElement rootElem = doc->documentElement();
	QDomElement elem = doc->createElement(propName);
	QDomText domText=doc->createTextNode ( propValue );
	elem.appendChild(domText);
	rootElem.appendChild(elem);
}


/**
 * called when the program is started the first time by a user
 * (ie. there is no .yammi directory existing in the user's home dir)
 */
bool YammiModel::startFirstTime()
{
	cout << "you seem to start Yammi for the first time!\n";
	cout << "creating directory .yammi in your home directory...";
	QDir d = QDir::home();  // now points to home directory
 	if ( !d.mkdir( ".yammi" ) ) {
 		cout << "\nERROR: Could not create directory .yammi in your home directory...";
 		return false;
 	}
 	d.cd(".yammi");
 	cout << " ..done\n";
 	cout << "creating subdirectory categories to store your categories...";
 	if ( !d.mkdir( "categories" ) ) {
 		cout << "\nERROR: Could not create directory categories...";
 		return false;
 	}
 	cout << " ..done\n";
 	cout << "creating subdirectory media to store your media info...";
 	if ( !d.mkdir( "media" ) ) {
 		cout << "\nERROR: Could not create directory media...";
 		return false;
 	}
	cout << " ..done\n";

	// touch history file, so it exists
	system( QString("touch %1/logfile.log").arg(config.yammiBaseDir) );
//	QFile f(config.yammiBaseDir+"/logfile.log");
//	if ( !f.open( IO_ReadOnly ) ) {
 	
	cout << "Everything successfully initialized, have fun!\n";
	return true;
}



/**
 * Reads the category from the found xml files in playlist directory.
 */
void YammiModel::readCategories()
{
	// read in all xml-files found in a given directory that represent a category
	cout << "reading categories...\n";
	
	QDir d(config.yammiBaseDir+"/categories");
	d.setFilter( QDir::Files);
	d.setSorting( QDir::DirsFirst );
	const QFileInfoList *list = d.entryInfoList();
	QFileInfoListIterator it( *list );      // create list iterator
	QFileInfo *fi;                          // pointer for traversing
	QDomDocument doc("category");

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
		QDomNode n = docElem.firstChild();
   	QString categoryName(docElem.attribute("name"));
		
		// add category to database
		MyList* ptr=new MyList;
		allCategories.append(ptr);
		categoryNames.append(categoryName);

		// add all songs contained
		while( !n.isNull() ) {
			QDomElement e = n.toElement();						// try to convert the node to an element.
			if( !e.isNull() ) { 											// the node is really an element.
				QString artist=e.attribute("artist");
				QString title=e.attribute("title");
				// search for item in allSongs
				bool found=false;
				for(Song* s=allSongs.first(); s; s=allSongs.next()) {
					if(s->title==title && s->artist==artist) {
						ptr->append(s);
						found=true;
						break;
					}
				}
				if(!found) {
					cout << "category item " << artist << " - " << title << " not found in song database\n";
					// as long as we modify tags only inside PM this should not occur!
				}
			}
			n = n.nextSibling();
		}
  }
	
	cout << "..done\n";
}

/**
 * Reads song history
 */
void YammiModel::readHistory()
{
	// read in history of songs from logfile
	cout << "reading song history...\n";
	
	QFile f(config.yammiBaseDir+"/logfile.log");
	if ( !f.open( IO_ReadOnly ) ) {
		cout << "could not read song history\n";
		return;
	}
	
	char buf[200];
	songHistory.setSortOrder(-1);
	// read in line per line
	while( f.readLine(buf, 200)>0 ) {
		QString entry(buf);
		// not very nice, I know... (change logfile also to xml format?)
		int pos1=entry.find('>', 0);
		int pos2=entry.find('>', pos1+1);
		int pos3=entry.find('>', pos2+1);
		
		QString artist=entry.mid(1, pos1-1);
		QString title=entry.mid(pos1+2, pos2-(pos1+2));
		QString timestamp=entry.mid(pos2+2, pos3-(pos2+2));
		
		// search for item in allSongs
		bool found=false;
		for(Song* s=allSongs.first(); s; s=allSongs.next()) {
			if(s->title==title && s->artist==artist) {
				songHistory.append(s);
				MyDateTime played;
				played.readFromString(timestamp);
				songHistoryTimestamp.append(&played);
				found=true;
				break;
			}
		}
		if(!found) {
//			cout << "history item " << artist << " - " << title << " not found in song database\n";
		}
  }
	f.close();	
	cout << "..done\n";
}


/// save categories (if changed) to xml-files
void YammiModel::saveCategories()
{
	// save all categories to xml-files
	cout << "saving categories...\n";
	
	// for all categories...
	QString categoryName=categoryNames.first();
	for(MyList* ptr=allCategories.first(); ptr; ptr=allCategories.next()) {
		// create xml-file
		QDomDocument doc( "category" );
		QString empty("<?xml version=\"1.0\" encoding=\"UTF-16\"?><!DOCTYPE songdb>\n<category>\n</category>\n");
		if ( !doc.setContent( empty ) ) {
			cout << "ERROR: could not save categories!\n";
			return;
		}
		
		QDomElement rootElem = doc.documentElement();
		rootElem.setAttribute("name", categoryName);
		
		// for all songs contained in that category...
		Song* s=ptr->first();
		for(; s; s=ptr->next()) {
			QDomElement elem = doc.createElement( "song" );
			elem.setAttribute( "artist", s->artist );
			elem.setAttribute( "title", s->title );
			rootElem.appendChild( elem );
		}
		// save category to file...
		QString save=doc.toString();
		QString categoryFilename=QString("%1.xml").arg(categoryName);
		QFile f2( config.yammiBaseDir+"/categories/"+categoryFilename );
		if ( !f2.open( IO_WriteOnly  ) )
			return;
		f2.writeBlock ( save, save.length() );
		f2.close();
		categoryName=categoryNames.next();
	}
	cout << "..done\n";
	categoriesChanged(false);
}



/** reads the xml-file with all songs, stores to QList allSongs
 *
 * returns	void
 */
void YammiModel::readSongDatabase()
{
	// read in our xml-file
	cout << "reading song database...\n";
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

	// get root element
	QDomElement docElem = doc.documentElement();
	QDomNode n = docElem.firstChild();
	while( !n.isNull() ) {
		QDomElement e = n.toElement();						// try to convert the node to an element.
		if( !e.isNull() ) { 											// the node is really an element.
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
			
			QString addedToStr=e.attribute("addedTo", "1/1/1970/00:00:00");
			// read date as "dd/mm/yyyy, hh:mm:ss"
			MyDateTime addedTo;
			addedTo.readFromString(addedToStr);
			
			
			Song* newSong=new Song(artist, title, album, filename, path, length, bitrate, addedTo, year, comment, trackNr);
			allSongs.append( newSong );

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
	cout << "..done (" << allSongs.count() << " songs)\n";
}

/// saves the songs in allSongs into an xml-file
void YammiModel::saveSongDatabase()
{
	cout << "saving database...\n";
	int sumDirtyTags=0;
	int sumDirtyFilenames=0;
	for(Song* s=allSongs.first(); s; s=allSongs.next()) {
		if(s->tagsDirty)
			sumDirtyTags++;
		if(s->filenameDirty)
			sumDirtyFilenames++;
	}
	cout << sumDirtyTags 			<< " dirty tags...\n";
	cout << sumDirtyFilenames << " dirty filenames...\n";

	
	// create xml-file
	QDomDocument doc( "songdb" );
	QString empty("<?xml version=\"1.0\" encoding=\"UTF-16\"?><!DOCTYPE songdb>\n<songs>\n</songs>\n");
	if ( !doc.setContent( empty ) ) {
		cout << "saving failed!\n";
		return;
	}
	QDomElement rootElem = doc.documentElement();
	
	// iterate through songs and save each song as a xml song element
	for(Song* s=allSongs.first(); s; s=allSongs.next()) {
		// lets append a new element to the end of our xml database
		QDomElement elem = doc.createElement( "song" );
		
		// consistencyMode: if song dirty, we make it consistent
		if(s->tagsDirty && config.tagsConsistent)
			s->saveTags();
		if(s->filenameDirty && config.filenamesConsistent)
			s->saveFilename();

		
		// save addedTo date as "dd/mm/yyyy, hh:mm:ss"
		elem.setAttribute( "addedTo", s->addedTo.writeToString());
		if(s->album!="") 					elem.setAttribute( "album", s->album );
		if(s->artist!="unknown")	elem.setAttribute( "artist", s->artist );
		if(s->bitrate!=0)					elem.setAttribute( "bitrate", QString("%1").arg(s->bitrate) );
		if(s->comment!="")				elem.setAttribute( "comment", s->comment );
		if(s->filename!="")				elem.setAttribute( "filename", s->filename );
		if(s->length!=0)					elem.setAttribute( "length", QString("%1").arg(s->length) );
		if(s->path!="")						elem.setAttribute( "path", s->path );
		if(s->title!="unknown")		elem.setAttribute( "title", s->title );
		if(s->year!=0)						elem.setAttribute( "year", QString("%1").arg(s->year) );
		if(s->trackNr!=0)					elem.setAttribute( "trackNr", QString("%1").arg(s->trackNr) );
		
		for(unsigned int i=0; i<s->mediaName.count(); i++) {
			QDomElement media = doc.createElement( "media" );
			media.setAttribute( "mediaName", s->mediaName[i]);
			media.setAttribute( "mediaLocation", s->mediaLocation[i]);
			elem.appendChild(media);
		}
		rootElem.appendChild( elem );
		
	}

		
	// save songdb to file... (but first we make a backup of old file)
	system("mv "+config.yammiBaseDir+"/songdb.xml "+config.yammiBaseDir+"/songdb_backup.xml" );
	QString save=doc.toString();
	QFile f2( config.yammiBaseDir+"/songdb.xml");
	if(!f2.open(IO_WriteOnly)) {
		cout << "\nERROR: could not write song database\n";
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
//mmm	tbSaveDatabase->setEnabled(_allSongsChanged || _categoriesChanged);
}

bool YammiModel::allSongsChanged()
{
	return _allSongsChanged;
}

void YammiModel::categoriesChanged(bool changed)
{
	_categoriesChanged=changed;
//mmm	tbSaveDatabase->setEnabled(_allSongsChanged || _categoriesChanged);
}

bool YammiModel::categoriesChanged()
{
	return _categoriesChanged;
}


/// traverses a directory recursively and processes all mp3 files
/// puts songs where heuristic is not sure into the problematicSongs list
void YammiModel::traverse(QString path)
{
	// leave out the following directories
	if(path==config.baseDir+"/trash" || path==config.baseDir+"/corrupted") {
		cout << "skipping directory " << path << "\n";
		return;
	}
 	
 	cout << "scanning directory " << path << "\n";
	QDir d(path);
	d.setFilter(QDir::Files | QDir::Dirs);
//	d.setNameFilter("*.mp3 *.MP3 *.wav");
	d.setSorting( QDir::DirsFirst );
	const QFileInfoList *list = d.entryInfoList();
	QFileInfoListIterator it( *list );								      // create list iterator

	for(QFileInfo *fi; (fi=it.current()); ++it ) {						// for each file/dir
		// if directory...		=> scan recursive
		if (fi->isDir()) {
			if(fi->fileName()!="." && fi->fileName()!="..")
				traverse(fi->filePath());
		}
		// if file...					=> check whether we have a valid mp3 file
		if ((fi->extension(FALSE)).upper()!="MP3")
			continue;
			
		// okay, we have an mp3 file, check whether already in database
		bool found=false;
		for(Song* s=allSongs.first(); s; s=allSongs.next()) {
			// this check might fail when filename has strange characters
			if(fi->fileName()==s->filename && fi->dirPath(true)==s->path) {
				found=true;
				
				// here we can fix/update our database with additional info...
				/*
				Song* fixSong=new Song(fi->filePath());
				eg. s->year=fixSong->year;
				eg. s->trackNr=fixSong->trackNr;
				delete(fixSong);
				allSongsChanged(true);
				*/
				break;
			}
		}
		if(found)
			continue;
		
		// okay, new song (at least new filename/path) => construct song object
		Song* newSong=new Song(fi->filePath());
		if (newSong->corrupted) {
			cout << "new song file " << fi->filePath() << " is corrupt, skipping and moving to " << config.baseDir << "corrupt/ \n";
			corruptSongs++;
			newSong->moveTo(config.baseDir+"/corrupted");
			continue;
		}
		
		// check whether heuristic is sure about artist/title
		if(!newSong->artistSure) {
			cout << "Please check  >>ARTIST<< on filename: " << newSong->filename << " (inserted into problematicSongs)";
			problematicSongs.append(newSong);
		}
		if(!newSong->titleSure) {
			cout << "Please check  >>TITLE<< on filename: " << newSong->filename << " (inserted into problematicSongs)";
			problematicSongs.append(newSong);
		}

				
		// check whether other version of this song existing (with same "artist-title" identification)
		for(Song* s=allSongs.first(); s; s=allSongs.next()) {
			if( (newSong->title==s->title && newSong->artist==s->artist) || (newSong->filename==s->filename) ) {
				QFileInfo x(s->location());
				if(!x.exists()) {
					// old file does not exist => probably file has been moved
					cout << "looks like file " << newSong->filename << " has been moved from " << s->path << " to " << newSong->path << ", correcting path info\n";
					s->setTo(newSong);
					allSongsChanged(true);
					delete(newSong);
					found=true;
					break;
				}
				
				cout << "seems like new song <" << newSong->artist << " - " << newSong->title << "> already existing, please check consistency!\n";
				problematicSongs.append(newSong);
 				break;
 			}
 		}
 		
 		if(!found) {
			// new song, not in database yet
			allSongs.append(newSong);
			cout << "Song added: " << newSong->displayName() << "\n";
			songsAdded++;
			allSongsChanged(true);
		}
	}
}
			


/**
 * Updates the xml-database by scanning harddisk.
 * - constructs song objects from all mp3 files found under baseDir
 * - checks whether already existing, whether modified
 * - if not => inserts new song object into database
 */
void YammiModel::updateSongDatabase()
{
	if(config.childSafe)
		return;
	cout << "scanning harddisk for new songs... \n";
	problematicSongs.clear();
	songsAdded=0;
	corruptSongs=0;
	traverse(config.scanDir);
 	cout << "..finished scanning!\n";
}


/**
 * checks consistency of all songs
 * @returns true, if consistent, false, if problematic songs
 */
bool YammiModel::checkConsistency()
{
	if(config.childSafe)
		return true;
	cout << "checking consistency of database... \n";
	problematicSongs.clear();
	
	// 1. file existing, correct tags, correct filename?
	int sumDirtyTags=0;
	int sumDirtyFilenames=0;
	for(Song* s=allSongs.first(); s; s=allSongs.next()) {
		if(s->checkConsistency()==false) {
			if(problematicSongs.containsRef(s)==0)
				problematicSongs.append(s);
		}
		if(s->tagsDirty)
			sumDirtyTags++;
		if(s->filenameDirty)
			sumDirtyFilenames++;
	}
		
	// 2. check for songs contained twice
	allSongs.setSortOrder(MyList::ByFilename);
	allSongs.sort();
	Song* last=allSongs.first();
	for(Song* s=allSongs.next(); s; s=allSongs.next()) {
		if(s->artist=="{wish}")
			continue;
		
		// check for songs contained twice in database (but pointing to same file+path)
		if(last->location()==s->location()) {
			cout << "two database entries pointing to same song/file: " << s->filename << ", deleting one\n";
			allSongs.remove();		// problem, coz we are iterating through this list???
			continue;
		}
		
		// check for songs contained twice in database (but different path?)
		if(last->filename==s->filename) {
			cout << "X: song contained twice: " << s->filename << ", keep both?\n";
			cout << "path1: " << s->path << ", path2: " << last->path << "\n";
			if(problematicSongs.containsRef(last)==0)
				problematicSongs.append(last);
			if(problematicSongs.containsRef(s)==0)
				problematicSongs.append(s);
		}
		last=s;
	}
	
	// 3. check for songs contained twice (this time not based on filename)
	allSongs.setSortOrder(MyList::ByArtist + 16*(MyList::ByTitle));
	allSongs.sort();
	last=allSongs.first();
	for(Song* s=allSongs.next(); s; s=allSongs.next()) {
		if(s->artist=="{wish}")
			continue;
	
		// check for songs contained twice in database (but pointing to same file+path)
		if(last->artist==s->artist && last->title==s->title) {
			cout << "!!!   song contained twice: " << s->filename << ", keep both?\n";
			if(problematicSongs.containsRef(last)==0)
				problematicSongs.append(last);
			if(problematicSongs.containsRef(s)==0)
				problematicSongs.append(s);
		}
	}
	
	if(sumDirtyTags+sumDirtyFilenames==0)
		cout << "your database is nice and clean!\n";
	else {
		cout << sumDirtyTags				<< " dirty tags\n";
		cout << sumDirtyFilenames		<< " dirty filenames\n";
	}
	
	cout << "..consistency checked\n";
	if(sumDirtyTags+sumDirtyFilenames>0) {
		allSongsChanged(true);
		return false;
	}
	return true;
}


void YammiModel::removeCategory(QString categoryName)
{
	QString name=categoryNames.first();
	for(MyList* ptr=allCategories.first(); ptr; ptr=allCategories.next(), name=categoryNames.next()) {
		if(name==categoryName) {
			cout << "removing item: " << name << "\n";
			allCategories.remove();
			categoryNames.remove();
			QDir d("/");
			QString todel=QString("%1/categories/%2.xml").arg(config.yammiBaseDir).arg(categoryName);
			d.remove(todel);
			categoriesChanged(true);
			break;
		}
	}
}

void YammiModel::newCategory(QString categoryName)
{
	MyList* newList=new MyList;
	allCategories.append(newList);
	categoryNames.append(categoryName);
	categoriesChanged(true);
}


/**
 * saves changed information (categories + songDatabase)
 */
void YammiModel::save()
{
	if(categoriesChanged() || allSongsChanged())
		saveCategories();
	if(allSongsChanged())
		saveSongDatabase();
}



/* removing a media, ie. removing the media entry in all songs that are on it
 */
void YammiModel::removeMedia(QString mediaToDelete)
{
	cout << "removing media: " << mediaToDelete << "\n";
	for(Song* s=allSongs.first(); s; s=allSongs.next()) {
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
}

