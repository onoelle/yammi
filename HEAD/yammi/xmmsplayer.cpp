/***************************************************************************
                          xmmsplayer.cpp  -  description
                             -------------------
    begin                : Tue Sep 24 2002
    copyright            : (C) 2002 by Oliver N�lle
    email                : oli.noelle@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "xmmsplayer.h"
#include "options.h"

#ifdef ENABLE_XMMS
#include <xmmsctrl.h>        // xmms control
#endif

#include <iostream>
using namespace std;
#include <stdlib.h>

#include "songentryint.h"


XmmsPlayer::XmmsPlayer(int session, YammiModel* model)
{
#ifdef ENABLE_XMMS
  this->session=session;
  this->model=model;
  this->playlist=&(model->songsToPlay);
  
  bool alreadyRunning=ensurePlayerIsRunning();
  if(alreadyRunning) {
    cout << "xmms is already running\n";
  }
  // check whether xmms is in shuffle mode: if yes, disable it
  // (confuses Yammi's playlistmanagement)
  if(xmms_remote_is_shuffle(session)) {
    xmms_remote_toggle_shuffle(session);
    xmmsShuffleWasActivated=true;
		cout << "switching off xmms shuffle mode (does confuse my playlist management otherwise)\n";
  }
  else {
	 	xmmsShuffleWasActivated=false;
  }
#endif
}


XmmsPlayer::~XmmsPlayer()
{
#ifdef ENABLE_XMMS
	if(xmmsShuffleWasActivated) {
    cout << "switching xmms shuffle mode back on...\n";
		xmms_remote_toggle_shuffle(session);
  }
#endif
}


/** Checks whether xmms is running, if not, starts it.
 * This call blocks until xmms is up.
 * Returns, whether xmms was already running.
 */
bool XmmsPlayer::ensurePlayerIsRunning()
{
#ifdef ENABLE_XMMS
  if(xmms_remote_is_running(session)) {
    return true;
  }
  
	cout << "xmms not running, starting it...\n";
	system("xmms &");

  while(!xmms_remote_is_running(session)) {
    myWait(100);
  }
  // to be sure, we wait another 100ms before starting interaction with Xmms
  myWait(100);
  cout << "...xmms is up!\n";
#endif
  return false;
}


/**
 * Writes all songs found in xmms playlist to Yammi's playlist.
 * Clears xmms playlist except for the first config.keepInXmms songs.
 * Only called on Yammi startup.
 */
void XmmsPlayer::syncPlayer2Yammi(MyList* playlist)
{
#ifdef ENABLE_XMMS
	// 1. delete all songs already played
	for(int i=xmms_remote_get_playlist_pos(session)-1; i>=0; i--) {
		xmms_remote_playlist_delete(session, i);
	}
	// 2. insert all (including currently played) songs into yammi playlist
  playlist->clear();
	for(int i=0; i<xmms_remote_get_playlist_length(session); i++) {
 		char buf[200];
		strcpy(buf, xmms_remote_get_playlist_file(session, i));
		Song* check=model->getSongFromFilename(QString(buf));
		if(!check) {	// song not found in database
			continue;
    }
    playlist->append(new SongEntryInt(check, playlist->count()+1));
	}
	// 3. delete all but the keepInXmms first songs
	for(int i=xmms_remote_get_playlist_length(session)-1; i>=model->config.keepInXmms; i--) {
		xmms_remote_playlist_delete(session, i);
	}

  playlistChanged();
  lastStatus=getStatus();
  statusChanged();  
#endif
}


/**
 * Tries to sync the xmms playlist with the Yammi playlist,
 * ie. writes all entries from Yammi's playlist into xmms' playlist.
 */
