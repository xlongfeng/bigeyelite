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

#ifndef BIGEYELITE_H
#define BIGEYELITE_H

#include <QObject>

class BigeyeLinker;

class BigeyeLite : public QObject
{
    Q_OBJECT

    Q_PROPERTY(LinkStatus linkStatus READ linkStatus WRITE setLinkStatus NOTIFY linkStatusChanged)

public:
    enum LinkStatus {
        Disconnected,
        Connected,
    };

    Q_ENUM(LinkStatus)

    static BigeyeLite *instance();

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    LinkStatus linkStatus()
    {
        return Disconnected;
    }

    void setLinkStatus(LinkStatus status)
    {

    }

signals:
    void linkStatusChanged();

private:
    explicit BigeyeLite(QObject *parent = nullptr);
    Q_DISABLE_COPY(BigeyeLite)

private:
    static BigeyeLite *self;
    BigeyeLinker *linker;
};

#endif // BIGEYELITE_H
