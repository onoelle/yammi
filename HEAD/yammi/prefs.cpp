/***************************************************************************
                          prefs.cpp  -  description
                             -------------------
    begin                : Sun Sep 9 2001
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
#include "options.h"
#include "prefs.h"
#include "yammigui.h"

#include <qobject.h>
#include <qdom.h>
#include <qmessagebox.h>

using namespace std;

extern YammiGui* gYammiGui;

/// sets preferences to the default values
Prefs::Prefs(){
	setDefaultValues();
}

Prefs::~Prefs(){
}

void Prefs::setDefaultValues(void) {
	// general	
	// media player: 0=XMMS, 1=Noatun
#ifdef ENABLE_NOATUN
	player = MEDIA_PLAYER_NOATUN;
#endif
#ifdef ENABLE_XMMS
	player = MEDIA_PLAYER_XMMS;
#endif

	yammiBaseDir = "";
	yammiVersion = "1.0-rc1";
	trashDir = "/mp3/trash/";
	scanDir = "/mp3/inbox/";
	filenamePattern = "%a - %t";
	guessingMode = GUESSING_MODE_SIMPLE;
  
	doubleClickAction = None;
	middleClickAction = None;
	controlClickAction = None;
	shiftClickAction = None;
	logging = false;
	childSafe = false;
	tagsConsistent = false;
	filenamesConsistent = false;
  ignoreCaseInFilenames = false;
	criticalSize = 700;
	secondSoundDevice="";
	groupThreshold = 5;
	lazyGrouping = false;
	searchThreshold = 20;
	searchMaximumNoResults = 200;

	keepInXmms = 3;

	fadeTime = 10000;
	fadeOutEnd = 50;
	fadeInStart = 70;
	
	// plugins	
	grabAndEncodeCmd = "yammiGrabAndEncode";
	shutdownScript = "dcop ksmserver ksmserver \"logout\" 0 2 0";

	// jukebox functions
	mediaDir = "/dev/cdrom/";
	swapDir = "/tmp/";
	swapSize = 200;
	mountMediaDir = true;
	prefsFound = false;

}


bool Prefs::loadConfig(QString baseDir)
{
  qDebug("reading preferences...");

  QDir d(baseDir);
  if(!d.cd(".yammi")) {
		if(startFirstTime(baseDir)) { 
			d.cd(".yammi");
		} else {
			return false;
		}
	}

	// set yammis base directory 
	yammiBaseDir = d.absPath();

	// create a document
	QDomDocument doc("prefs");

	// open configuration file for parsing
	QFile f(yammiBaseDir+"/prefs.xml");
	if (!f.open(IO_ReadOnly)) {
	  // set default value and save them
    qDebug("configuration file could not be found, taking default values");
    // oliver: asking the user whether he wants to save configuratioin does not make sense,
    // if we save it later anyway...
		setDefaultValues();
		addStandardPlugins();
		return saveConfig();
	}

	// set file for parsing
	if (!doc.setContent(&f)) {
		QMessageBox::critical(gYammiGui, QObject::tr("Yammi"),
			QObject::tr("Error parsing configuration file!"),
			QMessageBox::Abort, 0, 0);

		// close file
		f.close();

		return false;
	}

	// close file
	f.close();

	// 1: get prefs from file
	prefsFound = true;

	// general parameter
	QString prefsVersion = getProperty(doc, "yammiVersion", yammiVersion);
	if(prefsVersion != yammiVersion) {
		QMessageBox::information(gYammiGui, QObject::tr("Yammi"),
			QObject::tr("Reading preferences from an other version of Yammi\n"
         "In Yammi 0.8.2, the default action configuration\n(eg. for double click) has slightly changed...\n"
			   "...please check your settings!"),
			QMessageBox::Ok, 0, 0);
  }
	trashDir                     = getProperty(doc, "trashDir", trashDir);
	scanDir                      = getProperty(doc, "scanDir", scanDir);
	filenamePattern              = getProperty(doc, "filenamePattern", filenamePattern);
	guessingMode                 = getProperty(doc, "guessingMode", player);
	doubleClickAction            = (action) getProperty(doc, "doubleClickAction", doubleClickAction);
	middleClickAction            = (action) getProperty(doc, "middleClickAction", middleClickAction);
	controlClickAction           = (action) getProperty(doc, "controlClickAction", controlClickAction);
	shiftClickAction             = (action) getProperty(doc, "shiftClickAction", shiftClickAction);
	logging                      = getProperty(doc, "logging", logging);
	childSafe                    = getProperty(doc, "childSafe", childSafe);
	tagsConsistent               = getProperty(doc, "tagsConsistent", tagsConsistent);
	filenamesConsistent          = getProperty(doc, "filenamesConsistent", filenamesConsistent);
	ignoreCaseInFilenames        = getProperty(doc, "ignoreCaseInFilenames", ignoreCaseInFilenames);
	criticalSize                 = getProperty(doc, "criticalSize", criticalSize);
	secondSoundDevice            = getProperty(doc, "secondSoundDevice", secondSoundDevice);
	groupThreshold               = getProperty(doc, "groupThreshold", groupThreshold);
	if(groupThreshold < 1) {
		groupThreshold = 1;
	}
	lazyGrouping                 = getProperty(doc, "lazyGrouping", lazyGrouping);
	searchThreshold              = getProperty(doc, "searchThreshold", searchThreshold);
	searchMaximumNoResults       = getProperty(doc, "searchMaximumNoResults", searchMaximumNoResults);

	player = getProperty(doc, "mediaPlayer", player);
#ifndef ENABLE_XMMS
	// xmms not enabled => set to noatun
	if(player == MEDIA_PLAYER_XMMS)
		player = MEDIA_PLAYER_NOATUN;
#endif
#ifndef ENABLE_NOATUN
	// noatun not enabled => set to xmms
	if(player == MEDIA_PLAYER_NOATUN)
		player = MEDIA_PLAYER_XMMS;
#endif
#ifndef ENABLE_XMMS
#ifndef ENABLE_NOATUN
	qWarning("No media player support! (you should have support for at least one media player compiled in)");
	player = -1;
#endif
#endif

	// xmms specific
	keepInXmms                   = getProperty(doc, "keepInXmms", keepInXmms);

	// noatun specific
	fadeTime                     = getProperty(doc, "fadeTime", fadeTime);
	fadeOutEnd                   = getProperty(doc, "fadeOutEnd", fadeOutEnd);
	fadeInStart                  = getProperty(doc, "fadeInStart", fadeInStart);

	// plugins
	grabAndEncodeCmd             = getProperty(doc, "grabAndEncodeCmd", grabAndEncodeCmd);
	shutdownScript               = getProperty(doc, "shutdownScript", shutdownScript);
	
	pluginCommand                = getProperty(doc, "pluginCommand", pluginCommand);
	pluginMenuEntry              = getProperty(doc, "pluginMenuEntry", pluginMenuEntry);
	pluginCustomList             = getProperty(doc, "pluginCustomList", pluginCustomList);
	pluginConfirm                = getProperty(doc, "pluginConfirm", pluginConfirm);
	pluginMode                   = getProperty(doc, "pluginMode", pluginMode);
	
	// jukebox functions
	mediaDir                     = getProperty(doc, "mediaDir", mediaDir);
	mountMediaDir                = getProperty(doc, "mountMediaDir", mountMediaDir);
	swapDir                      = getProperty(doc, "swapDir", swapDir);
	swapSize                     = getProperty(doc, "swapSize", swapSize);

	if(prefsVersion != yammiVersion) {
		addStandardPlugins();
		saveConfig();
	}

	qDebug("..done");

	return true;
}

/// save preferences (if changed) to disk
bool Prefs::saveConfig(void)
{
	qDebug("Prefs::saveConfig() Saving preferences...");
	
	// create xml-file
	QDomDocument doc("prefs");
	QString empty("<?xml version=\"1.0\" encoding=\"UTF-16\"?>\n<prefs>\n</prefs>\n");
	if(!doc.setContent(empty)) {
		QMessageBox::critical(gYammiGui, QObject::tr("yammi"),
			QObject::tr("Saving preferences failed!"),
			QMessageBox::Ok, 0, 0);
		return false;
	}
	
	// iterate through properties and save each property as an element
	// general
	setProperty(doc, "yammiVersion", yammiVersion);
	setProperty(doc, "trashDir", trashDir);
	setProperty(doc, "scanDir", scanDir);
	setProperty(doc, "filenamePattern", filenamePattern);
	setProperty(doc, "guessingMode", guessingMode);
	setProperty(doc, "doubleClickAction", doubleClickAction);
	setProperty(doc, "middleClickAction", middleClickAction);
	setProperty(doc, "controlClickAction", controlClickAction);
	setProperty(doc, "shiftClickAction", shiftClickAction);
	setProperty(doc, "logging", logging);
	setProperty(doc, "childSafe", childSafe);
	setProperty(doc, "tagsConsistent", tagsConsistent);
	setProperty(doc, "filenamesConsistent", filenamesConsistent);
	setProperty(doc, "ignoreCaseInFilenames", ignoreCaseInFilenames);
	setProperty(doc, "criticalSize", criticalSize);
	setProperty(doc, "secondSoundDevice", secondSoundDevice);
	setProperty(doc, "groupThreshold", groupThreshold);
	setProperty(doc, "lazyGrouping", lazyGrouping);
	setProperty(doc, "searchThreshold", searchThreshold);
	setProperty(doc, "searchMaximumNoResults", searchMaximumNoResults);
	setProperty(doc, "mediaPlayer", player);
	// xmms
	setProperty(doc, "keepInXmms", keepInXmms);
	// noatun
	setProperty(doc, "fadeTime", fadeTime);
	setProperty(doc, "fadeOutEnd", fadeOutEnd);
	setProperty(doc, "fadeInStart", fadeInStart);
  
	// plugins
	setProperty(doc, "grabAndEncodeCmd", grabAndEncodeCmd);
	setProperty(doc, "shutdownScript", shutdownScript);
	setProperty(doc, "pluginCommand", pluginCommand);
	setProperty(doc, "pluginMenuEntry", pluginMenuEntry);
	setProperty(doc, "pluginCustomList", pluginCustomList);
	setProperty(doc, "pluginConfirm", pluginConfirm);
	setProperty(doc, "pluginMode", pluginMode);
  
	// jukebox functions
	setProperty(doc, "mediaDir", mediaDir);
	setProperty(doc, "mountMediaDir", mountMediaDir);
	setProperty(doc, "swapDir", swapDir);
	setProperty(doc, "swapSize", swapSize);

	
	// save to file...
	QString save = doc.toString();
	QFile f2(yammiBaseDir + "/prefs.xml");
	if(!f2.open(IO_WriteOnly)) {
		QMessageBox::critical(gYammiGui, QObject::tr("yammi"),
			QObject::tr("Could not save preferences!"),
			QMessageBox::Ok, 0, 0);
		return false;
	}
	f2.writeBlock ( save, save.length() );
	f2.close();

	qDebug("Prefs::saveConfig() ...done");
  return true;
}


/**
 * called when the program is started the first time by a user
 * (ie. there is no .yammi directory existing in the user's home dir)
 */
