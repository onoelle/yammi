/***************************************************************************
                          updatedatabasemediadialog.cpp  -  description
                             -------------------
    begin                : Fri Sep 13 2002
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

#include "updatedatabasemediadialog.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <kfiledialog.h>
#include <klocale.h>



UpdateDatabaseMediaDialog::UpdateDatabaseMediaDialog(QWidget* parent, Prefs* config)
: UpdateDatabaseMediaDialogBase(parent, i18n("Update Database (media) Dialog"), true) {
    this->config = config;
    LineEditMediaDir->setText(config->mediaDir);
    LineEditFilePattern->setText(config->scanPattern);
    CheckBoxMountMediaDir->setChecked(config->mountMediaDir);
    CheckBoxFollowSymLinks->setChecked(config->followSymLinks);
    connect( ButtonChooseMediaDir, SIGNAL( clicked() ), this, SLOT( chooseMediaDir() ) );
    connect( PushButtonOk, SIGNAL( clicked() ), this, SLOT( myAccept() ) );
}


UpdateDatabaseMediaDialog::~UpdateDatabaseMediaDialog() {}


// file dialog for media dir
void UpdateDatabaseMediaDialog::chooseMediaDir() {
    QString dir=KFileDialog::getExistingDirectory(LineEditMediaDir->text(), this, i18n("choose media directory"));
    if(!dir.isNull()) {
        LineEditMediaDir->setText(dir);
    }
}

void UpdateDatabaseMediaDialog::myAccept() {
    config->mediaDir = LineEditMediaDir->text();
    config->scanPattern = LineEditFilePattern->text();
    config->mountMediaDir = CheckBoxMountMediaDir->isChecked();
    config->followSymLinks = CheckBoxFollowSymLinks->isChecked();
    accept();
}
