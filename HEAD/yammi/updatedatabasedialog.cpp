/***************************************************************************
                          updatedatabasedialog.cpp  -  description
                             -------------------
    begin                : Mon Aug 12 2002
    copyright            : (C) 2002 by Oliver N�lle
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

#include <qlineedit.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qfiledialog.h>


#include "updatedatabasedialog.h"

UpdateDatabaseDialog::UpdateDatabaseDialog(QWidget *parent, const char *name )
  : UpdateDatabaseDialogBase(parent, name, true)
{
 	connect( ButtonChooseScanDir, SIGNAL( clicked() ), this, SLOT( chooseScanDir() ) );
}


UpdateDatabaseDialog::~UpdateDatabaseDialog()
{
}



// file dialog for scan dir
void UpdateDatabaseDialog::chooseScanDir()
{
	QString dir=QFileDialog::getExistingDirectory(LineEditScanDir->text(), this, QString("yammi"), QString("choose scan directory"), true);
	if(!dir.isNull()) {
		LineEditScanDir->setText(dir);
	}
}