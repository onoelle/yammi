/***************************************************************************
                          noatunplayer.h  -  description
                             -------------------
    begin                : Sun Jan 19 2003
    copyright            : (C) 2003 by Oliver Nölle
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

#ifndef NOATUNPLAYER_H
#define NOATUNPLAYER_H

#include "mediaplayer.h"
#include "yammimodel.h"
#include <qprocess.h>
#include <dcopclient.h>

/**
  *@author Oliver Nölle
  */

class NoatunPlayer : public MediaPlayer  {
  Q_OBJECT
public: 
	NoatunPlayer(YammiModel* model);
	~NoatunPlayer();
  bool check(QString nextSong);
public slots:
  // overriden methods
  bool playPause();
  bool play();
  bool pause();
  bool stop();
  bool skipForward(bool withoutCrossfading);
  bool skipBackward(bool withoutCrossfading);
  PlayerStatus getStatus();
  void syncPlayer2Yammi(Folder* folder);
  void syncYammi2Player(bool syncAll);
  bool jumpTo(int value);
  QString getCurrentFile();
  int getCurrentTime();
  int getTotalTime();
  void removePlayed();
  void quit();

protected:
  DCOPClient* client;
  bool ensurePlayerIsRunning();
  void playlistAdd(QString filename, int id=0);
  void sendDcopCommand(QString command, int id=0);
  int callGetInt(QString command, int id=0);
  int callGetIntX(QString command, int id=0);
  int getCurrentPlayerId();
  
  YammiModel*   model;   // this should be removed to make it cleaner!!!
  bool shuffleWasActivated;
  int playerId[10];        // the process ids of the noatun players (upto 10)
  int currentPlayer;
  
};

#endif
