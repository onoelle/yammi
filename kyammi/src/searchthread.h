/***************************************************************************
 *   Copyright (C) 2004 by Oliver N�lle                                    *
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
#ifndef YAMMISEARCHTHREAD_H
#define YAMMISEARCHTHREAD_H

#include <qthread.h>
#include "mylist.h"

class QString;

/**
@author Oliver N�lle
 * A thread for performing the fuzzy search in the background.
*/
class SearchThread : public QThread
{
public:
	SearchThread();
	~SearchThread();
	virtual void run();
	void stopThread()			{ stopThreadFlag = true; }
	void setSearchTerm(QString str)   {searchTerm = str; }
	
protected:
	bool stopThreadFlag;
	QString searchTerm;
	QString currentSearchTerm;
	bool searchRunning;
	MyList searchResults;
};


#endif
