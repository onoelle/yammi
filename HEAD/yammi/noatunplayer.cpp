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

#include "noatunplayer.h"
#include <iostream>
using namespace std;
#include <stdlib.h>
#include <qprocess.h>

#include "yammigui.h"

// should also be removed to make it cleaner!!!
extern YammiGui* gYammiGui;



NoatunPlayer::NoatunPlayer(YammiModel* model)
{
  this->model=model;

  // register ourselv?
  client = new DCOPClient();
  client->attach();
  QCString realAppId = client->registerAs("yammi");
  cout << "registered as: " << realAppId << "\n";

  // TODO: we assume we have two players
  QProcess proc;
  proc.addArgument("dcop");
  if ( !proc.start() ) {
    cout << "xcould not start dcop process\n";
    return;
  }
  while(proc.isRunning()) {}
  if(!proc.normalExit()) {
    cout << "xnormalExit is false\n";
  }
  QString replyStr;
  int count=0;
  while(replyStr=proc.readLineStdout()) {
    if(replyStr.startsWith("noatun")) {
      cout << count << ". noatun process found: " << replyStr << "\n";
      QString idStr=replyStr.mid(7);
      int id=atoi(idStr);
      cout << "id: " << id << "\n";
      playerId[count]=id;
      count++;
    }
  }
  cout << "stopping player 1...\n";
  currentPlayer=1;
  stop();
  cout << "starting player 0...\n";
  currentPlayer=0;
  play();
  
//  bool alreadyRunning=ensurePlayerIsRunning();
//  if(alreadyRunning) {
//    cout << "noatun is already running\n";
//  }
  // check whether noatun is in shuffle mode: if yes, disable it
  // (confuses Yammi's playlistmanagement)
  // TODO:
  /*
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
    cout << "switching noatun shuffle mode back on...\n";
    // TODO:
  }
}



//********************************************************




// check whether noatun is running, if not: starts it!
// returns, whether noatun was already running
bool NoatunPlayer::ensurePlayerIsRunning()
{
  // TODO:
  return true;
  cout << "noatun not running, starting it...\n";
  // to be sure, we wait another 100ms before starting interaction with Xmms
//  gYammiGui->myWait(100);
  cout << "...noatun is up!\n";
  return false;
}

int NoatunPlayer::getCurrentPlayerId()
{
  return playerId[currentPlayer];
}


bool NoatunPlayer::check(QString nextSong)
{
  int timeLeft=getTotalTime()-getCurrentTime();
  if(getStatus()==PLAYING && timeLeft<5000 && timeLeft>0) {
    // TODO: single-shot timer to fill other player?
    cout << "we start the next song: " << nextSong << "...\n";
    currentPlayer=(currentPlayer+1) % 2;
    sendDcopCommand(QString("removeCurrent()"));
    playlistAdd(nextSong);
//    QString command=QString("addFile(QString, bool) \"%1\" 1").arg(nextSong);
//    sendDcopCommand(command);
    return true;
  }
  return false;
}

void NoatunPlayer::playlistAdd(QString filename, int id)
{
  if(id==0) {
    id=getCurrentPlayerId();
  }
  QString str=QString("noatun-%1").arg(id);
//  QFileInfo file(filename);

  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

  arg << filename;
  arg << true;
  
  // the following is a comment from xmms-kde
  // crashes sometimes :-(
  if (!client->send(str.latin1(), "Noatun", "addFile(QString, bool)", data)) {
    cout << "nop\n";
  }
  else {
    cout << "yep\n";
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
  sendDcopCommand("forward()");
  return true;
}


/// skip backward in playlist
/// TODO: clean up, make independent of Yammi?
bool NoatunPlayer::skipBackward(bool withoutCrossfading)
{
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
  int id=getCurrentPlayerId();
  QString str=QString("noatun-%1").arg(id);

  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);

  arg << value;

  if (!client->send(str.latin1(), "Noatun", "skipTo(int)", data)) {
    cout << "xnop\n";
  }
  else {
    cout << "xyep\n";
  }
  return true;
}


QString NoatunPlayer::getCurrentFile()
{
  // TODO:
  return QString("not implemented yet...");
}

int NoatunPlayer::getCurrentTime()
{
  return callGetInt("position()");
}


int NoatunPlayer::getTotalTime()
{
  return callGetInt("length()");
}


void NoatunPlayer::removePlayed()
{
  // TODO
}

void NoatunPlayer::syncPlayer2Yammi(Folder* folder)
{
  cout << "syncing player2Yammi not implemented yet...\n";
}
void NoatunPlayer::syncYammi2Player(bool syncAll)
{
  cout << "syncing Yammi2Player not implemented yet...\n";
}


void NoatunPlayer::quit()
{
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

int NoatunPlayer::callGetIntX(QString command, int id)
{
  if(id==0) {
    id=getCurrentPlayerId();
  }
  QString str=QString("noatun-%1").arg(id);
  QProcess proc;
  proc.addArgument( "dcop" );
  proc.addArgument( str );
  proc.addArgument( "Noatun" );
  proc.addArgument(command);
  if ( !proc.launch("") ) {
    // error handling
    cout << "could not start dcop process\n";
    return -1;
  }
  while(proc.isRunning()) {
  }
  if(!proc.normalExit()) {
    cout << "normalExit is false\n";
  }
  QString replyStr=proc.readLineStdout();
  if(replyStr==0)
    return 0;
//  cout << "replyStr: " << replyStr << "\n";
  return(atoi(replyStr));
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
//    cout << "call() succeded\n";
    QDataStream reply(replyData, IO_ReadOnly);
    if (replyType == "int") {

      reply >> result;
    } else
      cout << "xmms-kde: unexpected type of dcop reply";
//      qDebug("xmms-kde: unexpected type of dcop reply");
  }
  return result;
}
