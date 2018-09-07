/*
 * Bigeye - Accessorial Tool for Daily Test
 *
 * Copyright (c) 2017-2018, longfeng.xiao <xlongfeng@126.com>
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

#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#endif

#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

#include "bigeye.h"

Bigeye::Bigeye(QObject *parent) :
    QObject(parent)
{
    logger = new QFile("bigeyelogger.txt", this);
    logger->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
#ifdef __linux__
    getDeviceInfo();
#endif
}

void Bigeye::debug(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    QString buf = QString::vasprintf(msg, ap);
    va_end(ap);
    QTextStream out(logger);
    const QString &datetime = currentDateTime();
    out << datetime << " [DEBUG] " << buf;
    endl(out);
    emit log(LoggerDebug, datetime, buf);
}

void Bigeye::info(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    QString buf = QString::vasprintf(msg, ap);
    va_end(ap);
    QTextStream out(logger);
    const QString &datetime = currentDateTime();
    out << datetime << " [INFO] " << buf;
    endl(out);
    emit log(LoggerInfo, datetime, buf);
}

void Bigeye::warning(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    QString buf = QString::vasprintf(msg, ap);
    va_end(ap);
    QTextStream out(logger);
    const QString &datetime = currentDateTime();
    out << datetime << " [WARNING] " << buf;
    endl(out);
    emit log(LoggerWarning, datetime, buf);
}

void Bigeye::critical(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    QString buf = QString::vasprintf(msg, ap);
    va_end(ap);
    QTextStream out(logger);
    const QString &datetime = currentDateTime();
    out << datetime << " [CRITICAL] " << buf;
    endl(out);
    emit log(LoggerCritical, datetime, buf);
}

void Bigeye::fatal(const char *msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    QString buf = QString::vasprintf(msg, ap);
    va_end(ap);
    QTextStream out(logger);
    const QString &datetime = currentDateTime();
    out << datetime << " [FATAL] " << buf;
    endl(out);
    emit log(LoggerFatal, datetime, buf);
}

QString Bigeye::currentDateTime() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:MM:ss");
}

void Bigeye::dispose(const QByteArray &bytes)
{
    for (int i = 0; i < bytes.size(); i++) {
        char ch = bytes.at(i);
        if (extendedDataSize-- > 0)
            continue;
        if (ch == 0x7e) {
            if (datagram.size() > 0) {
                const QByteArray &block = unescape(datagram);
                QDataStream istream(block);
                istream.setVersion(QDataStream::Qt_4_8);
                QString magic;
                QString command;
                istream >> magic >> command;
                if (magic == "Bigeye") {
                    const QByteArray &method = QString("%1 (QDataStream &)").arg(command).toLatin1();
                    const char *methodSignature = method.constData();
                    if (metaObject()->indexOfSlot(QMetaObject::normalizedSignature(methodSignature)) != -1)
                        QMetaObject::invokeMethod(this, command.toLatin1().constData(), Q_ARG(QDataStream &, istream));
                    else
                        defaultDispose(command, istream);
                } else {
                    qDebug() << "Magic string error";
                }
                datagram.clear();
            }
        } else {
            datagram.append(ch);
            if (datagram.size() > 1024 * 1024 * 10) {
                datagram.clear();
            }
        }
    }
}

QByteArray Bigeye::escape(const QByteArray &data)
{
    QByteArray buf;
    buf.append(0x7e);
    for (int i = 0; i < data.size(); i++) {
        char ch = data.at(i);
        if (ch == 0x7e) {
            buf.append(0x7d);
            buf.append(0x5e);
        } else if (ch == 0x7d) {
            buf.append(0x7d);
            buf.append(0x5d);
        } else {
            buf.append(ch);
        }
    }
    buf.append(0x7e);
    return buf;
}

QByteArray Bigeye::unescape(const QByteArray &data)
{
    QByteArray buf;
    bool isEscaped = false;

    for (int i = 0; i < data.size(); i++) {
        char ch = data.at(i);
        if (isEscaped) {
            isEscaped = false;
            if (ch == 0x5e) {
                buf.append(0x7e);
            } else if (ch == 0x5d) {
                buf.append(0x7d);
            } else {
                qDebug() << "Data crrupted";
            }
        } else {
            if (ch == 0x7d) {
                isEscaped = true;
            } else {
                buf.append(ch);
            }
        }
    }
    return buf;
}

#ifdef __linux__
QByteArray& Bigeye::getFramebuffer()
{
    lseek(framebufferFd, 0, SEEK_SET);
    read(framebufferFd, framebuffer.data(), framebuffer.size());
    return framebuffer;
}

void Bigeye::getDeviceInfo()
{
    deviceType = "unkonwn";
    QFile cmdline("/proc/cmdline");
    if (cmdline.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray data = cmdline.readAll();
        int start = data.indexOf("machine=");
        if (start != -1) {
            start += strlen("machine=");
            int end = data.indexOf(" ", start);
            deviceType = data.mid(start, end - start);
        }
        cmdline.close();
    }


    struct fb_var_screeninfo fb_varinfo;
    memset(&fb_varinfo, 0, sizeof(struct fb_var_screeninfo));

    const char *device = "/dev/fb0";
    framebufferFd = open(device, O_RDONLY);
    if (framebufferFd == -1) {
        qDebug() << "Error: Couldn't open" << device;
    }

    if (ioctl(framebufferFd, FBIOGET_VSCREENINFO, &fb_varinfo) != 0) {
        qDebug() << "ioctl FBIOGET_VSCREENINFO";
    }

    framebufferWidth = fb_varinfo.xres;
    framebufferHeight = fb_varinfo.yres;
    framebufferBitDepth = fb_varinfo.bits_per_pixel;
    framebufferLength = framebufferWidth * framebufferHeight * ((framebufferBitDepth + 7) >> 3);
    framebuffer.reserve(framebufferLength);
    framebuffer.resize(framebufferLength);
}
#endif