void XmmsPlayer::syncYammi2Player(bool syncAll)
{
#ifdef ENABLE_XMMS
	// check whether xmms playlist is empty
	if(xmms_remote_get_playlist_length(session)==0) {
		for(int i=0; i<(int)playlist->count() && (i<model->config.keepInXmms || syncAll); i++) {
			gchar url[300];
			strcpy(url, playlist->at(i)->song()->location());
			xmms_remote_playlist_add_url_string(session, url);
		}
		return;
	}

	// okay, one or more songs in xmms playlist

	// iterate through first <keepInXmms> or all (syncAll) songs in xmms playlist
	// if playlist too short => insert yammi entries
	int iXmms=0;
	int iYammi=0;
	for(; iXmms<model->config.keepInXmms || ( syncAll && iYammi<(int)playlist->count() ); iXmms++, iYammi++) {

		// check whether xmms playlist entry existing
		if(iXmms<(int)xmms_remote_get_playlist_length(session)) {
			// yes, existing => compare yammi and xmms entry
	 		char buf[300];
			strcpy(buf, xmms_remote_get_playlist_file(session, iXmms));
			Song* check=model->getSongFromFilename(QString(buf));

			// corresponding yammi entry existing?
			if(iYammi<(int)playlist->count()) {
				Song* s=playlist->at(iYammi)->song();
				if(check==s)
					continue;		// okay, both are the same

				// song mismatch between yammi and xmms

				// case 1: xmms song not in yammi database
				// => leave unknown song
				if(check==0) {
					iYammi--;
					continue;
				}

				// case 2: xmms song is yammi+1 (some song moved in front of it)
				// => insert the song that was inserted
				if(iYammi+1<(int)playlist->count() && check==playlist->at(iYammi+1)->song()) {
					// check if songfile is available...
					QString loc=model->checkAvailability(s);
					if(loc=="" || loc=="never") {
						iXmms--;
						continue;
					}
					gchar url[300];
					strcpy(url, loc);
					xmms_remote_playlist_ins_url_string(session, url, iXmms);
					continue;
				}

				// case 3: xmms+1 song is yammi (song removed from there) => delete
				xmms_remote_playlist_delete(session, iXmms);
				iXmms--;
				iYammi--;

			}
			else {		// yammi playlist too short
				if(check==0) {
//					cout << "xmms playlist longer than yammi playlist, but unknown song\n";
        }
				else {
//					cout << "xmms playlist longer than yammi playlist, deleting\n";
					xmms_remote_playlist_delete(session, iXmms);
					iXmms--;
					continue;
				}
			}
		}
		else {			// xmms playlist too short => check whether songs in songsToPlay
			if(iYammi<(int)playlist->count()) {
				Song* s=playlist->at(iYammi)->song();
				// check if songfile is available...
				QString loc=model->checkAvailability(s);
				if(loc=="" || loc=="never") {
					iXmms--;
					continue;
				}
				gchar url[300];
				strcpy(url, loc);
				xmms_remote_playlist_add_url_string(session, url);
			}
		}
	} // end of for

	// now process leftover songs in xmms playlist
	for(; iXmms<(int)xmms_remote_get_playlist_length(session); ) {
 		char buf[300];
		strcpy(buf, xmms_remote_get_playlist_file(session, iXmms));
		Song* check=model->getSongFromFilename(QString(buf));
		if(check==0) {
			continue;
    }
		xmms_remote_playlist_delete(session, iXmms);
	}

	// if xmms is not playing, we might have inserted songs before the active one
	// => set active song to first
	// caution! xmms reports as "not playing" sometimes (on song change?)
/*
	if(!xmms_remote_is_playing(0)) {
		xmms_remote_set_playlist_pos(0, 0);
	}
*/
#endif
}


/// start playing
bool XmmsPlayer::play()
{
#ifdef ENABLE_XMMS
	xmms_remote_play(session);
  return true;
#endif
}


/// pause
bool XmmsPlayer::pause()
{
#ifdef ENABLE_XMMS
	xmms_remote_pause(session);
  return true;
#endif
}


/// toggle between play and pause
bool XmmsPlayer::playPause()
{
#ifdef ENABLE_XMMS
  ensurePlayerIsRunning();
	xmms_remote_play_pause(session);
#endif
  return true;
}


/// skip forward in playlist (if desired without crossfading)
bool XmmsPlayer::skipForward(bool withoutCrossfading)
{
#ifdef ENABLE_XMMS
  ensurePlayerIsRunning();
  if(withoutCrossfading) {
    xmms_remote_pause(session);
  }
  int x= xmms_remote_get_playlist_pos(session);
	xmms_remote_set_playlist_pos(session, x+1);
  if(withoutCrossfading) {
    xmms_remote_play(session);
  }
#endif
  return true;
}


