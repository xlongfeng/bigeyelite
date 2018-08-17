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

#include <QList>
#include <QQueue>
#include <QPair>

#include "bigeye.h"

class QTimer;
class BigeyeLinker;

class BigeyeLite : public Bigeye
{
    Q_OBJECT

    Q_PROPERTY(LinkStatus linkStatus READ linkStatus MEMBER m_linkStatus WRITE setLinkStatus NOTIFY linkStatusChanged)

public:
    enum LinkStatus {
        Disconnected,
        Connected,
    };

    Q_ENUM(LinkStatus)

    ~BigeyeLite();

    static BigeyeLite *instance();

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    LinkStatus linkStatus()
    {
        return m_linkStatus;
    }

    void setLinkStatus(LinkStatus status)
    {
        m_linkStatus = status;
        emit linkStatusChanged();
    }

signals:
    void linkStatusChanged();

private slots:
    void onDeviceAttached();
    void onDeviceDetached();
    void onTransmitSequence();
    void onDataArrived(const QByteArray &bytes);

private:
    void powerButtonPress();
    void knobLeftRotate();
    void knobRightRotate();
    void enterButtonPress();
    void runningStateQuery();
    void repeaterFileWrite(const QString &filename, const QByteArray &content, int delay = 50);

private:
    explicit BigeyeLite(QObject *parent = nullptr);
    Q_DISABLE_COPY(BigeyeLite)

private:
    static BigeyeLite *self;
    BigeyeLinker *linker;
    LinkStatus m_linkStatus;

    static QList<QPair<QString, QString>> initSequence;
    QTimer *sequenceBlockTimer;
    QQueue<QPair<QByteArray, int>> sequenceBlock;
};

#endif // BIGEYELITE_H
