/***************************************************************************
                          dummyplayer.h  -  description
                             -------------------
    begin                : Thu Apr 24 2003
    copyright            : (C) 2003 by Oliver Nölle
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

#ifndef DUMMYPLAYER_H
#define DUMMYPLAYER_H

#include <mediaplayer.h>

/**a dummy player class with no behaviour
  *@author Oliver Nölle
  */

class DummyPlayer : public MediaPlayer  {
  Q_OBJECT
public: 
	DummyPlayer() {};
	~DummyPlayer() {};

  QString getName() {return "dummy";}

public:
  void clearPlaylist() {};
public slots:
  void check() {};
  PlayerStatus getStatus()           { return STOPPED; }
  bool play() {return true;}
  bool pause() {return true;}
  bool stop()  {return true;}
  bool playPause()  {return true;}
  bool skipForward(bool withoutCrossfading)   {return true;}
  bool skipBackward(bool withoutCrossfading)  {return true;}
  void syncPlayer2Yammi(MyList* playlist)  {};
  void syncYammi2Player(bool syncAll) {};
  bool jumpTo(int value) {return true;}
  QString getCurrentFile() {return "";}
  int getCurrentTime() {return 0;}
  int getTotalTime() {return 0;}
  void quit() {};

signals:
  void playlistChanged();
  void statusChanged();
};

#endif
