/***************************************************************************
                               artsplayer.cpp
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

#include "artsplayer.h"

#include <arts/kplayobject.h>
#include <arts/kplayobjectfactory.h>
#include <arts/kartsdispatcher.h>
#include <arts/kartsserver.h>

#include <kdebug.h>

#include "songentry.h"
#include "songentryint.h"


namespace Yammi {

    ArtsPlayer::ArtsPlayer(YammiModel *yammi) : MediaPlayer(yammi) {
        kdDebug() << "ArtsPlayer::ArtsPlayer(YammiModel *yammi)" << endl;
        m_dispatcher = new KArtsDispatcher( );
        kdDebug() << "creating KArtsServer...\n";
        m_server = new KArtsServer( );
        if(m_server == 0) {
            kdError() << "ERROR: could not create KArtsServer\n";
            return;
        }
        Arts::SoundServerV2 server = m_server->server();
        if(server.isNull()) {
            kdError() << "ERROR: could not get server from KArtsServer\n";
            return;
        }
        kdDebug() << "creating PlayObjectFactory...\n";
        m_factory = new KDE::PlayObjectFactory( server );
        kdDebug() << "...done\n";
        m_currentPlay = 0;
        m_currentSong = 0;
    }


    ArtsPlayer::~ArtsPlayer() {
        kdDebug() << "---ArtsPlayer::~ArtsPlayer(YammiModel *yammi)" << endl;
        quit( );
        delete m_factory;
        delete m_server;
        delete m_dispatcher;
    }

    
    void ArtsPlayer::check() {
        PlayerStatus s = getStatus( );
        if( s != status ) {
            if( status == PLAYING && s == STOPPED ) {
                if( m_currentSong == playlist->firstSong() ) {
                    kdDebug() << "Song finished... get new one!" << endl;
                    playlist->removeFirst( );
                    delete m_currentPlay;
                    m_currentPlay = 0;
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

    
    PlayerStatus ArtsPlayer::getStatus() {
        if(!m_currentPlay)
            return STOPPED;
        switch( m_currentPlay->state() ) {
        case Arts::posIdle:
            return STOPPED;
        case Arts::posPaused:
            return PAUSED;
        case Arts::posPlaying:
            return PLAYING;
        }
        return STOPPED; // shut up compiler warnings
    }

    
    void ArtsPlayer::clearPlaylist() {
        emit playlistChanged( );
    }

    
    bool ArtsPlayer::play() {
        if( m_currentPlay ) {
            if( m_currentPlay->state() != Arts::posPlaying ) {
                m_currentPlay->play( );
            }
            status = getStatus();
            emit statusChanged();
            return true;
        }
        return false;
    }

    
    bool ArtsPlayer::pause() {
        if( m_currentPlay && m_currentPlay->state() == Arts::posPlaying ) {
            m_currentPlay->pause();
        }
        status = getStatus( );
        emit statusChanged( );
        return true;
    }

    
    bool ArtsPlayer::stop() {
        pause();
        jumpTo(0);
/*
        if( m_currentPlay && m_currentPlay->state() != Arts::posIdle ) {
            m_currentPlay->halt();            
        }
        status = getStatus( );
        emit statusChanged( );*/
        return true;
    }

    
    bool ArtsPlayer::playPause() {
        if( !m_currentPlay ) {
            play( );  //start new play
        }
        else if( m_currentPlay && m_currentPlay->state() == Arts::posPlaying ) {
            m_currentPlay->pause();
        }
        else if ( m_currentPlay && ( m_currentPlay->state() == Arts::posPaused || m_currentPlay->state( ) == Arts::posIdle ) ) {
            m_currentPlay->play();
        }
        status = getStatus( );
        emit statusChanged( );
        return true;
    }

    
    bool ArtsPlayer::skipForward(bool) {
        if( playlist->count() < 2 ) {
            // there is no "next song"
            return false;
        }
        int savedStatus = getStatus();
        if( m_currentPlay ) {
            if(m_currentPlay->state() != Arts::posIdle ) {
                m_currentPlay->halt();
            }
            delete m_currentPlay;
            m_currentPlay = 0;
        }
        m_currentSong = 0;

        // remove the first song and sync playlist again
        playlist->removeFirst( );
        syncYammi2Player();
        emit playlistChanged();
        if(!m_currentSong) {
            return false;
        }        
        if(savedStatus == PLAYING) {
            play();
        }
        return true;
    }

    
    bool ArtsPlayer::skipBackward(bool withoutCrossfading) {
        Song* last=playlist->at(0)->song();
        // insert pseudo-song to be removed
        playlist->insert(0, new SongEntryInt(last, 0));
        return skipForward(withoutCrossfading);
    }

    
    void ArtsPlayer::syncYammi2Player() {
        bool haveToUpdate=model->skipUnplayableSongs();
        
        if(playlist->count()==0) {
            if( m_currentPlay && m_currentPlay->state() != Arts::posIdle ) {
                m_currentPlay->halt();
            }
            delete m_currentPlay;
            m_currentPlay = 0;
            m_currentSong = 0;
            if(haveToUpdate) {
                emit playlistChanged();
            }
            return;
        }
        if(m_currentSong != 0) {
            kdDebug() << "m_currentSong: " << m_currentSong->displayName() << endl;
        }
        if(playlist->count() > 0) {
            kdDebug() << "playlist->at(0): " << playlist->at(0)->song()->displayName() << endl;
        }
            
        if(m_currentSong == playlist->at(0)->song()) {
            if(haveToUpdate) {
                emit playlistChanged();
            }
            return;
        }
        if( m_currentPlay ) {
            if (m_currentPlay->state() != Arts::posIdle ) {
                m_currentPlay->halt();
            }
            delete m_currentPlay;
            m_currentPlay = 0;
        }
        m_currentSong = 0;
        
        QString location = model->checkAvailability( playlist->at(0)->song() );
        kdDebug() << "returned location: " << location << endl;
        m_currentSong = playlist->at(0)->song();
        KURL url;
        url.setPath(location);
        if(!url.isValid()) {
            kdDebug() << "ERROR: url not valid: " << url << endl;
            return;
        }
        m_currentPlay = m_factory->createPlayObject( url, true );
        // these 2 lines ensure that totalTime() returns meaningful values
        // (otherwise, totalTime() seems to return 0 as long as song has not started playing yet)
        m_currentPlay->play();
        m_currentPlay->pause();
/*
        while( playlist->at(0) ) {
            location = model->checkAvailability( playlist->at(0)->song() );
            if( location == "" || location == "never" ) {
                kdWarning() << "Song " << playlist->at(0)->song() << "not available, skipping\n";
                playlist->removeFirst();
            } else {
                m_currentSong = playlist->at(0)->song();
                m_currentPlay = m_factory->createPlayObject( location, true );
                // these 2 lines ensure that totalTime() returns meaningful values
                // (otherwise, totalTime() seems to return 0 as long as song has not started playing yet)
                kdDebug() << "applying workaround\n";
                m_currentPlay->play();
                m_currentPlay->pause();
                return;
            }
        }
*/
        if(haveToUpdate) {
            emit playlistChanged();
        }
        return;
    }

    
    bool ArtsPlayer::jumpTo(int value) {
        kdDebug() << "ArtsPlayer::jumpTo\n";
        if( !m_currentPlay) {
            return false;
        }
        if( value < 0 || value > getTotalTime() ) {
            return false;
        }
        status = getStatus();
        Arts::poTime t;
        t.seconds =  value / 1000;
        t.ms = value % 1000;
        m_currentPlay->seek( t );
        return true;
    }

    
    QString ArtsPlayer::getCurrentFile() {
        if( m_currentSong ) {
            return model->checkAvailability( m_currentSong );
        } else {
            return "";
        }
    }

    
    int ArtsPlayer::getCurrentTime() {
        if( m_currentPlay ) {
            return m_currentPlay->currentTime().seconds * 1000 + m_currentPlay->currentTime().ms;
        }
        return 0;
    }

    
    int ArtsPlayer::getTotalTime() {
        if( m_currentPlay ) {
            return m_currentPlay->overallTime().seconds * 1000 + m_currentPlay->overallTime().ms;
        }
        return 0;
    }
    
    
    void ArtsPlayer::quit() {
        if(m_currentPlay) {
            if(m_currentPlay->state() != Arts::posIdle ) {
                m_currentPlay->halt( );
            }
            delete m_currentPlay;
            m_currentPlay = 0;
        }
    }

};
