/***************************************************************************
                          mylistview.cpp  -  description
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

#include "mylistview.h"
#include "yammigui.h"

extern YammiGui* gYammiGui;

MyListView::MyListView(QWidget *parent, const char *name ) : QListView(parent,name)
{
	dragging=false;
	draggable=false;
	this->parent=parent;
}

MyListView::~MyListView()
{
}

void MyListView::simulateMouseMove()
{
	// we simulate a mouse movement to keep list scrolling if we are above or below it
	QMouseEvent e(QEvent::MouseMove, dragPoint, 0, 0);
	contentsMouseMoveEvent(&e);
}


void MyListView::contentsMouseMoveEvent ( QMouseEvent * e)
{
	if(!dragging) {
		// if not dragging we call super class to perform normal behavior on dragging
		QListView::contentsMouseMoveEvent(e);
		return;
	}
	
	// drag mode: we should check whether mouse has moved to new item
	QPoint point=e->globalPos();
	QListViewItem* item=itemAt(viewport()->mapFromGlobal(point));
	if(item==0)	{		// no valid item, mouse above or below listview?
//		cout << " viewport: " << viewport()->mapFromGlobal(point).y();
		bool above=viewport()->mapFromGlobal(point).y()<0;
		QListViewItem* swapItem;
		if(above) {
			swapItem=dragItem->itemAbove();
			if(swapItem) {
				if(swapItem!=firstChild())
					swapItem->moveItem(dragItem);
				else
					ensureItemVisible(swapItem);				
			}
		}
		else {
			swapItem=dragItem->itemBelow();
			if(swapItem)
				dragItem->moveItem(swapItem);
		}
//		if(swapItem) {
			ensureItemVisible(dragItem);
			// keep scrolling, if mouse not moving...
			QTimer *timer = new QTimer( this );
			connect( timer, SIGNAL(timeout()), this, SLOT(simulateMouseMove()) );
			timer->start( 400, TRUE );
//		}
		return;
	}
	
	
	Song* s=((SongListItem*)item)->song();
	if(s==dragSong)	// item has not moved
		return;
		
	bool up=(point.y() < dragPoint.y());
	if(up) {		// don't allow dragging to top song (is played)
		if(item==firstChild()) {
			cout << "dragging to top song not allowed!\n";
			return;
		}
	}
	
	dragItem->moveItem(item);
	if(up) {
		dragItem->itemAbove()->moveItem(dragItem);
	}
	dragPoint=point;
}

void MyListView::contentsMousePressEvent ( QMouseEvent * e)
{
	if(draggable && e->button()==LeftButton) {
		dragPoint=e->globalPos();
		dragItem=itemAt(viewport()->mapFromGlobal(dragPoint));
		// don't allow dragging of first item
		if(dragItem!=firstChild()) {
			if(dragItem) {
				// start dragging
				dragging=true;
				setCursor(Qt::sizeVerCursor);
				SongListItem* s=(SongListItem*)dragItem;
				dragSong= s->song();
			}
		}
	}
	QListView::contentsMousePressEvent(e);
}

void MyListView::contentsMouseReleaseEvent ( QMouseEvent * e)
{
	if(dragging) {
		gYammiGui->stopDragging();
		
/*
	// here we have to synchronize with xmms playlist
		gYammiGui->songsToPlay.clear();
		for ( QListViewItem* item=firstChild(); item; item=item->nextSibling() ) {
			gYammiGui->songsToPlay.append(((SongListItem*)item)->song());
		}
//		gYammiGui->folderActual->update(gYammiGui->songsToPlay);
//		gYammiGui->slotFolderChanged();
//		setCurrentItem(dragItem);
		
		gYammiGui->syncYammi2Xmms();
		dragging=false;
*/
	}
	QListView::contentsMouseReleaseEvent(e);
}

/*
void MyListView::contentsDragMoveEvent ( QDragMoveEvent* e )
{
	cout << "dragMoveEvent\n";
}

void MyListView::contentsDragEnterEvent ( QDragEnterEvent* e )
{
	cout << "dragEnterEvent\n";
}

void MyListView::contentsDragLeaveEvent ( QDragLeaveEvent* e )
{
	cout << "dragLeaveEvent\n";
}
*/
