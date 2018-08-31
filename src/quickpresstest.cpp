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

#include "quickpresstest.h"

QuickPressTest::QuickPressTest(QObject *parent) :
    AbstractTest(parent)
{
    QState *leftRotateState = new QState(runState);
    QState *rightRotateState = new QState(runState);

    pressSwitchTimer = new QTimer(this);
    pressSwitchTimer->setSingleShot(true);

    leftRotateState->addTransition(pressSwitchTimer, SIGNAL(timeout()), rightRotateState);
    rightRotateState->addTransition(pressSwitchTimer, SIGNAL(timeout()), leftRotateState);

    runState->setInitialState(leftRotateState);

    connect(leftRotateState, &QState::entered, [=] () {
        pressSwitchTimer->start(500);
        controller->knobLeftRotate();
    });

    connect(rightRotateState, &QState::entered, [=] () {
        pressSwitchTimer->start(500);
        controller->knobRightRotate();
    });
}
