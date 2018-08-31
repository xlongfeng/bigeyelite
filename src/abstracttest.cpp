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

#include "abstracttest.h"

AbstractTest::AbstractTest(QObject *parent) :
    QObject(parent)
{
    machine = new QStateMachine(this);
    initState = new QState();
    runState = new QState();
    finishState = new QFinalState();

    controller = BigeyeLite::instance();

    initState->addTransition(this, SIGNAL(started()), runState);
    initState->addTransition(this, SIGNAL(stopped()), finishState);
    runState->addTransition(this, SIGNAL(stopped()), finishState);

    machine->addState(initState);
    machine->addState(runState);
    machine->addState(finishState);
    machine->setInitialState(initState);

    connect(machine, SIGNAL(finished()), this, SIGNAL(finished()));
    connect(initState, SIGNAL(entered()), this, SLOT(enterInit()));
    connect(runState, SIGNAL(entered()), this, SLOT(enterRun()));
    connect(finishState, SIGNAL(entered()), this, SLOT(enterFinal()));
}

void AbstractTest::start()
{
    machine->start();
    setRunning(true);
}

void AbstractTest::stop()
{
    emit stopped();
    setRunning(false);
}

void AbstractTest::enterInit()
{
    emit started();
}

void AbstractTest::enterRun()
{
}

void AbstractTest::enterFinal()
{
}
