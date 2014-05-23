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

#include "foldermodel.h"

#include <QFileInfo>
#include <QMimeData>

#include "folder.h"
#include "foldersorted.h"
#include "prefs.h"
#include "yammigui.h"
#include "yammimodel.h"

extern YammiGui* gYammiGui;


const QString SongEntryMimeType = "pointer/SongEntry";

int FolderModel::rowCount(const QModelIndex & /*parent*/) const
{
    Folder* f = gYammiGui->chosenFolder;
    if (f) {
        return f->songlist().count();
    }
    return 0;
}

int FolderModel::columnCount(const QModelIndex & /*parent*/) const
{
    return MAX_COLUMN_NO;
}

QVariant helper_dataDisplayRoleDateTime(QDateTime dt)
{
    if (dt.isValid()) {
        return dt.date().toString(Qt::DefaultLocaleShortDate) +
                dt.time().toString(" hh:mm:ss");
    } else {
        return QVariant();
    }
}

QVariant FolderModel::dataDisplayRole(const QModelIndex &index) const
{
    QVariant ret;
    SongEntry* songEntry = SongEntry::qvAsSe(data(index, SongEntryPointerRole));
    if (songEntry) {
        Song* s = songEntry->song();
        switch (index.column()) {
        case COLUMN_POS:
        case COLUMN_MATCH:
        case COLUMN_REASON:
            ret = songEntry->getColumn(0);
            break;
        case COLUMN_PLAYED_ON:
            ret = helper_dataDisplayRoleDateTime(songEntry->getColumnData(0).toDateTime());
            break;
        case COLUMN_ARTIST:
            ret = s->artist;
            break;
        case COLUMN_TITLE:
            ret = s->title;
            break;
        case COLUMN_ALBUM:
            ret = s->album;
            break;
        case COLUMN_LENGTH:
            if (s->length != 0) {
                QString lengthStr=QString("%1").arg(s->length % 60);
                if (lengthStr.length()==1) {
                    lengthStr="0"+lengthStr;
                }
                ret = QString("%1:%2").arg((s->length) / 60).arg(lengthStr);
            }
            break;
        case COLUMN_YEAR:
            if (s->year != 0) {
                ret = QString("%1").arg(s->year);
            }
            break;
        case COLUMN_TRACKNR:
            if(s->trackNr!=0)	{
                ret = QString("%1").arg(s->trackNr);
            }
            break;
        case COLUMN_GENRE:
            ret = s->genre;
            break;
        case COLUMN_ADDED_TO:
            ret = helper_dataDisplayRoleDateTime(s->addedTo);
            break;
        case COLUMN_BITRATE:
            if (s->bitrate != 0) {
                ret = QString("%1").arg(s->bitrate);
            }
            break;
        case COLUMN_FILENAME:
            ret = s->filename;
            break;
        case COLUMN_EXTENSION:
            ret = QFileInfo(s->filename).suffix().toLower();
            break;
        case COLUMN_PATH:
            ret = s->path;
            break;
        case COLUMN_COMMENT:
            if (s->comment != "") {
                ret = s->comment;
            }
            break;
        case COLUMN_LAST_PLAYED:
            {
                MyDateTime never;
                never.setDate(QDate(1900,1,1));
                never.setTime(QTime(0,0,0));
                if (s->lastPlayed != never) {
                    ret = helper_dataDisplayRoleDateTime(s->lastPlayed);
                } else {
                    ret = tr("never");
                }
            }
            break;
        default:
            break;
        }
    }
    return ret;
}

QVariant FolderModel::dataTextColorRole(const QModelIndex &index) const
{
    QColor ret;
    if (gYammiGui->config()->thisIsSecondYammi) {
        ret = QColor(Qt::white);
    }

    SongEntry* songEntry = SongEntry::qvAsSe(data(index, SongEntryPointerRole));
    if (songEntry) {
        Song* s = songEntry->song();

        if (gYammiGui->getModel()->songsPlayed.containsSong(s)) // already played in green
            ret = QColor(Qt::darkGreen);

        if (gYammiGui->getModel()->songsToPlay.containsSong(s)) // enqueued songs in blue
              ret = QColor(Qt::blue );

        // availability: in playlist we really check whether available (maybe in swap dir)
        // otherwise we just check filename (for performance)
        if (gYammiGui->chosenFolder == gYammiGui->folderActual) {
            QString path = gYammiGui->getModel()->checkAvailability(s);
            if (path == "" ) // not yet available in light grey
                ret = QColor(Qt::lightGray);
            else if (path == "never" ) // never available in yellow
                ret = QColor(Qt::yellow);
            else if (s->filename == "") // swapped in dark grey
              ret = QColor(Qt::darkGray);
          } else {
              if (s->filename == "") {
                  ret = QColor(Qt::gray);
              }
          }

          if (gYammiGui->currentSong == s) // current song in red
              ret = QColor(Qt::red);
    }
    if (gYammiGui->config()->thisIsSecondYammi) {
        ret = ret.lighter(175);
    }
    return ret;
}

