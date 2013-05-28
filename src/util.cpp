/***************************************************************************
 *   Copyright (C) 2004 by Oliver Nölle                                    *
 *   oli.noelle@web.de                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "util.h"

#include <QDebug>
#include <QDir>
#include <QString>

/**
 * Deletes a directory if empty.
 * Cascades up the directory structure as long as the directories are empty.
 * Does not delete the directory given as second parameter.
 * @param path Directory to check (without trailing slash)
 * @param noDeleteDir (with trailing slash)
 */
void Util::deleteDirectoryIfEmpty(QString path, QString noDeleteDir) {
    if(path.length()<2 || path == noDeleteDir.left(noDeleteDir.length()-1)) {
        return;
    }
    QDir d;
    if(d.rmdir(path)) {
        int lastSlashIndex=path.lastIndexOf('/');
        QString frontPath=path.left(lastSlashIndex);
        deleteDirectoryIfEmpty(frontPath, noDeleteDir);
    }
}

/**
 * Makes sure that the given path exists.
 * Creates directories if needed.
 * Returns false if the path does not exist and could not be created.
 */
bool Util::ensurePathExists(QString path) {
    if(path.length()<=1) {
        return true;
    }
    int lastSlashIndex=path.lastIndexOf('/');
    if(lastSlashIndex==-1) {
        return false;
    }
    QString frontPath=path.left(lastSlashIndex);
    QDir frontPathDir;
    if(frontPath.length()>0) {
        if(!ensurePathExists(frontPath)) {
            return false;
        }
        frontPathDir=QDir(frontPath);
    } else {
        frontPathDir=QDir("/");
    }
    QString endDir=path.mid(lastSlashIndex+1);
    if(frontPathDir.exists(endDir)) {
        return true;
    }
    if(!frontPathDir.mkdir(endDir)) {
        qWarning() << "failed creating dir " << endDir << " in dir " << frontPath;
        return false;
    }
    return true;
}


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
    qint64 len = oldFile.read( buffer, BUFFER_SIZE );
    newFile.write( buffer, len );
  }

  //deallocate buffer
  delete[] buffer;
  buffer = NULL;
  return true;
}
