/***************************************************************************
                          preferencesdialog.h  -  description
                             -------------------
    begin                : Sat Sep 8 2001
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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <qwidget.h>
#include <PreferencesDialogBase.h>
#include "prefs.h"
#include "song.h"

/**
  *@author O.Nölle
  */

class PreferencesDialog : public Preferences  {
   Q_OBJECT

public: 
	PreferencesDialog(QWidget *parent, const char *name, bool modal, Prefs* config);
	~PreferencesDialog();
	
public slots:
	void myAccept();
	
	void chooseScanDir();
	void chooseTrashDir();
	void chooseMediaDir();
	void chooseSwapDir();
	
	void updateSongPlugin(int newPos);
	void updateSongPluginMenuEntry(const QString& newText);
	void updateSongPluginCmd(const QString& newText);
	void newSongPlugin();
	void deleteSongPlugin();
	
	void updatePlaylistPlugin(int newPos);
	void updatePlaylistPluginMenuEntry(const QString& newText);
	void updatePlaylistPluginCmd(const QString& newText);
	void updatePlaylistPluginCustomList(const QString& newText);
	void newPlaylistPlugin();
	void deletePlaylistPlugin();

protected:
	Prefs* config;
	QStringList* _pluginSongMenuEntry;
	QStringList* _pluginSongCmd;
	QStringList* _pluginPlaylistMenuEntry;
	QStringList* _pluginPlaylistCmd;
	QStringList* _pluginPlaylistCustomList;

};

#endif
