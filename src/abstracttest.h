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

#ifndef ABSTRACTTEST_H
#define ABSTRACTTEST_H

#include <QObject>
#include <QStateMachine>
#include <QState>
#include <QFinalState>

#include "bigeyelite.h"

class AbstractTest : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool running READ running MEMBER m_running WRITE setRunning NOTIFY runningChanged)

public:
    explicit AbstractTest(QObject *parent = nullptr);

    bool running()
    {
        return m_running;
    }

    void setRunning(bool flag)
    {
        m_running = flag;
        emit runningChanged();
    }

public slots:
    virtual void start();
    virtual void stop();
    virtual void enterInit();
    virtual void enterRun();
    virtual void enterFinal();

signals:
    void runningChanged();
    void started();
    void stopped();
    void finished();

protected:
    bool m_running = false;

    QStateMachine *machine;
    QState *initState;
    QState *runState;
    QFinalState *finishState;

    BigeyeLite *controller;
};

#endif // ABSTRACTTEST_H
