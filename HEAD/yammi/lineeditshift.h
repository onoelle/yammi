/***************************************************************************
                          lineeditshift.h  -  description
                             -------------------
    begin                : Tue Sep 24 2002
    copyright            : (C) 2002 by Oliver Nölle
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

#ifndef LINEEDITSHIFT_H
#define LINEEDITSHIFT_H

#include <qwidget.h>
#include <qlineedit.h>

/** Just a wrapper class around QLineEdit which propagates shift key events
 * to gYammiGui.
 */

class LineEditShift : public QLineEdit  {
   Q_OBJECT
public: 
	LineEditShift(QWidget *parent=0, const char *name=0);
	~LineEditShift();

	void			keyPressEvent(QKeyEvent* e);
	void			keyReleaseEvent(QKeyEvent* e);
  
};

#endif