bool Prefs::startFirstTime(QString& baseDir)
{
	cout << "you seem to start Yammi for the first time!\n";
	cout << "creating directory .yammi in " << baseDir << "...";

	QDir d(baseDir);
 	if(!d.mkdir(".yammi")) {
		QMessageBox::critical(gYammiGui, QObject::tr("Yammi"),
			QObject::tr("Could not create directory .yammi. Maybe you have no "
			   "write access to directory %1!").arg(baseDir),
			QMessageBox::Ok, 0, 0);
 		return false;
 	}

 	d.cd(".yammi");

	yammiBaseDir = d.absPath();

	qDebug("Prefs::startFirstTime() Creating subdirectory categories to store your categories...");
 	if(!d.mkdir("categories")) {
		QMessageBox::critical(gYammiGui, QObject::tr("Yammi"),
			QObject::tr("Could not create directory categories!"),
			QMessageBox::Ok, 0, 0);
 		return false;
 	}
	qDebug("Prefs::startFirstTime() ...done");

	qDebug("Prefs::startFirstTime() Creating subdirectory media to store your media info...");
 	if(!d.mkdir("media")) {
		QMessageBox::critical(gYammiGui, QObject::tr("Yammi"),
			QObject::tr("Could not create dirctory media!"),
			QMessageBox::Ok, 0, 0);
 		return false;
 	}
	qDebug("Prefs::startFirstTime() ...done");
 	
	qDebug("Prefs::startFirstTime() Every directory succesfully initialized");

	return true;
}

