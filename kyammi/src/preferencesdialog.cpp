/***************************************************************************
                          preferencesdialog.cpp  -  description
                             -------------------
    begin                : Sat Sep 8 2001
    copyright            : (C) 2001 by Brian O.Nlle
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

#include "options.h"
#include "preferencesdialog.h"

#include <kfiledialog.h>
#include <klocale.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qvaluelist.h>
#include <qcheckbox.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qspinbox.h>

#include "song.h"

PreferencesDialog::PreferencesDialog(QWidget *parent, const char *name, bool modal, Prefs* config )
: Preferences(parent, name, modal) {
    this->config=config;

    // general
    LineEditTrashDir->setText(config->trashDir);
    RadioButtonSimpleGuessmode->setChecked(config->guessingMode==config->GUESSING_MODE_SIMPLE);
    RadioButtonAdvancedGuessmode->setChecked(config->guessingMode==config->GUESSING_MODE_ADVANCED);
    CheckBoxLogging->setChecked(config->logging);
    CheckBoxChildSafe->setChecked(config->childSafe);
    CheckBoxTagsConsistent->setChecked(config->tagsConsistent);
    CheckBoxFilenamesConsistent->setChecked(config->filenamesConsistent);
    CheckBoxCapitalizeTags->setChecked(config->capitalizeTags);
    LineEditCriticalSize->setText(QString("%1").arg(config->criticalSize));
    LineEditSecondSoundDevice->setText(config->secondSoundDevice);
    SpinBoxGroupThreshold->setValue(config->groupThreshold);
    CheckBoxLazyGrouping->setChecked(config->lazyGrouping);
    LineEditSearchThreshold->setText(QString("%1").arg(config->searchThreshold));

    // xmms specific
    LineEditKeepInXmms->setText(QString("%1").arg(config->keepInXmms));
    #ifdef ENABLE_XMMS

    RadioButtonXmms->setChecked(config->mediaPlayer==config->MEDIA_PLAYER_XMMS);
    #else

    RadioButtonXmms->setEnabled(false);
    LineEditKeepInXmms->setEnabled(false);
    #endif

    // noatun specific
    LineEditFadeTime->setText(QString("%1").arg(config->fadeTime));
    LineEditFadeOutEnd->setText(QString("%1").arg(config->fadeOutEnd));
    LineEditFadeInStart->setText(QString("%1").arg(config->fadeInStart));
    RadioButtonNoatun->setChecked(config->mediaPlayer==config->MEDIA_PLAYER_NOATUN);

    // artsplayer specific
    RadioButtonArtsPlayer->setChecked(config->mediaPlayer==config->MEDIA_PLAYER_ARTSPLAYER);

    for(int i=0; i<Song::getMaxSongAction(); i++) {
        ComboBoxDoubleClickAction->insertItem(Song::getSongAction(i));
    }
    ComboBoxDoubleClickAction->setCurrentItem(config->doubleClickAction);
    for(int i=0; i<Song::getMaxSongAction(); i++) {
        ComboBoxMiddleClickAction->insertItem(Song::getSongAction(i));
    }
    ComboBoxMiddleClickAction->setCurrentItem(config->middleClickAction);

    // jukebox functions
    LineEditMediaDir->setText(config->mediaDir);
    CheckBoxMountMediaDir->setChecked(config->mountMediaDir);
    LineEditSwapDir->setText(config->swapDir);
    LineEditSwapSize->setText(QString("%1").arg(config->swapSize));

    // plugins
    //////////
    LineEditGrabAndEncodeCmd->setText(QString("%1").arg(config->grabAndEncodeCmd));
    LineEditShutdownScript->setText(QString("%1").arg(config->shutdownScript));

    insertPluginValues();

    // connections
    connect( ButtonChooseTrashDir, SIGNAL( clicked() ), this, SLOT( chooseTrashDir() ) );
    connect( ButtonChooseMediaDir, SIGNAL( clicked() ), this, SLOT( chooseMediaDir() ) );
    connect( ButtonChooseSwapDir, SIGNAL( clicked() ), this, SLOT( chooseSwapDir() ) );

    connect( ComboBoxPlugins, SIGNAL( activated(int) ), this, SLOT (updatePlugin(int) ) );
    connect( LineEditPluginMenuEntry, SIGNAL( textChanged(const QString&) ), this, SLOT (updatePluginMenuEntry(const QString&) ) );
    connect( LineEditPluginCommand, SIGNAL( textChanged(const QString&) ), this, SLOT (updatePluginCommand(const QString&) ) );
    connect( LineEditPluginCustomList, SIGNAL( textChanged(const QString&) ), this, SLOT (updatePluginCustomList(const QString&) ) );
    connect( ComboBoxPluginMode, SIGNAL( activated(int) ), this, SLOT (updatePluginMode(int) ) );
    connect( CheckBoxPluginConfirm, SIGNAL( toggled(bool) ), this, SLOT (updatePluginConfirm(bool) ) );
    connect( PushButtonNewPlugin, SIGNAL( clicked() ), this, SLOT( newPlugin() ) );
    connect( PushButtonDeletePlugin, SIGNAL( clicked() ), this, SLOT( deletePlugin() ) );

    connect( PushButtonAddStandardPlugins, SIGNAL( clicked() ), this, SLOT( addStandardPlugins() ) );
    connect( PushButtonPluginReplacements, SIGNAL( clicked() ), this, SLOT( showReplacements() ) );


    connect( ButtonOK, SIGNAL( clicked() ), this, SLOT( myAccept() ) );
}


PreferencesDialog::~PreferencesDialog() {}


void PreferencesDialog::insertPluginValues() {
    _pluginMenuEntry=config->pluginMenuEntry;
    _pluginCommand= config->pluginCommand;
    _pluginCustomList= config->pluginCustomList;
    _pluginMode= config->pluginMode;
    _pluginConfirm= config->pluginConfirm;

    ComboBoxPlugins->insertItem("choose entry");
    ComboBoxPlugins->insertStringList(_pluginMenuEntry);
    ComboBoxPluginMode->insertItem("single");
    ComboBoxPluginMode->insertItem("group");
    updatePlugin(0);
}

void PreferencesDialog::myAccept() {
    // general
    config->trashDir=LineEditTrashDir->text();
    config->mediaDir=LineEditMediaDir->text();
    if(RadioButtonSimpleGuessmode->isChecked())
        config->guessingMode=config->GUESSING_MODE_SIMPLE;
    if(RadioButtonAdvancedGuessmode->isChecked())
        config->guessingMode=config->GUESSING_MODE_ADVANCED;
    if(config->trashDir.right(1)!="/")
        config->trashDir+="/";
    if(config->scanDir.right(1)!="/")
        config->scanDir+="/";
    config->doubleClickAction=(Song::action)ComboBoxDoubleClickAction->currentItem();
    config->middleClickAction=(Song::action)ComboBoxMiddleClickAction->currentItem();
    config->logging=CheckBoxLogging->isChecked();
    config->tagsConsistent=CheckBoxTagsConsistent->isChecked();
    config->filenamesConsistent=CheckBoxFilenamesConsistent->isChecked();
    config->capitalizeTags=CheckBoxCapitalizeTags->isChecked();
    if(config->childSafe && !CheckBoxChildSafe->isChecked()) {
        bool ok;
        QString passwd=QString(QInputDialog::getText( "password", "enter password", QLineEdit::Password, QString(""), &ok, this ));
        if(passwd=="protect")
            config->childSafe=false;
    } else {
        config->childSafe=CheckBoxChildSafe->isChecked();
    }
    config->criticalSize=atoi(LineEditCriticalSize->text());
    config->secondSoundDevice=LineEditSecondSoundDevice->text();
    config->groupThreshold=SpinBoxGroupThreshold->value();
    config->lazyGrouping=CheckBoxLazyGrouping->isChecked();
    config->searchThreshold=atoi(LineEditSearchThreshold->text());

    // xmms specific
    config->keepInXmms=atoi(LineEditKeepInXmms->text());
    if(RadioButtonXmms->isChecked()) {
        config->mediaPlayer=config->MEDIA_PLAYER_XMMS;
    }
    if(RadioButtonNoatun->isChecked()) {
        config->mediaPlayer=config->MEDIA_PLAYER_NOATUN;
    }
    if(RadioButtonArtsPlayer->isChecked()) {
        config->mediaPlayer=config->MEDIA_PLAYER_ARTSPLAYER;
    }


    // noatun specific
    config->fadeTime=atoi(LineEditFadeTime->text());
    config->fadeOutEnd=atoi(LineEditFadeOutEnd->text());
    config->fadeInStart=atoi(LineEditFadeInStart->text());


    // plugins
    config->grabAndEncodeCmd=LineEditGrabAndEncodeCmd->text();
    config->shutdownScript=LineEditShutdownScript->text();

    config->pluginCommand = _pluginCommand;
    config->pluginMenuEntry = _pluginMenuEntry;
    config->pluginCustomList = _pluginCustomList;
    config->pluginMode = _pluginMode;
    config->pluginConfirm = _pluginConfirm;

    // jukebox functions
    config->mediaDir=LineEditMediaDir->text();
    if(config->mediaDir.right(1)!="/")
        config->mediaDir+="/";
    config->swapDir=LineEditSwapDir->text();
    if(config->swapDir.right(1)!="/")
        config->swapDir+="/";
    config->swapSize=atoi(LineEditSwapSize->text());
    config->mountMediaDir=CheckBoxMountMediaDir->isChecked();
    accept();
}


// file dialog for trash dir
void PreferencesDialog::chooseTrashDir() {
    QString dir=KFileDialog::getExistingDirectory(LineEditTrashDir->text(), this, i18n("choose trash directory"));
    if(!dir.isNull()) {
        LineEditTrashDir->setText(dir);
    }
}

// file dialog for media dir
void PreferencesDialog::chooseMediaDir() {
    QString dir=KFileDialog::getExistingDirectory(LineEditMediaDir->text(), this, i18n("choose media directory"));
    if(!dir.isNull()) {
        LineEditMediaDir->setText(dir);
    }
}

// file dialog for swap dir
void PreferencesDialog::chooseSwapDir() {
    QString dir=KFileDialog::getExistingDirectory(LineEditSwapDir->text(), this, i18n("choose swap directory"));
    if(!dir.isNull()) {
        LineEditSwapDir->setText(dir);
    }
}


void PreferencesDialog::updatePlugin(int newPos) {
    if(newPos==0) {
        LineEditPluginMenuEntry->setText("");
        LineEditPluginMenuEntry->setEnabled(false);
        LineEditPluginCommand->setText("");
        LineEditPluginCommand->setEnabled(false);
        LineEditPluginCustomList->setText("");
        LineEditPluginCustomList->setEnabled(false);
        PushButtonDeletePlugin->setEnabled(false);
        CheckBoxPluginConfirm->setEnabled(false);
        ComboBoxPluginMode->setEnabled(false);
    } else {
        LineEditPluginMenuEntry->setText(_pluginMenuEntry[newPos-1]);
        LineEditPluginMenuEntry->setEnabled(true);
        LineEditPluginCommand->setText(_pluginCommand[newPos-1]);
        LineEditPluginCommand->setEnabled(true);
        LineEditPluginCustomList->setText(_pluginCustomList[newPos-1]);
        LineEditPluginCustomList->setEnabled(_pluginMode[newPos-1]=="group");
        PushButtonDeletePlugin->setEnabled(true);
        CheckBoxPluginConfirm->setChecked(_pluginConfirm[newPos-1]=="true");
        CheckBoxPluginConfirm->setEnabled(true);
        ComboBoxPluginMode->setEnabled(true);
        if(_pluginMode[newPos-1]=="single")
            ComboBoxPluginMode->setCurrentItem(0);
        if(_pluginMode[newPos-1]=="group")
            ComboBoxPluginMode->setCurrentItem(1);
    }
}

void PreferencesDialog::updatePluginMenuEntry(const QString& newText) {
    int pos=ComboBoxPlugins->currentItem();
    if(pos==0)
        return;
    _pluginMenuEntry[pos-1] = newText;
    ComboBoxPlugins->changeItem(newText, pos);
}

void PreferencesDialog::updatePluginCommand(const QString& newText) {
    int pos=ComboBoxPlugins->currentItem();
    if(pos==0)
        return;
    _pluginCommand[pos-1] = newText;
}

void PreferencesDialog::updatePluginCustomList(const QString& newText) {
    int pos=ComboBoxPlugins->currentItem();
    if(pos==0)
        return;
    _pluginCustomList[pos-1] = newText;
}

void PreferencesDialog::updatePluginMode(int newPos) {
    int pos=ComboBoxPlugins->currentItem();
    if(pos==0)
        return;
    if(newPos==0) {
        _pluginMode[pos-1] = "single";
        LineEditPluginCustomList->setEnabled(false);
    }
    if(newPos==1) {
        _pluginMode[pos-1] = "group";
        LineEditPluginCustomList->setEnabled(true);
    }
}

void PreferencesDialog::updatePluginConfirm(bool checked) {
    int pos=ComboBoxPlugins->currentItem();
    if(pos==0)
        return;
    _pluginConfirm[pos-1] = checked ? "true" : "false";
}

void PreferencesDialog::newPlugin() {
    ComboBoxPlugins->insertItem("new item");
    _pluginMenuEntry.append("new item");
    _pluginCommand.append("new command");
    _pluginMode.append("single");
    _pluginCustomList.append("new custom list");
    _pluginConfirm.append("true");
    ComboBoxPlugins->setCurrentItem(ComboBoxPlugins->count()-1);
    updatePlugin(ComboBoxPlugins->count()-1);
}

void PreferencesDialog::deletePlugin() {
    int pos=ComboBoxPlugins->currentItem();
    if(pos==0)
        return;
    ComboBoxPlugins->removeItem(pos);
    _pluginMenuEntry.remove(_pluginMenuEntry.at(pos-1));
    _pluginCommand.remove(_pluginCommand.at(pos-1));
    _pluginMode.remove(_pluginMode.at(pos-1));
    _pluginCustomList.remove(_pluginCustomList.at(pos-1));
    _pluginConfirm.remove(_pluginConfirm.at(pos-1));
    ComboBoxPlugins->setCurrentItem(0);
    updatePlugin(0);
}

void PreferencesDialog::addStandardPlugins() {
    config->addStandardPlugins();
    insertPluginValues();
}

void PreferencesDialog::showReplacements() {
    QString msg("");
    msg+=i18n("Replacements for command (single mode)\n");
    msg+=i18n("or custom list (group mode):\n");
    msg+=Song::getReplacementsDescription();
    msg+=i18n("{newline} (newline)\n");
    msg+=i18n("{index} (index of a song within a selection)\n");
    msg+=i18n("{directoryDialog} (directory dialog, returns chosen directory)\n");
    msg+=i18n("{fileDialog} (file dialog, returns chosen file)\n");
    msg+=i18n("{inputString} (input string dialog, returns entered string)\n\n");

    msg+=i18n("Replacements for command (group mode):\n");
    msg+=i18n("{customList} (custom list, directly)\n");
    msg+=i18n("{customListFile} (filename of custom list)\n");
    msg+=i18n("{customListViaFile} (custom list, via file / cat command)\n");
    QMessageBox::information( this, "Yammi",msg);
}
