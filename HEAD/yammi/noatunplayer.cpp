/***************************************************************************
                          noatunplayer.cpp  -  description
                             -------------------
    begin                : Sun Jan 19 2003
    copyright            : (C) 2003 by Oliver Nölle
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

#include "noatunplayer.h"
#include <iostream>
using namespace std;
#include <stdlib.h>
#include <qprocess.h>
#include "songentryint.h"

NoatunPlayer::NoatunPlayer(YammiModel* model)
{
  this->model=model;
  playlist=&(model->songsToPlay);
  lastStatus=STOPPED;

#ifdef ENABLE_NOATUN
  // register ourselve
  client = new DCOPClient();
  client->attach();
  QCString realAppId = client->registerAs("yammi");
#endif

  ensurePlayerIsRunning();
  // find out which one of the players is playing and take that one as current player
  currentPlayer=1;
  if(getStatus()==STOPPED) {
    currentPlayer=0;
  }
   
  // this will be the inactive player
  currentPlayer=(currentPlayer+1) % 2;
  sendDcopCommandInt("setVolume(int)", 0);
  clearPlaylist();

  // this will be the active player
  currentPlayer=(currentPlayer+1) % 2;
  sendDcopCommandInt("setVolume(int)", 100);
  fade=100;
  connect( &fadeTimer, SIGNAL(timeout()), SLOT(onFade()) );
}


NoatunPlayer::~NoatunPlayer()
{
}



//********************************************************


/**
 * Clear playlist of active player.
 * A bit complicated because the dcop interface for noatun's playlist is very limited.
 */
void NoatunPlayer::clearPlaylist()
{
  // go back as long as the result of currentFile() changes...
  // TODO: a song queued twice?
  QString prev=getCurrentFile();
  sendDcopCommand(QString("back()"));
  for(int tries=0; prev!=getCurrentFile() && tries<100; tries++) {
    prev=getCurrentFile();
    sendDcopCommand(QString("back()"));
  }
  // now delete entries as long as currentFile()!=""
  for(QString file=getCurrentFile(); file!=""; file=getCurrentFile()) {
    sendDcopCommand(QString("removeCurrent()"));
  }
}



// check whether two instances of Noatun are running, if not: tries to start them
// returns true on success
bool NoatunPlayer::ensurePlayerIsRunning()
{
  int count=0;
  QString replyStr;
  for(int tries=0; count<2 && tries<10; tries++) {
    count=0;
    QProcess proc;
    proc.addArgument("dcop");
    if ( !proc.start() ) {
      cout << "ERROR: could not start dcop process\n";
      return false;
    }
    while(proc.isRunning()) {}
    if(!proc.normalExit()) {
      cout << "ERROR: normalExit is false for dcop command (trying to continue...)\n";
    }
    while(replyStr=proc.readLineStdout()) {
      if(replyStr.startsWith("noatun")) {
//        cout << count << ". noatun process found: " << replyStr << "\n";
        QString idStr=replyStr.mid(7);
        if(idStr=="") {
          cout << "!!!!!!!!\nit looks like you have the following option checked in noatun:\n";
          cout << "\"allow only one instance\"\n";
          cout << "=> disable it first, restart noatun and then restart yammi!\n";
          cout << "(I think I feel like crashing now...)\n!!!!!!!\n";
          return false;
        }
        int id=atoi(idStr);
        playerId[count]=id;
        count++;
      }
    }
    for(int i=count; i<2; i++) {
      // if not enough players running: start the missing ones!
      cout << "trying to start another instance of noatun...\n";
      system("noatun");
    }
  }
  if(count<2) {
    cout << "ERROR: could not start the two required noatun instances!\n";
    cout << "have you \"allow only one instance\" option checked? disable it first!\n";
    return false;
  }
  return true;
}

int NoatunPlayer::getCurrentPlayerId()
{
  return playerId[currentPlayer];
}

int NoatunPlayer::getOtherPlayerId()
{
  return playerId[(currentPlayer+1) % 2];
}



