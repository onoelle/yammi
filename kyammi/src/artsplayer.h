/***************************************************************************
                               artsplayer.h
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



#ifndef YAMMIARTSPLAYER_H
#define YAMMIARTSPLAYER_H

#include "mediaplayer.h"

namespace KDE {
class PlayObjectFactory;
class PlayObject;
}
class KArtsDispatcher;
class KArtsServer;

class SongEntry;

namespace Yammi {

/** Media Player for direct comunication with Arts
    
    The ArtsPlayer is a MediaPlayer for Yammi, which comunicates
    directly with the Arts server, allowing Yammi to run "native"
    on top of KDE, without any intermediary, external players
    
    A difference from this player to the other MediaPlayers is
    that this one does not keep an "internal" playlist - it fetches 
    the next song directly from yammi's playlist
    
    While Yammi's interaction with the ArtsPlayer is not optimal ( they
    comunicate via the two signals playlistChanged() and statusChanged() )
    this is necessary to keep Yammi working with external players,like Xmms
    and Noatun

    @author Luis De la Parra <lparrab -at- gmx.net>
*/
class ArtsPlayer : public MediaPlayer
{
Q_OBJECT
public:
	ArtsPlayer( YammiModel *yammi );
	virtual ~ArtsPlayer();
    
	virtual QString getName()  {return "ArtsPlayer";}
	virtual void clearPlaylist();

public slots:
	
    /** Reimplemented from MediaPlayer. */
    virtual bool finishInitialization();

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
	     Should stop the current song, and go back to the previously played song. Since ArtsPlayer does not keep it's own playlist, and the playlist in Yammi must be "consumed" in order for the MediaPlayer to work properly, there is never a previous song in the playlist because we always play the first item in the list.
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
	KArtsDispatcher* m_dispatcher;
	KArtsServer* m_server;
	KDE::PlayObjectFactory* m_factory;
	KDE::PlayObject* m_currentPlay;
	Song* m_currentSong;
};


};  //Yammi

#endif
