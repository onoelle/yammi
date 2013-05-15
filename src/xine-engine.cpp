/***************************************************************************
 *   Copyright (C) 2005   Christophe Thommeret <hftom@free.fr>             *
 *             (C) 2005   Ian Monroe <ian@monroe.nu>                       *
 *             (C) 2005,6 Mark Kretschmann <markey@web.de>                 *
 *             (C) 2004,5 Max Howell <max.howell@methylblue.com>           *
 *             (C) 2003,4 J. Kofler <kaffeine@gmx.net>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qapplication.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include "xine-engine.h"

#include "songentryint.h"


//define this to use xine in a more standard way
#define XINE_SAFE_MODE


///returns the configuration we will use. there is no KInstance, so using this hacked up method.
static inline QCString configPath() { return QFile::encodeName(locate( "data", "yammi/") + "xine-config" ); }


namespace Yammi {

    XineEngine::XineEngine(YammiModel *yammi)
            : MediaPlayer(yammi)
            , m_xine( 0 )
            , m_stream( 0 )
            , m_audioPort( 0 )
            , m_eventQueue( 0 )
            , m_post( 0 )
            , m_currentSong( 0 )
    {
        kdDebug() << "'Bringing joy to small mexican gerbils, a few weeks at a time.'\n";

        m_xine = xine_new();

        if (!m_xine) {
           KMessageBox::error( 0, i18n("Yammi could not initialize xine.") );
           //return false;
        }

        #ifdef XINE_SAFE_MODE
        xine_engine_set_param( m_xine, XINE_ENGINE_PARAM_VERBOSITY, 99 );
        #endif

        xine_config_load( m_xine, configPath() );
        kdDebug() << "w00t" << configPath() << endl;

        xine_init( m_xine );

        makeNewStream();

        #ifndef XINE_SAFE_MODE
        startTimer( 200 ); //prunes the scope
        #endif
    }

    XineEngine::~XineEngine()
    {
        if( m_xine )       xine_config_save( m_xine, configPath() );

        if( m_stream )     xine_close( m_stream );
        if( m_eventQueue ) xine_event_dispose_queue( m_eventQueue );
        if( m_stream )     xine_dispose( m_stream );
        if( m_audioPort )  xine_close_audio_driver( m_xine, m_audioPort );
        if( m_post )       xine_post_dispose( m_xine, m_post );
        if( m_xine )       xine_exit( m_xine );
    }

    bool
    XineEngine::makeNewStream()
    {
       m_audioPort = xine_open_audio_driver( m_xine, "auto", NULL );
       if( !m_audioPort ) {
          //TODO make engine method that is the same but parents the dialog for us
          KMessageBox::error( 0, i18n("xine was unable to initialize any audio drivers.") );
          return false;
       }

       m_stream = xine_stream_new( m_xine, m_audioPort, NULL );
       if( !m_stream ) {
          xine_close_audio_driver( m_xine, m_audioPort );
          m_audioPort = NULL;
          KMessageBox::error( 0, i18n("Yammi could not create a new xine stream.") );
          return false;
       }

       if( m_eventQueue )
          xine_event_dispose_queue( m_eventQueue );

       xine_event_create_listener_thread(
             m_eventQueue = xine_event_new_queue( m_stream ),
             &XineEngine::XineEventListener,
             (void*)this );

       #ifndef XINE_SAFE_MODE
       //implemented in xine-scope.h
       //m_post = scope_plugin_new( m_xine, m_audioPort );

       xine_set_param( m_stream, XINE_PARAM_METRONOM_PREBUFFER, 6000 );
       xine_set_param( m_stream, XINE_PARAM_IGNORE_VIDEO, 1 );
       #endif
       return true;
    }

    // Makes sure an audio port and a stream exist.
    bool
    XineEngine::ensureStream()
    {
       if( !m_stream )
          return makeNewStream();

       return true;
    }

    void
    XineEngine::syncYammi2Player()
    {
        kdDebug() << "XineEngine::syncYammi2Player()" << endl;

        bool haveToUpdate=model->skipUnplayableSongs();

        if(m_currentSong != 0) {
            kdDebug() << "m_currentSong: " << m_currentSong->displayName() << endl;
        }

        KURL url;
        SongEntry* songEntry = playlist->at(0);
        if (songEntry) {
            kdDebug() << "playlist->at(0): " << songEntry->song()->displayName() << endl;

            if(m_currentSong == songEntry->song()) {
                if(haveToUpdate) {
                    emit playlistChanged();
                }
                return;
            }
            if ( getStatus() != STOPPED ) {
                stop();
            }
            m_currentSong = 0;

            QString location = model->checkAvailability( songEntry->song() );
            kdDebug() << "returned location: " << location << endl;
            m_currentSong = songEntry->song();
            url.setPath(location);
            if(!url.isValid()) {
                kdDebug() << "ERROR: url not valid: " << url << endl;
                return;
            }
        }

        if( !ensureStream() )
            return;

        //Engine::Base::load( url, isStream );

       // for users who stubbonly refuse to use DMIX or buy a good soundcard
       // why doesn't xine do this? I cannot say.
       xine_close( m_stream );

       kdDebug() << "Before xine_open() *****" << endl;

       if( xine_open( m_stream, QFile::encodeName( url.url() ) ) )
       {
          kdDebug() << "After xine_open() *****" << endl;

          #ifndef XINE_SAFE_MODE
          //we must ensure the scope is pruned of old buffers
          timerEvent( 0 );

          xine_post_out_t *source = xine_get_audio_source( m_stream );
          xine_post_in_t  *target = (xine_post_in_t*)xine_post_input( m_post, const_cast<char*>("audio in") );
          xine_post_wire( source, target );
          #endif

          playlistChanged();

          return;
       }
       else
       {
          #ifdef XINE_PARAM_GAPLESS_SWITCH
            if ( xine_check_version(1,1,1) )
                xine_set_param( m_stream, XINE_PARAM_GAPLESS_SWITCH, 0);
          #endif
       }

       return;
    }

    void
    XineEngine::clearPlaylist()
    {
        emit playlistChanged( );
    }

    bool
    XineEngine::play()
    {
        if( xine_get_param( m_stream, XINE_PARAM_SPEED ) == XINE_SPEED_PAUSE )
        {
            xine_set_param( m_stream, XINE_PARAM_SPEED, XINE_SPEED_NORMAL );
            status = getStatus();
            emit statusChanged();
        } else {
            if( !ensureStream() )
                return false;

            const bool has_audio     = xine_get_stream_info( m_stream, XINE_STREAM_INFO_HAS_AUDIO );
            const bool audio_handled = xine_get_stream_info( m_stream, XINE_STREAM_INFO_AUDIO_HANDLED );

            if (has_audio && audio_handled && xine_play( m_stream, 0, 0/*offset*/ ))
            {
                status = getStatus();
                emit statusChanged();

                return true;
            }

            status = getStatus();
            emit statusChanged();

            xine_close( m_stream );
        }

        return false;
    }

    bool
    XineEngine::stop()
    {
#if 0
        if ( !m_stream )
           return false;

        xine_stop( m_stream );
        xine_close( m_stream );
        xine_set_param( m_stream, XINE_PARAM_AUDIO_CLOSE_DEVICE, 1);

        emit statusChanged();

        return true;
#else
        pause();
        jumpTo(0);
        /*
        if( m_currentPlay && m_currentPlay->state() != Arts::posIdle ) {
            m_currentPlay->halt();
        }
        status = getStatus( );
        emit statusChanged( );
        */
        return true;
#endif
    }

    bool
    XineEngine::pause()
    {
        if ( !m_stream )
            return false;

        if( xine_get_param( m_stream, XINE_PARAM_SPEED ) != XINE_SPEED_PAUSE )
        {
            xine_set_param( m_stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE );
            xine_set_param( m_stream, XINE_PARAM_AUDIO_CLOSE_DEVICE, 1);
            status = getStatus();
            emit statusChanged();

        }

        return true;
    }

    bool
    XineEngine::playPause()
    {
        switch (getStatus()) {
        case PLAYING:
            pause();
            break;
        case STOPPED:
        default:
            play();  //start new play
            break;
        }
        status = getStatus( );
        emit statusChanged( );
        return true;
    }

    void
    XineEngine::check()
    {
        PlayerStatus s = getStatus( );
        if( s != status ) {
            if( status == PLAYING && s == STOPPED ) {
                if( m_currentSong == playlist->firstSong() ) {
                    kdDebug() << "Song finished... get new one!" << endl;
                    playlist->removeFirst( );
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

    PlayerStatus
    XineEngine::getStatus()
    {
        if ( !m_stream)
           return STOPPED;

        switch( xine_get_status( m_stream ) )
        {
        case XINE_STATUS_PLAY: return xine_get_param( m_stream, XINE_PARAM_SPEED )  != XINE_SPEED_PAUSE ? PLAYING : PAUSED;
        case XINE_STATUS_IDLE: return STOPPED;
        case XINE_STATUS_STOP:
        default:               return STOPPED;
        }
    }

    QString
    XineEngine::getCurrentFile()
    {
        if( m_currentSong ) {
            return model->checkAvailability( m_currentSong );
        } else {
            return "";
        }
    }

    int
    XineEngine::getCurrentTime()
    {
        if ( getStatus() == STOPPED )
           return 0;

        int pos;
        int time = 0;
        int length;

        // Workaround for problems when you seek too quickly, see BUG 99808
        int tmp = 0, i = 0;
        while( ++i < 4 )
        {
            xine_get_pos_length( m_stream, &pos, &time, &length );
            if( time > tmp ) break;
            usleep( 100000 );
        }

        return time;
    }

    int
    XineEngine::getTotalTime()
    {
        if ( !m_stream )
           return 0;

        // xine often delivers nonsense values for VBR files and such, so we only
        // use the length for remote files

        int pos;
        int time;
        int length = 0;

        xine_get_pos_length( m_stream, &pos, &time, &length );
        if( length < 0 )
            length=0;

        return length;
    }

    bool
    XineEngine::skipForward(bool)
    {
        if( playlist->count() < 2 ) {
            // there is no "next song"
            return false;
        }
        int savedStatus = getStatus();
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
    }

    bool
    XineEngine::skipBackward(bool withoutCrossfading)
    {
        Song* last=playlist->at(0)->song();
        // insert pseudo-song to be removed
        playlist->insert(0, new SongEntryInt(last, 0));
        return skipForward(withoutCrossfading);
    }


    bool
    XineEngine::jumpTo( int ms )
    {
        if( !ensureStream() )
            return false;

        if( xine_get_param( m_stream, XINE_PARAM_SPEED ) == XINE_SPEED_PAUSE ) {
            // FIXME this is a xine API issue really, they need to add a seek function
            xine_play( m_stream, 0, (int)ms );
            xine_set_param( m_stream, XINE_PARAM_SPEED, XINE_SPEED_PAUSE );
        }
        else
            xine_play( m_stream, 0, (int)ms );

        status = getStatus();

        return true;
    }

    static time_t last_error_time = 0; // hysteresis on xine errors
    static int    last_error = XINE_MSG_NO_ERROR;

    void
    XineEngine::XineEventListener( void *p, const xine_event_t* xineEvent )
    {
        time_t current;

        if( !p ) return;

        #define xe static_cast<XineEngine*>(p)

        switch( xineEvent->type )
        {
        case XINE_EVENT_UI_SET_TITLE:

            kdDebug() << "XINE_EVENT_UI_SET_TITLE\n";

            QApplication::postEvent( xe, new QCustomEvent( 3003 ) );

            break;

        case XINE_EVENT_UI_PLAYBACK_FINISHED:
            kdDebug() << "XINE_EVENT_UI_PLAYBACK_FINISHED\n";

            //emit signal from GUI thread
            QApplication::postEvent( xe, new QCustomEvent(3000) );
            break;

        case XINE_EVENT_PROGRESS: {
            xine_progress_data_t* pd = (xine_progress_data_t*)xineEvent->data;

            QString
            msg = "%1 %2%";
            msg = msg.arg( QString::fromUtf8( pd->description ) )
                     .arg( KGlobal::locale()->formatNumber( pd->percent, 0 ) );

            QCustomEvent *e = new QCustomEvent( 3002 );
            e->setData( new QString( msg ) );

            QApplication::postEvent( xe, e );

        }   break;

        case XINE_EVENT_UI_MESSAGE:
        {
            kdDebug() << "message received from xine\n";

            xine_ui_message_data_t *data = (xine_ui_message_data_t *)xineEvent->data;
            QString message;

            switch( data->type )
            {
            case XINE_MSG_NO_ERROR:
            {
                //series of \0 separated strings, terminated with a \0\0
                char str[2000];
                char *p = str;
                for( char *msg = data->messages; !(*msg == '\0' && *(msg+1) == '\0'); ++msg, ++p )
                    *p = *msg == '\0' ? '\n' : *msg;
                *p = '\0';

                kdDebug() << str << endl;

                break;
            }

            case XINE_MSG_ENCRYPTED_SOURCE:
                break;

            case XINE_MSG_UNKNOWN_HOST:
                message = i18n("The host is unknown for the URL: <i>%1</i>"); goto param;
            case XINE_MSG_UNKNOWN_DEVICE:
                message = i18n("The device name you specified seems invalid."); goto param;
            case XINE_MSG_NETWORK_UNREACHABLE:
                message = i18n("The network appears unreachable."); goto param;
            case XINE_MSG_AUDIO_OUT_UNAVAILABLE:
                message = i18n("Audio output unavailable; the device is busy."); goto param;
            case XINE_MSG_CONNECTION_REFUSED:
                message = i18n("The connection was refused for the URL: <i>%1</i>"); goto param;
            case XINE_MSG_FILE_NOT_FOUND:
                message = i18n("xine could not find the URL: <i>%1</i>"); goto param;
            case XINE_MSG_PERMISSION_ERROR:
                message = i18n("Access was denied for the URL: <i>%1</i>"); goto param;
            case XINE_MSG_READ_ERROR:
                message = i18n("The source cannot be read for the URL: <i>%1</i>"); goto param;
            case XINE_MSG_LIBRARY_LOAD_ERROR:
                message = i18n("A problem occurred while loading a library or decoder."); goto param;

            case XINE_MSG_GENERAL_WARNING:
                message = i18n("General Warning"); goto explain;
            case XINE_MSG_SECURITY:
                message = i18n("Security Warning"); goto explain;
            default:
                message = i18n("Unknown Error"); goto explain;


            explain:

                // Don't flood the user with error messages
                if( (last_error_time + 10) > time( &current ) &&
                       data->type == last_error )
                {
                    last_error_time = current;
                    return;
                }
                last_error_time = current;
                last_error = data->type;

                if( data->explanation )
                {
                    message.prepend( "<b>" );
                    message += "</b>:<p>";
                    message += QString::fromUtf8( (char*)data + data->explanation );
                }
                else break; //if no explanation then why bother!

                //FALL THROUGH

            param:

                // Don't flood the user with error messages
                if((last_error_time + 10) > time(&current) &&
                   data->type == last_error)
                {
                    last_error_time = current;
                    return;
                }
                last_error_time = current;
                last_error = data->type;

                message.prepend( "<p>" );
                message += "<p>";

                if(data->explanation)
                {
                    message += "xine parameters: <i>";
                    message += QString::fromUtf8( (char*)data + data->parameters );
                    message += "</i>";
                }
                else message += i18n("Sorry, no additional information is available.");

                QApplication::postEvent( xe, new QCustomEvent(QEvent::Type(3001), new QString(message)) );
            }

        } //case
        case XINE_EVENT_UI_CHANNELS_CHANGED: //Flameeyes used this for last.fm track changes
            QApplication::postEvent( xe, new QCustomEvent(QEvent::Type(3005) ) );
        break;
        } //switch

        #undef xe
    }

    void
    XineEngine::quit()
    {
    }

    bool
    XineEngine::finishInitialization() {
        return true;
    }

} //namespace Yammi
