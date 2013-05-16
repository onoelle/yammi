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
#include "trackpositionslider.h"
#include "song.h"


TrackPositionSlider::TrackPositionSlider(Orientation orientation, QWidget *parent, const char *name) : QSlider(orientation, parent, name)
{
    setLineStep(1*1000);
    setPageStep(10*1000);
    setTracking( true );    
}

void TrackPositionSlider::mousePressEvent(QMouseEvent *e)
{
	if(e->button() == LeftButton) {
    	QMouseEvent reverse(QEvent::MouseButtonPress, e->pos(), MidButton, e->state());
    	QSlider::mousePressEvent(&reverse); 
    	emit sliderPressed();
	}
	else if(e->button() == MidButton) {
    	QMouseEvent reverse(QEvent::MouseButtonPress, e->pos(), MidButton, e->state());
    	QSlider::mousePressEvent(&reverse); 
	}
}


void TrackPositionSlider::wheelEvent(QWheelEvent* e)
{
	emit myWheelEvent(e->delta());
}

void TrackPositionSlider::setupTickmarks(Song* song)
{
    if(song == 0) {
        setRange(0, 0);
        setTickmarks(QSlider::NoMarks);
        setValue(0);
        setEnabled(false);
    }
    else {
        setRange(0, song->length*1000);
        setValue(0);
        setTickmarks(QSlider::Below);
        setTickInterval(1000*60);
        setEnabled(true);        
        updateGeometry();
    }
}
