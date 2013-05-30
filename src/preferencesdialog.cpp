/***************************************************************************
                          preferencesdialog.cpp  -  description
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

#include "preferencesdialog.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "ConsistencyCheckParameter.h"
#include "prefs.h"
#include "song.h"


PreferencesDialog::PreferencesDialog(QWidget *parent, Prefs* config )
    : QDialog(parent)
{
    setupUi(this);
    this->config=config;

    // general
    LineEditTrashDir->setText(config->trashDir);
    RadioButtonSimpleGuessmode->setChecked(config->guessingMode==config->GUESSING_MODE_SIMPLE);
    RadioButtonAdvancedGuessmode->setChecked(config->guessingMode==config->GUESSING_MODE_ADVANCED);
    CheckBoxLogging->setChecked(config->logging);
    CheckBoxChildSafe->setChecked(config->childSafe);
    CheckBoxCapitalizeTags->setChecked(config->capitalizeTags);
    CheckBoxTagsConsistent->setChecked(config->tagsConsistent);
    CheckBoxFilenamesConsistent->setChecked(config->filenamesConsistent);
    CheckBoxDirectoriesConsistent->setChecked(config->directoriesConsistent);
    LineEditFilenamePattern->setText(config->consistencyPara->filenamePattern);
    LineEditDirectoriesPattern->setText(config->consistencyPara->directoryPattern);
    
    LineEditPrelistenMp3Command->setText(config->prelistenMp3Command);
    LineEditPrelistenOggCommand->setText(config->prelistenOggCommand);
    LineEditPrelistenWavCommand->setText(config->prelistenWavCommand);
    LineEditPrelistenFlacCommand->setText(config->prelistenFlacCommand);
    LineEditPrelistenOtherCommand->setText(config->prelistenOtherCommand);
    SpinBoxGroupThreshold->setValue(config->groupThreshold);
    CheckBoxLazyGrouping->setChecked(config->lazyGrouping);
    LineEditSearchThreshold->setText(QString("%1").arg(config->searchThreshold));
    LineEditPlayqueueTemplate->setPlainText(config->playqueueTemplate);

#ifdef USE_XINE
    RadioButtonXineEngine->setChecked(config->mediaPlayer == config->MEDIA_PLAYER_XINEENGINE);
#else
    RadioButtonXineEngine->setVisible(false);
#endif

#ifdef USE_PHONON
    RadioButtonPhononEngine->setChecked(config->mediaPlayer == config->MEDIA_PLAYER_PHONONENGINE);
#else
    RadioButtonPhononEngine->setVisible(false);
#endif

#ifdef USE_QMEDIAPLAYER
    RadioButtonQMediaPlayerEngine->setChecked(config->mediaPlayer == config->MEDIA_PLAYER_QMEDIAPLAYERENGINE);
#else
    RadioButtonQMediaPlayerEngine->setVisible(false);
#endif

    for(int i=0; i<Song::getMaxSongAction(); i++) {
        ComboBoxDoubleClickAction->addItem(Song::getSongAction(i));
    }
    ComboBoxDoubleClickAction->setCurrentIndex(config->doubleClickAction);
    for(int i=0; i<Song::getMaxSongAction(); i++) {
        ComboBoxMiddleClickAction->addItem(Song::getSongAction(i));
    }
    ComboBoxMiddleClickAction->setCurrentIndex(config->middleClickAction);

    // plugins
    //////////
#if QT_VERSION >= 0x040200
    PluginUrlLabel->setOpenExternalLinks(true);
#endif

    insertPluginValues();

    // connections
    connect( ButtonChooseTrashDir, SIGNAL( clicked() ), this, SLOT( chooseTrashDir() ) );

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

    ComboBoxPlugins->addItem(tr("choose entry"));
    ComboBoxPlugins->insertItems(1, _pluginMenuEntry);
    ComboBoxPluginMode->addItem("single");
    ComboBoxPluginMode->addItem("group");
    updatePlugin(0);
}


void PreferencesDialog::addStandardPlugins() {
    if(!_pluginMenuEntry.contains(tr("Create CD Label"))) {
        newPlugin(tr("Create CD Label"), "cdlabelgen -c \"Title\" -s \"Subtitle\" -b -w -i \"{customList}\" > {fileDialog}", "group", "{index}. {artist} - {title} ({length})%", "true");
	}
	
    if(!_pluginMenuEntry.contains(tr("Export to m3u Playlist"))) {
        newPlugin(tr("Export to m3u Playlist"), "echo -n -e \"#EXTM3U\n{customList}\" > {fileDialog}", "group", "#EXTINF:{lengthInSeconds},{artist} - {title}{newline}{absoluteFilename}{newline}", "true");
	}
	
    if(!_pluginMenuEntry.contains(tr("MusicBrainz Search"))) {
        newPlugin(tr("MusicBrainz Search"), "konqueror http://www.musicbrainz.org/showtrm.html?trm=`/usr/bin/trm \"{absoluteFilename}\"`&", "single", "", "true");
	}
}





void PreferencesDialog::myAccept() {
    // general
    config->trashDir=LineEditTrashDir->text();
    if(RadioButtonSimpleGuessmode->isChecked())
        config->guessingMode=config->GUESSING_MODE_SIMPLE;
    if(RadioButtonAdvancedGuessmode->isChecked())
        config->guessingMode=config->GUESSING_MODE_ADVANCED;
    if(config->trashDir.right(1)!="/")
        config->trashDir+="/";
    if(config->scanDir.right(1)!="/")
        config->scanDir+="/";
    config->doubleClickAction=(Song::action)ComboBoxDoubleClickAction->currentIndex();
    config->middleClickAction=(Song::action)ComboBoxMiddleClickAction->currentIndex();
    config->logging=CheckBoxLogging->isChecked();
    config->capitalizeTags=CheckBoxCapitalizeTags->isChecked();
    config->tagsConsistent=CheckBoxTagsConsistent->isChecked();
    config->filenamesConsistent=CheckBoxFilenamesConsistent->isChecked();
    config->directoriesConsistent=CheckBoxDirectoriesConsistent->isChecked();
    config->consistencyPara->filenamePattern=LineEditFilenamePattern->text();
    config->consistencyPara->directoryPattern=LineEditDirectoriesPattern->text();
    
    if(config->childSafe && !CheckBoxChildSafe->isChecked()) {
        bool ok;
        QString passwd=QString(QInputDialog::getText(this, tr("password"), tr("enter password"), QLineEdit::Password, QString(""), &ok ));
        if(passwd=="protect")
            config->childSafe=false;
    } else {
        config->childSafe=CheckBoxChildSafe->isChecked();
    }
    config->prelistenMp3Command=LineEditPrelistenMp3Command->text();
    config->prelistenOggCommand=LineEditPrelistenOggCommand->text();
    config->prelistenWavCommand=LineEditPrelistenWavCommand->text();
    config->prelistenFlacCommand=LineEditPrelistenFlacCommand->text();
    config->prelistenOtherCommand=LineEditPrelistenOtherCommand->text();
    config->groupThreshold=SpinBoxGroupThreshold->value();
    config->lazyGrouping=CheckBoxLazyGrouping->isChecked();
    config->searchThreshold=LineEditSearchThreshold->text().toInt();
    config->playqueueTemplate = LineEditPlayqueueTemplate->toPlainText();
    
    if (RadioButtonXineEngine->isChecked()) {
        config->mediaPlayer = config->MEDIA_PLAYER_XINEENGINE;
    } else if (RadioButtonPhononEngine->isChecked()) {
        config->mediaPlayer = config->MEDIA_PLAYER_PHONONENGINE;
    } else if (RadioButtonQMediaPlayerEngine->isChecked()) {
        config->mediaPlayer = config->MEDIA_PLAYER_QMEDIAPLAYERENGINE;
    }

    // plugins
    config->pluginCommand = _pluginCommand;
    config->pluginMenuEntry = _pluginMenuEntry;
    config->pluginCustomList = _pluginCustomList;
    config->pluginMode = _pluginMode;
    config->pluginConfirm = _pluginConfirm;

    accept();
}


// file dialog for trash dir
void PreferencesDialog::chooseTrashDir() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("choose trash directory"), LineEditTrashDir->text());
    if(!dir.isNull()) {
        LineEditTrashDir->setText(dir);
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
        LineEditPluginMenuEntry->setCursorPosition(0);
        LineEditPluginMenuEntry->setEnabled(true);
        LineEditPluginCommand->setText(_pluginCommand[newPos-1]);
        LineEditPluginCommand->setCursorPosition(0);
        LineEditPluginCommand->setEnabled(true);
        LineEditPluginCustomList->setText(_pluginCustomList[newPos-1]);
        LineEditPluginCustomList->setCursorPosition(0);
        LineEditPluginCustomList->setEnabled(_pluginMode[newPos-1]=="group");
        PushButtonDeletePlugin->setEnabled(true);
        CheckBoxPluginConfirm->setChecked(_pluginConfirm[newPos-1]=="true");
        CheckBoxPluginConfirm->setEnabled(true);
        ComboBoxPluginMode->setEnabled(true);
        if(_pluginMode[newPos-1]=="single") {
            ComboBoxPluginMode->setCurrentIndex(0);
        }
        if(_pluginMode[newPos-1]=="group") {
            ComboBoxPluginMode->setCurrentIndex(1);
        }
    }
}

void PreferencesDialog::updatePluginMenuEntry(const QString& newText) {
    int pos=ComboBoxPlugins->currentIndex();
    if(pos==0)
        return;
    _pluginMenuEntry[pos-1] = newText;
    ComboBoxPlugins->setItemText(pos, newText);
}

void PreferencesDialog::updatePluginCommand(const QString& newText) {
    int pos=ComboBoxPlugins->currentIndex();
    if(pos==0)
        return;
    _pluginCommand[pos-1] = newText;
}

void PreferencesDialog::updatePluginCustomList(const QString& newText) {
    int pos=ComboBoxPlugins->currentIndex();
    if(pos==0)
        return;
    _pluginCustomList[pos-1] = newText;
}

void PreferencesDialog::updatePluginMode(int newPos) {
    int pos=ComboBoxPlugins->currentIndex();
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
    int pos=ComboBoxPlugins->currentIndex();
    if(pos==0)
        return;
    _pluginConfirm[pos-1] = checked ? "true" : "false";
}

void PreferencesDialog::newPlugin() {
    ComboBoxPlugins->addItem(tr("new item"));
    _pluginMenuEntry.append(tr("new item"));
    _pluginCommand.append(tr("new command"));
    _pluginMode.append(tr("single"));
    _pluginCustomList.append(tr("new custom list"));
    _pluginConfirm.append(tr("true"));
    ComboBoxPlugins->setCurrentIndex(ComboBoxPlugins->count()-1);
    updatePlugin(ComboBoxPlugins->count()-1);
}

void PreferencesDialog::newPlugin(QString name, QString command, QString mode, QString customList, QString confirm) {
    ComboBoxPlugins->addItem(name);
    _pluginMenuEntry.append(name);
    _pluginCommand.append(command);
    _pluginMode.append(mode);
    _pluginCustomList.append(customList);
    _pluginConfirm.append(confirm);
    ComboBoxPlugins->setCurrentIndex(ComboBoxPlugins->count()-1);
    updatePlugin(ComboBoxPlugins->count()-1);
}

void PreferencesDialog::deletePlugin() {
    int pos=ComboBoxPlugins->currentIndex();
    if(pos==0)
        return;
    ComboBoxPlugins->removeItem(pos);
    _pluginMenuEntry.removeAt(pos-1);
    _pluginCommand.removeAt(pos-1);
    _pluginMode.removeAt(pos-1);
    _pluginCustomList.removeAt(pos-1);
    _pluginConfirm.removeAt(pos-1);
    ComboBoxPlugins->setCurrentIndex(0);
    updatePlugin(0);
}

void PreferencesDialog::showReplacements() {
    QString msg("");
    msg+=tr("Replacements for command (single mode)\n");
    msg+=tr("or custom list (group mode):\n");
    msg+=Song::getReplacementsDescription();
    msg+=tr("{newline} (newline)\n");
    msg+=tr("{index} (index of a song within a selection)\n");
    msg+=tr("{directoryDialog} (directory dialog, returns chosen directory)\n");
    msg+=tr("{fileDialog} (file dialog, returns chosen file)\n");
    msg+=tr("{inputString} (input string dialog, returns entered string)\n\n");

    msg+=tr("Replacements for command (group mode):\n");
    msg+=tr("{customList} (custom list, directly)\n");
    msg+=tr("{customListFile} (filename of custom list)\n");
    msg+=tr("{customListViaFile} (custom list, via file / cat command)\n");
    QMessageBox::information( this, "Yammi",msg);
}
