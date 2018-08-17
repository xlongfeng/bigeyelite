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

#ifndef BIGEYE_H
#define BIGEYE_H

#include <QObject>

class Bigeye : public QObject
{
    Q_OBJECT

public:
    explicit Bigeye(QObject *parent = nullptr);

protected:
    QString getDeviceType() const
    {
        return deviceType;
    }
    int getFramebufferWidth() const
    {
        return framebufferWidth;
    }
    int getFramebufferHeight() const
    {
        return framebufferHeight;
    }
    int getFramebufferBitDepth() const
    {
        return framebufferBitDepth;
    }
    int getFramebufferLength() const
    {
        return framebufferLength;
    }
    QByteArray& getFramebuffer();

    void setExtendDataSize(int size)
    {
        extendedDataSize = size;
    }

    virtual void defaultDispose(const QString &, QDataStream &)
    {

    }

    void dispose(const QByteArray &bytes);
    QByteArray escape(const QByteArray &data);
    QByteArray unescape(const QByteArray &data);

private:
    void getDeviceInfo();

private:
    QByteArray datagram;
    int extendedDataSize;

    QString deviceType;

    int framebufferFd;
    int framebufferWidth;
    int framebufferHeight;
    int framebufferBitDepth;
    int framebufferLength;
    QByteArray framebuffer;
};

#endif // BIGEYE_H
