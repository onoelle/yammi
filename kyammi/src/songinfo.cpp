/***************************************************************************
                          songinfo.cpp  -  description
                             -------------------
    begin                : Fri Aug 10 2001
    copyright            : (C) 2001 by Brian O.Nlle
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

#include "songinfo.h"

SongInfo::SongInfo(QWidget *parent) : SongInfoDialog(parent, "song info", true)
{
/*	initCommon();
	if(songCount==1) {
		initSingle(songList->firstSong());
	}
	else {
		kdError() << "not implemented yet...\n";
	}
*/
}
/*
SongInfo::initCommon() {
	songCount=songList->count();
    _length = 0;
    _size = 0;
    _genreNr = 0;
    // fill combobox with genres, but sort them first
    QStringList genreList;
    genreList.append("");
    for(int genreNr=0; genreNr<=CMP3Info::getMaxGenreNr(); genreNr++) {
        genreList.append(CMP3Info::getGenre(genreNr));
    }
    genreList.sort();
    for ( QStringList::Iterator it = genreList.begin(); it != genreList.end(); ++it ) {
        si.ComboBoxGenre->insertItem((*it).latin1());
    }
}

SongInfo::initSingle(Song* s) {
	editSong.updateReadableFields(s);
    // TODO: read file size or take saved value?
	
	QFile file(s->location());
    if(file.exists()) {
        _size=file.size();
	}
	

    // insert all media, over that songs are distributed
    for(unsigned int m=0; m<s->mediaName.count(); m++) {
        bool found=false;
        for(int n=0; n<si.ComboBoxMedia->count(); n++) {
            if(si.ComboBoxMedia->text(n)==s->mediaName[m]) {
                found=true;
			}
        }
        if(!found) {
            si.ComboBoxMedia->insertItem(s->mediaName[m]);
		}
    }


//    _trackNr=QString("%1").arg(s->trackNr);
//    _year=QString("%1").arg(s->year);
//    _bitrate=QString("%1 kb/s").arg(s->bitrate);
    _proposedFilename=s->constructFilename();
}
*/
/**
 * Returns true, if dialog was accepted AND there were modifications.
 */
/*
bool SongInfo::execute() {
    si.LineEditArtist->setText(editSong.artist);
    si.LineEditTitle->setText(editSong.title);
    si.LineEditAlbum->setText(editSong.album);
    si.LineEditComment->setText(editSong.comment);
    if(editSong.year!=0) {
        si.LineEditYear->setText(QString("%1").arg(editSong.year));
	}
    if(editSong.trackNr!=0) {
        si.LineEditTrack->setText(QString("%1").arg(editSong.trackNr));
	}
    if(editSong.isValid()) {
        si.LineEditAddedTo->setText(editSong.addedTo.writeToString());
	}
	else {
        si.LineEditAddedTo->setText("!");
	}
    if(editSong.lastTimePlayed.isValid()) {
        MyDateTime never;
        never.setDate(QDate(1900,1,1));
        never.setTime(QTime(0,0,0));
        if(editSong.lastTimePlayed!=never) {
            si.LineEditLastPlayed->setText(editSong.lastTimePlayed.writeToString());
		}
        else {
            si.LineEditLastPlayed->setText("never");
		}
    } else {
        si.LineEditLastPlayed->setText("!");
    }

    si.ReadOnlyPath->setText(editSong.path);
    si.ReadOnlyFilename->setText(editSong.filename);
    si.ReadOnlyProposedFilename->setText(editSong.proposedFilename);


    si.LabelHeading->setText(_artist+" - "+_title);
    QString x;
    si.ReadOnlyLength->setText(x.sprintf(i18n("%2d:%02d (mm:ss)"), editSong.length/60, editSong.length % 60));

    si.ReadOnlySize->setText( QString(i18n("%1 MB (%2 Bytes)"))
                              .arg( (float)editSong.size/(float)(1024*1024) , 4,'f', 2 )
                              .arg( (float)editSong.size                    ,10,'f', 0 )
                            );
    si.ReadOnlyBitrate->setText(editSong.bitrate);

    if(_genreNr==-1) {
        si.ComboBoxGenre->setCurrentItem(0);
	}
    else {
        int found=genreList.findIndex(CMP3Info::getGenre(editSong.genreNr));
        if(found!=-1) {
            si.ComboBoxGenre->setCurrentItem(found);
		}
    }
	
    // show dialog
    int result=si.exec();
    if(result!=QDialog::Accepted) {
        return false;
	}

	// get values and compare to original ones
    
	// get genreNr
    int sortedGenreNr=si.ComboBoxGenre->currentItem();
    int tryGenreNr=-1;
    if(sortedGenreNr!=0) {
        tryGenreNr=CMP3Info::getGenreIndex(genreList[sortedGenreNr]);
    }


    bool change=false;
    if(si.LineEditArtist->text()!="!" && si.LineEditArtist->text()!=s->artist) {
        s->artist=si.LineEditArtist->text();
        change=true;
    }
    if(si.LineEditTitle->text()!="!" && si.LineEditTitle->text()!=s->title) {
        s->title=si.LineEditTitle->text();
        change=true;
    }
    if(change) { // for artist and title: mark categories as dirty on change!
        model->markPlaylists(s);
    }
    if(si.LineEditAlbum->text()!="!" && si.LineEditAlbum->text()!=s->album) 		{
        s->album=si.LineEditAlbum->text();
        change=true;
    }
    if(si.LineEditComment->text()!="!" && si.LineEditComment->text()!=s->comment)	{
        s->comment=si.LineEditComment->text();
        change=true;
    }
    if(si.LineEditYear->text()!="!") {
        int tryYear=atoi(si.LineEditYear->text());
        if(tryYear!=s->year) {
            s->year=tryYear;
            change=true;
        }
    }
    if(si.LineEditTrack->text()!="!") {
        int tryTrackNr=atoi(si.LineEditTrack->text());
        if(tryTrackNr!=s->trackNr) {
            s->trackNr=tryTrackNr;
            change=true;
        }
    }
    MyDateTime newAddedTo;
    newAddedTo.readFromString(si.LineEditAddedTo->text());
    if(newAddedTo.isValid()) {
        if(newAddedTo!=s->addedTo) {
            s->addedTo=newAddedTo;
            change=true;
        }
    }

    if(tryGenreNr!=-1) {
        if(tryGenreNr!=s->genreNr) {
            s->genreNr=tryGenreNr;
            change=true;
        }
    }

    return(change);
}
*/
SongInfo::~SongInfo(){
}

