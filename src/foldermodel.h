/***************************************************************************
 *   Copyright (C) 2013 by Bernhard Übelacker                              *
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
#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include <QAbstractTableModel>


class FolderModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Columns { COLUMN_POS,
                   COLUMN_PLAYED_ON,
                   COLUMN_MATCH,
                   COLUMN_REASON,
                   COLUMN_ARTIST,
                   COLUMN_TITLE,
                   COLUMN_ALBUM,
                   COLUMN_LENGTH,
                   COLUMN_YEAR,
                   COLUMN_TRACKNR,
                   COLUMN_GENRE,
                   COLUMN_ADDED_TO,
                   COLUMN_BITRATE,
                   COLUMN_FILENAME,
                   COLUMN_PATH,
                   COLUMN_COMMENT,
                   COLUMN_LAST_PLAYED,
                   MAX_COLUMN_NO
                 };

    enum UserItemDataRole { SongEntryPointerRole = Qt::UserRole + 1 };

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QStringList mimeTypes() const;
    QMimeData* mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

    void reset();

private:
    QVariant dataDisplayRole(const QModelIndex &index, int role) const;
    QVariant dataTextColorRole(const QModelIndex &index, int role) const;
    QVariant dataSongEntryPointerRole(const QModelIndex &index, int role) const;
    QVariant dataTextAlignmentRole(const QModelIndex &index, int role) const;
};

#endif //FOLDERMODEL_H
