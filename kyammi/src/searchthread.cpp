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

#include <kdebug.h>
#include "fuzzsrch.h"
#include "song.h"
#include "mylist.h"
#include "yammigui.h"
#include "yammimodel.h"
#include "songentryint2.h"

using namespace std;

extern YammiGui* gYammiGui;

SearchThread::SearchThread()
: QThread()
{
	currentSearchTerm="";
	searchTerm="";	
	searchRunning=false;
	stopThreadFlag=false;
}


SearchThread::~SearchThread() {}

void SearchThread::run() {
    while(!stopThreadFlag) {
		if(searchTerm!=currentSearchTerm) {
			searchRunning=true;
			kdDebug() << "search starting...\n";
			currentSearchTerm=searchTerm;
			searchResults.clear();
			
			// start fuzzy search
			QString searchStr=" " + currentSearchTerm +" ";

			FuzzySearch fs;
			fs.initialize(searchStr.lower(), 2, 4);			// STEP 1
			// search through all songs
			Song* s=gYammiGui->getModel()->allSongs.firstSong();
			QString composed;
			for(; s; s=gYammiGui->getModel()->allSongs.nextSong()) {
				composed=" " + s->artist + " - " + s->title + " - " + s->album + " - " + s->comment + " ";
				if(s->artist=="" || s->title=="") {							
					// if tags incomplete use filename for search
					composed=s->filename+"- "+composed;
				}
				fs.checkNext(composed.lower(), (void*)s);				// STEP 2
				if(searchTerm!=currentSearchTerm) {
					kdDebug() << "search interrupted(1)...\n";
					break;
				}
			}
			if(searchTerm==currentSearchTerm) {
				// background search completed without change in search term
				fs.newOrder();											// STEP 3
				BestMatchEntry** bme;
				bme=fs.getBestMatchesList();				// STEP 4

				// insert n best matches into search result list
				searchResults.clear();
				int noSelected=0;
				int showThreshold=gYammiGui->config().searchThreshold*10;
				int selectThreshold=700;
				for(int noResults=0; noResults<200 && bme[noResults] && (bme[noResults])->sim > showThreshold; noResults++) {
					searchResults.append( new SongEntryInt2 ((Song*)bme[noResults]->objPtr, bme[noResults]->sim) );
					if(bme[noResults]->sim>selectThreshold) {
						noSelected++;
					}
				}
				if(searchTerm==currentSearchTerm) {
					gYammiGui->updateSearchResults(&searchResults);
					// publish results
					searchRunning=false;
					kdDebug() << "search completed...\n";
				}
			}
		}
		if(!searchRunning) {
			// TODO: sleep until searchField changed
			this->msleep(100);
		}
	}	
}


