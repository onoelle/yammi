/***************************************************************************
                          xmmsplayer.cpp  -  description
                             -------------------
    begin                : Tue Sep 24 2002
    copyright            : (C) 2002 by Oliver Nölle
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

#include "options.h"
#ifdef ENABLE_XMMS
 
#include "xmmsplayer.h"
#include <iostream>
using namespace std;
#include <stdlib.h>
#include <xmmsctrl.h>        // xmms control

// TODO: remove! use signals!
#include "yammigui.h"
extern YammiGui* gYammiGui;


XmmsPlayer::XmmsPlayer(int session, YammiModel* model)
{
  this->session=session;
  this->model=model;
  this->playlist=&(model->songsToPlay);
  
  bool alreadyRunning=ensureXmmsIsRunning();
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
}


XmmsPlayer::~XmmsPlayer()
{
	if(xmmsShuffleWasActivated) {
    cout << "switching xmms shuffle mode back on...\n";
		xmms_remote_toggle_shuffle(session);
  }
}


/** Checks whether xmms is running, if not, starts it.
 * This call blocks until xmms is up.
 * Returns, whether xmms was already running.
 */
bool XmmsPlayer::ensureXmmsIsRunning()
{
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
  return false;
}


/**
 * Writes all songs found in xmms playlist to Yammi's playlist.
 * Clears xmms playlist except for the first config.keepInXmms songs.
 * Only called on Yammi startup.
 */
void XmmsPlayer::syncPlayer2Yammi(MyList* playlist)
{
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
		if(!check)	// song not found in database
			continue;
    // TODO: we have to append a SongEntryInt!!!
    playlist->append(new SongEntryInt(check, playlist->count()+1));
	}
	// 3. delete all but the keepInXmms first songs
	for(int i=xmms_remote_get_playlist_length(session)-1; i>=model->config.keepInXmms; i--) {
		xmms_remote_playlist_delete(session, i);
	}

  playlistChanged();
  lastStatus=getStatus();
  statusChanged();  
}


/**
 * Tries to sync the xmms playlist with the Yammi playlist,
 * ie. writes all entries from Yammi's playlist into xmms' playlist.
 */
void XmmsPlayer::syncYammi2Player(bool syncAll)
{
	// check whether xmms playlist is empty
	if(xmms_remote_get_playlist_length(session)==0) {
		for(int i=0; i<(int)playlist->count() && (i<model->config.keepInXmms || syncAll); i++) {
			gchar url[300];
			strcpy(url, playlist->at(i)->song()->location());
//			cout << QString("xmms_remote_playlist_add_url_string %1").arg(url) << "\n";
			xmms_remote_playlist_add_url_string(session, url);
//			cout << "..done\n";
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
//			cout << QString("xmms_remote_get_playlist_file %1").arg(iXmms) << "\n";
			strcpy(buf, xmms_remote_get_playlist_file(session, iXmms));
//			cout << "..done\n";
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
//					cout << QString("xmms_remote_playlist_ins_url_string %1 %2").arg(url).arg(iXmms) << "\n";
					xmms_remote_playlist_ins_url_string(session, url, iXmms);
//					cout << "..done\n";
					continue;
				}

				// case 3: xmms+1 song is yammi (song removed from there) => delete
//				cout << QString("xmms_remote_playlist_delete %1").arg(iXmms) << "\n";
				xmms_remote_playlist_delete(session, iXmms);
//				cout << "..done\n";
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
//				cout << "trying to fill up xmms playlist with song from yammi\n";
				Song* s=playlist->at(iYammi)->song();
				// check if songfile is available...
				QString loc=model->checkAvailability(s);
				if(loc=="" || loc=="never") {
					iXmms--;
					continue;
				}
				gchar url[300];
				strcpy(url, loc);
//				cout << QString("xmms_remote_playlist_add_url %1").arg(url) << "\n";
				xmms_remote_playlist_add_url_string(session, url);
//				cout << "..done\n";
			}
		}
	} // end of for

	// now process leftover songs in xmms playlist
	for(; iXmms<(int)xmms_remote_get_playlist_length(session); ) {
 		char buf[300];
//		cout << QString("xmms_remote_get_playlist_file %1").arg(iXmms) << "\n";
		strcpy(buf, xmms_remote_get_playlist_file(session, iXmms));
//		cout << "..done\n";
		Song* check=model->getSongFromFilename(QString(buf));
		if(check==0)
			continue;
//		cout << QString("xmms_remote_playlist_delete %1").arg(iXmms) << "\n";
		xmms_remote_playlist_delete(session, iXmms);
//		cout << "..done\n";
	}

	// if xmms is not playing, we might have inserted songs before the active one
	// => set active song to first
	// caution! xmms reports as not playing sometimes (immediately after skip forward?)
/*
	if(!xmms_remote_is_playing(0)) {
		xmms_remote_set_playlist_pos(0, 0);
	}
*/

}


