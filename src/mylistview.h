/***************************************************************************
                          mylistview.h  -  description
                             -------------------
    begin                : Sat Jan 12 2002
    copyright            : (C) 2002 by Oliver Nölle
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

#ifndef MYLISTVIEW_H
#define MYLISTVIEW_H

#include <qlistview.h>
#include <qcursor.h>

class Song;
class QKeyEvent;


/**extends the QListView class, esp. for dragging songs...
  *@author Oliver Nölle
  */

class MyListView : public QListView  {
    Q_OBJECT
public:
    MyListView(QWidget *parent=0, const char *name=0);
    ~MyListView();

    bool dragging;
    int sortedBy;
    int oldSortOrder;
    bool dontTouchFirst;
    void simulateKeyPressEvent(QKeyEvent* e);

protected:
    Song* dragSong;
    QPoint dragPoint;
    int dragStartedAtIndex;
    QListViewItem* dragItem;
    QWidget* parent;

    void contentsMouseMoveEvent ( QMouseEvent * e);
    void contentsMousePressEvent ( QMouseEvent * e);
    void contentsMouseReleaseEvent ( QMouseEvent * e);

protected slots:
    void simulateMouseMove();
public slots:
    void sortColumnChanged(int column);
};

#endif
