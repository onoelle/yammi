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
#include <kdebug.h>
#include <qstring.h>
#include <qdir.h>

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
        int lastSlashIndex=path.findRev('/');
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
    int lastSlashIndex=path.findRev('/');
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
        kdWarning() << "failed creating dir " << endDir << " in dir " << frontPath << endl;
        return false;
    }
    return true;
}


