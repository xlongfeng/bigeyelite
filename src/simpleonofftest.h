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

#ifndef SIMPLEONOFFTEST_H
#define SIMPLEONOFFTEST_H

#include "abstracttest.h"

class QTimer;

class SimpleOnOffTest : public AbstractTest
{
    Q_OBJECT

public:
    explicit SimpleOnOffTest(QObject *parent = nullptr);

private:
    QTimer *powerSwitchTimer;
};

#endif // SIMPLEONOFFTEST_H
