/***************************************************************************
                          xmmsplayer.h  -  description
                             -------------------
    begin                : Tue Sep 24 2002
    copyright            : (C) 2002 by Oliver Nölle
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


#ifndef XMMSPLAYER_H
#define XMMSPLAYER_H

#include "options.h"


#include "mediaplayer.h"

/**
 * The MediaPlayer class tailored to XMMS.
 */


class XmmsPlayer : public MediaPlayer {
    Q_OBJECT

public:
    XmmsPlayer(int session, YammiModel* model);
    ~XmmsPlayer();

    QString getName() {
        return "xmms";
    }

protected:
    bool          ensurePlayerIsRunning();
    int           session;      // xmms session id
    int           timeLeft;
    bool			    xmmsShuffleWasActivated;
    bool			    xmmsRepeatWasActivated;
    void			    myWait(int msecs);

public:
    // overriden methods
    void          clearPlaylist();
public slots:
    void check();
    PlayerStatus getStatus();
    bool play();
    bool pause();
    bool stop();
    bool playPause();
    bool skipForward(bool withoutCrossfading);
    bool skipBackward(bool withoutCrossfading);
    void syncYammi2Player();
    bool jumpTo(int value);
    QString getCurrentFile();
    int getCurrentTime();
    int getTotalTime();
    void quit();

signals:
    void playlistChanged();
    void statusChanged();
};


#endif
