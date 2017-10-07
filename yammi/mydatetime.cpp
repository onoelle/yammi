/***************************************************************************
                          mydatetime.cpp  -  description
                             -------------------
    begin                : Fri Nov 2 2001
    copyright            : (C) 2001 by Brian O.Nölle
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

#include "mydatetime.h"


MyDateTime::MyDateTime()
{
	this->setDate(QDate::currentDate());
	this->setTime(QTime::currentTime());
}

MyDateTime::~MyDateTime() {}
	
MyDateTime::MyDateTime(QDateTime t)
{
	this->setDate(t.date());
	this->setTime(t.time());
}
	

QString MyDateTime::writeToString()
{
    return this->toString("dd/MM/yyyy, hh:mm:ss");
}


void MyDateTime::readFromString(QString str)
{
    *this = QDateTime::fromString(str, "dd/MM/yyyy, hh:mm:ss");
}
