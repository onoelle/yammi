/***************************************************************************
                          updatedatabasemediadialog.h  -  description
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

#ifndef UPDATEDATABASEMEDIADIALOG_H
#define UPDATEDATABASEMEDIADIALOG_H

#include <qwidget.h>
#include <UpdateDatabaseMediaDialogBase.h>
#include "prefs.h"

/**
  *@author Oliver N�lle
  */

class UpdateDatabaseMediaDialog : public UpdateDatabaseMediaDialogBase  {
    Q_OBJECT
public:
    UpdateDatabaseMediaDialog(QWidget* parent, Prefs* config);
    ~UpdateDatabaseMediaDialog();

public slots:
    void chooseMediaDir();
    void myAccept();

protected:
    Prefs* config;
};

#endif
