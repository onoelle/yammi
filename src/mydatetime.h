/***************************************************************************
                          mydatetime.h  -  description
                             -------------------
    begin                : Fri Nov 2 2001
    copyright            : (C) 2001 by Brian O.Nölle
    email                :  yammi-developer@lists.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MYDATETIME_H
#define MYDATETIME_H

#include <QDateTime>

/**extends the QT base class with operators to read/write to a string
  *@author Brian O.Nölle
  */

class MyDateTime : public QDateTime  {
public: 
	MyDateTime();
	MyDateTime(QDateTime t);
	~MyDateTime();
	
	QString writeToString();
	void readFromString(QString str);
};

#endif
