/***************************************************************************
                          noatunplayer.cpp  -  description
                             -------------------
    begin                : Sun Jan 19 2003
    copyright            : (C) 2003 by Oliver N�le
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

#include "options.h"

#include "noatunplayer.h"
#include <iostream>
#include <kurl.h>
#include <kdebug.h>
#include <kprocess.h>
using namespace std;
#include <stdlib.h>
#include <qprocess.h>
#include <qwaitcondition.h>
#include "songentryint.h"

NoatunPlayer::NoatunPlayer(YammiModel* model) : MediaPlayer( model ) {
    // register ourselve
    client = new DCOPClient();
    client->attach();
    QCString realAppId = client->registerAs("yammi");

    currentPlayer=1;
    fade=100;
    ensurePlayerIsRunning(false);
}   


bool NoatunPlayer::finishInitialization()
{
    ensurePlayerIsRunning(true);
    // find out which one of the players is playing and take that one as current player
    if(getStatus()==STOPPED) {
        currentPlayer=0;
    }

    // this will be the inactive player
    currentPlayer=(currentPlayer+1) % 2;
    sendDcopCommandInt("setVolume(int)", 0);
    clearActivePlayerPlaylist();

    // this will be the active player
    currentPlayer=(currentPlayer+1) % 2;
    sendDcopCommandInt("setVolume(int)", 100);
    connect( &fadeTimer, SIGNAL(timeout()), SLOT(onFade()) );
    return true;
}


NoatunPlayer::~NoatunPlayer() {}



//********************************************************


/**
 * Clear playlist of active player.
 */
void NoatunPlayer::clearActivePlayerPlaylist() {
    sendDcopCommand(QString("clear()"));
    return;
}



/**
 * check whether two instances of Noatun are running, if not: tries to start them
 * returns true on success
 */
bool NoatunPlayer::ensurePlayerIsRunning(bool block) {
    int count=0;
    QString replyStr;
    for(int tries=0; count<2 && tries<10; tries++) {
        count=0;
        QProcess proc;
        proc.addArgument("dcop");
        if ( !proc.start() ) {
            kdError() << "could not start dcop process\n";
            return false;
        }
        while(proc.isRunning()) {}
        if(!proc.normalExit()) {
            kdError() << "normalExit is false for dcop command (trying to continue...)\n";
        }
        while(replyStr=proc.readLineStdout()) {
            if(replyStr.startsWith("noatun")) {
                //        kdDebug() << count << ". noatun process found: " << replyStr << "\n";
                QString idStr=replyStr.mid(7);
                if(idStr=="") {
                    kdError() << "it looks like you have the following option checked in noatun:\n";
                    kdError() << "\"allow only one instance\"\n";
                    kdError() << "=> disable it first, restart noatun and then restart yammi!\n";
                    kdFatal() << "(I think I feel like crashing now...)\n!!!!!!!\n";                    
                    return false;
                }
                int id=atoi(idStr);
                playerId[count]=id;
                count++;
            }
        }
        for(int i=count; i<2; i++) {
            // if not enough players running: start the missing ones!
            kdDebug() << "trying to start another instance of noatun...\n";
// does not work well:
/*            KProcess proc;
            proc << "noatun";
            proc.start(KProcess::Block); */
            system("noatun");
        }
/*        if(count < 2) {
            // wait for noatun to startup
            QWaitCondition wait;
            wait.wait(1000);
        }*/
    }
    if(count < 2) {
        kdError() << "could not start the two required noatun instances!\n";
        kdError() << "have you \"allow only one instance\" option checked? disable it first!\n";
        return false;
    }
    return true;
}

int NoatunPlayer::getCurrentPlayerId() {
    return playerId[currentPlayer];
}

int NoatunPlayer::getOtherPlayerId() {
    return playerId[(currentPlayer+1) % 2];
}



