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
#include "yammigui.h"

extern YammiGui* gYammiGui;


YammiModel::YammiModel()
{
}

YammiModel::~YammiModel()
{
}


/// read preferences from xml-file
void YammiModel::readPreferences()
{
	cout << "reading preferences...\n";
	noPrefsFound=true;
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
	noPrefsFound=false;
	// general
	QString prefsVersion		= getProperty(&doc, "yammiVersion", config.yammiVersion);
	if(prefsVersion!=config.yammiVersion)
		cout << "reading preferences from other version of Yammi, should not create any problems\n";
	config.trashDir					=	getProperty(&doc, "trashDir", config.trashDir);
	config.scanDir					=	getProperty(&doc, "scanDir", config.scanDir);
	config.doubleClickAction=	(action) getProperty(&doc, "doubleClickAction", config.doubleClickAction);
	config.middleClickAction=	(action) getProperty(&doc, "middleClickAction", config.middleClickAction);
	config.controlClickAction=(action) getProperty(&doc, "controlClickAction", config.controlClickAction);
	config.shiftClickAction=	(action) getProperty(&doc, "shiftClickAction", config.shiftClickAction);
	config.logging					=	getProperty(&doc, "logging", config.logging);
	config.childSafe				=	getProperty(&doc, "childSafe", config.childSafe);
	config.tagsConsistent		=	getProperty(&doc, "tagsConsistent", config.tagsConsistent);
	config.filenamesConsistent =	getProperty(&doc, "filenamesConsistent", config.filenamesConsistent);
	config.criticalSize			=	getProperty(&doc, "criticalSize", config.criticalSize);
	config.secondSoundDevice=	getProperty(&doc, "secondSoundDevice", config.secondSoundDevice);
	config.groupThreshold	=		getProperty(&doc, "groupThreshold", config.groupThreshold);
	config.searchThreshold	=	getProperty(&doc, "searchThreshold", config.searchThreshold);
	config.searchMaximumNoResults	=	getProperty(&doc, "searchMaximumNoResults", config.searchMaximumNoResults);
	config.keepInXmms				=	getProperty(&doc, "keepInXmms", config.keepInXmms);
	
	// plugins
	config.grabAndEncodeCmd	=	getProperty(&doc, "grabAndEncodeCmd", config.grabAndEncodeCmd);
	config.shutdownScript		=	getProperty(&doc, "shutdownScript", config.shutdownScript);
	
	config.pluginSongCmd		=	getProperty(&doc, "pluginSongCmd", config.pluginSongCmd);
	config.pluginPlaylistCmd=	getProperty(&doc, "pluginPlaylistCmd", config.pluginPlaylistCmd);
	config.pluginSongMenuEntry			=	getProperty(&doc, "pluginSongMenuEntry", config.pluginSongMenuEntry);
	config.pluginPlaylistMenuEntry	=	getProperty(&doc, "pluginPlaylistMenuEntry", config.pluginPlaylistMenuEntry);
	config.pluginPlaylistCustomList	=	getProperty(&doc, "pluginPlaylistCustomList", config.pluginPlaylistCustomList);
	
	// jukebox functions
	config.mediaDir					=	getProperty(&doc, "mediaDir", config.mediaDir);
	config.mountMediaDir		=	getProperty(&doc, "mountMediaDir", config.mountMediaDir);
	config.swapDir					=	getProperty(&doc, "swapDir", config.swapDir);
	config.swapSize					=	getProperty(&doc, "swapSize", config.swapSize);
	
	cout << "..done\n";
}


