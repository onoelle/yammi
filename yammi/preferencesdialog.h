/***************************************************************************
                          preferencesdialog.h  -  description
                             -------------------
    begin                : Sat Sep 8 2001
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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "ui_PreferencesDialogBase.h"

class Prefs;


/**
  *@author O.Nölle
  */

class PreferencesDialog : public QDialog, public Ui::Preferences  {
    Q_OBJECT

public:
    PreferencesDialog(QWidget *parent, Prefs* config);
    ~PreferencesDialog();

public slots:
    void myAccept();

    void chooseTrashDir();

    void updatePlugin(int newPos);
    void updatePluginMenuEntry(const QString& newText);
    void updatePluginCommand(const QString& newText);
    void updatePluginCustomList(const QString& newText);
    void updatePluginMode(int newPos);
    void updatePluginConfirm(bool checked);
    void newPlugin();
    void newPlugin(QString name, QString command, QString mode, QString customList, QString confirm);
    void deletePlugin();
    void addStandardPlugins();
    void showReplacements();

protected:
    Prefs* config;
    QStringList _pluginMenuEntry;
    QStringList _pluginCommand;
    QStringList _pluginCustomList;
    QStringList _pluginMode;
    QStringList _pluginConfirm;

    void insertPluginValues();

};

#endif
