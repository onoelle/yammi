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
#include "song.h"
#include "mylist.h"

#include <kdebug.h>
#include <klineedit.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>


SongInfo::SongInfo(QWidget* parent, MyList* selectedSongs) : SongInfoDialog(parent, "song info", true)
{
    this->selectedSongs = selectedSongs;
}

void SongInfo::activateUpdates()
{
    connect(LineEditArtist, SIGNAL(textChanged(const QString&)), this, SLOT(update()));
    connect(LineEditTitle, SIGNAL(textChanged(const QString&)), this, SLOT(update()));
    connect(LineEditAlbum, SIGNAL(textChanged(const QString&)), this, SLOT(update()));
    connect(LineEditComment, SIGNAL(textChanged(const QString&)), this, SLOT(update()));
    connect(LineEditTrack, SIGNAL(textChanged(const QString&)), this, SLOT(update()));
    connect(LineEditYear, SIGNAL(textChanged(const QString&)), this, SLOT(update()));
    connect(ComboBoxGenre, SIGNAL(textChanged(const QString&)), this, SLOT(update()));
    connect(CheckBoxCorrectFilename, SIGNAL(stateChanged(int)), this, SLOT(update()));
    connect(CheckBoxCorrectPath, SIGNAL(stateChanged(int)), this, SLOT(update()));
    update();
}

/**
 * update proposed filename/path and enabled state of OK button.
 */
void SongInfo::update()
{
    kdDebug() << "update() called" << endl;
    
    QString _proposedFilename;
    QString _proposedPath;
     
    bool change = false;
    bool firstSong = true;
    bool filenameCorrections = false;
    bool pathCorrections = false;
    for(Song* s = selectedSongs->firstSong(); s; s = selectedSongs->nextSong()) {
        Song* x = new Song();
        x->updateReadableFields(s);
        
        if(applyChanges(x)) {
            change = true;
        }
        QString _constructedFilename = x->constructFilename();
        QString _constructedPath = x->constructPath();
        if(firstSong) {
            _proposedFilename = _constructedFilename;
            _proposedPath = _constructedPath;
            firstSong = false;
        }
        else {   
            if(_proposedFilename != _constructedFilename)
                _proposedFilename = "!";
            if(_proposedPath != _constructedPath)
                _proposedPath = "!";
        }
        if(_constructedFilename != s->filename) {
            filenameCorrections = true;
        }
        if(_constructedPath != s->path) {
            pathCorrections = true;
        }
    }
    
    QColor color;
    if(filenameCorrections) {
        color.setNamedColor("red");
    }
    else {
        color.setNamedColor("grey");
    }
    ReadOnlyProposedFilename->setPaletteForegroundColor(color);
    ReadOnlyProposedFilename->setText(_proposedFilename);
    
    if(pathCorrections) {
        color.setNamedColor("red");
    }
    else {
        color.setNamedColor("grey");
    }
    ReadOnlyProposedPath->setPaletteForegroundColor(color);
    ReadOnlyProposedPath->setText(_proposedPath);
    
    if(change || (filenameCorrections && CheckBoxCorrectFilename->isChecked()) || (pathCorrections && CheckBoxCorrectPath->isChecked()) ) {
        PushButtonOK->setEnabled(true);
    }
    else {
        PushButtonOK->setEnabled(false);
    }
}

bool SongInfo::applyChanges(Song* s) {
    bool change = false;
    if(LineEditArtist->text()!="!" && LineEditArtist->text()!=s->artist) {
        s->artist = LineEditArtist->text();
        change=true;
    }
    if(LineEditTitle->text()!="!" && LineEditTitle->text()!=s->title) {
        s->title = LineEditTitle->text();
        change=true;
    }
    if(LineEditAlbum->text()!="!" && LineEditAlbum->text()!=s->album) {
        s->album = LineEditAlbum->text();
        change=true;
    }
    if(LineEditComment->text()!="!" && LineEditComment->text()!=s->comment) {
        s->comment = LineEditComment->text();
        change=true;
    }
    if(LineEditYear->text()!="!") {
        int tryYear=atoi(LineEditYear->text());
        if(tryYear!=s->year) {
            change=true;
            s->year = tryYear;
        }
    }
    if(LineEditTrack->text()!="!") {
        int tryTrackNr=atoi(LineEditTrack->text());
        if(tryTrackNr!=s->trackNr) {
            change=true;
            s->trackNr = tryTrackNr;
        }
    }
    
    if(ComboBoxGenre->currentText() != "!" && ComboBoxGenre->currentText() != s->genre) {
        s->genre = ComboBoxGenre->currentText();
        change=true;
    }
    return change;
}

SongInfo::~SongInfo(){
}

