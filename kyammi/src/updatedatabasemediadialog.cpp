/***************************************************************************
                          updatedatabasemediadialog.cpp  -  description
                             -------------------
    begin                : Fri Sep 13 2002
    copyright            : (C) 2002 by Oliver N�lle
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

#include "updatedatabasemediadialog.h"

#include <qlineedit.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <kfiledialog.h>
#include <klocale.h>



UpdateDatabaseMediaDialog::UpdateDatabaseMediaDialog(QWidget *parent, const char *name)
  : UpdateDatabaseMediaDialogBase(parent, name, true)
{
 	connect( ButtonChooseMediaDir, SIGNAL( clicked() ), this, SLOT( chooseMediaDir() ) );
}


UpdateDatabaseMediaDialog::~UpdateDatabaseMediaDialog(){
}


// file dialog for media dir
void UpdateDatabaseMediaDialog::chooseMediaDir()
{
	QString dir=KFileDialog::getExistingDirectory(LineEditMediaDir->text(), this, i18n("choose media directory"));
	if(!dir.isNull()) {
		LineEditMediaDir->setText(dir);
	}
}
