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

#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>

#include "song.h"


class TrackPositionSliderMouseEvent: public QMouseEvent
{
public:
    TrackPositionSliderMouseEvent(QMouseEvent& e) : QMouseEvent(e) {}
    void setButton(Qt::MouseButton button) { b = button; mouseState = button; }
};


TrackPositionSlider::TrackPositionSlider(Qt::Orientation orientation, QWidget *parent) : QSlider(orientation, parent)
{
    setSingleStep(1*1000);
    setPageStep(10*1000);
    setTracking( true );    
}

void TrackPositionSlider::mousePressEvent(QMouseEvent *e)
{
    TrackPositionSliderMouseEvent reverse(*e);
    reverse.setButton(Qt::MidButton);
    if(e->button() == Qt::LeftButton) {
        QSlider::mousePressEvent(&reverse);
        emit sliderMoved(value());
    } else if(e->button() == Qt::MidButton) {
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
        setTickPosition(QSlider::NoTicks);
        setValue(0);
        setEnabled(false);
    }
    else {
        setRange(0, song->length*1000);
        setValue(0);
        setTickPosition(QSlider::TicksBelow);
        setTickInterval(1000*60);
        setEnabled(true);        
        updateGeometry();
    }
}
