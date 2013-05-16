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

#include <Q3Header>
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>

#include "mylistview.h"
#include "yammigui.h"
#include "songlistitem.h"
#include "song.h"
#include "folder.h"


using namespace std;

extern YammiGui* gYammiGui;

MyListView::MyListView(QWidget *parent, const char *name ) : Q3ListView(parent,name) {
    dragging=false;
    dontTouchFirst=false;
    sortedBy=0;
    oldSortOrder=0;
    this->parent=parent;
    setItemMargin(2);
    connect(this->header(), SIGNAL(clicked(int)), this, SLOT(sortColumnChanged(int)));
}

MyListView::~MyListView() {}

/**
 * Updates the sortedBy attribute when user changes sort column or order in listview.
 */
void MyListView::sortColumnChanged(int column) {
    int newColumn=column+1;
    if(sortOrder()==Qt::DescendingOrder) {
        sortedBy=-newColumn;
    } else {
        sortedBy=newColumn;
    }
}

void MyListView::simulateMouseMove() {
    // we simulate a mouse movement to keep list scrolling if we are above or below it
    QMouseEvent e(QEvent::MouseMove, dragPoint, 0, 0);
    contentsMouseMoveEvent(&e);
}


/**
 * Prepares for dragging, but dragging is only allowed if
 * - folder is sorted
 * - song list is sorted ascending by first column (which must be the "Pos" column)
 */
void MyListView::contentsMousePressEvent ( QMouseEvent * e) {
    if(gYammiGui->chosenFolder->isSorted() && sortedBy==1 && e->button()==Qt::LeftButton) {
        dragPoint=e->globalPos();
        dragItem=itemAt(viewport()->mapFromGlobal(dragPoint));
        if(dragItem) {
            //      qDebug() << "dragItem found: " << ((SongListItem*)dragItem)->song()->displayName();
            // check whether we allow dragging of first item
            if(!dontTouchFirst || dragItem!=firstChild()) {
                // start dragging
                //        qDebug() << "dragging started";
                dragging=true;

                // do we have to disable sorting???
                // as we possibly move items not according sort order?
                // (can't move songs bug)
                oldSortOrder=sortedBy;
                setSorting(-1);

                dragStartedAtIndex=dragItem->itemPos();
                setCursor(Qt::sizeVerCursor);
                dragSong=((SongListItem*)dragItem)->song();
            }
        } else {
            //      qDebug() << "no dragItem found";
        }
    }
    Q3ListView::contentsMousePressEvent(e);
}


/**
 * called on dragging the mouse with button pressed
 */
void MyListView::contentsMouseMoveEvent ( QMouseEvent * e) {
    if(!dragging) {
        // if not dragging we call super class to perform normal behavior on dragging
        Q3ListView::contentsMouseMoveEvent(e);
        return;
    }

    // dragging: check whether mouse has moved to new item
    QPoint point=e->globalPos();
    //  qDebug() << "e->globalPos(): x: " << point.x() << ", y: " << point.y();
    QPoint mappedPoint=viewport()->mapFromGlobal(point);
    /*  qDebug() << "mappedPoint: x: " << mappedPoint.x() << ", y: " << mappedPoint.y();
      QPoint widgetMappedPoint=mapFromGlobal(point);
      qDebug() << "widgetMappedPoint: x: " << widgetMappedPoint.x() << ", y: " << widgetMappedPoint.y();
      
      QPoint point2=QCursor::pos();
      qDebug() << "QCursor::pos(): x: " << point2.x() << ", y: " << point2.y();
      QPoint mappedPoint2=viewport()->mapFromGlobal(point2);
      qDebug() << "mappedPoint2: x: " << mappedPoint.x() << ", y: " << mappedPoint.y();
      QPoint widgetMappedPoint2=mapFromGlobal(point2);
      qDebug() << "widgetMappedPoint2: x: " << widgetMappedPoint2.x() << ", y: " << widgetMappedPoint2.y();
      */
    Q3ListViewItem* item=itemAt(mappedPoint);
    /*  if(item!=0) {
          qDebug() << "item: " << ((SongListItem*)item)->song()->displayName();
      }
    	QListViewItem* item2=itemAt(widgetMappedPoint);
      if(item2!=0) {
          qDebug() << "item2: " << ((SongListItem*)item2)->song()->displayName();
      }
    	QListViewItem* item3=itemAt(mappedPoint2);
      if(item3!=0) {
          qDebug() << "item3: " << ((SongListItem*)item3)->song()->displayName();
      }
    	QListViewItem* item4=itemAt(widgetMappedPoint2);
      if(item4!=0) {
          qDebug() << "item4: " << ((SongListItem*)item4)->song()->displayName();
      }
    */

    // no valid item, mouse above or below listview?
    if(item==0)	{
        //    qDebug() << "item==0";
        bool above=viewport()->mapFromGlobal(point).y()<0;
        Q3ListViewItem* swapItem;
        if(above) {
            swapItem=dragItem->itemAbove();
            if(swapItem) {
                if(dontTouchFirst && swapItem==firstChild())
                    return;
                swapItem->moveItem(dragItem);
                ensureItemVisible(swapItem);
            }
        } else {
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


    //  qDebug() << "drag requested: \ndragSong: " << dragSong->displayName() << "\ns: " << s->displayName();
    //  qDebug() << "point.y(): " << point.y() << ", dragPoint.y(): " << dragPoint.y();
    bool up=(point.y() < dragPoint.y());
    if(up) {		// don't allow dragging to top song (is played)
        if(dontTouchFirst && item==firstChild()) {
            //			qDebug() << "dragging to top song not allowed in this folder!";
            return;
        }
    }

    //  qDebug() << "moving item!";
    dragItem->moveItem(item);
    if(up) {
        dragItem->itemAbove()->moveItem(dragItem);
    }
    dragPoint=point;
}


/**
 * Called on release of mouse button, possibly end of dragging a song.
 */
void MyListView::contentsMouseReleaseEvent ( QMouseEvent * e) {
    if(dragging) {
        dragging=false;
        setCursor(Qt::arrowCursor);
        // check, whether item really moved to new position
        if(dragItem->itemPos()!=dragStartedAtIndex) {
            gYammiGui->stopDragging();			// only invoke this, if dragSong was moved
        }
        // reset sort order (can't move songs bug)
        bool asc=oldSortOrder>0;
        if(!asc) {
            oldSortOrder=-oldSortOrder;
        }
        int column=oldSortOrder-1;
        setSorting(column);
    }
    Q3ListView::contentsMouseReleaseEvent(e);
}

void MyListView::simulateKeyPressEvent(QKeyEvent* e)
{
    keyPressEvent(e);
}

