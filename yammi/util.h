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
#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QTime>

class QDebug;


/**
 @author Oliver Nölle
 * Some static utility methods.
 */
class Util {
public:
	static void deleteDirectoryIfEmpty(QString path, QString noDeleteDir);
	static bool ensurePathExists(QString path);	
};

class LogClass {
public:
    LogClass(QString i_function, bool silent = false);
    ~LogClass();
    QDebug log();
private:
    QString m_function;
    QTime m_start;
    bool m_silent;
};
#define LOGSTART_SILENT LogClass log(__FUNCTION__, true)
#define LOGSTART LogClass log(__FUNCTION__)
#define yDebug log.log


bool moveFile(const QString &oldName, const QString &newName);
bool copyFile(const QString &oldFilePath, const QString &newFilePath);

#endif