/// skip backward in playlist (if desired without crossfading)
/// TODO: clean up, make independent of Yammi?
bool XmmsPlayer::skipBackward(bool withoutCrossfading)
{
#ifdef ENABLE_XMMS
  ensurePlayerIsRunning();
  if(withoutCrossfading) {
    xmms_remote_pause(session);
  }

  int pos=xmms_remote_get_playlist_pos(session);
	gchar url[500];
  Song* last=playlist->at(0)->song();
  
  strcpy(url, last->location());
	xmms_remote_playlist_ins_url_string(session, url, pos);
	xmms_remote_set_playlist_pos(session, pos);
  playlistChanged();
  
  if(withoutCrossfading) {
    xmms_remote_play(session);
  }
#endif
  return true;
}

/// stop playback
bool XmmsPlayer::stop()
{
#ifdef ENABLE_XMMS
  ensurePlayerIsRunning();
	xmms_remote_stop(session);
#endif
  return true;
}


PlayerStatus XmmsPlayer::getStatus()
{
#ifdef ENABLE_XMMS
  if(xmms_remote_is_playing(session)) {
    // xmms playing or paused
    if(!xmms_remote_is_paused(session))
      return PLAYING;
    else
      return PAUSED;
  }
  else
#endif
    return STOPPED;
}

/// get current position in song
int XmmsPlayer::getCurrentTime()
{
#ifdef ENABLE_XMMS
  return xmms_remote_get_output_time(session);
#else
  return 0;
#endif
}

/// get total time of current song
int XmmsPlayer::getTotalTime()
{
#ifdef ENABLE_XMMS
  int pos=xmms_remote_get_playlist_pos(session);
  return xmms_remote_get_playlist_time(session, pos);
#else
  return 0;
#endif
}

/// jump to position in song
bool XmmsPlayer::jumpTo(int value)
{
#ifdef ENABLE_XMMS
  xmms_remote_jump_to_time(session, value);
#endif
  return true;
}

/// return current filename
QString XmmsPlayer::getCurrentFile()
{
#ifdef ENABLE_XMMS
  int pos=xmms_remote_get_playlist_pos(session);
  return QString(xmms_remote_get_playlist_file(session, pos));
#else
  return "";
#endif
}



void XmmsPlayer::check()
{
#ifdef ENABLE_XMMS
  // 1. check, whether status has changed
  // bug in XMMS?: sometimes reports a wrong status on song change (not playing)!!! 
  PlayerStatus newStatus=getStatus();
  if(newStatus!=lastStatus) {
    lastStatus=newStatus;
    if(newStatus==STOPPED) {
      // heuristic: if player stopped "near end of song" (beware of crossfading plugin!)
      // and only one song left in playlist
      // => we remove it
      if(playlist->count()==1 && timeLeft<15000) {
        xmms_remote_playlist_delete(session, 0);
        playlist->removeFirst();
        playlistChanged();
      }
    }
    statusChanged();
  }

  timeLeft=getTotalTime()-getCurrentTime();

  // 2. remove already played songs
//  if(lastStatus!=STOPPED) {
    for(int i=0; true ; i++) {
	    int check=xmms_remote_get_playlist_pos(session);
      if(check==0) {
	   	  break;
      }
  	  QString file(xmms_remote_get_playlist_file(session, 0));
	    Song* firstXmmsSong=model->getSongFromFilename(file);

      // the following call sometimes seems to crash xmms
      // (and does not return until xmms is killed => freezes yammi)
      //************************************************************
 	    xmms_remote_playlist_delete(session, 0);

      if(playlist->count()>=1) {
        if(playlist->at(0)->song()==firstXmmsSong) {
          playlist->removeFirst();
        }
      }
      playlistChanged();
    }
//  }
#endif
}


/// quit player
void XmmsPlayer::quit()
{
#ifdef ENABLE_XMMS
  xmms_remote_quit(session);
#endif
}

void XmmsPlayer::myWait(int msecs)
{
	QTime t;
	t.start();
	while(t.elapsed()<msecs);
}


/*	some xmms statistics...
		gint len=xmms_remote_get_playlist_length(0);
		gint pTime=xmms_remote_get_playlist_time(0, 0);
		gint rate, freq, nch;
		xmms_remote_get_info(0, &rate, &freq, &nch);
		cout << "outputTime: " << outputTime << ", length: " << len << ", pTime: "
		<< pTime << ", rate: " << rate << ", freq: " << freq << ", nch " << nch << "\n";
*/
