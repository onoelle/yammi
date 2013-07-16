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

#include "qmediaplayer-engine.h"

#ifdef USE_QMEDIAPLAYER

#include <QDebug>
#include <QFile>
#include <QUrl>

#include "song.h"
#include "songentryint.h"
#include "yammimodel.h"


namespace Yammi {

    QMediaPlayerEngine::QMediaPlayerEngine(YammiModel *yammi)
            : MediaPlayer(yammi),
              m_currentSong(0),
              m_rememberJumpTo(0)
    {
        qDebug() << "QMediaPlayerEngine::QMediaPlayerEngine";

        m_mediaPlayer = new QMediaPlayer(this);
    }

    QMediaPlayerEngine::~QMediaPlayerEngine()
    {
        qDebug() << "QMediaPlayerEngine::~QMediaPlayerEngine";

        delete m_mediaPlayer;
    }

    void
    QMediaPlayerEngine::syncYammi2Player()
    {
        qDebug() << "QMediaPlayerEngine::syncYammi2Player";

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

        QUrl current = m_mediaPlayer->currentMedia().canonicalUrl();
        if ( (!file.fileName().isEmpty()) ||
             (!current.isEmpty() && current != QUrl::fromLocalFile(file.fileName())) )
        {
            m_mediaPlayer->setMedia(QUrl::fromLocalFile(file.fileName()));
            emit playlistChanged();
        }

       return;
    }

    void
    QMediaPlayerEngine::clearPlaylist()
    {
        qDebug() << "QMediaPlayerEngine::clearPlaylist";

        emit playlistChanged( );
    }

    bool
    QMediaPlayerEngine::play()
    {
        qDebug() << "QMediaPlayerEngine::play";

        m_mediaPlayer->play();
        if (m_rememberJumpTo) {
            jumpTo(m_rememberJumpTo);
        }
        m_rememberJumpTo = 0;
        emit statusChanged();

        return false;
    }

    bool
    QMediaPlayerEngine::stop()
    {
        qDebug() << "QMediaPlayerEngine::stop";

        m_mediaPlayer->stop();
        status = getStatus();
        emit statusChanged();

        return true;
    }

    bool
    QMediaPlayerEngine::pause()
    {
        qDebug() << "QMediaPlayerEngine::pause";

        m_mediaPlayer->pause();
        emit statusChanged();

        return true;
    }

    bool
    QMediaPlayerEngine::playPause()
    {
        qDebug() << "QMediaPlayerEngine::playPause";

        switch (getStatus()) {
        case PLAYING:
            pause();
            break;
        case STOPPED:
        default:
            play();  //start new play
            break;
        }
        return true;
    }

    void
    QMediaPlayerEngine::check()
    {
        //qDebug() << "QMediaPlayerEngine::check";

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
    }

    PlayerStatus
    QMediaPlayerEngine::getStatus()
    {
        //qDebug() << "QMediaPlayerEngine::getStatus";

        PlayerStatus ret = STOPPED;

        switch (m_mediaPlayer->state()) {
        case QMediaPlayer::StoppedState:
            ret = STOPPED;
            break;
        case QMediaPlayer::PlayingState:
            ret = PLAYING;
            break;
        case QMediaPlayer::PausedState:
            ret = PAUSED;
            break;
        default:
            break;
        }

        return ret;
   }

    QString
    QMediaPlayerEngine::getCurrentFile()
    {
        //qDebug() << "QMediaPlayerEngine::getCurrentFile";

        if( m_currentSong ) {
            return model->checkAvailability( m_currentSong );
        } else {
            return "";
        }
    }

    int
    QMediaPlayerEngine::getCurrentTime()
    {
        //qDebug() << "QMediaPlayerEngine::getCurrentTime" << m_mediaPlayer->position();

        return m_mediaPlayer->position();
    }

    int
    QMediaPlayerEngine::getTotalTime()
    {
        //qDebug() << "QMediaPlayerEngine::getTotalTime" << m_mediaPlayer->duration();

        return m_mediaPlayer->duration();
    }

    bool
    QMediaPlayerEngine::skipForward(bool)
    {
        qDebug() << "QMediaPlayerEngine::skipForward";

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
    QMediaPlayerEngine::skipBackward(bool withoutCrossfading)
    {
        qDebug() << "QMediaPlayerEngine::skipBackward";

        Song* last = 0;
        if (!playlist->isEmpty()) {
            last = playlist->at(0)->song();
        }
        // insert pseudo-song to be removed
        playlist->insert(0, new SongEntryInt(last, 0));
        return skipForward(withoutCrossfading);
    }


    bool
    QMediaPlayerEngine::jumpTo( int ms )
    {
        qDebug() << "QMediaPlayerEngine::jumpTo" << ms << "status" << getStatus();

        if (getStatus() == PLAYING) {
            m_mediaPlayer->setPosition(ms);
            emit statusChanged();
        } else {
            m_rememberJumpTo = ms;
        }

        return true;
    }

    void
    QMediaPlayerEngine::quit()
    {
        qDebug() << "QMediaPlayerEngine::quit";
    }

    bool
    QMediaPlayerEngine::finishInitialization()
    {
        qDebug() << "QMediaPlayerEngine::finishInitialization";

        return true;
    }

} //namespace Yammi

#endif //USE_QMEDIAPLAYER
