/***************************************************************************
                          xmmsplayer.h  -  description
                             -------------------
    begin                : Tue Sep 24 2002
    copyright            : (C) 2002 by Oliver Nölle
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

#ifndef XMMSPLAYER_H
#define XMMSPLAYER_H

#include "mediaplayer.h"
#include "yammimodel.h"

/** The MediaPlayer class tailored to Xmms.
 * Work in progress!!!
 */

class XmmsPlayer : public MediaPlayer
{
  Q_OBJECT

public: 
	XmmsPlayer(int session, YammiModel* model);
	~XmmsPlayer();

  bool ensureXmmsIsRunning();

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
  int           session;      // xmms session id
  YammiModel*   model;   // this should be removed to make it cleaner!!!
	bool			    xmmsShuffleWasActivated;
};

#endif
