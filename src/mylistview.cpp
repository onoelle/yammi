/***************************************************************************
                          mylistview.cpp  -  description
                             -------------------
    begin                : Sat Jan 12 2002
    copyright            : (C) 2002 by Oliver Nölle
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

#include "mylistview.h"

#include <QEvent>
#include <QHeaderView>
#include <QMouseEvent>
#include <QKeyEvent>

#include "song.h"
#include "folder.h"
#include "yammigui.h"


using namespace std;

extern YammiGui* gYammiGui;

MyListView::MyListView(QWidget *parent) : QTableView(parent) {
    dragging=false;
    dontTouchFirst=false;

    setWordWrap(false);
    setSortingEnabled(true);
    setSelectionMode(MyListView::ExtendedSelection);
    setSelectionBehavior(MyListView::SelectRows);
    setShowGrid(false);
    verticalHeader()->setDefaultSectionSize(18);
    verticalHeader()->setVisible(false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QTableView::InternalMove);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader()->setMovable(true);
}

MyListView::~MyListView() {}

void MyListView::setSorting(bool enabled, int column, Qt::SortOrder sortOrder)
{
    setSortingEnabled(enabled);
    if (enabled) {
        horizontalHeader()->setSortIndicator(column, sortOrder);
    }
}

void MyListView::simulateKeyPressEvent(QKeyEvent* e)
{
    keyPressEvent(e);
}

void MyListView::mouseReleaseEvent(QMouseEvent* event)
{
    QTableView::mouseReleaseEvent(event);
    if (event->button() == Qt::MidButton) {
        emit middleClicked();
    }
}

/** inverts selection in songListView */
void MyListView::invertSelection() {
    QModelIndex li0 = model()->index(0,0);
    QModelIndex li1 = model()->index(model()->rowCount()-1, 0);
    selectionModel()->select(QItemSelection(li0,li1), QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
}

int MyListView::sortedBy()
{
    return horizontalHeader()->sortIndicatorSection();
}

Qt::SortOrder MyListView::sortOrder()
{
    return horizontalHeader()->sortIndicatorOrder();
}
