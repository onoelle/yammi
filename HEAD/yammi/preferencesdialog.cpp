/***************************************************************************
                          preferencesdialog.cpp  -  description
                             -------------------
    begin                : Sat Sep 8 2001
    copyright            : (C) 2001 by Brian O.Nlle
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

#include "preferencesdialog.h"

#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qvaluelist.h>
#include <qcheckbox.h>
#include <qinputdialog.h>


PreferencesDialog::PreferencesDialog(QWidget *parent, const char *name, bool modal, Prefs* config ) : Preferences(parent,name,modal)
{
	this->config=config;
	
	// general
	LineEditTrashDir->setText(config->trashDir);
	LineEditScanDir->setText(config->scanDir);
	CheckBoxLogging->setChecked(config->logging);
	CheckBoxChildSafe->setChecked(config->childSafe);
	CheckBoxTagsConsistent->setChecked(config->tagsConsistent);
	CheckBoxFilenamesConsistent->setChecked(config->filenamesConsistent);
	LineEditCriticalSize->setText(QString("%1").arg(config->criticalSize));
	LineEditSecondSoundDevice->setText(config->secondSoundDevice);
	LineEditGroupThreshold->setText(QString("%1").arg(config->groupThreshold));
	LineEditSearchThreshold->setText(QString("%1").arg(config->searchThreshold));
	LineEditSearchMaximumNoResults->setText(QString("%1").arg(config->searchMaximumNoResults));
	LineEditKeepInXmms->setText(QString("%1").arg(config->keepInXmms));

	for(int i=0; i<maxSongAction; i++)
		ComboBoxDoubleClickAction->insertItem(songAction[i]);
	ComboBoxDoubleClickAction->setCurrentItem(config->doubleClickAction);
	for(int i=0; i<maxSongAction; i++)
		ComboBoxMiddleClickAction->insertItem(songAction[i]);
	ComboBoxMiddleClickAction->setCurrentItem(config->middleClickAction);
	for(int i=0; i<maxSongAction; i++)
		ComboBoxControlClickAction->insertItem(songAction[i]);
	ComboBoxControlClickAction->setCurrentItem(config->controlClickAction);
	for(int i=0; i<maxSongAction; i++)
		ComboBoxShiftClickAction->insertItem(songAction[i]);
	ComboBoxShiftClickAction->setCurrentItem(config->shiftClickAction);

	// jukebox functions
	LineEditMediaDir->setText(config->mediaDir);
	CheckBoxMountMediaDir->setChecked(config->mountMediaDir);
	LineEditSwapDir->setText(config->swapDir);
	LineEditSwapSize->setText(QString("%1").arg(config->swapSize));
	
	// plugins
	LineEditGrabAndEncodeCmd->setText(QString("%1").arg(config->grabAndEncodeCmd));
	LineEditShutdownScript->setText(QString("%1").arg(config->shutdownScript));
	
	_pluginSongMenuEntry=new QStringList(*config->pluginSongMenuEntry);
	_pluginSongCmd=new QStringList(*config->pluginSongCmd);
	_pluginPlaylistMenuEntry=new QStringList(*config->pluginPlaylistMenuEntry);
	_pluginPlaylistCmd=new QStringList(*config->pluginPlaylistCmd);
	_pluginPlaylistCustomList=new QStringList(*config->pluginPlaylistCustomList);	
	
	ComboBoxSongPlugin->insertItem("choose entry");
	ComboBoxSongPlugin->insertStringList(*_pluginSongMenuEntry);
	updateSongPlugin(0);
	ComboBoxPlaylistPlugin->insertItem("choose entry");
	ComboBoxPlaylistPlugin->insertStringList(*config->pluginPlaylistMenuEntry);
	updatePlaylistPlugin(0);

	// connections
	connect( ButtonChooseScanDir, SIGNAL( clicked() ), this, SLOT( chooseScanDir() ) );
	connect( ButtonChooseTrashDir, SIGNAL( clicked() ), this, SLOT( chooseTrashDir() ) );
	connect( ButtonChooseMediaDir, SIGNAL( clicked() ), this, SLOT( chooseMediaDir() ) );
	connect( ButtonChooseSwapDir, SIGNAL( clicked() ), this, SLOT( chooseSwapDir() ) );
	connect( ComboBoxSongPlugin, SIGNAL( activated(int) ), this, SLOT (updateSongPlugin(int) ) );
	connect( LineEditPluginSongMenuEntry, SIGNAL( textChanged(const QString&) ), this, SLOT (updateSongPluginMenuEntry(const QString&) ) );	
	connect( LineEditPluginSongCmd, SIGNAL( textChanged(const QString&) ), this, SLOT (updateSongPluginCmd(const QString&) ) );	
	connect( PushButtonNewSongPlugin, SIGNAL( clicked() ), this, SLOT( newSongPlugin() ) );
	connect( PushButtonDeleteSongPlugin, SIGNAL( clicked() ), this, SLOT( deleteSongPlugin() ) );
	
	connect( ComboBoxPlaylistPlugin, SIGNAL( activated(int) ), this, SLOT (updatePlaylistPlugin(int) ) );
	connect( LineEditPluginPlaylistMenuEntry, SIGNAL( textChanged(const QString&) ), this, SLOT (updatePlaylistPluginMenuEntry(const QString&) ) );	
	connect( LineEditPluginPlaylistCmd, SIGNAL( textChanged(const QString&) ), this, SLOT (updatePlaylistPluginCmd(const QString&) ) );		
	connect( LineEditPluginPlaylistCustomList, SIGNAL( textChanged(const QString&) ), this, SLOT (updatePlaylistPluginCustomList(const QString&) ) );		
	connect( PushButtonNewPlaylistPlugin, SIGNAL( clicked() ), this, SLOT( newPlaylistPlugin() ) );
	connect( PushButtonDeletePlaylistPlugin, SIGNAL( clicked() ), this, SLOT( deletePlaylistPlugin() ) );
	connect( ButtonOK, SIGNAL( clicked() ), this, SLOT( myAccept() ) );
}

PreferencesDialog::~PreferencesDialog(){
}

void PreferencesDialog::myAccept()
{
 	// general
 	config->trashDir=LineEditTrashDir->text();
 	config->scanDir=LineEditScanDir->text();
 	config->mediaDir=LineEditMediaDir->text();
 	if(config->trashDir.right(1)!="/")
 		config->trashDir+="/";
 	if(config->scanDir.right(1)!="/")
 		config->scanDir+="/";
 	config->doubleClickAction=(action)ComboBoxDoubleClickAction->currentItem();
 	config->middleClickAction=(action)ComboBoxMiddleClickAction->currentItem();
 	config->controlClickAction=(action)ComboBoxControlClickAction->currentItem();
 	config->shiftClickAction=(action)ComboBoxShiftClickAction->currentItem();
 	config->logging=CheckBoxLogging->isChecked();
 	config->tagsConsistent=CheckBoxTagsConsistent->isChecked();
 	config->filenamesConsistent=CheckBoxFilenamesConsistent->isChecked();
 	if(config->childSafe && !CheckBoxChildSafe->isChecked()) {
 		bool ok;
 		QString passwd=QString(QInputDialog::getText( "password", "enter password", QLineEdit::Password, QString(""), &ok, this ));
 		if(passwd=="protect")
 		config->childSafe=false;
 	}
 	else {
 		config->childSafe=CheckBoxChildSafe->isChecked();
 	}
 	config->criticalSize=atoi(LineEditCriticalSize->text());
 	config->secondSoundDevice=LineEditSecondSoundDevice->text();
 	config->groupThreshold=atoi(LineEditGroupThreshold->text());
 	config->searchThreshold=atoi(LineEditSearchThreshold->text());
 	config->searchMaximumNoResults=atoi(LineEditSearchMaximumNoResults->text());
 	config->keepInXmms=atoi(LineEditKeepInXmms->text());
 	// plugins
 	config->grabAndEncodeCmd=LineEditGrabAndEncodeCmd->text();
 	config->shutdownScript=LineEditShutdownScript->text();

	config->pluginSongMenuEntry=new QStringList(*_pluginSongMenuEntry);
 	config->pluginSongCmd=new QStringList(*_pluginSongCmd);
 	config->pluginPlaylistMenuEntry=new QStringList(*_pluginPlaylistMenuEntry);
 	config->pluginPlaylistCmd=new QStringList(*_pluginPlaylistCmd);
 	config->pluginPlaylistCustomList=new QStringList(*_pluginPlaylistCustomList);
 	
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


// file dialog for scan dir
void PreferencesDialog::chooseScanDir()
{
	QString dir=QFileDialog::getExistingDirectory(LineEditScanDir->text(), this, QString("yammi"), QString("choose scan directory"), true);
	if(!dir.isNull()) {
		LineEditScanDir->setText(dir);
	}
}
				
// file dialog for trash dir
void PreferencesDialog::chooseTrashDir()
{
	QString dir=QFileDialog::getExistingDirectory(LineEditTrashDir->text(), this, QString("yammi"), QString("choose trash directory"), true);
	if(!dir.isNull()) {
		LineEditTrashDir->setText(dir);
	}
}
				
// file dialog for media dir
void PreferencesDialog::chooseMediaDir()
{
	QString dir=QFileDialog::getExistingDirectory(LineEditMediaDir->text(), this, QString("yammi"), QString("choose media directory"), true);
	if(!dir.isNull()) {
		LineEditMediaDir->setText(dir);
	}
}

// file dialog for swap dir
void PreferencesDialog::chooseSwapDir()
{
	QString dir=QFileDialog::getExistingDirectory(LineEditSwapDir->text(), this, QString("yammi"), QString("choose swap directory"), true);
	if(!dir.isNull()) {
		LineEditSwapDir->setText(dir);
	}
}


void PreferencesDialog::updateSongPlugin(int newPos)
{
	if(newPos==0) {
		LineEditPluginSongMenuEntry->setText("");
		LineEditPluginSongCmd->setText("");
		LineEditPluginSongMenuEntry->setEnabled(false);
		LineEditPluginSongCmd->setEnabled(false);
		PushButtonDeleteSongPlugin->setEnabled(false);
	}
	else {
		LineEditPluginSongMenuEntry->setText((*_pluginSongMenuEntry)[newPos-1]);
		LineEditPluginSongCmd->setText((*_pluginSongCmd)[newPos-1]);
		LineEditPluginSongMenuEntry->setEnabled(true);
		LineEditPluginSongCmd->setEnabled(true);
		PushButtonDeleteSongPlugin->setEnabled(true);
	}
}

void PreferencesDialog::updateSongPluginMenuEntry(const QString& newText)
{
	int pos=ComboBoxSongPlugin->currentItem();
	if(pos==0)
		return;
	(*_pluginSongMenuEntry)[pos-1]=newText;
	ComboBoxSongPlugin->changeItem(newText, pos);
}

void PreferencesDialog::updateSongPluginCmd(const QString& newText)
{
	int pos=ComboBoxSongPlugin->currentItem();
	if(pos==0)
		return;
	(*_pluginSongCmd)[pos-1]=newText;
}

void PreferencesDialog::newSongPlugin()
{
	ComboBoxSongPlugin->insertItem("new item");
	_pluginSongMenuEntry->append("new item");
	_pluginSongCmd->append("new command");
	ComboBoxSongPlugin->setCurrentItem(ComboBoxSongPlugin->count()-1);
	updateSongPlugin(ComboBoxSongPlugin->count()-1);
}

void PreferencesDialog::deleteSongPlugin()
{
	int pos=ComboBoxSongPlugin->currentItem();
	if(pos==0)
		return;
	ComboBoxSongPlugin->removeItem(pos);
	_pluginSongMenuEntry->remove(_pluginSongMenuEntry->at(pos-1));
	_pluginSongCmd->remove(_pluginSongCmd->at(pos-1));
	ComboBoxSongPlugin->setCurrentItem(0);
	updateSongPlugin(0);
}



void PreferencesDialog::updatePlaylistPlugin(int newPos)
{
	if(newPos==0)
	{
		LineEditPluginPlaylistMenuEntry->setText("");
		LineEditPluginPlaylistCmd->setText("");
		LineEditPluginPlaylistCustomList->setText("");
		LineEditPluginPlaylistMenuEntry->setEnabled(false);
		LineEditPluginPlaylistCmd->setEnabled(false);
		LineEditPluginPlaylistCustomList->setEnabled(false);
		PushButtonDeletePlaylistPlugin->setEnabled(false);
	}
	else {
		LineEditPluginPlaylistMenuEntry->setText((*_pluginPlaylistMenuEntry)[newPos-1]);
		LineEditPluginPlaylistCmd->setText((*_pluginPlaylistCmd)[newPos-1]);
		LineEditPluginPlaylistCustomList->setText((*_pluginPlaylistCustomList)[newPos-1]);
		LineEditPluginPlaylistMenuEntry->setEnabled(true);
		LineEditPluginPlaylistCmd->setEnabled(true);
		LineEditPluginPlaylistCustomList->setEnabled(true);
		PushButtonDeletePlaylistPlugin->setEnabled(true);
	}
}

void PreferencesDialog::updatePlaylistPluginMenuEntry(const QString& newText)
{
	int pos=ComboBoxPlaylistPlugin->currentItem();
	if(pos==0)
		return;
	(*_pluginPlaylistMenuEntry)[pos-1]=newText;
	ComboBoxPlaylistPlugin->changeItem(newText, pos);
}

void PreferencesDialog::updatePlaylistPluginCmd(const QString& newText)
{
	int pos=ComboBoxPlaylistPlugin->currentItem();
	if(pos==0)
		return;
	(*_pluginPlaylistCmd)[pos-1]=newText;
}
void PreferencesDialog::updatePlaylistPluginCustomList(const QString& newText)
{
	int pos=ComboBoxPlaylistPlugin->currentItem();
	if(pos==0)
		return;
	(*_pluginPlaylistCustomList)[pos-1]=newText;
}

void PreferencesDialog::newPlaylistPlugin()
{
	ComboBoxPlaylistPlugin->insertItem("new item");
	_pluginPlaylistMenuEntry->append("new item");
	_pluginPlaylistCmd->append("new command");
	_pluginPlaylistCustomList->append("new custom list");
	ComboBoxPlaylistPlugin->setCurrentItem(ComboBoxPlaylistPlugin->count()-1);
	updatePlaylistPlugin(ComboBoxPlaylistPlugin->count()-1);
}
void PreferencesDialog::deletePlaylistPlugin()
{
	int pos=ComboBoxPlaylistPlugin->currentItem();
	if(pos==0)
		return;
	ComboBoxPlaylistPlugin->removeItem(pos);
	_pluginPlaylistMenuEntry->remove(_pluginPlaylistMenuEntry->at(pos-1));
	_pluginPlaylistCmd->remove(_pluginPlaylistCmd->at(pos-1));
	_pluginPlaylistCustomList->remove(_pluginPlaylistCustomList->at(pos-1));
	ComboBoxSongPlugin->setCurrentItem(0);
	updatePlaylistPlugin(0);
}
