/***************************************************************************
                          mediaplayer.h  -  description
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

#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include "folder.h"


/** This class should in future abstract away the details of a supported media player (such as XMMS, Noatun, Winamp, ...).
 * So far, only tailored for XMMS.
 */

enum PlayerStatus { STOPPED, PAUSED, PLAYING};


class MediaPlayer : public QObject
{
  Q_OBJECT

public: 
	MediaPlayer();
	virtual ~MediaPlayer();

public slots:
  virtual PlayerStatus getStatus()           { return status; }
  virtual bool play() = 0;
  virtual bool pause() = 0;
  virtual bool stop() = 0;
  virtual bool playPause() = 0;
  virtual bool skipForward(bool withoutCrossfading) = 0;
  virtual bool skipBackward(bool withoutCrossfading) = 0;
  virtual void syncPlayer2Yammi(Folder* folder) = 0;
  virtual void syncYammi2Player(bool syncAll) = 0;
  virtual bool jumpTo(int value) = 0;
  virtual QString getCurrentFile() = 0;
  virtual int getCurrentTime() = 0;
  virtual int getTotalTime() = 0;
  virtual void removePlayed() = 0;
  virtual void quit() = 0;

  
  
protected:
  PlayerStatus status;
  
};

#endif
