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

#include <qobject.h>
#include "mylist.h"
#include "yammimodel.h"


enum PlayerStatus { STOPPED, PAUSED, PLAYING};


/** This class abstracts away the details of a supported media player.
 * So far, the only supported players are XMMS and noatun.
 * Support for Winamp might come, if I should ever try to create a Windows-version of Yammi...
 *
 * Most methods should be straight-forward.
 * The player should emit a signal playlistChanged() when a songchange took place.
 */
class MediaPlayer : public QObject
{
  Q_OBJECT

public: 
	MediaPlayer();
	virtual ~MediaPlayer();

protected:
  PlayerStatus status;
  YammiModel*   model;        // TODO: can we remove this to make it cleaner?
  MyList* playlist;


public slots:
  virtual void check() = 0;
  virtual PlayerStatus getStatus()           { return status; }
  virtual bool play() = 0;
  virtual bool pause() = 0;
  virtual bool stop() = 0;
  virtual bool playPause() = 0;
  virtual bool skipForward(bool withoutCrossfading) = 0;
  virtual bool skipBackward(bool withoutCrossfading) = 0;
  virtual void syncPlayer2Yammi(MyList* playlist) = 0;
  virtual void syncYammi2Player(bool syncAll) = 0;
  virtual bool jumpTo(int value) = 0;
  virtual QString getCurrentFile() = 0;
  virtual int getCurrentTime() = 0;
  virtual int getTotalTime() = 0;
  virtual void quit() = 0;

signals:
  virtual void playlistChanged() = 0;         // should be emitted, if the playlist of player changed (eg. song change)
  
};

#endif
