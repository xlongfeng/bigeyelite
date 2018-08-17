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
#include <QCoreApplication>
#include <QDataStream>
#include <QTimer>

#include "bigeyelinker.h"
#include "bigeyelite.h"

BigeyeLite *BigeyeLite::self = nullptr;

QList<QPair<QString, QString>> BigeyeLite::initSequence = {
    /* Power button */
    {"/sys/class/gpio/export", "48"},
    {"/sys/class/gpio/gpio48/direction", "high"},

    /* Unused */
    {"/sys/class/gpio/export", "49"},

    /* Knob trigger */
    {"/sys/class/gpio/export", "50"},
    {"/sys/class/gpio/gpio50/direction", "low"},

    /* Enter Button */
    {"/sys/class/gpio/export", "51"},
    {"/sys/class/gpio/gpio51/direction", "high"},

    /* Power Detect */
    {"/sys/class/gpio/export", "52"},

    /* Knob value */
    {"/sys/class/gpio/export", "53"},
    {"/sys/class/gpio/gpio53/direction", "high"},
};

BigeyeLite::BigeyeLite(QObject *parent) :
    Bigeye(parent),
    m_linkStatus(Disconnected)
{
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
    linker = new BigeyeLinker(this);
    connect(linker, SIGNAL(deviceAttached()), this, SLOT(onDeviceAttached()));
    connect(linker, SIGNAL(deviceDetached()), this, SLOT(onDeviceDetached()));
    connect(linker, SIGNAL(dataArrived(QByteArray)), this, SLOT(onDataArrived(QByteArray)));

    sequenceBlockTimer = new QTimer(this);
    sequenceBlockTimer->setSingleShot(true);
    connect(sequenceBlockTimer, SIGNAL(timeout()), this, SLOT(onTransmitSequence()));

    linker->start();
}

BigeyeLite::~BigeyeLite()
{
    linker->requestSafeExit();
}

BigeyeLite *BigeyeLite::instance()
{
    if (self == nullptr) {
        self = new BigeyeLite();
    }

    return self;
}

void BigeyeLite::onDeviceAttached()
{
    for (auto p: initSequence) {
        repeaterFileWrite(p.first, p.second.toLatin1());
    }

    setLinkStatus(Connected);
}

void BigeyeLite::onDeviceDetached()
{
    sequenceBlockTimer->stop();
    setLinkStatus(Disconnected);
}

void BigeyeLite::onTransmitSequence()
{
    if (!sequenceBlock.isEmpty()) {
        auto p = sequenceBlock.dequeue();
        linker->tramsmitBytes(escape(p.first));
        sequenceBlockTimer->start(p.second);
    }
}

void BigeyeLite::onDataArrived(const QByteArray &bytes)
{
    dispose(bytes);
}

void BigeyeLite::powerButtonPress()
{
    repeaterFileWrite("/sys/class/gpio/gpio48/value", "0", 1500);
    repeaterFileWrite("/sys/class/gpio/gpio48/value", "1");
}

void BigeyeLite::knobLeftRotate()
{
    repeaterFileWrite("/sys/class/gpio/gpio53/value", "1");
    repeaterFileWrite("/sys/class/gpio/gpio50/value", "1");
    repeaterFileWrite("/sys/class/gpio/gpio50/value", "0");
}

void BigeyeLite::knobRightRotate()
{
    repeaterFileWrite("/sys/class/gpio/gpio53/value", "0");
    repeaterFileWrite("/sys/class/gpio/gpio50/value", "1");
    repeaterFileWrite("/sys/class/gpio/gpio50/value", "0");
}

void BigeyeLite::enterButtonPress()
{
    repeaterFileWrite("/sys/class/gpio/gpio51/value", "0");
    repeaterFileWrite("/sys/class/gpio/gpio51/value", "1");
}

void BigeyeLite::runningStateQuery()
{

}

void BigeyeLite::repeaterFileWrite(const QString &filename, const QByteArray &content, int delay)
{
    QByteArray block;
    QDataStream istream(&block, QIODevice::WriteOnly);
    istream.setVersion(QDataStream::Qt_4_8);
    istream << QString("Bigeye");
    istream << QString("repeaterFileWrite");
    istream << filename
            << content
            << false;

    sequenceBlock.enqueue(qMakePair(block, delay));

    if (!sequenceBlockTimer->isActive())
        sequenceBlockTimer->start(10);
}

void BigeyeLite::start()
{
    qDebug() << "start";
    powerButtonPress();
    knobLeftRotate();
    knobRightRotate();
    enterButtonPress();
}

void BigeyeLite::stop()
{
    qDebug() << "stop";
}
