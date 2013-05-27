/***************************************************************************
                          songinfo.h  -  description
                             -------------------
    begin                : Fri Aug 10 2001
    copyright            : (C) 2001 by Brian O.Nölle
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

#ifndef SONGINFO_H
#define SONGINFO_H

#include <ui_SongInfoDialog.h>

class MyList;
class Song;


/**
  *@author Brian O.Nölle
  */

class SongInfo : public QDialog, public Ui::SongInfoDialog  {
   Q_OBJECT
public: 
    SongInfo(QWidget *parent, MyList* selectedSongs);
    void activateUpdates();
    ~SongInfo();

protected slots:
    void update();

protected:
    bool applyChanges(Song* s);
    MyList* selectedSongs;
};

#endif
