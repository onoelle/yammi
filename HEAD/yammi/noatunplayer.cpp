/***************************************************************************
                          noatunplayer.cpp  -  description
                             -------------------
    begin                : Sun Jan 19 2003
    copyright            : (C) 2003 by Oliver N�lle
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

#include "noatunplayer.h"
#include <iostream>
using namespace std;
#include <stdlib.h>
#include <qprocess.h>

// TODO: remove, use signal!
#include "yammigui.h"
extern YammiGui* gYammiGui;



NoatunPlayer::NoatunPlayer(YammiModel* model)
{
  this->model=model;
  playlist=&(model->songsToPlay);

  // register ourselve
  client = new DCOPClient();
  client->attach();
  QCString realAppId = client->registerAs("yammi");
  cout << "dcop registered as: " << realAppId << "\n";

  int count=0;
  QString replyStr;
  for(int tries=0; count<2 && tries<10; tries++) {
    count=0;
    QProcess proc;
    proc.addArgument("dcop");
    if ( !proc.start() ) {
      cout << "ERROR: could not start dcop process\n";
      return;
    }
    while(proc.isRunning()) {}
    if(!proc.normalExit()) {
      cout << "ERROR: normalExit is false for dcop command\n";
    }
    while(replyStr=proc.readLineStdout()) {
      if(replyStr.startsWith("noatun")) {
        cout << count << ". noatun process found: " << replyStr << "\n";
        QString idStr=replyStr.mid(7);
        if(idStr=="") {
          cout << "!!!!!!!!\nit looks like you have the following option checked in noatun:\n";
          cout << "\"allow only one instance\"\n";
          cout << "=> disable it first, restart noatun and then restart yammi!\n";
          cout << "(I think I feel like crashing now...)\n!!!!!!!\n";
          return;
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
    return;
  }

  // TODO: clear both playlists?
  cout << "stopping player 1...\n";
  currentPlayer=1;
  clearPlaylist();
  sendDcopCommandInt("setVolume(int)", 0);
//  stop();


  cout << "starting player 0...\n";
  currentPlayer=0;

//  clearPlaylist();
//  syncPlayer2Yammi(playlist);
  sendDcopCommandInt("setVolume(int)", 100);
  fade=100;
  connect( &fadeTimer, SIGNAL(timeout()), SLOT(onFade()) );
  
/* TODO:
  if(xmms_remote_is_shuffle(session)) {
    xmms_remote_toggle_shuffle(session);
    shuffleWasActivated=true;
		cout << "switching off xmms shuffle mode (does confuse my playlist management otherwise)\n";
  }
  else {
	 	shuffleWasActivated=false;
  }
  */
}

