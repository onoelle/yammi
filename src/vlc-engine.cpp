/***************************************************************************
 *   Copyright (C) 2005   Christophe Thommeret <hftom@free.fr>             *
 *             (C) 2005   Ian Monroe <ian@monroe.nu>                       *
 *             (C) 2005,6 Mark Kretschmann <markey@web.de>                 *
 *             (C) 2004,5 Max Howell <max.howell@methylblue.com>           *
 *             (C) 2003,4 J. Kofler <kaffeine@gmx.net>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* 2013-05-28 copied from xine-engine.h */
/* 2013-05-28 copied from phonon-engine.h */
/* 2013-07-15 copied from vlc-engine.h */

#include "vlc-engine.h"

#ifdef USE_VLC

#include <QDebug>
#include <QFile>
#include <QUrl>

#include "song.h"
#include "songentryint.h"
#include "yammimodel.h"


namespace Yammi {

    VlcEngine::VlcEngine(YammiModel *yammi)
            : MediaPlayer(yammi),
              m_inst(0),
              m_mediaPlayer(0),
              m_currentSong(0),
              m_rememberJumpTo(0)
    {
        qDebug() << "VlcEngine::VlcEngine";

        const char* argv[] = {
            "--intf=dummy",
            "--vout=dummy",
            "--ignore-config",
            "--reset-plugins-cache",
            "--no-media-library",
            "--no-one-instance",
            "--no-osd",
            "--no-stats",
            "--no-video-title-show",
            "--noauto-preparse",
            "--album-art", "0",
            "--album-art=0",
            "-v", // -vvv -vv -v
            0 };
        int argc = (sizeof(argv) / sizeof(char*))-1;
        m_inst = libvlc_new(argc, argv);
        m_mediaPlayer = libvlc_media_player_new(m_inst);
    }

    VlcEngine::~VlcEngine()
    {
        qDebug() << "VlcEngine::~VlcEngine";

        libvlc_media_player_release(m_mediaPlayer);
        libvlc_release(m_inst);
    }

    void
    VlcEngine::syncYammi2Player()
    {
        qDebug() << "VlcEngine::syncYammi2Player";

        bool haveToUpdate=model->skipUnplayableSongs();

        if(m_currentSong != 0) {
            qDebug() << "m_currentSong: " << m_currentSong->displayName();
        }

        QFile file;
        SongEntry* songEntry = 0;
        if (!playlist->isEmpty()) {
            songEntry = playlist->at(0);
        }
        if (songEntry) {
            //qDebug() << "playlist->at(0): " << songEntry->song()->displayName();

            if(m_currentSong == songEntry->song()) {
                if(haveToUpdate) {
                    emit playlistChanged();
                }
                return;
            }
            if ( getStatus() != STOPPED ) {
                stop();
            }
            m_currentSong = 0;

            QString location = model->checkAvailability( songEntry->song() );
            qDebug() << "returned location: " << location;
            file.setFileName(location);
            m_currentSong = songEntry->song();
            if (!file.exists()) {
                qDebug() << "ERROR: location not valid: " << file.fileName();
                return;
            }
        }

        QUrl current;
        libvlc_media_t* m = libvlc_media_player_get_media(m_mediaPlayer);
        if (m) {
            current = libvlc_media_get_mrl(m);
            libvlc_media_release(m);
        }
        if ( (!file.fileName().isEmpty()) ||
             (!current.isEmpty() && current != QUrl::fromLocalFile(file.fileName())) )
        {
            m = libvlc_media_new_path(m_inst, file.fileName().toUtf8().constData());
            if (m) {
                libvlc_media_add_option(m, "file-caching=60000");
                    // To avoid "buffer way too late".
                    // Seen with VLC 2.0.3-5 in debian wheezy (can be reproduced with VLC gui also).
                libvlc_media_player_set_media(m_mediaPlayer, m);
                libvlc_media_release(m);
            }
            emit playlistChanged();
        }

       return;
    }

    void
    VlcEngine::clearPlaylist()
    {
        qDebug() << "VlcEngine::clearPlaylist";

        emit playlistChanged( );
    }

    bool
    VlcEngine::play()
    {
        qDebug() << "VlcEngine::play";

        libvlc_media_player_play(m_mediaPlayer);
        if (m_rememberJumpTo) {
            jumpTo(m_rememberJumpTo);
        }
        emit statusChanged();

        return false;
    }

    bool
    VlcEngine::stop()
    {
        qDebug() << "VlcEngine::stop";

        libvlc_media_player_stop(m_mediaPlayer);
        status = getStatus();
        emit statusChanged();

        return true;
    }

