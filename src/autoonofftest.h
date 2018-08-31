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

#ifndef AUTOONOFFTEST_H
#define AUTOONOFFTEST_H

#include <QDateTime>

#include "abstracttest.h"

class QTimer;

class AutoOnOffTest : public AbstractTest
{
    Q_OBJECT

    Q_PROPERTY(QString powerOffCountdown READ powerOffCountdown MEMBER m_powerOffCountdown NOTIFY powerOffCountdownChanged)
    Q_PROPERTY(QString powerOnCountdown READ powerOnCountdown MEMBER m_powerOnCountdown NOTIFY powerOnCountdownChanged)
    Q_PROPERTY(int powerOnCount READ powerOnCount MEMBER m_powerOnCount WRITE setPowerOnCount NOTIFY powerOnCountChanged)

public:
    enum ValidPowerState {
        PowerStateNotCare,
        PowerStateShouldBeOff,
        PowerStateShouldBeOn,
    };
    Q_ENUM(ValidPowerState)

    explicit AutoOnOffTest(QObject *parent = nullptr);

    Q_INVOKABLE void setPowerOffDuration(int day, int hour, int minute, int second);
    Q_INVOKABLE void setPowerOnDuration(int day, int hour, int minute, int second);

    const QString powerOffCountdown() const;
    const QString powerOnCountdown() const;

    int powerOnCount() const
    {
        return m_powerOnCount;
    }

    void setPowerOnCount(int count)
    {
        m_powerOnCount = count;
        emit powerOnCountChanged();
    }

signals:
    void powerOffCountdownChanged();
    void powerOnCountdownChanged();
    void powerOnCountChanged();

public slots:
    virtual void start() override;
    virtual void stop() override;
    virtual void enterFinal() override;

private slots:
    void onTick();
    void enterPowerCheckState();
    void exitPowerCheckState();
    void onPowerStateException();
    void onPowerStateSwitchTimeout();

private:
    void powerOnCountIncrease()
    {
        ++m_powerOnCount;
         emit powerOnCountChanged();
    }

private:
    int powerOnSecond = 0, powerOnSecondDecreasing = 0;
    int powerOffSecond = 0, powerOffSecondDecreasing = 0;
    QTimer *tickTimer;

    QString m_powerOffCountdown;
    QString m_powerOnCountdown;
    int m_powerOnCount = 0;

    ValidPowerState validPowerState = PowerStateNotCare;
    QTimer *powerOffCountdownTimer;
    QTimer *powerOnCountdownTimer;
    QTimer *powerStateSwitchTimer;
};

#endif // AUTOONOFFTEST_H
