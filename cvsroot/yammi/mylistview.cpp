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



void MyListView::contentsMouseMoveEvent ( QMouseEvent * e)
{
	if(dragging) {
		// now we should check whether item has moved to new position
		QPoint point=e->globalPos();
		QListViewItem* item=itemAt(viewport()->mapFromGlobal(point));
		if(item==0)
			return;
		Song* s=((SongListItem*)item)->song();
		if(s!=dragSong) {
			bool up=(point.y() < dragPoint.y());
			if(up) {		// don't allow dragging to top song (is played)
//				if(s==gYammiGui->songsToPlay.at(0)) {
				if(item==firstChild()) {
					cout << "dragging to top song not allowed!\n";
					return;
				}
			}
			delete(dragItem);
			if(up) {
				dragItem=new SongListItem(this, dragSong, (SongListItem*)item->itemAbove());
			}
			else {
				dragItem=new SongListItem(this, dragSong, (SongListItem*)item);
			}
			dragPoint=point;
			setCurrentItem(dragItem);
			setSelected(dragItem, true);
		}
	}
	else {
		QListView::contentsMouseMoveEvent(e);
	}
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