/**
 * Add all the standard plugins (on switch to new version)
 */
void Prefs::addStandardPlugins()
{
	qDebug("Prefs::addStandardPlugins() adding Yammi's standard plugins to the plugin list");

	if(!pluginMenuEntry.contains("Create CD Label")) {
		pluginMenuEntry.append("Create CD Label");
		pluginCommand.append("cdlabelgen -c \"Title\" -s \"Subtitle\" -b -w -i \"%l\" > %Y");
		pluginCustomList.append("%i. %a - %t (%l)%");
		pluginConfirm.append("true");
		pluginMode.append("group");
	}
	
	if(!pluginMenuEntry.contains("Export to m3u Playlist")) {
		pluginMenuEntry.append("Export to m3u Playlist");
		pluginCommand.append("echo -e \"#EXTM3U\n%l\" > %Y");
		pluginCustomList.append("#EXTINF:%s,%a - %t%n%f%n");
		pluginConfirm.append("true");
		pluginMode.append("group");
	}
	
	if(!pluginMenuEntry.contains("Burn with K3b(audio)")) {
		pluginMenuEntry.append("Burn with K3b(audio)");
		pluginCommand.append("echo -e \"#EXTM3U\n%l\" > /tmp/burnlist.m3u && k3b --audio /tmp/burnlist.m3u &");
		pluginCustomList.append("#EXTINF:%s,%a - %t%n%f%n");
		pluginConfirm.append("true");
		pluginMode.append("group");
	}
	if(!pluginMenuEntry.contains("Burn with K3b(data)")) {
		pluginMenuEntry.append("Burn with K3b(data)");
		pluginCommand.append("k3b --data %L &");
		pluginCustomList.append("\"%f\" ");
		pluginConfirm.append("true");
		pluginMode.append("group");
	}
/*
	if(!pluginMenuEntry->contains("")) {
		pluginMenuEntry->append("");
		pluginCommand->append("");
		pluginCustomList->append("");
		pluginConfirm->append("true");
		pluginMode->append("group");
	}
*/
	qDebug("Prefs::addStandardPlugins() done");
	
}

