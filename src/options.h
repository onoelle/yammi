/***************************************************************************
                          options.h  -  description
                             -------------------
    begin                : Mon Sep 9 2002
    copyright            : (C) 2002 by Oliver Nölle
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

#ifndef OPTIONS_H
#define OPTIONS_H

#include "config.h"
#include <iostream>

/*
  dirty workaround until qt4 can be used ...
  */
typedef std::ostream& (endlFunc)(std::ostream&);
class KURL;
class QString;
class QDebug {
public:
    QDebug& operator<<(const int) {
        return *this;
    }
    QDebug& operator<<(const char*) {
        return *this;
    }
    QDebug& operator<<(const endlFunc*) {
        return *this;
    }
    QDebug& operator<<(const KURL&) {
        return *this;
    }
    QDebug& operator<<(const QString&) {
        return *this;
    }
};

QDebug& qDebug();
QDebug& qWarning();
QDebug& qError();
QDebug& qFatal();

bool moveFile(const QString &oldName, const QString &newName);
bool copyFile(const QString &oldFilePath, const QString &newFilePath);

#endif //OPTIONS_H