// fade: 0 = beginning of fading, 100 = fading complete
void NoatunPlayer::onFade() {
//    kdDebug() << getCurrentPlayerId() << "onFade()" << endl;
    int currentTime=getCurrentTime();
    int totalTime=getTotalTime();
//    kdDebug() << "currentTime: " << currentTime << "totalTime: " << totalTime << endl;
    if(totalTime == -1) {
        // this is a workaround for noatun sometimes not starting a song
        sendDcopCommand("play()");
    }
    if(currentTime <= 0) {
        fade++;
        if(fade>6) {
            if(totalTime <= 0) {
                kdDebug() << "current song apparently not playable (totalTime <= 0), skipping..." << endl;
                kdDebug() << "file: " << playlist->at(0)->song()->location() << endl;
                model->skipUnplayableSongs(true);
                if(playlist->count() >= 2) {
                    QString location=model->checkAvailability(playlist->at(1)->song());
                    if(location!="" && location!="never") {
                        playlistAdd(location, true);
                        playlist->removeFirst();
                    }
                    playlistChanged();
                    fade = 0;
                }
            }
        }
    }
    else {
        fade = currentTime * 100 / model->config()->fadeTime;
    }
    if(fade<100) {
        sendDcopCommandInt("setVolume(int)", 100-(fade*(100-model->config()->fadeOutEnd)/100), fadeOut);
        sendDcopCommandInt("setVolume(int)", model->config()->fadeInStart+(fade*(100-model->config()->fadeInStart)/100), fadeIn);
        fadeTimer.start( 200, TRUE );
    } else {
        fade=100;
        // set volume to 100 of fadeIn player, remove song of fadeOut player
        sendDcopCommandInt("setVolume(int)", 100, fadeIn);
        sendDcopCommandInt("setVolume(int)", 0, fadeOut);
        sendDcopCommand(QString("clear()"), fadeOut);
    }

}


/**
 * Checks, whether a song change should take place.
 */
void NoatunPlayer::check() {
    // 1. check, whether status has changed
    PlayerStatus newStatus=getStatus();
    if(newStatus!=lastStatus) {
        lastStatus=newStatus;
        if(newStatus==STOPPED) {
            // heuristic: if we were at end of last song and only one song left in playlist
            // => we should remove it
            if(playlist->count()==1 && timeLeft<2000) {
                sendDcopCommand(QString("clear()"));
                playlist->removeFirst();
                playlistChanged();
            }
            if(playlist->count()>1 && model->config()->fadeTime==0) {
                // no crossfading configured
                // heuristic: if we were at end of last song, we should start next one
                if(timeLeft<2000) {
                    //          kdDebug() << "heuristic, starting song change...\n";
                    startSongChange();
                }
            }
        }
        statusChanged();
    }

    timeLeft=getTotalTime()-getCurrentTime();

    // 2. check, whether we should initiate a song change (start crossfading)
    if(model->config()->fadeTime>0) {
        if(getStatus()==PLAYING && timeLeft<model->config()->fadeTime && timeLeft>0) {
            if(fade<100) {  // don't start fading if we are still fading last song? TODO: really?
                kdDebug() << "start crossfading, but we are still crossfading (" << fade << ")\n";
                sendDcopCommand(QString("clear()"), getOtherPlayerId());
            }
            startSongChange();
        }
    } else {
        if(getStatus()==PLAYING && timeLeft<1000 && timeLeft>0) {
            //      kdDebug() << "should prepare for next songchange without crossfading...\n";
        }
    }
}


/**
 * Starts a song change, if possible (if >=2 songs in playlist)
 */
