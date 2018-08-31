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
class AbstractTest;

class BigeyeLite : public Bigeye
{
    Q_OBJECT

    Q_PROPERTY(LinkStatus linkStatus READ linkStatus MEMBER m_linkStatus WRITE setLinkStatus NOTIFY linkStatusChanged)
    Q_PROPERTY(PowerState powerState READ powerState MEMBER m_powerState WRITE setPowerState NOTIFY powerStateChanged)

public:
    enum LinkStatus {
        Disconnected,
        Connected,
    };
    Q_ENUM(LinkStatus)

    enum PowerState {
        PowerUnknown,
        PowerOff,
        PowerOn,
    };
    Q_ENUM(PowerState)

    ~BigeyeLite();

    static BigeyeLite *instance();

    void powerButtonPress();
    void knobLeftRotate();
    void knobRightRotate();
    void enterButtonPress();

    LinkStatus linkStatus()
    {
        return m_linkStatus;
    }

    void setLinkStatus(LinkStatus status)
    {
        m_linkStatus = status;
        emit linkStatusChanged();
    }

    PowerState powerState()
    {
        return m_powerState;
    }

    void setPowerState(PowerState status)
    {
        if (m_powerState != status) {
            m_powerState = status;
            emit powerStateChanged();
        }
    }

signals:
    void linkStatusChanged();
    void powerStateChanged();
    void startTest();
    void stopTest();

private slots:
    void onDeviceAttached();
    void onDeviceDetached();

    void onTransmitSequence();
    void onDataArrived(const QByteArray &bytes);

    void onHandshakeTimeout();
    void respHandshake(QDataStream &stream);

    void respPowerStateChanged(QDataStream &stream);

    void respRepeaterFileRead(QDataStream &stream);

private:
    void enqueueBlock(const QByteArray &block, int delay = 50);
    void repeaterFileWrite(const QString &filename, const QByteArray &content, int delay = 50);
    void repeaterFileRead(const QString &filename, int delay = 50);
    void powerStatePollInit();

private:
    explicit BigeyeLite(QObject *parent = nullptr);
    Q_DISABLE_COPY(BigeyeLite)

private:
    static BigeyeLite *self;
    BigeyeLinker *linker;
    LinkStatus m_linkStatus = Disconnected;
    PowerState m_powerState = PowerUnknown;

    QTimer *handshakeTimer;
    int handshakeSequence = 0;

    static QList<QPair<QString, QString>> initSequence;

    QTimer *sequenceBlockTimer;
    QQueue<QPair<QByteArray, int>> sequenceBlock;
    bool sequenceBlockTimerRunning = false;
};

#endif // BIGEYELITE_H
