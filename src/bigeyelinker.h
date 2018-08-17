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

#ifndef BIGEYELINKER_H
#define BIGEYELINKER_H

#include <libusb-1.0/libusb.h>

#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QList>
#include <QTime>
#include <QDebug>


class BigeyeLinker: public QThread
{
    Q_OBJECT

public:
    explicit BigeyeLinker(QObject *parent = nullptr);
    ~BigeyeLinker();

    bool isSafeExitRequested() const
    {
        return interrupt;
    }
    void clearSafeExitRequested()
    {
        interrupt = false;
    }

    void requestSafeExit()
    {
        interrupt = true;
    }

    void stop();

    void enqueueReceiveBytes(const QByteArray &bytes);
    QByteArray dequeueReceiveBytes();

    void tramsmitBytes(const QByteArray &bytes);

    bool discardPrecedingData();

signals:
    void dataArrived(const QByteArray &bytes);

protected:
    virtual void run();

private:
    void findDevice();
    void startReceive();
    void stopReceive();
    static void transmitTransferCallback(libusb_transfer *transfer);
    static void receiveTransferCallback(libusb_transfer *transfer);
    static int hotplugCallback(libusb_context *ctx, libusb_device *dev,
                                libusb_hotplug_event event, void *user_data);
    static void dumpUsbInformation(libusb_device *dev);

private:
    bool interrupt;

    class USBTransferBlock {
    public:
        USBTransferBlock(BigeyeLinker *parent, const QByteArray &bytes) :
            parent(parent),
            transfer(Q_NULLPTR)
        {
            init(bytes);
            alloc();
        }

        ~USBTransferBlock()
        {
            free();
        }

        static USBTransferBlock *create(BigeyeLinker *parent, const QByteArray &bytes)
        {
            USBTransferBlock *block;
            if (transferBlockFreeList.isEmpty()) {
                block = new USBTransferBlock(parent, bytes);
                qDebug() << "new transfer block";
            } else {
                block = transferBlockFreeList.takeFirst();
                block->init(bytes);
            }
            return block;
        }

        static USBTransferBlock *create(BigeyeLinker *parent, int length = DefaultBufferSize)
        {
            QByteArray bytes;
            bytes.resize(length);
            return create(parent, bytes);
        }

        static void destroyAll()
        {
            qDeleteAll(transferBlockFreeList);
        }

        void reclaim()
        {
            transferBlockFreeList.append(this);
        }

        void alloc()
        {
            transfer = libusb_alloc_transfer(0);
            if (transfer == Q_NULLPTR) {
            }
        }

        void free()
        {
            if (transfer != Q_NULLPTR) {
                libusb_free_transfer(transfer);
                transfer = Q_NULLPTR;
            }
        }

        void init(const QByteArray &bytes)
        {
            buffer = bytes;
            data = (unsigned char *)buffer.data();
            size = buffer.size();
        }

        void fillBulk(libusb_device_handle *dev_handle, unsigned char endpoint,
                      libusb_transfer_cb_fn callback, unsigned int timeout=0)
        {
            libusb_fill_bulk_transfer(transfer, dev_handle, endpoint, data, size, callback, this, timeout);
        }

        void submit()
        {
            libusb_submit_transfer(transfer);
        }

        void cancel()
        {
            libusb_cancel_transfer(transfer);
        }

        static QList<USBTransferBlock *> transferBlockFreeList;

        BigeyeLinker *parent;
        QByteArray buffer;
        unsigned char *data;
        int size;
        libusb_transfer *transfer;
        static const int DefaultBufferSize = 16384;
    };

    QQueue<QByteArray> receiveQueue;

    USBTransferBlock *pingReceiveBlock;
    USBTransferBlock *pongReceiveBlock;

    libusb_context *ctx;
    libusb_device_handle *deviceHandle;
    bool hasHotplug;
    libusb_hotplug_event deviceStatus;
    libusb_hotplug_callback_handle hotplug;

    QTime discardTime;
    bool discarding;
};

#endif // BIGEYELINKER_H
