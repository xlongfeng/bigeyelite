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

#include "bigeyelinker.h"

QList<BigeyeLinker::USBTransferBlock *> BigeyeLinker::USBTransferBlock::transferBlockFreeList;

BigeyeLinker::BigeyeLinker(QObject *parent) : QThread(parent),
    interrupt(false),
    deviceStatus(LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
    discarding(true)
{
    int rc = libusb_init(&ctx);
    if (LIBUSB_SUCCESS != rc) {
        qDebug() << "libusb_init" << libusb_error_name(rc);
    }

    pingReceiveBlock = USBTransferBlock::create(this);
    pongReceiveBlock = USBTransferBlock::create(this);
}

BigeyeLinker::~BigeyeLinker()
{
    libusb_exit(ctx);

    USBTransferBlock::destroyAll();
}

void BigeyeLinker::stop()
{
    requestSafeExit();
    libusb_hotplug_deregister_callback(ctx, hotplug);
    wait();
}

void BigeyeLinker::enqueueReceiveBytes(const QByteArray &bytes)
{
    receiveQueue.enqueue(bytes);
}

QByteArray BigeyeLinker::dequeueReceiveBytes()
{
    if (receiveQueue.isEmpty())
        return QByteArray();

    return receiveQueue.dequeue();
}

void BigeyeLinker::tramsmitBytes(const QByteArray &bytes)
{
    // qDebug() << "tramsmitBytes" << bytes.size();
    USBTransferBlock *transmitBlock = USBTransferBlock::create(this, bytes);
    transmitBlock->fillBulk(deviceHandle, 1, transmitTransferCallback);
    transmitBlock->submit();
}

bool BigeyeLinker::discardPrecedingData()
{
    if (discarding) {
         if (discardTime.elapsed() > 1000)
             discarding = false;
         else {
             return true;
         }
    }
    return false;
}

void BigeyeLinker::run()
{
    hasHotplug = libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG) != 0;
    qDebug() << "BigeyeLinker" << "hotplug"
             << (hasHotplug ? "support" : "unsupport");

    clearSafeExitRequested();

    if (hasHotplug) {
        int rc = libusb_hotplug_register_callback(
                    ctx, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
                    LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
                    (libusb_hotplug_flag)LIBUSB_HOTPLUG_ENUMERATE,
                    0x2009, 0x0805, LIBUSB_HOTPLUG_MATCH_ANY,
                    hotplugCallback, this, &hotplug);
        if (LIBUSB_SUCCESS != rc) {
            qDebug() << "libusb_init" << libusb_error_name(rc);
        }
    } else {

    }

    forever {
        if (isSafeExitRequested())
            break;
        if (!hasHotplug && deviceStatus == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
            findDevice();
            msleep(1500);
        } else {
            // qDebug() << "BigeyeLinker" << "handle";
            libusb_handle_events_completed(nullptr, nullptr);
        }
    }

    if (deviceHandle) {
        stopReceive();
        libusb_close(deviceHandle);
        deviceHandle = nullptr;
    }

    qDebug() << "BigeyeLinker" << "end";
}

void BigeyeLinker::findDevice()
{
    libusb_device **list;
    int count = libusb_get_device_list(ctx, &list);
    if (count > 0) {
        for (int i = 0; i < count; i++) {
            libusb_device *dev = list[i];
            libusb_device_descriptor deviceDescriptor;
            int rc = libusb_get_device_descriptor(dev, &deviceDescriptor);
            if (rc == LIBUSB_SUCCESS) {
                if (deviceDescriptor.idVendor == 0x2009 && deviceDescriptor.idProduct == 0x0805) {
                    hotplugCallback(ctx, dev, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, this);
                    break;
                }
            } else {
                qDebug() << "libusb_get_device_descriptor failed: " << rc << i;
            }
        }
        libusb_free_device_list(list, 1);
    }
}

void BigeyeLinker::startReceive()
{
    if (libusb_kernel_driver_active(deviceHandle, 0) == 1)
        libusb_detach_kernel_driver(deviceHandle, 0);
    libusb_claim_interface(deviceHandle, 0);

    discarding = true;
    discardTime.restart();

    pingReceiveBlock->fillBulk(deviceHandle, 129, receiveTransferCallback);
    pingReceiveBlock->submit();

    pongReceiveBlock->fillBulk(deviceHandle, 129, receiveTransferCallback);
    pongReceiveBlock->submit();
}

void BigeyeLinker::stopReceive()
{
    pongReceiveBlock->cancel();
    pingReceiveBlock->cancel();
    libusb_release_interface(deviceHandle, 0);
}

