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


#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "bigeyelite.h"

class LogModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(Bigeye::LoggerLevel levelLimit READ levelLimit WRITE setLevelLimit MEMBER m_levelLimit NOTIFY levelLimitChanged)

public:
    enum LogRole {
        LevelRole = Qt::DisplayRole,
        DateTimeRole = Qt::UserRole,
        MessageRole,
    };
    Q_ENUM(LogRole)

    explicit LogModel(QObject *parent = nullptr);

    Bigeye::LoggerLevel levelLimit()
    {
        return m_levelLimit;
    }

    void setLevelLimit(Bigeye::LoggerLevel level)
    {
        m_levelLimit = level;
    }

    int rowCount(const QModelIndex & = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void remove(int row);

signals:
    void levelLimitChanged();

public slots:
    void append(Bigeye::LoggerLevel level, const QString &datetime, const QString &msg);

private:
    Bigeye::LoggerLevel m_levelLimit = Bigeye::LoggerInfo;

    struct Log {
        Bigeye::LoggerLevel level;
        QString datetime;
        QString message;
    };

    QList<Log> m_logs;
};

#endif // LOGMODEL_H
