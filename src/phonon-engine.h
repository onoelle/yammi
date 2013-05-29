/***************************************************************************
                               phonon-engine.h
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


#ifndef PHONON_ENGINE_H
#define PHONON_ENGINE_H

#include <Phonon/AudioOutput>
#include <Phonon/MediaObject>

#include "mediaplayer.h"

class Song;


namespace Yammi {

    class PhononEngine : public MediaPlayer {
        Q_OBJECT
    public:
        PhononEngine(YammiModel *yammi);
        virtual ~PhononEngine();

        virtual QString getName()  {return "PhononPlayer";}
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

        void tick(qint64 time);
        void stateChanged(Phonon::State newState, Phonon::State oldState);
        void sourceChanged(const Phonon::MediaSource &source);
        void aboutToFinish();
        void finished();

    private:
        Phonon::MediaObject* m_mediaObject;
        Phonon::AudioOutput* m_audioOutput;
        Song*                m_currentSong;
        int                  m_rememberJumpTo;
    };

}; //namespace Yammi

#endif //PHONON_ENGINE_H
