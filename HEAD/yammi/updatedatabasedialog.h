/***************************************************************************
                          updatedatabasedialog.h  -  description
                             -------------------
    begin                : Mon Aug 12 2002
    copyright            : (C) 2002 by Oliver Nölle
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

#ifndef UPDATEDATABASEDIALOG_H
#define UPDATEDATABASEDIALOG_H

#include <qwidget.h>
#include <UpdateDatabaseDialogBase.h>

/**
  *@author Oliver Nölle
  */

class UpdateDatabaseDialog : public UpdateDatabaseDialogBase  {
   Q_OBJECT
public: 
	UpdateDatabaseDialog(QWidget *parent=0, const char *name=0);
	~UpdateDatabaseDialog();

public slots:
  void chooseScanDir();  
};

#endif
