
#include <QString>
#include <QDir>

#include "options.h"


// copied from http://albumshaper.cvs.sourceforge.net/viewvc/albumshaper/albumshaper2/src/backend/tools/fileTools.cpp?revision=1.9&content-type=text%2Fplain
//==============================================
//  copyright            : (C) 2003-2005 by Will Stokes
//==============================================
//  This program is free software; you can redistribute it
//  and/or modify it under the terms of the GNU General
//  Public License as published by the Free Software
//  Foundation; either version 2 of the License, or
//  (at your option) any later version.
//==============================================
bool moveFile(const QString &oldName, const QString &newName)
{
  QDir rootDir;

  //attempt to rename file
  if(!rootDir.rename( oldName, newName))
  {
    //move failed, copy file and remove original

    //copy failed! sound alert and do not remove original!!!
    if(!copyFile(oldName, newName))
      return false;

    //copy succeded, remove original and return
    rootDir.remove(oldName);
  }

  //move succeeded either directly or via copying and removing original file
  return true;
}
//==============================================
bool copyFile(const QString &oldFilePath, const QString &newFilePath)
{
  //same file, no need to copy
  if(oldFilePath.compare(newFilePath) == 0)
    return true;

  //load both files
  QFile oldFile(oldFilePath);
  QFile newFile(newFilePath);
  bool openOld = oldFile.open( QIODevice::ReadOnly );
  bool openNew = newFile.open( QIODevice::WriteOnly );

  //if either file fails to open bail
  if(!openOld || !openNew) { return false; }

  //copy contents
  uint BUFFER_SIZE = 16000;
  char* buffer = new char[BUFFER_SIZE];
  while(!oldFile.atEnd())
  {
    Q_ULONG len = oldFile.readBlock( buffer, BUFFER_SIZE );
    newFile.writeBlock( buffer, len );
  }

  //deallocate buffer
  delete[] buffer;
  buffer = NULL;
  return true;
}
