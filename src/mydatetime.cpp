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

#include <stdio.h>
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
	QString x;		
	x=x.sprintf("%02d/%02d/%04d, %02d:%02d:%02d", date().day(), date().month(), date().year(), time().hour(), time().minute(), time().second() );
//	QString str=QString("%1/%2/%3, %4:%5:%6").arg( date().day() ).arg( date().month() ).arg( date().year() ).arg( time().hour() ).arg( time().minute() ).arg( time().second() );
	return x;
}


void MyDateTime::readFromString(QString str)
{
	int dd, mm, yyyy;
	int h, m, s;
	char sep1, sep2, sep3, sep3b, sep4, sep5;
	// read date as "dd/mm/yyyy, hh:mm:ss"
	sscanf(str, "%d%c%d%c%d%c%c%d%c%d%c%d", &dd, &sep1, &mm, &sep2, &yyyy, &sep3, &sep3b, &h, &sep4, &m, &sep5, &s);
    // qDebug() << "dd: " << dd << ", mm: " << mm << ", yyyy: " << yyyy;
	this->setDate(QDate (yyyy, mm, dd));
	this->setTime(QTime (h, m, s));
}
