/***************************************************************************
                          lineeditshift.cpp  -  description
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

#include "lineeditshift.h"

//#include <iostream.h>
#include <qevent.h>
#include <qnamespace.h>

#include "yammigui.h"

extern YammiGui* gYammiGui;


LineEditShift::LineEditShift(QWidget *parent, const char *name )
 : QLineEdit(parent,name)
{
}

LineEditShift::~LineEditShift()
{
}


void LineEditShift::keyPressEvent(QKeyEvent* e)
{
	int key=e->key();
	switch(key) {
		case Qt::Key_Shift:
//      cout << "shift pressed (in lineedit)\n";
			gYammiGui->shiftPressed=true;
      // desired fallthrough
    default:
      QLineEdit::keyPressEvent(e);
  }
}

void LineEditShift::keyReleaseEvent(QKeyEvent* e)
{
	int key=e->key();
	switch(key) {
		case Qt::Key_Shift:
//      cout << "shift released (in lineedit)\n";
			gYammiGui->shiftPressed=false;
      // desired fallthrough
    default:
      QLineEdit::keyReleaseEvent(e);
  }
}
