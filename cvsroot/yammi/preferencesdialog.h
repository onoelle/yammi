/***************************************************************************
                          preferencesdialog.h  -  description
                             -------------------
    begin                : Sat Sep 8 2001
    copyright            : (C) 2001 by Brian O.Nlle
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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <qwidget.h>
#include <PreferencesDialogBase.h>

/**
  *@author Brian O.Nlle
  */

class PreferencesDialog : public Preferences  {
   Q_OBJECT

public: 
	PreferencesDialog(QWidget *parent=0, const char *name=0, bool modal=true);
	~PreferencesDialog();
	
public slots:
	void chooseScanDir();
	void chooseTrashDir();

};

#endif
