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

#include <qfiledialog.h>

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include "prefs.h"



UpdateDatabaseDialog::UpdateDatabaseDialog(QWidget *parent, Prefs* config )
: UpdateDatabaseDialogBase(parent, tr("Update Database (harddisk)"), true) {
    this->config = config;
    CheckBoxFollowSymlinks->setChecked(config->followSymLinks);
    LineEditScanDir->setText(config->scanDir);
    LineEditFilePattern->setText(config->scanPattern);
    connect( ButtonChooseScanDir, SIGNAL( clicked() ), this, SLOT( chooseScanDir() ) );
    connect( PushButtonOk, SIGNAL( clicked() ), this, SLOT( myAccept() ) );
}


UpdateDatabaseDialog::~UpdateDatabaseDialog() {}

void UpdateDatabaseDialog::myAccept() {
    config->scanDir = LineEditScanDir->text();
    config->scanPattern = LineEditFilePattern->text();
    config->followSymLinks = CheckBoxFollowSymlinks->isChecked();
    accept();
}


// file dialog for scan dir
void UpdateDatabaseDialog::chooseScanDir() {
    QString dir = QFileDialog::getExistingDirectory(LineEditScanDir->text(), this, NULL, tr("choose scan directory"));
    if(!dir.isNull()) {
        LineEditScanDir->setText(dir);
    }
}