// fade: 0 = beginning of fading, 100 = fading complete
void NoatunPlayer::onFade()
{
  int fadeTime=getCurrentTime();
  fade=fadeTime*100/model->config.fadeTime;
  if(fade<100) {
    sendDcopCommandInt("setVolume(int)", 100-(fade*(100-model->config.fadeOutEnd)/100), fadeOut);
    sendDcopCommandInt("setVolume(int)", model->config.fadeInStart+(fade*(100-model->config.fadeInStart)/100), fadeIn);
    fadeTimer.start( 200, TRUE );
  }
  else {
    fade=100;
    // set volume to 100 of fadeIn player, remove song of fadeOut player
    sendDcopCommandInt("setVolume(int)", 100, fadeIn);
    sendDcopCommandInt("setVolume(int)", 0, fadeOut);
    sendDcopCommand(QString("removeCurrent()"), fadeOut);
  }
  
}


/**
 * Checks, whether a song change should take place.
 */
void NoatunPlayer::check()
{
  // 1. check, whether status has changed
  PlayerStatus newStatus=getStatus();
  if(newStatus!=lastStatus) {
    lastStatus=newStatus;
    if(newStatus==STOPPED) {
      // heuristic: if we were at end of last song and only one song left in playlist
      // => we should remove it
      if(playlist->count()==1 && timeLeft<2000) {
        sendDcopCommand(QString("removeCurrent()"));
        playlist->removeFirst();
        playlistChanged();
      }
      if(playlist->count()>1 && model->config.fadeTime==0) {
        // no crossfading configured
        // heuristic: if we were at end of last song, we should start next one
        if(timeLeft<2000) {
//          cout << "heuristic, starting song change...\n";
          startSongChange();
        }
      }
    }
    statusChanged();
  }

  timeLeft=getTotalTime()-getCurrentTime();

  // 2. check, whether we should initiate a song change (start crossfading)
  if(model->config.fadeTime>0) {
    if(getStatus()==PLAYING && timeLeft<model->config.fadeTime && timeLeft>0) {
      if(fade<100) {  // don't start fading if we are still fading last song? TODO: really?
        cout << "start crossfading, but we are still crossfading (" << fade << ")\n";
      }
      startSongChange();
    }
  }
  else {
    if(getStatus()==PLAYING && timeLeft<1000 && timeLeft>0) {
//      cout << "should prepare for next songchange without crossfading...\n";      
    }
  }
}


/**
 * Starts a song change, if possible (if >=2 songs in playlist)
 */
void NoatunPlayer::startSongChange(bool withoutCrossfading)
{
  if(playlist->count()<2) {
    // we can't make a song change if there is no next song
    return;
  }
  PlayerStatus status=getStatus();
  if(model->config.fadeTime==0 || withoutCrossfading || status!=PLAYING) {
    // no crossfading
    clearPlaylist();
    QString location=model->checkAvailability(playlist->at(1)->song());
    if(location!="" && location!="never") {
      playlistAdd(location, status==PLAYING || model->config.fadeTime==0);
      playlist->removeFirst();
    }
    playlistChanged();
    statusChanged();
  }
  else {
    // crossfading
    fade=0;
    fadeOut=getCurrentPlayerId();
    currentPlayer=(currentPlayer+1) % 2;
    fadeIn=getCurrentPlayerId();
    fadeTimer.start( 200, TRUE );       // change volume every 200ms

    clearPlaylist();
    QString location=model->checkAvailability(playlist->at(1)->song());
    if(location!="" && location!="never") {
      playlistAdd(location, true);
      playlist->removeFirst();
    }
    playlistChanged();
  }
}