/// start playing
bool XmmsPlayer::play()
{
	xmms_remote_play(session);
  return true;
}


/// pause
bool XmmsPlayer::pause()
{
	xmms_remote_pause(session);
  return true;
}


/// toggle between play and pause
bool XmmsPlayer::playPause()
{
  ensureXmmsIsRunning();
	xmms_remote_play_pause(session);
  return true;
}


/// skip forward in playlist (if desired without crossfading)
bool XmmsPlayer::skipForward(bool withoutCrossfading)
{
  ensureXmmsIsRunning();
  if(withoutCrossfading) {
    xmms_remote_pause(session);
  }
  int x= xmms_remote_get_playlist_pos(session);
	xmms_remote_set_playlist_pos(session, x+1);
  if(withoutCrossfading) {
    xmms_remote_play(session);
  }
  return true;
}


/// skip backward in playlist (if desired without crossfading)
/// TODO: clean up, make independent of Yammi?
bool XmmsPlayer::skipBackward(bool withoutCrossfading)
{
  ensureXmmsIsRunning();
  if(withoutCrossfading) {
    xmms_remote_pause(session);
  }
  int count=playlist->count();
	if(count==0)			// empty folder songsPlayed => can't skip backwards
		return false;

	// 1. get and remove last song from songsPlayed
	Song* last=playlist->at(count-1)->song();
	playlist->remove(count-1);
	gYammiGui->folderSongsPlayed->updateTitle();
//	cout << "last: " << last->displayName() << "\n";

	int pos=xmms_remote_get_playlist_pos(session);
	gchar url[500];
	strcpy(url, last->location());
	xmms_remote_playlist_ins_url_string(session, url, pos);
	xmms_remote_set_playlist_pos(session, pos);
	gYammiGui->currentSong=0;
	gYammiGui->folderActual->insertSong(last, 0);

	// update necessary?
	if(gYammiGui->chosenFolder==gYammiGui->folderActual || gYammiGui->chosenFolder==gYammiGui->folderSongsPlayed)
		gYammiGui->slotFolderChanged();
	else
		gYammiGui->songListView->triggerUpdate();

  if(withoutCrossfading) {
    xmms_remote_play(session);
  }
  return true;
}

/// stop playback
bool XmmsPlayer::stop()
{
  ensureXmmsIsRunning();
	xmms_remote_stop(session);
  return true;
}


PlayerStatus XmmsPlayer::getStatus()
{
  if(xmms_remote_is_playing(session)) {
    // xmms playing or paused
    if(!xmms_remote_is_paused(session))
      return PLAYING;
    else
      return PAUSED;
  }
  else
    return STOPPED;
}

/// get current position in song
int XmmsPlayer::getCurrentTime()
{
  return xmms_remote_get_output_time(session);
}

/// get total time of current song
int XmmsPlayer::getTotalTime()
{
  int pos=xmms_remote_get_playlist_pos(session);
  return xmms_remote_get_playlist_time(session, pos);
}

/// jump to position in song
bool XmmsPlayer::jumpTo(int value)
{
  xmms_remote_jump_to_time(session, value);
  return true;
}

/// return current filename
QString XmmsPlayer::getCurrentFile()
{
  int pos=xmms_remote_get_playlist_pos(session);
  return QString(xmms_remote_get_playlist_file(session, pos));
}



void XmmsPlayer::check()
{
  // 1. check, whether status has changed
  PlayerStatus newStatus=getStatus();
  if(newStatus!=lastStatus) {
    lastStatus=newStatus;
    statusChanged();
  }

  if(!xmms_remote_is_playing(session)) {
    // if player stopped and only one song left in playlist, we should probably(?) remove it
    // not too nice and clean, I know...
    if(xmms_remote_get_playlist_length(session)==1) {
 	    xmms_remote_playlist_delete(session, 0);
      if(playlist->count()>=1) {
        playlist->removeFirst();
        playlistChanged();
      }
    }
  }
  else {
    for(int i=0; true ; i++) {
	    int check=xmms_remote_get_playlist_pos(session);
      if(check==0)
	   	  break;
//	cout << "removing played songs: " << i << "\n";
//	cout << QString("xmms_remote_get_playlist_file 0\n");
  	  QString file(xmms_remote_get_playlist_file(session, 0));
//	cout << "..done, returned: " << file << "\n";
	    Song* firstXmmsSong=model->getSongFromFilename(file);
//	cout << QString("__xmms_remote_playlist_delete 0\n");

 	// the following call sometimes seems to crash xmms
 	// (and does not return until xmms is killed => freezes yammi)
 	//************************************************************
 	    xmms_remote_playlist_delete(session, 0);
//	myWait(100);
//				cout << "..done\n";
      if(playlist->count()>=1) {
        if(playlist->at(0)->song()==firstXmmsSong) {
          playlist->removeFirst();
        }
      }
    }
  }
}


/// quit player
void XmmsPlayer::quit()
{
  xmms_remote_quit(session);
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


#endif

