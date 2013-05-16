/***************************************************************************
                          mediaplayer.cpp  -  description
                             -------------------
    begin                : Tue Sep 24 2002
    copyright            : (C) 2002 by Oliver Nölle
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

#include "mediaplayer.h"

MediaPlayer::MediaPlayer( YammiModel *yammi )
{
	model = yammi;
	playlist = &(model->songsToPlay);
	status = lastStatus = STOPPED;
}

MediaPlayer::~MediaPlayer()
{
}