/**
 * Adds a file to the current noatun player.
 * Due to noatun's api via dcop, we can only add a song and start playing it
 * immediately (or otherwise we can't "access" the song any more(eg. start playing it)).
 * Therefore we simulate a passive add by pausing immediately after adding the song.
*/
void NoatunPlayer::playlistAdd(QString filename, bool autoStart, bool fakePassiveAdd)
{
  int id=getCurrentPlayerId();
  QString str=QString("noatun-%1").arg(id);
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

  arg << filename;
  arg << fakePassiveAdd;
  
#ifdef ENABLE_NOATUN
  if (!client->send(str.latin1(), "Noatun", "addFile(QString, bool)", data)) {
    cout << "nop\n";
  }
#endif
  if(!autoStart) {
    pause();
  }
}


/// start playing
bool NoatunPlayer::play()
{
	sendDcopCommand("play()");
  return true;
}


/// pause
bool NoatunPlayer::pause()
{
  if(getStatus()==PAUSED) {
    return true;
  }
	sendDcopCommand("playpause()");
  if(fade<100) {
    // if we are currently crossfading, we should pause both players
    sendDcopCommand("playpause()", getOtherPlayerId());
  }
  return true;
}


/// toggle between play and pause
bool NoatunPlayer::playPause()
{
	sendDcopCommand("playpause()");
  if(fade<100) {
    // if we are currently crossfading, we should playPause both players
    sendDcopCommand("playpause()", getOtherPlayerId());
  }
  return true;
}


/// skip forward in playlist
bool NoatunPlayer::skipForward(bool withoutCrossfading)
{
  startSongChange(withoutCrossfading);
  return true;
}


/// skip backward in playlist
bool NoatunPlayer::skipBackward(bool withoutCrossfading)
{
  Song* last=playlist->at(0)->song();
  // insert pseudo-song to be removed
  playlist->insert(0, new SongEntryInt(last, 0));
  startSongChange(withoutCrossfading);
  return true;
}

/// stop playback
bool NoatunPlayer::stop()
{
	sendDcopCommand("stop()");
  if(fade<100) {
    // if we are currently crossfading, we should stop both players
    sendDcopCommand("stop()", getOtherPlayerId());
  }
  return true;
}


PlayerStatus NoatunPlayer::getStatus()
{
  int state=callGetInt("state()");
  // case 1: noatun is playing
  if(state==2)
    return PLAYING;
  // case 2: noatun is paused
  if(state==1)
    return PAUSED;
  // case 3: noatun is stopped
  if(state==0)
    return STOPPED;
  cout << "unknown state received from noatun\n";
  return STOPPED;
}

bool NoatunPlayer::jumpTo(int value)
{
  sendDcopCommandInt("skipTo(int)", value);
  return true;
}


QString NoatunPlayer::getCurrentFile()
{
  return callGetString("title()");
}

int NoatunPlayer::getCurrentTime()
{
  return callGetInt("position()");
}


int NoatunPlayer::getTotalTime()
{
  return callGetInt("length()");
}

// only called on startup of yammi
void NoatunPlayer::syncPlayer2Yammi(MyList* playlist)
{
  // go back as long as the result of currentFile() changes...
  // TODO: a song queued three times?
  QString prev2=getCurrentFile();
  sendDcopCommand(QString("back()"));
  QString prev=getCurrentFile();
  sendDcopCommand(QString("back()"));
  for(int tries=0; prev!=getCurrentFile() || prev2!=getCurrentFile() && tries<500; tries++) {
    prev2=prev;
    prev=getCurrentFile();
    sendDcopCommand(QString("back()"));
  }
  
  // now delete all other entries (as long as currentFile()!="")
  for(QString file=getCurrentFile(); file!=""; file=getCurrentFile()) {
    Song* toAdd=model->getSongFromFilename(file);
    if(toAdd!=0) {
      playlist->append(new SongEntryInt(toAdd, playlist->count()+1));
    }
    else {
      cout << "song not in database: " << file << "\n";
    }
    sendDcopCommand(QString("removeCurrent()"));
  }
  playlistChanged();
  lastStatus=getStatus();
  statusChanged();
}


