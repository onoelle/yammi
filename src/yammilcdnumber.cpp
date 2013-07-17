/***************************************************************************
 *   Copyright (C) 2013 by Bernhard Übelacker                              *
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

#include "yammilcdnumber.h"


YammiLCDNumber::YammiLCDNumber(QWidget *parent)
    : QLCDNumber(parent)
{
    m_lcdDisplayDuration = false;

    setDigitCount(6);
    display("-88:88");
    setFrameShape(QFrame::NoFrame);
    connect(this, SIGNAL(clicked()), this, SLOT(lcdDisplayToggle()));
}

void YammiLCDNumber::mousePressEvent(QMouseEvent* event)
{
    QLCDNumber::mousePressEvent(event);
    emit clicked();
}

void YammiLCDNumber::lcdDisplayToggle()
{
    m_lcdDisplayDuration = !m_lcdDisplayDuration;
}

void YammiLCDNumber::update(int position, int duration)
{
    if (!m_lcdDisplayDuration) {
        display(QString("%1:%2")
                .arg((position/1000) / 60, 2, 10, QChar('0'))
                .arg((position/1000) % 60, 2, 10, QChar('0')));
        setToolTip("Played");
    } else {
        int left = duration - position;
        if (left < 0) left = 0;
        display(QString("-%1:%2")
                .arg((left/1000) / 60, 2, 10, QChar('0'))
                .arg((left/1000) % 60, 2, 10, QChar('0')));
        setToolTip("Remaining");
    }
}
