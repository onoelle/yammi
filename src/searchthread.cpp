/***************************************************************************
 *   Copyright (C) 2004 by Oliver Nölle                                    *
 *   oli.noelle@web.de                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "searchthread.h"

#include <QDebug>
#include <QMutex>
#include <QWaitCondition>

#include "fuzzsrch.h"
#include "mylist.h"
#include "prefs.h"
#include "song.h"
#include "songentryint2.h"
#include "yammigui.h"
#include "yammimodel.h"


SearchThread::SearchThread(YammiGui* yammiGui)
: QThread()
{
    searchResults = new MyList;
	gYammiGui=yammiGui;
	currentSearchTerm="";
	searchTerm="";	
	searchRunning=false;
	stopThreadFlag=false;
//	connect(this, SIGNAL(searchResultsAvailable(MyList*)), yammiGui, SLOT(updateSearchResults(MyList*)));
}


SearchThread::~SearchThread()
{
    delete searchResults;
}

void SearchThread::run() {
    QMutex mutex;
    while(!stopThreadFlag) {
		if(searchTerm!=currentSearchTerm) {
			searchRunning=true;
			currentSearchTerm=searchTerm;
            searchResults->clear();
			if(currentSearchTerm=="") {
                gYammiGui->requestSearchResultsUpdate(searchResults);
				searchRunning=false;
				continue;
			}
			// start fuzzy search
			QString searchStr=" " + currentSearchTerm +" ";
			FuzzySearch fs;
			fs.initialize(searchStr.lower(), 2, 4);			// STEP 1
			// search through all songs
			QString composed;
            for(MyList::iterator it = gYammiGui->getModel()->allSongs.begin(); it != gYammiGui->getModel()->allSongs.end(); it++) {
                Song* s = (*it)->song();
				if(searchTerm!=currentSearchTerm) {
					break;
				}
				composed=" " + s->artist + " - " + s->title + " - " + s->album + " - " + s->comment + " ";
				if(s->artist=="" || s->title=="") {							
					// if tags incomplete use filename for search
					composed=s->filename+"- "+composed;
				}
				fs.checkNext(composed.lower(), (void*)s);				// STEP 2
			}
			if(searchTerm==currentSearchTerm) {
				// background search completed without change in search term
				fs.newOrder();											// STEP 3
				BestMatchEntry** bme;
				bme=fs.getBestMatchesList();				// STEP 4

				// insert n best matches into search result list
                searchResults->clear();
				int noSelected=0;
				int showThreshold=gYammiGui->config()->searchThreshold*10;
				int selectThreshold=700;
				for(int noResults=0; noResults<200 && bme[noResults] && (bme[noResults])->sim > showThreshold; noResults++) {
                    searchResults->append( new SongEntryInt2 ((Song*)bme[noResults]->objPtr, bme[noResults]->sim) );
					if(bme[noResults]->sim>selectThreshold) {
						noSelected++;
					}
				}
				if(searchTerm==currentSearchTerm) {
                    gYammiGui->requestSearchResultsUpdate(searchResults);
					searchRunning=false;
				}
			}
		}
		if(!searchRunning && searchTerm==currentSearchTerm) {
            mutex.lock();
            gYammiGui->searchFieldChangedIndicator.wait(&mutex);
            mutex.unlock();
		}
	}
    qDebug() << "searchThread stopped";
}


