/***************************************************************************
 *   Copyright (C) 2004 by Oliver Nölle                                    *
 *   oli.noelle@web.de                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef YAMMIDCOPIFACE_H
#define YAMMIDCOPIFACE_H

#include <dcopobject.h>

class YammiDcopIface: virtual public DCOPObject {
    K_DCOP
k_dcop:
    virtual void playPause() = 0;
    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void skipForward() = 0;
    virtual void skipBackward() = 0;
    virtual void aplayOff() = 0;
    virtual void aplayLNP() = 0;
    virtual void aplayRandom() = 0;
    virtual int currentTime() = 0;
    virtual int totalTime() = 0;
    virtual void seek(int pos) = 0;
    virtual QString songInfo() = 0;
    virtual QString songArtist() = 0;
    virtual QString songTitle() = 0;
    virtual int songTrack() = 0;
    virtual QString songTrack2D() = 0;
    virtual QString songAlbum() = 0;
    virtual QString songGenre() = 0;
    virtual QString songComment() = 0;
    virtual int songYear() = 0;
};
#endif // YAMMIDCOPIFACE_H
