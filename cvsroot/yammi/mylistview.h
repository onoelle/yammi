/***************************************************************************
                          mylistview.h  -  description
                             -------------------
    begin                : Sat Jan 12 2002
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

#ifndef MYLISTVIEW_H
#define MYLISTVIEW_H

#include "songlistitem.h"
#include "song.h"
#include <qwidget.h>
#include <qlistview.h>
#include <iostream.h>

/**extends the QListView class, esp. for dragging songs...
  *@author Oliver Nölle
  */

class MyListView : public QListView  {
   Q_OBJECT
public: 
	MyListView(QWidget *parent=0, const char *name=0);
	~MyListView();
	
	bool draggable;
	bool dragging;

protected:
	Song* dragSong;
	QPoint dragPoint;
	QListViewItem* dragItem;
	QWidget* parent;

/*
	void contentsDragMoveEvent ( QDragMoveEvent* e );
	void contentsDragEnterEvent ( QDragEnterEvent* e );
	void contentsDragLeaveEvent ( QDragLeaveEvent* e );
	*/
	void contentsMouseMoveEvent ( QMouseEvent * e);
	void contentsMousePressEvent ( QMouseEvent * e);
	void contentsMouseReleaseEvent ( QMouseEvent * e);

};

#endif
