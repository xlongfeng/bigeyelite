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
    Bigeye(parent)
{
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
    linker = new BigeyeLinker(this);
    connect(linker, SIGNAL(deviceAttached()), this, SLOT(onDeviceAttached()));
    connect(linker, SIGNAL(deviceDetached()), this, SLOT(onDeviceDetached()));
    connect(linker, SIGNAL(dataArrived(QByteArray)), this, SLOT(onDataArrived(QByteArray)));

    handshakeTimer = new QTimer(this);
    connect(handshakeTimer, SIGNAL(timeout()), this, SLOT(onHandshakeTimeout()));

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
    handshakeTimer->start(1000);
}

void BigeyeLite::onDeviceDetached()
{
    setPowerState(PowerUnknown);
    handshakeTimer->stop();
    sequenceBlockTimer->stop();
    setLinkStatus(Disconnected);
}

void BigeyeLite::onTransmitSequence()
{
    if (!sequenceBlock.isEmpty()) {
        auto p = sequenceBlock.dequeue();
        linker->tramsmitBytes(p.first);
        sequenceBlockTimer->start(p.second);
    } else {
        sequenceBlockTimerRunning = false;
    }
}

void BigeyeLite::onDataArrived(const QByteArray &bytes)
{
    dispose(bytes);
}

void BigeyeLite::onHandshakeTimeout()
{
    QByteArray block;
    QDataStream istream(&block, QIODevice::WriteOnly);
    istream.setVersion(QDataStream::Qt_4_8);
    istream << QString("Bigeye");
    istream << QString("handshake");
    istream << QString("repeater") << ++handshakeSequence;
    enqueueBlock(block);

    qDebug() << "handshake repeater" << handshakeSequence;
}

void BigeyeLite::respHandshake(QDataStream &stream)
{
    QString client;
    int sequence;
    stream >> client >> sequence;
    if (stream.status() == QDataStream::Ok
            && client == "repeater"
            && sequence == handshakeSequence) {
        handshakeTimer->stop();
        for (auto p: initSequence) {
            repeaterFileWrite(p.first, p.second.toLatin1());
        }
        setLinkStatus(Connected);
        powerStatePollInit();
    }
}

void BigeyeLite::respPowerStateChanged(QDataStream &stream)
{
    int tmp;
    PowerState state;
    stream >> tmp;
    state = static_cast<PowerState>(tmp);
    setPowerState(state);
}

void BigeyeLite::respRepeaterFileRead(QDataStream &stream)
{
    QString filename;
    QByteArray content;
    bool exist;

    stream >> filename >> content >> exist;
    if (stream.status() == QDataStream::Ok && exist) {
        if (filename == "/sys/class/gpio/gpio52/value") {
            if (content[0] == '1') {
                setPowerState(PowerOn);
            } else {
                setPowerState(PowerOff);
            }
        }
    }
}

void BigeyeLite::powerButtonPress()
{
    repeaterFileWrite("/sys/class/gpio/gpio48/value", "0", 1500);
    repeaterFileWrite("/sys/class/gpio/gpio48/value", "1", 500);
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

void BigeyeLite::enqueueBlock(const QByteArray &block, int delay)
{
    if (delay == 0) {
        linker->tramsmitBytes(escape(block));
        return;
    }

    if (sequenceBlockTimerRunning) {
        sequenceBlock.enqueue(qMakePair(escape(block), delay));
    } else {
        linker->tramsmitBytes(escape(block));
        sequenceBlockTimerRunning = true;
        sequenceBlockTimer->start(delay);
    }
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

    enqueueBlock(block, delay);
}

void BigeyeLite::repeaterFileRead(const QString &filename, int delay)
{
    QByteArray block;
    QDataStream istream(&block, QIODevice::WriteOnly);
    istream.setVersion(QDataStream::Qt_4_8);
    istream << QString("Bigeye");
    istream << QString("repeaterFileRead");
    istream << filename;

    enqueueBlock(block, delay);
}

void BigeyeLite::powerStatePollInit()
{
    QByteArray block;
    QDataStream istream(&block, QIODevice::WriteOnly);
    istream.setVersion(QDataStream::Qt_4_8);
    istream << QString("Bigeye");
    istream << QString("powerStatePollInit");
    istream << QString("/sys/class/gpio/gpio52/value");
    istream << reinterpret_cast<int>(100);

    enqueueBlock(block);
}
