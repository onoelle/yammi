#include "gstplayer.h"
#include <kdebug.h>
#include "songentry.h"
#include "songentryint.h"

/*
TODO: Add some config stuff 
 - ElementFactory::make("osssink", "playAudio");
 */

namespace Yammi {

GstPlayer::GstPlayer(YammiModel* yammi) : MediaPlayer(yammi) {
        #ifdef ENABLE_GSTREAMER
        kdDebug() << "GstPlayer::GstPlayer(YammiModel *yammi)" << endl;

        m_player = new KDE::GSTPlay::Play(KDE::GSTPlay::Play::PIPE_AUDIO_BUFFER_THREADED, this, "Play");
        //audiosink = KDE::GST::ElementFactory::make("osssink", NULL);
        //if(audiosink)
        //m_player->setAudioSink(audiosink);
        kdDebug() << "GstPlayer: player created" << endl;

        //this will tell us our position in our stream
        connect(m_player, SIGNAL(timeTick(long long)),SLOT(slotSetPosition(long long)));
        //this will tell us the complete length of the stream
        connect(m_player, SIGNAL(streamLength(long long)),SLOT(slotSetDuration(long long)));
        //this will tell us if the end has been reached
        connect(m_player, SIGNAL(streamEnd()), SLOT(slotDone()));
        kdDebug() << "GstPlayer: player connected" << endl;
        m_player->setState(KDE::GST::Element::STATE_NULL);
        kdDebug() << "GstPlayer: state set" << endl;
        #endif

        m_currentSong = 0;
        m_done = false;
        kdDebug() << "GstPlayer::GstPlayer(YammiModel *yammi) done!" << endl;
        
    }

    GstPlayer::~GstPlayer() {
        #ifdef ENABLE_GSTREAMER
        kdDebug() << "---GstPlayer::~GstPlayer(YammiModel *yammi)" << endl;
        quit( );
        delete m_player;
        //delete audiosink;
        #endif

    }


    void GstPlayer::check() {
        #ifdef ENABLE_GSTREAMER
        if(m_done) {
            if( m_currentSong == playlist->firstSong() ) {
                kdDebug() << "Song finished... get new one!" << endl;
                playlist->removeFirst( );
                m_currentSong = 0;
                emit playlistChanged( );
            }
            // if there are more songs in yammi's list, play them
            play( );
            status = getStatus();
            emit statusChanged( );

        }
        #endif

    }

    PlayerStatus GstPlayer::getStatus() {
        #ifdef ENABLE_GSTREAMER
        if( m_player->getState() == KDE::GST::Element::STATE_PLAYING )
            return PLAYING;
        if( m_player->getState() == KDE::GST::Element::STATE_PAUSED )
            return PAUSED;
        else
            // STATE_VOID_PENDING
            // STATE_NULL
            // STATE_READY
            #endif

            return STOPPED;
    }


    void GstPlayer::clearPlaylist() {
        #ifdef ENABLE_GSTREAMER
        //we don't have a playlist
        m_currentSong = 0;
        emit playlistChanged( );
        #endif

    }


    bool GstPlayer::play() {
        #ifdef ENABLE_GSTREAMER
        if( m_currentSong != 0 && getStatus() != PLAYING ) {
            m_player->setState(KDE::GST::Element::STATE_PLAYING);
            status = getStatus();
            emit statusChanged();
            return true;
        }
        #endif
        return false;
    }


    bool GstPlayer::pause() {
        #ifdef ENABLE_GSTREAMER
        if( m_currentSong != 0 && getStatus() == PLAYING ) {
            m_player->setState(KDE::GST::Element::STATE_PAUSED);
            status = getStatus();
            emit statusChanged();
            return true;
        }
        #endif
        return false;

    }


    bool GstPlayer::stop() {
        #ifdef ENABLE_GSTREAMER
        if( m_currentSong != 0 && (getStatus() == PLAYING || getStatus() == PAUSED)) {
            m_player->setState(KDE::GST::Element::STATE_READY);
            status = getStatus();
            emit statusChanged();
            return true;
        }
        #endif
        return false;
    }


    bool GstPlayer::playPause() {
        #ifdef ENABLE_GSTREAMER
        if( getStatus() == PLAYING ) {
            return pause();
        }
        #endif
        return play();
    }


    bool GstPlayer::skipForward(bool) {
        #ifdef ENABLE_GSTREAMER
        //shouldn't skip forward on the last song empty the list?
        if( playlist->count() < 2 ) {
            // there is no "next song"
            return false;
        }
        int savedStatus = getStatus();

        // stop
        if( getStatus() == PLAYING ) {
            m_player->setState(KDE::GST::Element::STATE_NULL);
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
        #else

        return false;
        #endif

    }


    bool GstPlayer::skipBackward(bool withoutCrossfading) {
        #ifdef ENABLE_GSTREAMER
        Song* last=playlist->at(0)->song();
        // insert pseudo-song to be removed
        playlist->insert(0, new SongEntryInt(last, 0));
        #endif

        return skipForward(withoutCrossfading);
    }


    void GstPlayer::syncYammi2Player() {
        #ifdef ENABLE_GSTREAMER
        bool haveToUpdate=model->skipUnplayableSongs();

        if(playlist->count()==0) {
            if( getStatus()!=STOPPED ) {
                stop();
            }
            m_currentSong = 0;
            if(haveToUpdate) {
                emit playlistChanged();
            }
            return;
        }

        if(m_currentSong == playlist->at(0)->song()) {
            if(haveToUpdate) {
                emit playlistChanged();
            }
            return;
        }

        if (getStatus()!= STOPPED ) {
            m_player->setState(KDE::GST::Element::STATE_NULL);
        }

        m_done = false;
        QString location = model->checkAvailability( playlist->at(0)->song() );
        m_currentSong = playlist->at(0)->song();
        m_positionNs = 0;
        m_durationNs = 0;
        m_player->setLocation(location);

        if(haveToUpdate) {
            emit playlistChanged();
        }
        return;
        #endif

    }


    bool GstPlayer::jumpTo(int value) {
        #ifdef ENABLE_GSTREAMER
        if( m_currentSong == 0) {
            return false;
        }
        if( value < 0 || value > getTotalTime() ) {
            return false;
        }
        m_player->seekToTime(value * 1000000LL);

        return true;
        #else

        return false;
        #endif

    }


    QString GstPlayer::getCurrentFile() {
        #ifdef ENABLE_GSTREAMER
        if( m_currentSong ) {
            return model->checkAvailability( m_currentSong );
        } else {
            return "";
        }
        #else
        return "";
        #endif

    }

    void GstPlayer::slotSetPosition(long long d) {
        #ifdef ENABLE_GSTREAMER
        m_positionNs = d;
        #endif

    }

    int GstPlayer::getCurrentTime() {
        #ifdef ENABLE_GSTREAMER
        return m_positionNs/1000000LL;
        #endif

    }

    void GstPlayer::slotSetDuration(long long d) {
        #ifdef ENABLE_GSTREAMER
        m_durationNs = d;
        #endif

    }

    int GstPlayer::getTotalTime() {
        #ifdef ENABLE_GSTREAMER
        return m_durationNs/1000000LL;
        #endif

    }

    void GstPlayer::quit() {
        stop();
        m_currentSong = 0;
    }

    void GstPlayer::slotDone() {
        #ifdef ENABLE_GSTREAMER
        m_done = true;
        #endif

    }
};
