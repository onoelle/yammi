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

#include <QTableView>

class Song;
class QKeyEvent;
class QMouseEvent;


/**extends the QListView class, esp. for dragging songs...
  *@author Oliver Nölle
  */

class MyListView : public QTableView  {
    Q_OBJECT
public:
    MyListView(QWidget *parent=0);
    ~MyListView();

    bool dragging;
    bool dontTouchFirst;
    void simulateKeyPressEvent(QKeyEvent* e);
    void setSorting(bool enabled, int column = -1, Qt::SortOrder sortOrder = Qt::AscendingOrder);
    int sortedBy();
    Qt::SortOrder sortOrder();

signals:
    void middleClicked();

protected:
    virtual void mouseReleaseEvent(QMouseEvent* event);

public slots:
    void invertSelection();
};

#endif