void NoatunPlayer::syncYammi2Player(bool syncAll)
{
  if(playlist->count()==0) {
    // playlist empty
    sendDcopCommand(QString("removeCurrent()"));
    return;
  }

  // 1. sync current song
  QString noatunCurrent=getCurrentFile();
  // the following is necessary for swapped songs...
	QString location=model->checkAvailability(playlist->at(0)->song());
  while(location=="" || location=="never") {
    cout << "song not available (try to first enqueue and load from a media)\n";
    playlist->removeFirst();
    location=model->checkAvailability(playlist->at(0)->song());
    playlistChanged();
  }
  if(location=="" || location=="never") {
    return;
  }
  QString yammiCurrent=location.right(location.length()-location.findRev('/')-1);
  if(noatunCurrent!=yammiCurrent) {
//    cout << "setting Noatun's current to Yammi's current\n";
//    cout << "noatun file: |" << noatunCurrent << "|, yammi current: " << yammiCurrent << "\n";
    clearPlaylist();
    playlistAdd(location, false);
    sendDcopCommandInt("setVolume(int)", 100);
  }

  // if desired: write complete playlist of Yammi to current noatun instance
  if(syncAll) {
    for(unsigned int i=1; i<playlist->count(); i++) {
      QString location=model->checkAvailability(playlist->at(i)->song());
      if(location=="" || location=="never") {
        continue;
      }
      playlistAdd(location, true, false);
    }
    return;
  }
  for(unsigned int i=0; i<playlist->count(); i++) {
    sendDcopCommand(QString("back()"));
  }
}


void NoatunPlayer::quit()
{
}


void NoatunPlayer::sendDcopCommandInt(QString command, int param, int id)
{
  if(id==0) {
    id=getCurrentPlayerId();
  }
  QString str=QString("noatun-%1").arg(id);

  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);
  arg << param;

#ifdef ENABLE_NOATUN
  if (!client->send(str.latin1(), "Noatun", command.latin1(), data)) {
    cout << "some error using DCOP.\n";
  }
#endif
}


void NoatunPlayer::sendDcopCommand(QString command, int id)
{
  if(id==0) {
    id=getCurrentPlayerId();
  }
  QString str=QString("noatun-%1").arg(id);

  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

#ifdef ENABLE_NOATUN
  if (!client->send(str.latin1(), "Noatun", command.latin1(), data)) {
    cout << "xmms-kde: there was some error using DCOP.\n";
  }
  else {
//    cout << "xmms-kde: success\n";
  }
#endif
}


int NoatunPlayer::callGetInt(QString command, int id)
{
  if(id==0) {
    id=getCurrentPlayerId();
  }
  QByteArray data, replyData;
  QCString replyType;
  QDataStream arg(data, IO_WriteOnly);

  int result=0;

#ifdef ENABLE_NOATUN
  QString str=QString("noatun-%1").arg(id);
  if (!client->call(str.latin1(), QString("Noatun").latin1(), command.latin1(), data, replyType, replyData)) {
    cout << "call() failed, maybe noatun was closed?\n";
    cout << "I will check now and restart noatun if necessary...\n";
    ensurePlayerIsRunning();
  }
  else {
    QDataStream reply(replyData, IO_ReadOnly);
    if (replyType == "int") {
      reply >> result;
    } else
      cout << "unexpected type of dcop reply (int expected): " << replyType << "\n";
  }
#endif
  return result;
}


QString NoatunPlayer::callGetString(QString command, int id)
{
  if(id==0) {
    id=getCurrentPlayerId();
  }
  QByteArray data, replyData;
  QCString replyType;
  QDataStream arg(data, IO_WriteOnly);

  QString result="";

#ifdef ENABLE_NOATUN
  QString str=QString("noatun-%1").arg(id);
  if (!client->call(str.latin1(), QString("Noatun").latin1(), command.latin1(), data, replyType, replyData))
    cout << "call() failed\n";
  else {
    QDataStream reply(replyData, IO_ReadOnly);
    if (replyType == "QString") {
      reply >> result;
    } else
      cout << "unexpected type of dcop reply (QString expected): " << replyType << "\n";
  }
#endif
  return result;
}