void NoatunPlayer::startSongChange(bool withoutCrossfading) {
    kdDebug() << getCurrentPlayerId() << "startSongChange()" << endl;
    model->skipUnplayableSongs(true);
    if(playlist->count()<2) {
        // we can't make a song change if there is no next song
        return;
    }
    PlayerStatus status=getStatus();
    if(model->config()->fadeTime==0 || withoutCrossfading || status!=PLAYING) {
        // no crossfading
        clearActivePlayerPlaylist();
        QString location=model->checkAvailability(playlist->at(1)->song());
        if(location!="" && location!="never") {
            playlistAdd(location, status==PLAYING || model->config()->fadeTime==0);
            playlist->removeFirst();
        }
        playlistChanged();
        statusChanged();
    } else {
        // crossfading
        fade=0;
        fadeOut=getCurrentPlayerId();
        currentPlayer=(currentPlayer+1) % 2;
        fadeIn=getCurrentPlayerId();
        //    clearActivePlayerPlaylist();
        QString location=model->checkAvailability(playlist->at(1)->song());
        if(location!="" && location!="never") {
            playlistAdd(location, true);
            // problem with noatun communication:
            // sometimes player does not start to play
            playlist->removeFirst();
        }
        else {
            kdDebug() << "no valid location for next song" << endl;
        }
        playlistChanged();
        fadeTimer.start( 200, TRUE );       // change volume every 200ms
    }
}


/**
 * Adds a file to the current noatun player.
*/
void NoatunPlayer::playlistAdd(QString filename, bool autoStart) {
    kdDebug() << getCurrentPlayerId() << "playlistAdd()" << endl;
    int id=getCurrentPlayerId();
    QString str=QString("noatun-%1").arg(id);
    QByteArray data;
    QDataStream arg(data, IO_WriteOnly);

    KURL url;
    url.setPath(filename);
    QString escapedPath = url.url();
    arg << escapedPath;
    arg << autoStart;

    if (!client->send(str.local8Bit(), "Noatun", "addFile(QString, bool)", data)) {
        kdDebug() << "could not add file to noatun playlist, dcop send() failed\n";
    }
}


/// start playing
bool NoatunPlayer::play() {
    sendDcopCommand("play()");
    return true;
}


/// pause
bool NoatunPlayer::pause() {
    if(getStatus()==PAUSED) {
        return true;
    }
    sendDcopCommand("playpause()");
    if(fade<100) {
        // if we are currently crossfading, we should pause both players
        sendDcopCommand("clear()", getOtherPlayerId());
    }
    return true;
}


/// toggle between play and pause
bool NoatunPlayer::playPause() {
    sendDcopCommand("playpause()");
    if(fade<100) {
        // if we are currently crossfading, we should playPause both players
        sendDcopCommand("clear()", getOtherPlayerId());
    }
    return true;
}


/// skip forward in playlist
bool NoatunPlayer::skipForward(bool withoutCrossfading) {
    kdDebug() << getCurrentPlayerId() << "skipForward()" << endl;
    if( playlist->count() < 2 ) {
        // there is no "next song"
        return false;
    }
    int savedStatus = getStatus();
    if( savedStatus != PLAYING ) {
        // remove the first song and sync playlist again
        playlist->removeFirst( );
        syncYammi2Player();    
        emit playlistChanged();
    }
    else {    
        startSongChange(withoutCrossfading);
    }
    return true;
}


/// skip backward in playlist
bool NoatunPlayer::skipBackward(bool withoutCrossfading) {
    kdDebug() << getCurrentPlayerId() << "skipBackward()" << endl;
    Song* last=playlist->at(0)->song();
    // insert pseudo-song to be removed
    playlist->insert(0, new SongEntryInt(last, 0));
    startSongChange(withoutCrossfading);
    return true;
}

/// stop playback
bool NoatunPlayer::stop() {
    pause();
    jumpTo(0);
    if(fade < 100) {
        // if we are currently crossfading, we should stop both players
        sendDcopCommand("clear()", getOtherPlayerId());
    }
    return true;
}


PlayerStatus NoatunPlayer::getStatus() {
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
    kdDebug() << "unknown state received from noatun\n";
    return STOPPED;
}

bool NoatunPlayer::jumpTo(int value) {
    if(callGetString("title()") == "" && playlist->count() > 0) {
        kdDebug() << "workaround for seeking if song not active in noatun: play(), pause()\n";
        sendDcopCommand("play()");
        for(int tries=0; tries<100 && callGetInt("state()") != PLAYING; tries++) {
            kdDebug() << "waiting for noatun to change state...\n";
        }
        sendDcopCommand("playpause()");
    }            
    sendDcopCommandInt("skipTo(int)", value);
    return true;
}


