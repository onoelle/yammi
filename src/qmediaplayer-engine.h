/***************************************************************************
                               qmediaplayer-engine.h
                             -------------------
    begin                : Tue Dec 02 2003
    copyright            : (C) 2003 by Luis De la Parra
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
/* 2010-08-25 copied from artsplayer.h */
/* 2013-05-28 copied from xine-engine.h */
/* 2013-05-30 copied from phonon-engine.h */


#ifndef QMEDIAPLAYER_ENGINE_H
#define QMEDIAPLAYER_ENGINE_H

#ifdef USE_QMEDIAPLAYER

#include <QMediaPlayer>

#include "mediaplayer.h"

class Song;


namespace Yammi {

    class QMediaPlayerEngine : public MediaPlayer {
        Q_OBJECT
    public:
        QMediaPlayerEngine(YammiModel *yammi);
        virtual ~QMediaPlayerEngine();

        virtual QString getName()  {return "QMediaPlayer";}
        virtual void clearPlaylist();

    public slots:

        virtual bool finishInitialization();

        /** Checks if the current song has finished and stxine the next one */
        virtual void check();

        /** @return the player's current status */
        virtual PlayerStatus getStatus();

        /** Start playing the next song in Yammi's playlist
            @return true if the next song could be started */
        virtual bool play();

        /** Pauses the current song, if playing
            @return true if the current song could be paused */
        virtual bool pause();

        /** Stops the current song
            @return true if the current song could be stopped */
        virtual bool stop();

        /** Toggle between Play and Pause states in the current song
            @return true if the player could be toggeld*/
        virtual bool playPause();

        /** Stop playing the current song and start playing the next song in Yammi's playlist
            @param  withoutCrossfading - not implemented
            @return true if the next song was started */
        virtual bool skipForward(bool withoutCrossfading);

        /** This function is NOT implemented.
            Should stop the current song, and go back to the previously played song. Since XinePlayer does not keep it's own playlist, and the playlist in Yammi must be "consumed" in order for the MediaPlayer to work properly, there is never a previous song in the playlist because we always play the first item in the list.
            @param  withoutCrossfading - not implemented
            @return false */
        virtual bool skipBackward(bool withoutCrossfading);

        /** Only syncs the first song of yammi playlist as the current xine player object. */
        virtual void syncYammi2Player();

        /** --> Seek : jump to position value within the song
            @param value the position in the song to seek to */
        virtual bool jumpTo(int value);

        /** @return The file corresponding to the current song. */
        virtual QString getCurrentFile();

        /** @return The position in the current song, in milliseconds. */
        virtual int getCurrentTime();

        /** @return The duration of the current song, in milliseconds */
        virtual int getTotalTime();

        /** Prepares the Player to be destroyed ( stop player, some housecleaning )*/
        virtual void quit();

    private:
        QMediaPlayer*        m_mediaPlayer;
        Song*                m_currentSong;
        int                  m_rememberJumpTo;
    };

}; //namespace Yammi

#endif //USE_QMEDIAPLAYER

#endif //QMEDIAPLAYER_ENGINE_H
