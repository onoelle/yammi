/***************************************************************************
                          yammiapplication.h  -  description
                             -------------------
    begin                : Thu Sep 26 2002
    copyright            : (C) 2002 by Oliver N�lle
    email                : oli.noelle@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef YAMMIAPPLICATION_H
#define YAMMIAPPLICATION_H

#include <qapplication.h>

/** This class has only the purpose of propagating the commitDate() call to the YammiGui so far...
 */
class YammiApplication : public QApplication  {
public: 
	YammiApplication(int argc, char **argv);
	~YammiApplication();

  void commitData(QSessionManager& sm);
};

#endif
