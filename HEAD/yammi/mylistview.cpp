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
#include "yammigui.h"

using namespace std;

extern YammiGui* gYammiGui;

MyListView::MyListView(QWidget *parent, const char *name ) : QListView(parent,name)
{
	dragging=false;
	dontTouchFirst=false;
	sortedBy=0;
	this->parent=parent;
	setItemMargin(2);	
	connect(this->header(), SIGNAL(clicked(int)), this, SLOT(sortColumnChanged(int)));
}

MyListView::~MyListView()
{
}

/**
 * TODO: check!!!
 */
void MyListView::sortColumnChanged(int column)
{
  int newColumn=column+1;
	if(sortedBy==newColumn)
		sortedBy=-sortedBy;
	else
		sortedBy=newColumn;
}

void MyListView::simulateMouseMove()
{
	// we simulate a mouse movement to keep list scrolling if we are above or below it
	QMouseEvent e(QEvent::MouseMove, dragPoint, 0, 0);
	contentsMouseMoveEvent(&e);
}


/**
 * Prepares for dragging, but dragging is only allowed if
 * - folder is sorted
 * - song list is sorted ascending by first column (which must be the "Pos" column)
 */
void MyListView::contentsMousePressEvent ( QMouseEvent * e)
{
  
  if(gYammiGui->chosenFolder->isSorted() && sortedBy==1 && e->button()==LeftButton) {
		dragPoint=e->globalPos();
		dragItem=itemAt(viewport()->mapFromGlobal(dragPoint));
		if(dragItem) {
			// check whether we allow dragging of first item
			if(!dontTouchFirst || dragItem!=firstChild()) {
				// start dragging
				dragging=true;
				dragStartedAtIndex=dragItem->itemPos();
				setCursor(Qt::sizeVerCursor);
				dragSong=((SongListItem*)dragItem)->song();
			}
		}
	}
	QListView::contentsMousePressEvent(e);
}



void MyListView::contentsMouseMoveEvent ( QMouseEvent * e)
{
	if(!dragging) {
		// if not dragging we call super class to perform normal behavior on dragging
		QListView::contentsMouseMoveEvent(e);
		return;
	}
	
	// dragging: check whether mouse has moved to new item
	QPoint point=e->globalPos();
	QListViewItem* item=itemAt(viewport()->mapFromGlobal(point));
	
	// no valid item, mouse above or below listview?
	if(item==0)	{
		bool above=viewport()->mapFromGlobal(point).y()<0;
		QListViewItem* swapItem;
		if(above) {
			swapItem=dragItem->itemAbove();
			if(swapItem) {
				if(dontTouchFirst && swapItem==firstChild())
					return;
				swapItem->moveItem(dragItem);
				ensureItemVisible(swapItem);				
			}
		}
		else {
			swapItem=dragItem->itemBelow();
			if(swapItem)
				dragItem->moveItem(swapItem);
		}
		ensureItemVisible(dragItem);
		// keep scrolling, if mouse not moving...
		QTimer *timer = new QTimer( this );
		connect( timer, SIGNAL(timeout()), this, SLOT(simulateMouseMove()) );
		timer->start( 400, TRUE );
		return;
	}
	
	
	Song* s=((SongListItem*)item)->song();
	if(s==dragSong)	// item has not moved
		return;
		
	bool up=(point.y() < dragPoint.y());
	if(up) {		// don't allow dragging to top song (is played)
		if(dontTouchFirst && item==firstChild()) {
			cout << "dragging to top song not allowed in this folder!\n";
			return;
		}
	}
	
	dragItem->moveItem(item);
	if(up) {
		dragItem->itemAbove()->moveItem(dragItem);
	}
	dragPoint=point;
}


void MyListView::contentsMouseReleaseEvent ( QMouseEvent * e)
{
	if(dragging) {
		dragging=false;
		setCursor(Qt::arrowCursor);
		// check, whether item really moved to new position
		if(dragItem->itemPos()!=dragStartedAtIndex)
			gYammiGui->stopDragging();			// only invoke this, if dragSong was moved
	}
	QListView::contentsMouseReleaseEvent(e);
}
