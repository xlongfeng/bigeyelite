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

#include "simpleonofftest.h"

SimpleOnOffTest::SimpleOnOffTest(QObject *parent) :
    AbstractTest(parent)
{
    QState *poweronState = new QState(runState);
    QState *poweroffState = new QState(runState);

    powerSwitchTimer = new QTimer(this);
    powerSwitchTimer->setSingleShot(true);

    poweronState->addTransition(powerSwitchTimer, SIGNAL(timeout()), poweroffState);
    poweroffState->addTransition(powerSwitchTimer, SIGNAL(timeout()), poweronState);

    runState->setInitialState(poweronState);

    connect(poweronState, &QState::entered, [=] () {
        qDebug() << "enter poweron state";
        powerSwitchTimer->start(3000);
        controller->powerButtonPress();
    });

    connect(poweroffState, &QState::entered, [=] () {
        qDebug() << "enter poweroff state";
        powerSwitchTimer->start(3000);
        controller->powerButtonPress();
    });
}