/// get int property
int Prefs::getProperty(const QDomDocument& doc, const QString& propName, int propDefault)
{
	return getProperty(doc, propName, QString().setNum(propDefault)).toInt();
}

/// get bool property
bool Prefs::getProperty(const QDomDocument& doc, const QString& propName, bool propDefault)
{
	return getProperty(doc, propName, QString(propDefault ? "1" : "0")) == "1";
}

/// get string property
QString Prefs::getProperty(const QDomDocument& doc, const QString& propName, const QString& propDefault)
{	
	QDomNodeList list = doc.elementsByTagName(propName);
	if(list.count() > 0) {
		QDomNode node = list.item(0); // we only retrieve first item
		if(!node.isNull() && node.isElement()) {
			QDomElement elem = node.toElement();
			return elem.text();
		}
	}

	// no tag found -> set default value
	qDebug("Prefsl::getProperty() setting %s to default value: %s",
		propName.latin1(), propDefault.latin1());

	return propDefault;
}

/// get string list property
QStringList Prefs::getProperty(const QDomDocument& doc, const QString& propName, const QStringList& propDefault)
{	
	QDomNodeList list = doc.elementsByTagName(propName);
	int noEntries = list.count();

	// default value for tag
	if(noEntries == 0) {
		qDebug("Prefsl::getProperty() setting %s to default list",
			propName.latin1());
		return propDefault;
	}

	// iterate through all items and append to stringList
	QStringList stringList;
	for(int i = 0; i < noEntries; i++) {
		QDomNode node = list.item(i);
		QDomElement elem = node.toElement();
		stringList.append(elem.text());
	}

	return stringList;
}

/// set an int property
void Prefs::setProperty(QDomDocument& doc, const QString& propName, int propValue)
{
	setProperty(doc, propName, QString("%1").arg(propValue));
}

/// set a bool property
void Prefs::setProperty(QDomDocument& doc, const QString& propName, bool propValue)
{
	setProperty(doc, propName, QString("%1").arg(propValue));
}
/// set a string property
void Prefs::setProperty(QDomDocument& doc, const QString& propName, const QString& propValue)
{
	QDomElement rootElem = doc.documentElement();
	QDomElement elem = doc.createElement(propName);
	QDomText domText=doc.createTextNode ( propValue );
	elem.appendChild(domText);
	rootElem.appendChild(elem);
}

/// set a string list property
void Prefs::setProperty(QDomDocument& doc, const QString& propName, const QStringList& propValue)
{
	QDomElement rootElem = doc.documentElement();
	for(unsigned int i=0; i<propValue.count(); i++) {
		QDomElement elem = doc.createElement(propName);
		QDomText domText=doc.createTextNode ( propValue[i] );
		elem.appendChild(domText);
		rootElem.appendChild(elem);
	}
}


