/***************************************************************************
                               gstplayer.h
                             -------------------
    begin                : Sat 4.9.2004
    copyright            : (C) 2004 Martin Albrecht
    email                : malb@informatik.uni-bremen.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 Player to interact with GStreamer via KDE::GST bindings. All work has been done
 elsewhere I just copy & pasted stuff from JuK's Gstreamerplayer and Yammi's ArtsPlayer 
 together.
**/

#ifndef YAMMIGSTPLAYER_H
#define YAMMIGSTPLAYER_H

#include "mediaplayer.h"
#include "options.h"

#ifdef ENABLE_GSTREAMER
#include <kde/gstplay/play.h>
#include <kde/gst/element.h>
#include <kde/gst/elementfactory.h>
#include <kde/gst/gstreamer.h>

#endif

class SongEntry;

namespace Yammi {

class GstPlayer : public MediaPlayer {
        Q_OBJECT
public:
        GstPlayer( YammiModel *yammi );
        virtual ~GstPlayer();

        virtual QString getName()  {
            return "GstPlayer";
        }
        virtual void clearPlaylist();

public slots:

        /** Reimplemented from MediaPlayer.
            Checks if the current song has finished and starts the next one */
        virtual void check();

        /** Reimplemented from MediaPlayer.
            @return the player's current status */
        virtual PlayerStatus getStatus();

        /** Reimplemented from MediaPlayer.
            Start playing the next song in Yammi's playlist
            @return true if the next song could be started */
        virtual bool play();

        /** Reimplemented from MediaPlayer.
             Pauses the current song, if playing
            @return true if the current song could be paused */
        virtual bool pause();

        /** Reimplemented from MediaPlayer.
             Stops the current song
            @return true if the current song could be stopped */
        virtual bool stop();

        /** Reimplemented from MediaPlayer.
             Toggle between Play and Pause states in the current song
            @return true if the player could be toggeld*/
        virtual bool playPause();

        /** Reimplemented from MediaPlayer.
             Stop playing the current song and start playing the next song in Yammi's playlist
            @param  withoutCrossfading - not implemented
            @return true if the next song was started */
        virtual bool skipForward(bool withoutCrossfading);

        /** Reimplemented from MediaPlayer.
            This function is NOT implemented.
            Stops the current song, and goes back to the previously played song. 
            @param  withoutCrossfading - not implemented
            @return false */
        virtual bool skipBackward(bool withoutCrossfading);
        /**
            * Reimplemented from MediaPlayer.
            * Only syncs the first song of yammi playlist as the current arts player object.
            */
        virtual void syncYammi2Player();
        /** Reimplemented from MediaPlayer.
             --> Seek : jump to position value within the song
             @param value the position in the song to seek to */
        virtual bool jumpTo(int value);
        /** Reimplemented from MediaPlayer.
             @return The file corresponding to the current song. */
        virtual QString getCurrentFile();
        /** Reimplemented from MediaPlayer.
             @return The position in the current song, in milliseconds. */
        virtual int getCurrentTime();
        /** Reimplemented from MediaPlayer.
             @return The duration of the current song, in milliseconds */
        virtual int getTotalTime();
        /** Reimplemented from MediaPlayer.
             Prepares the Player to be destroyed ( stop player, some housecleaning )*/
        virtual void quit();

private:
        #ifdef ENABLE_GSTREAMER

        KDE::GSTPlay::Play *m_player; /** does all the work for us */
        KDE::GST::Element::Element *audiosink;
        #endif

        long long m_durationNs; /** complete length in Nanosecs */
        long long m_positionNs; /** current position in nanosecs */
        bool m_done;            /** track done ? */
        Song* m_currentSong;    /** you're listening to: **/

private slots:
        void slotSetPosition(long long d);
        void slotSetDuration(long long d);
        void slotDone();
    };


}
;  //Yammi

#endif
