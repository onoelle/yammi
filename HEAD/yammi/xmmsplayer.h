/***************************************************************************
                          xmmsplayer.h  -  description
                             -------------------
    begin                : Tue Sep 24 2002
    copyright            : (C) 2002 by Oliver N�lle
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

/** The MediaPlayer class tailored to XMMS.
 * Work in progress!!!
 */

class XmmsPlayer : public MediaPlayer
{
  Q_OBJECT

public: 
	XmmsPlayer(int session, YammiModel* model);
	~XmmsPlayer();

protected:
  bool ensureXmmsIsRunning();
  int           session;      // xmms session id
	bool			    xmmsShuffleWasActivated;
	void			    myWait(int msecs);

public slots:
  // overriden methods
  void check();
  PlayerStatus getStatus();
  bool play();
  bool pause();
  bool stop();
  bool playPause();
  bool skipForward(bool withoutCrossfading);
  bool skipBackward(bool withoutCrossfading);
  void syncPlayer2Yammi(MyList* playlist);
  void syncYammi2Player(bool syncAll);
  bool jumpTo(int value);
  QString getCurrentFile();
  int getCurrentTime();
  int getTotalTime();
  void quit();

signals:
  void playlistChanged();
};

#endif
