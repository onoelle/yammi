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

#include "songentry.h"

#include <kdebug.h>

namespace Yammi {

ArtsPlayer::ArtsPlayer(YammiModel *yammi)
 : MediaPlayer(yammi)
{
	kdDebug()<<"+++ArtsPlayer::ArtsPlayer(YammiModel *yammi)"<<endl;
	m_dispatcher = new KArtsDispatcher( );
	m_server = new KArtsServer( );
	m_factory = new KDE::PlayObjectFactory( m_server->server() );
	m_currentPlay = 0;
	m_currentSong = 0;
}


ArtsPlayer::~ArtsPlayer()
{
	kdDebug()<<"---ArtsPlayer::~ArtsPlayer(YammiModel *yammi)"<<endl;
	quit( );
	delete m_factory;
	delete m_server;
	delete m_dispatcher;
}

void ArtsPlayer::check()
{
	PlayerStatus s = getStatus( );
	if( s != status )
	{
		if( status == PLAYING && s == STOPPED )
		{
			if( m_currentSong->song( ) == playlist->firstSong( ) )
			{
				kdDebug()<<"Song finished... get new one!"<<endl;
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

PlayerStatus ArtsPlayer::getStatus()
{
	if(!m_currentPlay)
		return STOPPED;
	switch( m_currentPlay->state() )
	{
		case Arts::posIdle:
			return STOPPED;
		case Arts::posPaused:
			return PAUSED;
		case Arts::posPlaying:
			return PLAYING;
	}
	return STOPPED; // shut up compiler warnings
}

void ArtsPlayer::clearPlaylist()
{
	emit playlistChanged( );
}

bool ArtsPlayer::play()
{
	if( m_currentPlay )
	{
		 if( m_currentPlay->state() != Arts::posPlaying )
			m_currentPlay->play( );
			
		status = getStatus();
		emit statusChanged();
		return true;
	}
	
	m_currentSong = playlist->at( 0 ); //firstSong( );
	if(!m_currentSong)
		return false;
		
	QString location = model->checkAvailability( m_currentSong->song() );
	if( location == "" || location == "never" )
	{
		kdWarning()<<"Song "<<m_currentSong->song()<<"cannot be found : "<<location<<endl;
		return false;
	}
	m_currentPlay = m_factory->createPlayObject( location, true );
	m_currentPlay->play( );
	status = getStatus( );
	emit statusChanged( );
	return true;
}

bool ArtsPlayer::pause()
{
	if( m_currentPlay && m_currentPlay->state() == Arts::posPlaying )
		m_currentPlay->pause();
	status = getStatus( );
	emit statusChanged( );
	return true;
}

bool ArtsPlayer::stop()
{
	if( m_currentPlay && m_currentPlay->state() != Arts::posIdle )
		m_currentPlay->halt();
	status = getStatus( );
	emit statusChanged( );
	return true;
}

bool ArtsPlayer::playPause()
{
	if( !m_currentPlay )
		play( );  //start new play
	else if( m_currentPlay && m_currentPlay->state() == Arts::posPlaying )
		m_currentPlay->pause();
	else if ( m_currentPlay && ( m_currentPlay->state() == Arts::posPaused || m_currentPlay->state( ) == Arts::posIdle ) )
		m_currentPlay->play();
	status = getStatus( );
	emit statusChanged( );
	return true;
}

bool ArtsPlayer::skipForward(bool withoutCrossfading)
{
	if( playlist->count() < 2 )
		return false;  // there is no "next song"
	if( m_currentPlay && m_currentPlay->state() != Arts::posIdle )
		m_currentPlay->halt();
	delete m_currentPlay;
	m_currentPlay = 0;
	m_currentSong = 0;
	
	//remove the first song and then just check for the next availabe one. play( ) will do the rest
	playlist->removeFirst( );
	QString location = "";
	while( playlist->at(0) )
	{
		location = model->checkAvailability( playlist->at(0)->song() );
		if( location == "" || location == "never" )
			playlist->removeFirst( );
		else
			break;
	}
	return play( );
}

bool ArtsPlayer::skipBackward(bool withoutCrossfading)
{
	return false;  // ????????
}

void ArtsPlayer::syncPlayer2Yammi(MyList* playlist)
{
	emit statusChanged( );
	emit playlistChanged();
}

void ArtsPlayer::syncYammi2Player(bool syncAll)
{
}

bool ArtsPlayer::jumpTo(int value)
{
	if( !m_currentPlay || value < 0 || value > getTotalTime() )
		return false;
	Arts::poTime t;
	t.seconds =  value / 1000;
	m_currentPlay->seek( t );
	return true;
}

QString ArtsPlayer::getCurrentFile()
{
	if( m_currentSong ) 
	{
		return model->checkAvailability( m_currentSong->song() );
	}
	else
	{
		return "";
	}
}

int ArtsPlayer::getCurrentTime()
{
	if( m_currentPlay ) {
		return m_currentPlay->currentTime().seconds * 1000;
	}
	return 0;
}

int ArtsPlayer::getTotalTime()
{
	if( m_currentPlay )
	{
		return m_currentPlay->overallTime().seconds * 1000;
	}
	return 0;
}

void ArtsPlayer::quit()
{
	if(m_currentPlay && m_currentPlay->state() != Arts::posIdle )
		m_currentPlay->halt( );
		
	delete m_currentPlay;
	m_currentPlay = 0;
}

};
