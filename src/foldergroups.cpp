/***************************************************************************
                          foldergroups.cpp  -  description
                             -------------------
    begin                : Tue Feb 27 2001
    copyright            : (C) 2001 by Brian O.Nölle
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

#include "foldergroups.h"

#include "prefs.h"
#include "yammigui.h"


extern YammiGui* gYammiGui;

FolderGroups::FolderGroups( QTreeWidget* parent, QString title)
: Folder( parent, title ) {}

void FolderGroups::update(MyList* allSongs, int sortBy) {
    // we have to delete all existing items first!
    while (childCount()) {
        QTreeWidgetItem* toDelete = QTreeWidgetItem::child(0);
        // change to Folder* toDelete=(Folder*)firstChild();
        delete(toDelete);
    }

    // set sort order for grouping
    allSongs->setSortOrderAndSort(sortBy, true);

    int threshold = gYammiGui->config()->groupThreshold;
    if(threshold<=0) {
        threshold=1;
    }
    QString last("xxxyyyzzz");
    int groupCount=0;
    bool same=false;
    MyList currentGroup;

    for (MyList::iterator it = allSongs->begin(); it != allSongs->end(); it++) {
        Song* s = (*it)->song();
        QString next;
        switch(sortBy) {
        case MyList::ByArtist:
            next=s->artist;
            break;
        case MyList::ByAlbum:
            next=s->album;
            break;
        case MyList::ByGenre:
            next=s->genre;
            break;
        case MyList::ByYear:
            next=QString("%1").arg(s->year);
            break;
        }

        if(gYammiGui->config()->lazyGrouping) {
            // lazy grouping is not guaranteed to work, as the sorting might be different
            // (we sort once, and scan in linear time)
            QString last2=last.upper();
            last2=last2.replace(QRegExp("_"), " ");
            QString next2=next.upper();
            next2=next2.replace(QRegExp("_"), " ");
            same=(last2==next2);
        } else
            same=(last==next);

        if(same) {
            // add to currentGroup
            currentGroup.appendSong(s);
        } else {
            // check size of list, and create a folder if>threshold
            if(currentGroup.count()>=threshold) {
                groupCount++;
                createGroupFolder(&currentGroup, sortBy);
            }
            switch(sortBy) {
            case MyList::ByArtist:
                last=s->artist;
                break;
            case MyList::ByAlbum:
                last=s->album;
                break;
            case MyList::ByGenre:
                last=s->genre;
                break;
            case MyList::ByYear:
                last=QString("%1").arg(s->year);
                break;
            }

            //			if(last=="")          // we don't group empty values together, do we?
            //				last="xxxyyyzzz";
            currentGroup.clear();
            currentGroup.appendSong(s);
        }
    }
    // for last group also:
    if(currentGroup.count()>=threshold) {
        groupCount++;
        createGroupFolder(&currentGroup, sortBy);
    }
    setText(0, fName+QString(" (%1)").arg(groupCount));
    sortChildren(0, Qt::AscendingOrder);
    allSongs->setSortOrderAndSort(MyList::ByArtist);
}


void FolderGroups::createGroupFolder(MyList* group, int sortBy) {
    // create folder
    Folder *f = new Folder( this, "" );

    // add songs of currentGroup
    bool sameArtist=true;
    MyList::iterator it = group->begin();
    Song* firstSong = (*it)->song();
    QString theArtist = (*it)->song()->artist;
    for (; it != group->end(); it++) {
        Song* toAdd = (*it)->song();
        f->addEntry(new SongEntry(toAdd));
        if(sortBy!=MyList::ByGenre) {
            toAdd->classified=true;					// do not set on genre???
        }
        if(toAdd->artist!=theArtist) {
            sameArtist=false;
        }
    }

    // create folder name
    QString folderName("");
    switch(sortBy) {
    case MyList::ByArtist:								// name folder by artist
        folderName=firstSong->artist;
        if(folderName=="") {
            folderName=tr("- no artist -");
        }
        break;
    case MyList::ByAlbum:								// name folder by artist (if only one) + album
        if(sameArtist)
            folderName=firstSong->artist+" - "+firstSong->album;
        else
            folderName=firstSong->album;
        if(folderName=="") {
            folderName=tr("- no album -");
        }
        break;
    case MyList::ByGenre:
        folderName=firstSong->genre;
        if(folderName=="") {
            folderName=tr("- no genre -");
        }
        break;
    case MyList::ByYear:
        if(firstSong->year!=0) {
            folderName=QString("%1").arg(firstSong->year);
        } else {
            folderName=tr("- no year -");
        }
        break;
    }
    f->setFolderName(folderName);
    f->setText(0, folderName+QString(" (%1)").arg(group->count()));
}


FolderGroups::~FolderGroups() {}

