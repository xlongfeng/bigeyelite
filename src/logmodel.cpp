/*
 * Bigeye Lite - Accessorial Tool for Daily Test
 *
 * Copyright (c) 2018, longfeng.xiao <xlongfeng@126.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QDebug>

#include "logmodel.h"

LogModel::LogModel(QObject *parent) :
    QAbstractListModel(parent)
{
    connect(BigeyeLite::instance(), SIGNAL(log(Bigeye::LoggerLevel,QString,QString)), this, SLOT(append(Bigeye::LoggerLevel,QString,QString)));
}

int LogModel::rowCount(const QModelIndex &) const
{
    return m_logs.count();
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < rowCount())
        switch (role) {
        case LevelRole: return m_logs.at(index.row()).level;
        case DateTimeRole: return m_logs.at(index.row()).datetime;
        case MessageRole: return m_logs.at(index.row()).message;
        default: return QVariant();
    }
    return QVariant();
}

QHash<int, QByteArray> LogModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { LevelRole, "level" },
        { DateTimeRole, "datetime" },
        { MessageRole, "message" },
    };
    return roles;
}

void LogModel::append(Bigeye::LoggerLevel level, const QString &datetime, const QString &msg)
{
    if (level < m_levelLimit)
        return;

    int row = rowCount();
    beginInsertRows(QModelIndex(), row, row);
    m_logs.insert(row, {level, datetime, msg});
    endInsertRows();
}

void LogModel::remove(int row)
{
    if (row < 0 || row >= m_logs.count())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    m_logs.removeAt(row);
    endRemoveRows();
}
