/***************************************************************************
                          preferencesdialog.cpp  -  description
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

#include "preferencesdialog.h"

#include <qfiledialog.h>
#include <qpushbutton.h>


PreferencesDialog::PreferencesDialog(QWidget *parent, const char *name, bool modal ) : Preferences(parent,name,modal)
{
	// additional initialization
	connect( ButtonChooseScanDir, SIGNAL( clicked() ), this, SLOT( chooseScanDir() ) );
}

PreferencesDialog::~PreferencesDialog(){
}


// file dialog for scan dir
void PreferencesDialog::chooseScanDir()
{
	QString dir=QFileDialog::getExistingDirectory(LineEditScanDir->text(), this, QString("yammi"), QString("choose scan directory"), true);
	if(!dir.isNull()) {
		LineEditScanDir->setText(dir);
	}
}
				
// file dialog for scan dir
void PreferencesDialog::chooseTrashDir()
{
	QString dir=QFileDialog::getExistingDirectory(LineEditTrashDir->text(), this, QString("yammi"), QString("choose trash directory"), true);
	if(!dir.isNull()) {
		LineEditTrashDir->setText(dir);
	}
}
				

