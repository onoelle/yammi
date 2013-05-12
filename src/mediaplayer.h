/***************************************************************************
                          mediaplayer.h  -  description
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

#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <qobject.h>
#include "mylist.h"
#include "yammimodel.h"


enum PlayerStatus { STOPPED, PAUSED, PLAYING};


/** This class abstracts away the details of a supported media player.
 *
 * Most methods should be straight-forward.
 *
 * Communication between MediaPlayer class and Yammi:
 * 1. On startup, Yammi creates the MediaPlayer object.
 * 2. Yammi calls check() every 100? ms. (TODO: we should instead use a thread owned by the player)
 * 3. If the player detects a change in status or a song change, it has to signal statusChanged() or playlistChanged()
 * 4. Whenever Yammi changes it's playlist, it calls syncYammi2Player()
 *    to possibly fill up the playlist of the player.
 */
class MediaPlayer : public QObject {
    Q_OBJECT

public:
    MediaPlayer( YammiModel *yammi );
    virtual bool finishInitialization() = 0;
    virtual ~MediaPlayer();
    virtual QString getName() = 0;

    virtual void clearPlaylist() = 0;

public slots:
    virtual void check() = 0;
    virtual PlayerStatus getStatus() {
        return status;
    }
    virtual bool play() = 0;
    virtual bool pause() = 0;
    virtual bool stop() = 0;
    virtual bool playPause() = 0;
    virtual bool skipForward(bool withoutCrossfading) = 0;
    virtual bool skipBackward(bool withoutCrossfading) = 0;
    virtual void syncYammi2Player() = 0;
    virtual bool jumpTo(int value) = 0;
    virtual QString getCurrentFile() = 0;
    virtual int getCurrentTime() = 0;
    virtual int getTotalTime() = 0;
    virtual void quit() = 0;

signals:
    // should be emitted, if the playlist of player changed (eg. song change)
    void playlistChanged();
    // should be emitted, if status of player changed (eg. play -> pause)
    void statusChanged();

protected:
    PlayerStatus status;
    PlayerStatus lastStatus;
    YammiModel*   model;
    MyList* playlist;


};

#endif