/// save preferences (if changed) to disk
void YammiModel::savePreferences()
{
	cout << "saving preferences...\n";
	
	// create xml-file
	QDomDocument doc( "prefs" );
	QString empty("<?xml version=\"1.0\" encoding=\"UTF-16\"?>\n<prefs>\n</prefs>\n");
	if ( !doc.setContent( empty ) ) {
		cout << "ERROR: saving preferences failed!\n";
		return;
	}
	
	// iterate through properties and save each property as an element
	// general
	setProperty(&doc, "yammiVersion",				config.yammiVersion);
	setProperty(&doc, "trashDir", 					config.trashDir);
	setProperty(&doc, "scanDir", 						config.scanDir);
	setProperty(&doc, "doubleClickAction",	config.doubleClickAction);
	setProperty(&doc, "middleClickAction",	config.middleClickAction);
	setProperty(&doc, "controlClickAction",	config.controlClickAction);
	setProperty(&doc, "shiftClickAction",		config.shiftClickAction);
	setProperty(&doc, "logging", 						config.logging);
	setProperty(&doc, "childSafe", 					config.childSafe);
	setProperty(&doc, "tagsConsistent", 		config.tagsConsistent);
	setProperty(&doc, "filenamesConsistent",config.filenamesConsistent);
	setProperty(&doc, "criticalSize", 			config.criticalSize);
	setProperty(&doc, "secondSoundDevice",	config.secondSoundDevice);
	setProperty(&doc, "groupThreshold", 		config.groupThreshold);
	setProperty(&doc, "searchThreshold", 		config.searchThreshold);
	setProperty(&doc, "searchMaximumNoResults",		config.searchMaximumNoResults);
	setProperty(&doc, "keepInXmms", 				config.keepInXmms);
	// plugins
	setProperty(&doc, "grabAndEncodeCmd",		config.grabAndEncodeCmd);
	setProperty(&doc, "shutdownScript",			config.shutdownScript);
	setProperty(&doc, "pluginSongCmd",			config.pluginSongCmd);
	setProperty(&doc, "pluginPlaylistCmd",	config.pluginPlaylistCmd);
	setProperty(&doc, "pluginSongMenuEntry",config.pluginSongMenuEntry);
	setProperty(&doc, "pluginPlaylistMenuEntry",	config.pluginPlaylistMenuEntry);
	setProperty(&doc, "pluginPlaylistCustomList",	config.pluginPlaylistCustomList);
	// jukebox functions
	setProperty(&doc, "mediaDir", 					config.mediaDir);
	setProperty(&doc, "mountMediaDir", 			config.mountMediaDir);
	setProperty(&doc, "swapDir", 						config.swapDir);
	setProperty(&doc, "swapSize", 					config.swapSize);

	
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

/// for multiple entries: QStringList
QStringList* YammiModel::getProperty(const QDomDocument* doc, const QString propName, QStringList* propDefault)
{	
	QDomNodeList list=doc->elementsByTagName ( propName );
	int noEntries=list.count();
	if(noEntries==0) {
		cout << "setting " << propName << " to default list\n";
		return propDefault;
	}
	// iterate through all items and append to stringList
	QStringList* stringList=new QStringList();
	for(int i=0; i<noEntries; i++) {
		QDomNode node=list.item(i);
		QDomElement elem=node.toElement();
		stringList->append(elem.text());
	}
	return stringList;
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

/// set a string property
void YammiModel::setProperty(QDomDocument* doc, const QString propName, const QStringList* propValue)
{
	QDomElement rootElem = doc->documentElement();
	for(unsigned int i=0; i<propValue->count(); i++) {
		QDomElement elem = doc->createElement(propName);
		QDomText domText=doc->createTextNode ( (*propValue)[i] );
		elem.appendChild(domText);
		rootElem.appendChild(elem);
	}
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
	config.yammiBaseDir=d.absPath();
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
 	
	cout << "Everything successfully initialized, have fun!\n";
	return true;
}



/**
 * Reads the category from the found xml files in playlist directory.
 */
void YammiModel::readCategories()
{
	// read in all xml-files found in a given directory that represent a category
	cout << "reading categories..." << flush;
	
	QDir d(config.yammiBaseDir+"/categories");
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
		cout << "\ncould not open history file (first time started?)... => no songs in history\n";
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
		cout << "saving history failed!\n";
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
		cout << "\nERROR: could not write history\n";
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
			cout << "ERROR: could not save categories!\n";
			return;
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
	// read in our xml-file
	noDatabaseFound=true;
	cout << "reading song database..." << flush;
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
		if(version=="0.5.3") {
			msg+="However, the database format did not change, so no worries!";
		}
		else {
			msg+="Yammi can only read a song database saved with version 0.5.3,\n";
			msg+="sorry.............\n";
			msg+="You can either get version 0.5.3 of Yammi\n";
			msg+="and save your database with it,\n";
			msg+="or you start with an empty database\n";
			msg+="and scan your harddisk for your songs\n";
			msg+="(categories and history will be lost, too!)";
		}
		QMessageBox::information(gYammiGui, "Yammi", msg, "OK");
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
		cout << "saving failed!\n";
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
	gYammiGui->tbSaveDatabase->setEnabled(_allSongsChanged || _categoriesChanged);
}

bool YammiModel::allSongsChanged()
{
	return _allSongsChanged;
}

void YammiModel::categoriesChanged(bool changed)
{
	_categoriesChanged=changed;
	gYammiGui->tbSaveDatabase->setEnabled(_allSongsChanged || _categoriesChanged);
}

bool YammiModel::categoriesChanged()
{
	return _categoriesChanged;
}


/**
 * Updates the xml-database by scanning harddisk
 * - scans recursively, starting from config.scanDir
 * - constructs song objects from all mp3 files found
 * - checks whether already existing, whether modified
 * - if not => inserts new song object into database
 */
void YammiModel::updateSongDatabase(QString mediaName, QProgressDialog* progress)
{
	if(config.childSafe)
		return;
	problematicSongs.clear();
	songsAdded=0;
	corruptSongs=0;
	
	if(mediaName==0) {
		cout << "scanning harddisk for new songs... \n";
		// check that scanDir is an existing directory
		QDir d(config.scanDir);
		if(!d.exists()) {
			cout << "base directory for scanning does not exist!\n";
			cout << "set value \"scanDir\" in preferences to correct value!\n";
		}
		else {
			traverse(config.scanDir, progress);
			progress->reset();
 			cout << "..finished scanning!\n";
	 	}
	}
	else {				
		cout << "scanning removable media for new songs... \n";
		
		// mount media dir
		if(config.mountMediaDir) {
			// linux specific
			QString cmd;
			cmd=QString("mount %1").arg(config.mediaDir);
			system(cmd);
		}

		// check that mediaDir is an existing directory
		QDir d(config.mediaDir);
		if(!d.exists()) {
			cout << "directory for removable media not existing/readable!\n";
			cout << "set value \"mediaDir\" in preferences to correct value!\n";
		}
		else {
			traverse(config.mediaDir, progress, mediaName);
 			cout << "..finished scanning!\n";
	 	}
		// umount media dir
		if(config.mountMediaDir) {
			// linux specific
			QString cmd;
			cmd=QString("umount %1").arg(config.mediaDir);
			system(cmd);
		}
	}
}




/// traverses a directory recursively and processes all mp3 files
/// puts songs where heuristic is not sure into the problematicSongs list
void YammiModel::traverse(QString path, QProgressDialog* progress, QString mediaName)
{
	// leave out the following directories
	if(path+"/"==config.trashDir || path+"/"==config.swapDir) {
		cout << "skipping trash or swap directory: " << path << "\n";
		return;
	}
 	
 	cout << "scanning directory " << path << "\n";
	progress->setLabelText("scanning directory "+path+"...");
  progress->setProgress(0);
  qApp->processEvents();
	
	QDir d(path);
	
	d.setFilter(QDir::Files | QDir::Dirs);
	d.setSorting( QDir::DirsFirst | QDir::Reversed );
	const QFileInfoList* list = d.entryInfoList();
	int filesToScan=list->count();
	progress->setTotalSteps(filesToScan);
	
	QFileInfoListIterator it( *list );								      // create list iterator

	int filesScanned=0;
	for(QFileInfo *fi; (fi=it.current()); ++it ) {						// for each file/dir
		filesScanned++;
		if(progress->wasCancelled())
			return;
		// if directory...		=> scan recursive
		if (fi->isDir()) {
			if(fi->fileName()!="." && fi->fileName()!="..")
				traverse(fi->filePath(), progress, mediaName);
			continue;
		}
	  progress->setProgress(filesScanned);
	  qApp->processEvents();
		// if file...					=> check whether we have a valid mp3 file
		if ((fi->extension(FALSE)).upper()!="MP3")
			continue;
			
		// okay, we have an mp3 file, try to add to database
		addSongToDatabase(fi->filePath(), mediaName);
	}
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
 	Song* newSong=new Song(filename, mediaName);
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
	songsAdded++;
 	allSongsChanged(true);
}
			



/**
 * checks consistency of all songs
 * @returns true, if consistent, false, if problematic songs were found
 */
bool YammiModel::checkConsistency(QProgressDialog* progress)
{
	if(config.childSafe)
		return true;
	cout << "checking consistency of database... \n";
	problematicSongs.clear();
	

	// 1. file existing, correct tags, correct filename?
	progress->setLabelText("Step 1: check tags and filenames");
	progress->setTotalSteps(allSongs.count());
	progress->setProgress(0);
	qApp->processEvents();
	
	int sumDirtyTags=0;
	int sumDirtyFilenames=0;
	int i=0;
	for(Song* s=allSongs.firstSong(); s; s=allSongs.nextSong(), i++) {
		if(i%10==0) {
			progress->setProgress(i);
		}
		if(progress->wasCancelled())
			break;		
		QString diagnosis=s->checkConsistency(config.tagsConsistent, config.filenamesConsistent);
		if(diagnosis!="") {
			problematicSongs.append(new SongEntryString(s, diagnosis));
		}
		if(s->tagsDirty)
			sumDirtyTags++;
		if(s->filenameDirty)
			sumDirtyFilenames++;
	}
		
	// 2. check for songs contained twice
	progress->setLabelText("Step 2: check for song entries pointing to same file");
	progress->setTotalSteps(allSongs.count());
	progress->setProgress(0);
	qApp->processEvents();
	
	allSongs.setSortOrderAndSort(MyList::ByFilename + 16*(MyList::ByPath));
	Song* last=allSongs.firstSong();
	i=0;
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
			cout << "two database entries pointing to same song/file: " << s->filename << ", deleting one\n";
			allSongs.remove();		// problem, coz we are iterating through this list???
			allSongsChanged(true);
			continue;
		}
		
		last=s;
	}
	
	// 3. check for two songs with identical primary key
	progress->setLabelText("Step 3: check for two songs with identical primary keys");
	progress->setTotalSteps(allSongs.count());
	progress->setProgress(0);
	qApp->processEvents();
	
//	allSongs.setSortOrder(MyList::ByArtist + 16*(MyList::ByTitle) + 256*(MyList::ByAlbum));
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
		if(config.tagsConsistent)
			cout << sumDirtyTags				<< " dirty tags\n";
		if(config.filenamesConsistent)
			cout << sumDirtyFilenames		<< " dirty filenames\n";
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

