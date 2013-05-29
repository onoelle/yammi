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

#include "phonon-engine.h"

#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QDesktopServices>
#include <QEvent>
#include <QFile>
#include <QMessageBox>
#include <QMutex>
#include <QWaitCondition>

#include "prefs.h"
#include "songentryint.h"
#include "yammimodel.h"


namespace Yammi {

    PhononEngine::PhononEngine(YammiModel *yammi)
            : MediaPlayer(yammi),
              m_currentSong(0),
              m_rememberJumpTo(0)
    {
        qDebug() << "PhononEngine::PhononEngine";

        m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
        m_mediaObject = new Phonon::MediaObject(this);

        //m_mediaObject->setTickInterval(1000);

        connect(m_mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
        connect(m_mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)), this, SLOT(stateChanged(Phonon::State,Phonon::State)));
        connect(m_mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)), this, SLOT(sourceChanged(Phonon::MediaSource)));
        connect(m_mediaObject, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));
        connect(m_mediaObject, SIGNAL(finished()), this, SLOT(finished()));

        Phonon::createPath(m_mediaObject, m_audioOutput);
    }

    PhononEngine::~PhononEngine()
    {
        qDebug() << "PhononEngine::~PhononEngine";

        delete m_audioOutput;
        delete m_mediaObject;
    }

    void
    PhononEngine::syncYammi2Player()
    {
        qDebug() << "PhononEngine::syncYammi2Player";

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

        if (!file.fileName().isEmpty() || m_mediaObject->currentSource().fileName() != file.fileName()) {
            m_mediaObject->setCurrentSource(file.fileName());
            emit playlistChanged();
        }

       return;
    }

    void
    PhononEngine::clearPlaylist()
    {
        qDebug() << "PhononEngine::clearPlaylist";

        emit playlistChanged( );
    }

    bool
    PhononEngine::play()
    {
        qDebug() << "PhononEngine::play";

        m_mediaObject->play();
        emit statusChanged();

        return false;
    }

    bool
    PhononEngine::stop()
    {
        qDebug() << "PhononEngine::stop";

#if QT_VERSION > QT_VERSION_CHECK(4, 6, 3)
        m_mediaObject->stop();
        status = getStatus();
        emit statusChanged();
#else
        pause();
        jumpTo(0);
            /* workaround when yammi plays the next track instead of stop playing
               Was also the case with xine-engine.
               Watched on Squeeze.
               Is not needed on Wheezy */
#endif

        return true;
    }

    bool
    PhononEngine::pause()
    {
        qDebug() << "PhononEngine::pause";

        m_mediaObject->pause();
        emit statusChanged();

        return true;
    }

    bool
    PhononEngine::playPause()
    {
        qDebug() << "PhononEngine::playPause";

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
    PhononEngine::check()
    {
        //qDebug() << "PhononEngine::check";

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
    PhononEngine::getStatus()
    {
        //qDebug() << "PhononEngine::getStatus";

        PlayerStatus ret = STOPPED;

        switch (m_mediaObject->state()) {
        case Phonon::ErrorState:
            if (m_mediaObject->errorType() == Phonon::FatalError) {
                qCritical() << "PhononEngine::getStatus: Fatal Error:" << m_mediaObject->errorString();
            } else {
                qDebug() << "PhononEngine::getStatus: Warning:" << m_mediaObject->errorString();
            }
            break;
        case Phonon::PlayingState:
            ret = PLAYING;
            break;
        case Phonon::StoppedState:
            ret = STOPPED;
            break;
        case Phonon::PausedState:
            ret = PAUSED;
            break;
        case Phonon::BufferingState:
            ret = PAUSED;
            break;
        default:
            break;
        }

        return ret;
   }

    QString
    PhononEngine::getCurrentFile()
    {
        qDebug() << "PhononEngine::getCurrentFile";

        if( m_currentSong ) {
            return model->checkAvailability( m_currentSong );
        } else {
            return "";
        }
    }

    int
    PhononEngine::getCurrentTime()
    {
        //qDebug() << "PhononEngine::getCurrentTime";

        return m_mediaObject->currentTime();
    }

    int
    PhononEngine::getTotalTime()
    {
        qDebug() << "PhononEngine::getTotalTime";

        return m_mediaObject->totalTime();
    }

    bool
    PhononEngine::skipForward(bool)
    {
        qDebug() << "PhononEngine::skipForward";

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
    PhononEngine::skipBackward(bool withoutCrossfading)
    {
        qDebug() << "PhononEngine::skipBackward";

        Song* last = 0;
        if (!playlist->isEmpty()) {
            last = playlist->at(0)->song();
        }
        // insert pseudo-song to be removed
        playlist->insert(0, new SongEntryInt(last, 0));
        return skipForward(withoutCrossfading);
    }


    bool
    PhononEngine::jumpTo( int ms )
    {
        qDebug() << "PhononEngine::jumpTo" << ms << "status" << getStatus();

        if (getStatus() == PLAYING) {
            m_mediaObject->seek(ms);
            emit statusChanged();
        } else {
            m_rememberJumpTo = ms;
        }

        return true;
    }

    void
    PhononEngine::quit()
    {
        qDebug() << "PhononEngine::quit";
    }

    bool
    PhononEngine::finishInitialization()
    {
        qDebug() << "PhononEngine::finishInitialization";

        return true;
    }

    void
    PhononEngine::tick(qint64 /*time*/)
    {
        //qDebug() << "PhononEngine::tick" << time;
    }

    void
    PhononEngine::stateChanged(Phonon::State newState, Phonon::State /* oldState */)
    {
        qDebug() << "PhononEngine::stateChanged" << newState;

        switch (newState) {
        case Phonon::ErrorState:
            if (m_mediaObject->errorType() == Phonon::FatalError) {
                //QMessageBox::warning(this, tr("Fatal Error"), m_mediaObject->errorString());
            } else {
                //QMessageBox::warning(this, tr("Error"), m_mediaObject->errorString());
            }
            break;
        case Phonon::PlayingState:
            if (m_rememberJumpTo) {
                jumpTo(m_rememberJumpTo);
            }
            m_rememberJumpTo = 0;
            break;
        case Phonon::StoppedState:
            break;
        case Phonon::PausedState:
            break;
        case Phonon::BufferingState:
        default:
            break;
        }
        emit statusChanged();
    }

    void
    PhononEngine::sourceChanged(const Phonon::MediaSource &/*source*/)
    {
        //qDebug() << "PhononEngine::sourceChanged" << source.url();
    }

    void
    PhononEngine::aboutToFinish()
    {
        //qDebug() << "PhononEngine::aboutToFinish";
    }

    void
    PhononEngine::finished()
    {
        //qDebug() << "PhononEngine::finished";
#ifdef Q_OS_WIN32
        stop();
        /* for some reason when one songs ends in windows we do not change here to the next song */
#endif
    }

} //namespace Yammi
