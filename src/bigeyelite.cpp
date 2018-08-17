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

#include <QDebug>

#include "bigeyelinker.h"
#include "bigeyelite.h"

BigeyeLite *BigeyeLite::self = nullptr;

BigeyeLite::BigeyeLite(QObject *parent) :
    QObject(parent)
{
    linker = new BigeyeLinker(this);
    linker->start();
}

BigeyeLite *BigeyeLite::instance()
{
    if (self == nullptr) {
        self = new BigeyeLite();
    }

    return self;
}

void BigeyeLite::start()
{
    qDebug() << "start";
}

void BigeyeLite::stop()
{
    qDebug() << "stop";
}