/***************************************************************************
                          noatunplayer.h  -  description
                             -------------------
    begin                : Sun Jan 19 2003
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

#ifndef NOATUNPLAYER_H
#define NOATUNPLAYER_H

#include "mediaplayer.h"
#include <qprocess.h>
#include <dcopclient.h>
#include <qtimer.h>


/**
 * The mediaplayer class tailored for controlling two noatun instances.
 */
class NoatunPlayer : public MediaPlayer  {
    Q_OBJECT
public:
    NoatunPlayer(YammiModel* model);
    ~NoatunPlayer();
    bool finishInitialization();

    QString getName() {
        return "noatun";
    }

protected:
    DCOPClient* client;
    QTimer fadeTimer;
    int timeLeft;
    bool ensurePlayerIsRunning(bool block);
    void playlistAdd(QString filename, bool autoStart);
    void sendDcopCommand(QString command, int id=0);
    void sendDcopCommandInt(QString command, int param, int id=0);
    int callGetInt(QString command, int id=0);
    QString callGetString(QString command, int id=0);
    int getCurrentPlayerId();
    int getOtherPlayerId();
    void clearActivePlayerPlaylist();
    void startSongChange(bool withoutCrossfading=false);

    bool shuffleWasActivated;
    int playerId[10];        // the process ids of the noatun players (upto 10)
    int currentPlayer;

    int fade;
    int fadeIn;
    int fadeOut;

public:
    // overriden methods
    void clearPlaylist() {};
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

protected slots:
    void onFade();
};

#endif