NoatunPlayer::~NoatunPlayer()
{
	if(shuffleWasActivated) {
    // TODO:
//    cout << "switching noatun shuffle mode back on...\n";
  }
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



// check whether noatun is running, if not: starts it!
// returns, whether noatun was already running
bool NoatunPlayer::ensurePlayerIsRunning()
{
  return true;
  cout << "noatun not running, starting it...\n";
  // TODO: start noatun if necessary
  cout << "...noatun is up!\n";
  return false;
}

int NoatunPlayer::getCurrentPlayerId()
{
  return playerId[currentPlayer];
}

int NoatunPlayer::getOtherPlayerId()
{
  return playerId[(currentPlayer+1) % 2];
}

void NoatunPlayer::onFade()
{
  if(fade<100) {
    sendDcopCommandInt("setVolume(int)", 100-(fade/2), fadeOut);
    sendDcopCommandInt("setVolume(int)", 50+(fade/2), fadeIn);
    fade+=3;
    fadeTimer.start( 200, TRUE );
  }
  else {
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
  // don't start fading if we are still fading last song
  if(fade<100) {
    return;
  }
  int timeLeft=getTotalTime()-getCurrentTime();
  if(getStatus()==PLAYING && timeLeft<10000 && timeLeft>0) {
    startSongChange();
  }
}


void NoatunPlayer::startSongChange()
{
  fade=0;     // 0 = 100 % last song, 100 = 100 % next song
  fadeOut=getCurrentPlayerId();
  currentPlayer=(currentPlayer+1) % 2;
  fadeIn=getCurrentPlayerId();
  fadeTimer.start( 200, TRUE );

  clearPlaylist();
  if(playlist->count()>=2) {
    playlistAdd(playlist->at(1)->song()->location(), true);
    playlist->removeFirst();
    // TODO: notify yammi!
  }
}

void NoatunPlayer::playlistAdd(QString filename, bool autoStart, int id)
{
  if(id==0) {
    id=getCurrentPlayerId();
  }
  QString str=QString("noatun-%1").arg(id);
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

  arg << filename;
  arg << true;
  
  if (!client->send(str.latin1(), "Noatun", "addFile(QString, bool)", data)) {
    cout << "nop\n";
  }
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
  return true;
}


/// toggle between play and pause
bool NoatunPlayer::playPause()
{
	sendDcopCommand("playpause()");
  return true;
}


/// skip forward in playlist
bool NoatunPlayer::skipForward(bool withoutCrossfading)
{
  startSongChange();
  return true;
/*
  if(withoutCrossfading) {
  }
  sendDcopCommand("forward()");
  return true;
*/
}


/// skip backward in playlist
bool NoatunPlayer::skipBackward(bool withoutCrossfading)
{
  // TODO:
  cout << "not implemented yet...\n";
  sendDcopCommand("back()");
  return true;
}

/// stop playback
bool NoatunPlayer::stop()
{
	sendDcopCommand("stop()");
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


void NoatunPlayer::syncPlayer2Yammi(MyList* playlist)
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
    cout << "file found: |" << file << "|, calling removeCurrent()\n";
    Song* toAdd=model->getSongFromFilename(file);
    if(toAdd!=0) {
      // TODO: we have to append a SongEntryInt!!!
      playlist->appendSong(toAdd);
    }
    else {
      cout << "song not in database: " << file << "\n";
    }
    sendDcopCommand(QString("removeCurrent()"));
  }
  gYammiGui->slotFolderChanged();
}


void NoatunPlayer::syncYammi2Player(bool syncAll)
{
  cout << "syncing Yammi2player(0)...\n";
  if(playlist->count()<=0)
    return;

  cout << "syncing Yammi2player(1)...\n";
  // 1. sync current song
  QString current=getCurrentFile();
  if(current!=playlist->at(0)->song()->filename) {
    cout << "setting Noatun's current to Yammi's current\n";
    cout << "noatun file: |" << current << "|, yammi current: " << playlist->at(0)->song()->filename << "\n";
    clearPlaylist();
    playlistAdd(playlist->at(0)->song()->location(), false);
    sendDcopCommandInt("setVolume(int)", 100);
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

  if (!client->send(str.latin1(), "Noatun", command.latin1(), data)) {
    cout << "some error using DCOP.\n";
  }
}


void NoatunPlayer::sendDcopCommand(QString command, int id)
{
  if(id==0) {
    id=getCurrentPlayerId();
  }
  QString str=QString("noatun-%1").arg(id);

  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

  if (!client->send(str.latin1(), "Noatun", command.latin1(), data)) {
    cout << "xmms-kde: there was some error using DCOP.\n";
  }
  else {
//    cout << "xmms-kde: success\n";
  }
}


int NoatunPlayer::callGetInt(QString command, int id)
{
  if(id==0) {
    id=getCurrentPlayerId();
  }
  QByteArray data, replyData;
  QCString replyType;
  QDataStream arg(data, IO_WriteOnly);

  int result;

  QString str=QString("noatun-%1").arg(id);
  if (!client->call(str.latin1(), QString("Noatun").latin1(), command.latin1(), data, replyType, replyData))
    cout << "call() failed\n";
  else {
    QDataStream reply(replyData, IO_ReadOnly);
    if (replyType == "int") {
      reply >> result;
    } else
      cout << "unexpected type of dcop reply (int expected): " << replyType << "\n";
  }
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

  QString result;

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
  return result;
}
