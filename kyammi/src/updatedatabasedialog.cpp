/***************************************************************************
                          updatedatabasedialog.cpp  -  description
                             -------------------
    begin                : Mon Aug 12 2002
    copyright            : (C) 2002 by Oliver Nölle
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


#include "updatedatabasedialog.h"

#include <kfiledialog.h>
#include <klocale.h>

#include <qlineedit.h>
#include <qpushbutton.h>




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
	QString dir = KFileDialog::getExistingDirectory(LineEditScanDir->text(), this, i18n("choose scan directory"));
  if(!dir.isNull()) {
		LineEditScanDir->setText(dir);
	}
}
