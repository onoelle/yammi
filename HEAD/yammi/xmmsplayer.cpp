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

#include "xmmsplayer.h"
#include <iostream>
#include <stdlib.h>
// xmms control
#include <xmmsctrl.h>

#include "yammigui.h"

// should also be removed to make it cleaner!!!
extern YammiGui* gYammiGui;


XmmsPlayer::XmmsPlayer(int session, YammiModel* model)
{
  this->session=session;
  this->model=model;
  ensureXmmsIsRunning();
  // check whether xmms is in shuffle mode: if yes, set it to normal
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
		xmms_remote_toggle_shuffle(session);
  }
}


// check whether xmms is running, if not: start it!
// returns, whether xmms was already running
bool XmmsPlayer::ensureXmmsIsRunning()
{
  if(xmms_remote_is_running(session)) {
    return true;
  }
  
	cout << "xmms not running, trying to start it...\n";
	system("xmms &");

  // TODO: wait until xmms is started or move the following into a single-shot timer???
  while(!xmms_remote_is_running(session)) {
    float x=17.3*session/42.7;
    for(int i=1; i<10000; i++)
      x=x*x;
//    cout << "waiting for xmms to have started...\n";
  }
  return false;
}


/**
 * writes all songs found in xmms playlist to yammi's playlist
 * also, clears xmms playlist except for the first config.keepInXmms
 */
void XmmsPlayer::syncPlayer2Yammi(Folder* folder)
{
	// 1. delete all songs already played
	for(int i=xmms_remote_get_playlist_pos(session)-1; i>=0; i--) {
		xmms_remote_playlist_delete(session, i);
	}
	// 2. insert all (including currently played) songs into yammi playlist
	folder->clearSongs();
	for(int i=0; i<xmms_remote_get_playlist_length(session); i++) {
 		char buf[200];
		strcpy(buf, xmms_remote_get_playlist_file(session, i));
		Song* check=model->getSongFromFilename(QString(buf));
		if(!check)	// song not found in database
			continue;
		folder->addSong(check);
	}
	// 3. delete all but the keepInXmms first songs
	for(int i=xmms_remote_get_playlist_length(session)-1; i>=model->config.keepInXmms; i--) {
		xmms_remote_playlist_delete(session, i);
	}
}


/**
 * tries to sync the xmms playlist with the yammi playlist
 * ie. writes all entries from yammi playlist into xmms playlist
 */
void XmmsPlayer::syncYammi2Player(bool syncAll)
{
	// check whether xmms playlist is empty
	if(xmms_remote_get_playlist_length(session)==0) {
		for(int i=0; i<(int)model->songsToPlay.count() && (i<model->config.keepInXmms || syncAll); i++) {
			gchar url[300];
			strcpy(url, model->songsToPlay.at(i)->song()->location());
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
	for(; iXmms<model->config.keepInXmms || ( syncAll && iYammi<(int)model->songsToPlay.count() ); iXmms++, iYammi++) {

		// check whether xmms playlist entry existing
		if(iXmms<(int)xmms_remote_get_playlist_length(session)) {
			// yes, existing => compare yammi and xmms entry
	 		char buf[300];
//			cout << QString("xmms_remote_get_playlist_file %1").arg(iXmms) << "\n";
			strcpy(buf, xmms_remote_get_playlist_file(session, iXmms));
//			cout << "..done\n";
			Song* check=model->getSongFromFilename(QString(buf));

			// corresponding yammi entry existing?
			if(iYammi<(int)model->songsToPlay.count()) {
				Song* s=model->songsToPlay.at(iYammi)->song();
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
				if(iYammi+1<(int)model->songsToPlay.count() && check==model->songsToPlay.at(iYammi+1)->song()) {
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
//TODO:?					myWait(50);
					iXmms--;
					continue;
				}
			}
		}
		else {			// xmms playlist too short => check whether songs in songsToPlay
			if(iYammi<(int)model->songsToPlay.count()) {
//				cout << "trying to fill up xmms playlist with song from yammi\n";
				Song* s=model->songsToPlay.at(iYammi)->song();
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
// TODO:?				myWait(50);
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
// TODO:?		myWait(50);
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


/// skip forward in playlist (if controlPressed without crossfading)
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


/// skip backward in playlist (if controlPressed without crossfading)
/// TODO: clean up, make independent of Yammi?
bool XmmsPlayer::skipBackward(bool withoutCrossfading)
{
  ensureXmmsIsRunning();
  if(withoutCrossfading) {
    xmms_remote_pause(session);
  }
  int count=model->songsPlayed.count();
	if(count==0)			// empty folder songsPlayed => can's skip backwards
		return false;

	// 1. get and remove last song from songsPlayed
	Song* last=model->songsPlayed.at(count-1)->song();
	model->songsPlayed.remove(count-1);
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
    // xmms playing or playing but paused
    if(!xmms_remote_is_paused(session)) {
      // case 1: xmms is playing
      return PLAYING;
    }
    else {
      // case 2: xmms is paused
      return PAUSED;
    }
  }
  else {
    // case 3: xmms is stopped
    return STOPPED;
  }
}

bool XmmsPlayer::jumpTo(int value)
{
  xmms_remote_jump_to_time(session, value);
  return true;
}

QString XmmsPlayer::getCurrentFile()
{
  int pos=xmms_remote_get_playlist_pos(session);
  return QString(xmms_remote_get_playlist_file(session, pos));
}

int XmmsPlayer::getCurrentTime()
{
  return xmms_remote_get_output_time(session);
}


int XmmsPlayer::getTotalTime()
{
  int pos=xmms_remote_get_playlist_pos(session);
  return xmms_remote_get_playlist_time(session, pos);
}


void XmmsPlayer::removePlayed()
{
  if(!xmms_remote_is_playing(session)) {
    // if player stopped and only one song left in playlist, we should probably(?) remove it
    // not too nice and clean, I know...
    if(xmms_remote_get_playlist_length(session)==1) {
 	    xmms_remote_playlist_delete(session, 0);
      model->songsToPlay.removeFirst();
    }
  }
  else {
    for(int i=0; true ; i++) {
	    int check=xmms_remote_get_playlist_pos(session);
      if(!check>0)
	   	  break;
//	cout << "removing played songs: " << i << "\n";
//	cout << QString("xmms_remote_get_playlist_file 0\n");
  	  QString file(xmms_remote_get_playlist_file(session, 0));
//	cout << "..done, returned: " << file << "\n";
	    Song* x=model->getSongFromFilename(file);
//	cout << QString("__xmms_remote_playlist_delete 0\n");

 	// the following call sometimes seems to crash xmms
 	// (and does not return until xmms is killed => freezes yammi)
 	//************************************************************
 	    xmms_remote_playlist_delete(session, 0);
//	myWait(100);
//				cout << "..done\n";
   	if(x==model->songsToPlay.at(0)->song())
 	  	model->songsToPlay.removeFirst();
    }
  }
}

void XmmsPlayer::quit()
{
  xmms_remote_quit(session);
}


/*	some xmms statistics...
		gint len=xmms_remote_get_playlist_length(0);
		gint pTime=xmms_remote_get_playlist_time(0, 0);
		gint rate, freq, nch;
		xmms_remote_get_info(0, &rate, &freq, &nch);
		cout << "outputTime: " << outputTime << ", length: " << len << ", pTime: "
		<< pTime << ", rate: " << rate << ", freq: " << freq << ", nch " << nch << "\n";
*/