QString NoatunPlayer::getCurrentFile() {
    return callGetString("title()");
}

int NoatunPlayer::getCurrentTime() {
    return callGetInt("position()");
}


int NoatunPlayer::getTotalTime() {
    return callGetInt("length()");
}



void NoatunPlayer::syncYammi2Player() {
    kdDebug() << getCurrentPlayerId() << "syncYammi2Player()" << endl;
    bool haveToUpdate = model->skipUnplayableSongs();
    
    if(playlist->count()==0) {
        // playlist empty
        sendDcopCommand(QString("clear()"));
        sendDcopCommand(QString("clear()"), getOtherPlayerId());
        if(haveToUpdate) {
            emit playlistChanged();
        }
        return;
    }

    
    // 1. sync current song
    QString noatunCurrent=getCurrentFile();

    QString location = model->checkAvailability( playlist->at(0)->song() );
    QString yammiCurrent=location.right(location.length()-location.findRev('/')-1);
    if(noatunCurrent!=yammiCurrent) {
        clearActivePlayerPlaylist();
        playlistAdd(location, false);
        sendDcopCommandInt("setVolume(int)", 100);
    }
    if(haveToUpdate) {
        emit playlistChanged();
    }
}


void NoatunPlayer::quit() {
    if(getStatus() == PLAYING) {
        pause();
    }
}


void NoatunPlayer::sendDcopCommandInt(QString command, int param, int id) {
    if(id==0) {
        id=getCurrentPlayerId();
    }
    QString str=QString("noatun-%1").arg(id);

    QByteArray data;
    QDataStream arg(data, IO_WriteOnly);
    arg << param;

    if (!client->send(str.latin1(), "Noatun", command.latin1(), data)) {
        kdDebug() << "communicating with noatun: some error using DCOP.\n";
    }
}


void NoatunPlayer::sendDcopCommand(QString command, int id) {
    if(id==0) {
        id=getCurrentPlayerId();
    }
    QString str=QString("noatun-%1").arg(id);

    QByteArray data;
    QDataStream arg(data, IO_WriteOnly);

    if (!client->send(str.latin1(), "Noatun", command.latin1(), data)) {
        kdDebug() << "communicating with noatun: some error using DCOP.\n";
    }
}


int NoatunPlayer::callGetInt(QString command, int id) {
    if(id==0) {
        id=getCurrentPlayerId();
    }
    QByteArray data, replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly);

    int result=0;

    QString str=QString("noatun-%1").arg(id);
    if (!client->call(str.latin1(), QString("Noatun").latin1(), command.latin1(), data, replyType, replyData)) {
        kdDebug() << "call() failed, maybe noatun was closed?\n";
        kdDebug() << "I will check now and restart noatun if necessary...\n";
        ensurePlayerIsRunning(true);
    } else {
        QDataStream reply(replyData, IO_ReadOnly);
        if (replyType == "int") {
            reply >> result;
        } else
            kdDebug() << "unexpected type of dcop reply (int expected): " << replyType << "\n";
    }
    return result;
}


QString NoatunPlayer::callGetString(QString command, int id) {
    if(id==0) {
        id=getCurrentPlayerId();
    }
    QByteArray data, replyData;
    QCString replyType;
    QDataStream arg(data, IO_WriteOnly);

    QString result="";

    QString str=QString("noatun-%1").arg(id);
    if (!client->call(str.latin1(), QString("Noatun").latin1(), command.latin1(), data, replyType, replyData))
        kdDebug() << "call() failed\n";
    else {
        QDataStream reply(replyData, IO_ReadOnly);
        if (replyType == "QString") {
            reply >> result;
        } else
            kdDebug() << "unexpected type of dcop reply (QString expected): " << replyType << "\n";
    }
    return result;
}