    bool
    VlcEngine::pause()
    {
        qDebug() << "VlcEngine::pause";

        libvlc_media_player_pause(m_mediaPlayer);
        emit statusChanged();

        return true;
    }

    bool
    VlcEngine::playPause()
    {
        qDebug() << "VlcEngine::playPause";

        switch (getStatus()) {
        case PLAYING:
            pause();
            break;
        case STOPPED:
        default:
            play();  //start new play
            jumpTo(getCurrentTime()); // To avoid "buffer way too late" (see comment in syncYammi2Player)
            break;
        }
        return true;
    }

    void
    VlcEngine::check()
    {
        //qDebug() << "VlcEngine::check";

        PlayerStatus s = getStatus( );
        if( s != status ) {
            if( status == PLAYING && s == STOPPED ) {
                if( m_currentSong == playlist->firstSong() ) {
                    qDebug() << "Song finished... get new one!";
                    playlist->removeFirst( );
                    m_currentSong = 0;
                    emit playlistChanged( );
                }
                // if there are more songs in yammi's list, play them
                play( );
            }
            status = s;
            emit statusChanged( );
        }
        if (m_rememberJumpTo) {
            play();
        }
    }

    PlayerStatus
    VlcEngine::getStatus()
    {
        //qDebug() << "VlcEngine::getStatus";

        PlayerStatus ret = STOPPED;

        switch (libvlc_media_player_get_state(m_mediaPlayer)) {
        case libvlc_Opening:
        case libvlc_Buffering:
        case libvlc_Playing:
            ret = PLAYING;
            break;
        case libvlc_Paused:
            ret = PAUSED;
            break;
        case libvlc_Stopped:
            ret = STOPPED;
            break;
        case libvlc_NothingSpecial:
        case libvlc_Ended:
        case libvlc_Error:
        default:
            break;
        }

        return ret;
   }

    QString
    VlcEngine::getCurrentFile()
    {
        qDebug() << "VlcEngine::getCurrentFile";

        if( m_currentSong ) {
            return model->checkAvailability( m_currentSong );
        } else {
            return "";
        }
    }

    int
    VlcEngine::getCurrentTime()
    {
        //qDebug() << "VlcEngine::getCurrentTime" << libvlc_media_player_get_time(m_mediaPlayer);

        return libvlc_media_player_get_time(m_mediaPlayer);
    }

    int
    VlcEngine::getTotalTime()
    {
        int totalTime = 0;
        libvlc_media_t* m = libvlc_media_player_get_media(m_mediaPlayer);
        if (m) {
            totalTime = libvlc_media_get_duration(m);
            libvlc_media_release(m);
        }
        qDebug() << "VlcEngine::getTotalTime" << totalTime;
        return totalTime;
    }

    bool
    VlcEngine::skipForward(bool)
    {
        qDebug() << "VlcEngine::skipForward";

        if( playlist->count() < 2 ) {
            // there is no "next song"
            return false;
        }
        int savedStatus = getStatus();
        m_currentSong = 0;

        // remove the first song and sync playlist again
        playlist->removeFirst( );
        syncYammi2Player();
        if(!m_currentSong) {
            return false;
        }
        if(savedStatus == PLAYING) {
            play();
        }
        return true;
    }

    bool
    VlcEngine::skipBackward(bool withoutCrossfading)
    {
        qDebug() << "VlcEngine::skipBackward";

        Song* last = 0;
        if (!playlist->isEmpty()) {
            last = playlist->at(0)->song();
        }
        // insert pseudo-song to be removed
        playlist->insert(0, new SongEntryInt(last, 0));
        return skipForward(withoutCrossfading);
    }


    bool
    VlcEngine::jumpTo( int ms )
    {
        int totalTime = getTotalTime();

        if (getStatus() == PLAYING && totalTime > 0) {
            float pos = (float)ms / (float)totalTime;
            libvlc_media_player_set_position(m_mediaPlayer, pos);
            m_rememberJumpTo = 0;
            qDebug() << "VlcEngine::jumpTo libvlc_media_player_set_position ms=" << ms << "pos=" << pos << "status=" << getStatus();
            emit statusChanged();
        } else {
            if (ms >= 0) {
                qDebug() << "VlcEngine::jumpTo m_rememberJumpTo ms=" << ms << "status=" << getStatus();
                m_rememberJumpTo = ms;
            } else {
                qDebug() << "ms is invalid" << ms;
            }
        }

        return true;
    }

    void
    VlcEngine::quit()
    {
        qDebug() << "VlcEngine::quit";
    }

    bool
    VlcEngine::finishInitialization()
    {
        qDebug() << "VlcEngine::finishInitialization";

        return true;
    }

} //namespace Yammi

#endif //USE_VLC