QVariant FolderModel::dataTextAlignmentRole(const QModelIndex &index) const
{
    QVariant ret;

    switch (index.column()) {
    case COLUMN_LENGTH:
    case COLUMN_YEAR:
    case COLUMN_TRACKNR:
    case COLUMN_ADDED_TO:
    case COLUMN_BITRATE:
        ret = QVariant(Qt::AlignRight | Qt::AlignVCenter);
        break;
    default:
        break;
    }

    return ret;
}

QVariant FolderModel::dataSongEntryPointerRole(const QModelIndex &index) const
{
    QVariant ret;

    if (index.row() >= 0 && index.column() >= 0) {
        Folder* f = gYammiGui->chosenFolder;
        SongEntry* songEntry = NULL;
        if (f) {
            songEntry = f->songlist().at(index.row());
        }
        if (songEntry) {
            ret = SongEntry::seAsQv(songEntry);
        }
    }
    return ret;
}

QVariant FolderModel::dataSongEntrySortDataRole(const QModelIndex &index) const
{
    QVariant ret;

    ret = dataDisplayRole(index);

    int column = index.column();
    SongEntry* songEntry = SongEntry::qvAsSe(data(index, SongEntryPointerRole));
    if (songEntry) {
        Song* s = songEntry->song();

        switch (column) {
        case COLUMN_PLAYED_ON:
            ret = songEntry->getColumnData(0);
            if (ret == tr("never"))
                ret = QDateTime();
            break;
        case COLUMN_ADDED_TO:
            ret = s->addedTo;
            break;
        case COLUMN_LAST_PLAYED:
            ret = s->lastPlayed;
            break;
        case COLUMN_BITRATE:
            ret = s->bitrate;
            break;
        case COLUMN_LENGTH:
            ret = s->length;
            break;
        case COLUMN_TRACKNR:
            ret = s->trackNr;
            break;
        default:
            break;
        }
    }

    return ret;
}

QVariant FolderModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        return dataDisplayRole(index);
    case Qt::TextColorRole:
        return dataTextColorRole(index);
    case Qt::TextAlignmentRole:
        return dataTextAlignmentRole(index);
    case SongEntryPointerRole:
        return dataSongEntryPointerRole(index);
    case SongEntrySortDataRole:
        return dataSongEntrySortDataRole(index);
    }
    return QVariant();
}

QVariant FolderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant ret = QVariant();
    if (role ==Qt::DisplayRole ) {
        if (orientation == Qt::Horizontal ) {
            ret = gYammiGui->getColumnName(section);
        }
    }
    return ret;
}

Qt::DropActions FolderModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::ItemFlags FolderModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if (!gYammiGui->dragDropAllowed(index.row())) {
        return defaultFlags;
    }

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QStringList FolderModel::mimeTypes() const
 {
     QStringList types;
     types << SongEntryMimeType;
     return types;
 }

QMimeData* FolderModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            QVariant qv = data(index, SongEntryPointerRole);
            quint64 ptr = (quint64)qv.value<void*>();
            QPoint rowcol(index.column(), index.row());
            stream << ptr << rowcol;
        }
    }

    mimeData->setData(SongEntryMimeType, encodedData);
    return mimeData;
}

bool FolderModel::dropMimeData(const QMimeData *mimedata, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex &parent)
{
    if (!mimedata->hasFormat(SongEntryMimeType)) {
        return false;
    }

    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (action != Qt::MoveAction) {
        return false;
    }

    QByteArray encodedData = mimedata->data(SongEntryMimeType);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    while (!stream.atEnd()) {
        quint64 ptr;
        QPoint rowcol;
        stream >> ptr >> rowcol;

        MyList* songlist = &gYammiGui->chosenFolder->songlist();
        SongEntry* seDraged = (SongEntry*)ptr;
        SongEntry* seParent = SongEntry::qvAsSe(data(parent, SongEntryPointerRole));

        if (songlist && seDraged && seParent && rowcol.x() == 0 && seDraged != seParent) {

            int posParent = ((SongEntryInt*)seParent)->intInfo;
            int posDraged = ((SongEntryInt*)seDraged)->intInfo;
            int offset = (posDraged < posParent ? 1 : 0);           /* this look terrible and when more than one is slected the order is not preserved
                                                                       also works only because we disallow drag and drop when not sorted by Pos */

            SongEntry* e = songlist->takeAt(songlist->indexOf(seDraged));
            songlist->insert(songlist->indexOf(seParent) + offset, e);
        }
    }
    FolderSorted* fs = dynamic_cast<FolderSorted*>(gYammiGui->chosenFolder);
    if (fs) {
        fs->correctOrder();
    }

    return true;
}

void FolderModel::reset()
{
    beginResetModel();
    endResetModel();
}