void BigeyeLinker::transmitTransferCallback(libusb_transfer *transfer)
{
    USBTransferBlock *usbTransferBlock = (USBTransferBlock *)transfer->user_data;
    BigeyeLinker *self = (BigeyeLinker *)usbTransferBlock->parent;

    switch (transfer->status) {
    case LIBUSB_TRANSFER_COMPLETED:
        usbTransferBlock->reclaim();
        break;
    case LIBUSB_TRANSFER_STALL:
    case LIBUSB_TRANSFER_ERROR:
    case LIBUSB_TRANSFER_TIMED_OUT:
    case LIBUSB_TRANSFER_OVERFLOW:
        // break;
    case LIBUSB_TRANSFER_CANCELLED:
        qDebug() << "Transmit transfer callback" << transfer->status;
        break;
    case LIBUSB_TRANSFER_NO_DEVICE:
        hotplugCallback(self->ctx, nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, self);
        break;
    default:
        qDebug() << "Transmit transfer callback" << transfer->status;
        break;
    }
}

void BigeyeLinker::receiveTransferCallback(libusb_transfer *transfer)
{
    USBTransferBlock *usbTransferBlock = (USBTransferBlock *)transfer->user_data;
    BigeyeLinker *self = (BigeyeLinker *)usbTransferBlock->parent;
    const char *buffer = (const char *)transfer->buffer;

    switch (transfer->status) {
    case LIBUSB_TRANSFER_COMPLETED:
        if (!self->discardPrecedingData())
            self->dataArrived(QByteArray(buffer, transfer->actual_length));
        usbTransferBlock->submit();
        break;
    case LIBUSB_TRANSFER_STALL:
    case LIBUSB_TRANSFER_ERROR:
    case LIBUSB_TRANSFER_TIMED_OUT:
    case LIBUSB_TRANSFER_OVERFLOW:
        // break;
    case LIBUSB_TRANSFER_CANCELLED:
        qDebug() << "Receive transfer callback" << transfer->status;
        break;
    case LIBUSB_TRANSFER_NO_DEVICE:
        hotplugCallback(self->ctx, nullptr, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, self);
        break;
    default:
        qDebug() << "Receive transfer callback" << transfer->status;
        break;
    }
}

int BigeyeLinker::hotplugCallback(libusb_context *ctx, libusb_device *dev,
                            libusb_hotplug_event event, void *user_data)
{
    Q_UNUSED(ctx)
    BigeyeLinker *self = (BigeyeLinker *)user_data;
    int rc;

    if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
        msleep(1500);
        rc = libusb_open(dev, &self->deviceHandle);
        if (LIBUSB_SUCCESS != rc) {
            qDebug() << "libusb_open" << libusb_error_name(rc);
        } else {
            self->deviceStatus = LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED;
            dumpUsbInformation(dev);
            self->startReceive();
        }
    } else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
        self->deviceStatus = LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT;
        if (self->deviceHandle) {
            self->stopReceive();
            libusb_close(self->deviceHandle);
            self->deviceHandle = nullptr;
        }
        qDebug() << "libusb_close";
    } else {
        qDebug() << "Unhandled event" << event;
    }

    return 0;
}

void BigeyeLinker::dumpUsbInformation(libusb_device *dev)
{
    qDebug() << "Bigeye usb device attached";
    qDebug() << "Bus Number" << libusb_get_bus_number(dev);
    qDebug() << "Port Number" << libusb_get_port_number(dev);
    qDebug() << "Device Address" << libusb_get_device_address(dev);
    qDebug() << "Device Speed" << libusb_get_device_speed(dev);

    int rc;

    libusb_device_descriptor deviceDescriptor;
    rc = libusb_get_device_descriptor(dev, &deviceDescriptor);
    qDebug() << "Num of configurations" << deviceDescriptor.bNumConfigurations;

    libusb_config_descriptor *configDescriptor;
    rc = libusb_get_active_config_descriptor(dev, &configDescriptor);
    if (LIBUSB_SUCCESS != rc)
        return;

    qDebug() << "Active configuration value" << configDescriptor->bConfigurationValue;
    qDebug() << "Num of interfaces" << configDescriptor->bNumInterfaces;
    for (int i = 0; i < configDescriptor->bNumInterfaces; i++) {
        qDebug() << "Interface at" << i;
        const libusb_interface &interface = configDescriptor->interface[i];
        qDebug() << "  Num alsetting" << interface.num_altsetting;
        for (int j = 0; j < interface.num_altsetting; j++) {
            const libusb_interface_descriptor &interfaceDescriptor = interface.altsetting[j];
            qDebug() << "    Interface number" << interfaceDescriptor.bInterfaceNumber;
            qDebug() << "    Interface number endpoints" << interfaceDescriptor.bNumEndpoints;
            for (int k = 0; k < interfaceDescriptor.bNumEndpoints; k++) {
                const libusb_endpoint_descriptor &endpoint = interfaceDescriptor.endpoint[k];
                qDebug() << "      Endpoint address" << endpoint.bEndpointAddress;
                qDebug() << "      Endpoint attributes" << endpoint.bmAttributes;
                qDebug() << "      Endpoint max packet size" << endpoint.wMaxPacketSize;
            }
        }
    }

    libusb_free_config_descriptor(configDescriptor);
}
