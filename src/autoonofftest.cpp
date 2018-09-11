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

#include <QTimer>
#include <QDebug>

#include "autoonofftest.h"

AutoOnOffTest::AutoOnOffTest(QObject *parent) :
    AbstractTest(parent)
{
    tickTimer = new QTimer(this);
    connect(tickTimer, SIGNAL(timeout()), this, SLOT(onTick()));

    QState *powerCheckState = new QState(runState);
    QState *powerOffState = new QState(runState);
    QState *powerOnState = new QState(runState);

    powerOffCountdownTimer = new QTimer(this);
    powerOffCountdownTimer->setSingleShot(true);
    powerOnCountdownTimer = new QTimer(this);
    powerOnCountdownTimer->setSingleShot(true);

    powerCheckState->addTransition(powerOffCountdownTimer, SIGNAL(timeout()), powerOnState);
    powerCheckState->addTransition(powerOnCountdownTimer, SIGNAL(timeout()), powerOffState);
    powerOffState->addTransition(controller, SIGNAL(powerStateChanged()), powerCheckState);
    powerOnState->addTransition(controller, SIGNAL(powerStateChanged()), powerCheckState);

    runState->setInitialState(powerCheckState);

    connect(powerCheckState, SIGNAL(entered()), this, SLOT(enterPowerCheckState()));
    connect(powerCheckState, SIGNAL(exited()), this, SLOT(exitPowerCheckState()));

    powerStateSwitchTimer = new QTimer(this);
    powerStateSwitchTimer->setSingleShot(true);
    connect(powerStateSwitchTimer, SIGNAL(timeout()), this, SLOT(onPowerStateSwitchTimeout()));

    connect(powerOffState, &QState::entered, [&] () {
        controller->debug("Enter power off state");
        validPowerState = PowerStateShouldBeOff;
        controller->powerButtonPress();
        controller->knobLeftRotate();
        controller->enterButtonPress();
        powerStateSwitchTimer->start(20 * 1000);
    });

    connect(powerOffState, &QState::exited, [&] () {
        controller->debug("Exit power off state");
        powerStateSwitchTimer->stop();
    });

    connect(powerOnState, &QState::entered, [&] () {
        controller->debug("Enter power on state");
        validPowerState = PowerStateShouldBeOn;
        controller->powerButtonPress();
        powerStateSwitchTimer->start(20 * 1000);
    });

    connect(powerOnState, &QState::exited, [&] () {
        controller->debug("Exit power on state");
        powerStateSwitchTimer->stop();
    });
}

void AutoOnOffTest::setPowerOffDuration(int day, int hour, int minute, int second)
{
    powerOffSecond = ((day * 24 + hour) * 60 + minute) * 60 + second;
    powerOffSecond = powerOffSecond < 5 ? 5 : powerOffSecond;
}

void AutoOnOffTest::setPowerOnDuration(int day, int hour, int minute, int second)
{
    powerOnSecond = ((day * 24 + hour) * 60 + minute) * 60 + second;
    powerOnSecond = powerOnSecond < 30 ? 30 : powerOnSecond;
}

const QString AutoOnOffTest::powerOffCountdown() const
{
    int tmp = powerOffSecondDecreasing;
    int hour, minute, second;
    hour = tmp / 3600;
    tmp %= 3600;
    minute = tmp / 60;
    second = tmp % 60;
    return QString("%1:%2:%3").arg(hour, 3, 10, QChar('0')).arg(minute, 2, 10, QChar('0')).arg(second, 2, 10, QChar('0'));
}

const QString AutoOnOffTest::powerOnCountdown() const
{
    int tmp = powerOnSecondDecreasing;
    int hour, minute, second;
    hour = tmp / 3600;
    tmp %= 3600;
    minute = tmp / 60;
    second = tmp % 60;
    return QString("%1:%2:%3").arg(hour, 3, 10, QChar('0')).arg(minute, 2, 10, QChar('0')).arg(second, 2, 10, QChar('0'));
}

void AutoOnOffTest::start()
{
    tickTimer->start(1000);
    setPowerOnCount(0);
    validPowerState = PowerStateNotCare;
    AbstractTest::start();
    controller->info("Auto On-Off test start");
}

void AutoOnOffTest::stop()
{
    tickTimer->stop();
    AbstractTest::stop();
    controller->info("Auto On-Off test stop");
}

void AutoOnOffTest::enterFinal()
{
}

void AutoOnOffTest::onTick()
{
    if (powerOffSecondDecreasing > 0) {
        --powerOffSecondDecreasing;
        emit powerOffCountdownChanged();
    }

    if (powerOnSecondDecreasing > 0) {
        --powerOnSecondDecreasing;
        emit powerOnCountdownChanged();
    }
}

void AutoOnOffTest::enterPowerCheckState()
{
    controller->debug("Enter power check state: validPowerState <%d> powerState <%d>", validPowerState, controller->powerState());
    connect(controller, SIGNAL(powerStateChanged()), this, SLOT(onPowerStateException()));
    if (validPowerState == PowerStateShouldBeOff) {
        if (controller->powerState() == BigeyeLite::PowerOff) {
            controller->info("Device power off");
            powerOffCountdownTimer->start(powerOffSecond * 1000);
            powerOffSecondDecreasing = powerOffSecond;
            powerOnSecondDecreasing = 0;
            emit powerOffCountdownChanged();
            emit powerOnCountdownChanged();
        } else {
            controller->fatal("Power state should be off");
            stop();
        }
    } else if (validPowerState == PowerStateShouldBeOn) {
        if (controller->powerState() == BigeyeLite::PowerOn) {
            controller->info("Device power on");
            powerOnCountdownTimer->start(powerOnSecond * 1000);
            powerOnSecondDecreasing = powerOnSecond;
            powerOffSecondDecreasing = 0;
            emit powerOffCountdownChanged();
            emit powerOnCountdownChanged();
            powerOnCountIncrease();
        } else {
            controller->fatal("Power state should be on");
            stop();
        }
    } else { /* validPowerState == PowerStateNotCare */
        switch (controller->powerState()) {
        case BigeyeLite::PowerOff:
            controller->info("Device power off");
            powerOffCountdownTimer->start(powerOffSecond * 1000);
            powerOffSecondDecreasing = powerOffSecond;
            powerOnSecondDecreasing = 0;
            emit powerOffCountdownChanged();
            emit powerOnCountdownChanged();
            break;
        case BigeyeLite::PowerOn:
            controller->info("Device power on");
            powerOnCountdownTimer->start(powerOnSecond * 1000);
            powerOnSecondDecreasing = powerOnSecond;
            powerOffSecondDecreasing = 0;
            emit powerOffCountdownChanged();
            emit powerOnCountdownChanged();
            break;
        default:
            controller->fatal("Power state unknown");
            stop();
            break;
        }
    }
}

void AutoOnOffTest::exitPowerCheckState()
{
    disconnect(controller, SIGNAL(powerStateChanged()), this, SLOT(onPowerStateException()));
}

void AutoOnOffTest::onPowerStateException()
{
    if (controller->powerState() == BigeyeLite::PowerOff)
        controller->fatal("Abnormal power off");
    else if (controller->powerState() == BigeyeLite::PowerOn)
        controller->fatal("Abnormal power on");
    stop();
}

void AutoOnOffTest::onPowerStateSwitchTimeout()
{
    if (controller->powerState() == BigeyeLite::PowerOff)
        controller->fatal("Unable to power on");
    else if (controller->powerState() == BigeyeLite::PowerOn)
        controller->fatal("Unable to power off");
    stop();
}
