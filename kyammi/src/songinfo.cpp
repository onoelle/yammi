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

#include <kdebug.h>
#include <klineedit.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>


SongInfo::SongInfo(QWidget *parent, Song* editSong) : SongInfoDialog(parent, "song info", true)
{
    if(editSong != 0) {
        this->editSong = new Song();
        this->editSong->updateReadableFields(editSong);
        this->editSong->updateWritableFields(editSong);
    }
    else {
        this->editSong = 0;
    }
    connect(LineEditArtist, SIGNAL(textChanged(const QString&)), this, SLOT(updateProposedFilename()));
    connect(LineEditTitle, SIGNAL(textChanged(const QString&)), this, SLOT(updateProposedFilename()));
    connect(LineEditAlbum, SIGNAL(textChanged(const QString&)), this, SLOT(updateProposedFilename()));
    connect(LineEditComment, SIGNAL(textChanged(const QString&)), this, SLOT(updateProposedFilename()));
    connect(LineEditTrack, SIGNAL(textChanged(const QString&)), this, SLOT(updateProposedFilename()));
    connect(LineEditYear, SIGNAL(textChanged(const QString&)), this, SLOT(updateProposedFilename()));
    connect(ComboBoxGenre, SIGNAL(textChanged(const QString&)), this, SLOT(updateProposedFilename()));
}

void SongInfo::updateProposedFilename()
{
    if(editSong != 0) {
        // only activated if single song info dialog
        editSong->artist = LineEditArtist->text();
        editSong->title = LineEditTitle->text();
        editSong->album = LineEditAlbum->text();
        editSong->comment = LineEditComment->text();
        editSong->year = atoi(LineEditYear->text());
        editSong->trackNr = atoi(LineEditTrack->text());
        QString proposedFilename = editSong->constructFilename();
        QString proposedPath = editSong->constructPath();
        QColor color;
        if(proposedFilename != editSong->filename) {
            color.setNamedColor("red");
        }
        else {
            color.setNamedColor("grey");
        }
        ReadOnlyProposedFilename->setPaletteForegroundColor(color);
        ReadOnlyProposedFilename->setText(proposedFilename);
        if(proposedPath != editSong->path) {
            color.setNamedColor("red");
        }
        else {
            color.setNamedColor("grey");
        }
        ReadOnlyProposedPath->setPaletteForegroundColor(color);
        ReadOnlyProposedPath->setText(proposedPath);
    }
}

SongInfo::~SongInfo(){
}

