/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "pluginsidecar.h"

#include <QDebug>

/*!
 * \class PluginSidecar
 * \brief PluginSidecar is wrapper for a series of D-Bus intefaces
 */

PluginSidecar &PluginSidecar::instance()
{
    static PluginSidecar ins;
    return ins;
}

QPointer<DeviceManagerInterface> PluginSidecar::getDeviceInterface()
{
    return deviceInterface.data();
}

bool PluginSidecar::connectToServer()
{
    qInfo() << "Start initilize dbus: `DeviceManagerInterface`";
    // Note: the plugin depends on `dde-file-manager-server`!
    // the plugin will not work if `dde-file-manager-server` not run.
    deviceInterface.reset(new DeviceManagerInterface(
                          "com.deepin.filemanager.service",
                          "/com/deepin/filemanager/service/DeviceManager",
                          QDBusConnection::sessionBus(),
                          this
                          ));

    if (!deviceInterface->isValid()) {
        qCritical() << "DeviceManagerInterface cannot link!!!";
        deviceInterface->deleteLater();
        deviceInterface.reset(nullptr);
        return false;
    }

    initConnection();

    qInfo() << "Finish initilize dbus: `DeviceManagerInterface`";
    return true;
}

void PluginSidecar::initConnection()
{
    // hanlder server signals
    // method refrecent to `DiskControlWidget::onAskStopScanning`
    connect(deviceInterface.data(), &DeviceManagerInterface::AskStopScanningWhenDetach, this, [this](const QString &id) {
        emit askStopScanning("detach", id);
    });

    connect(deviceInterface.data(), &DeviceManagerInterface::AskStopScanningWhenDetachAll, this, [this]() {
        emit askStopScanning("detach_all", "");
    });

    connect(deviceInterface.data(), &DeviceManagerInterface::AskStopSacnningWhenUnmount, this, [this](const QString &id) {
        emit askStopScanning("unmount", id);
    });

    // monitor server status
    watcher.reset(new QDBusServiceWatcher("com.deepin.filemanager.service", deviceInterface->connection()));
    connect(watcher.data(), &QDBusServiceWatcher::serviceUnregistered, this, &PluginSidecar::serviceUnregistered);
    connect(watcher.data(), &QDBusServiceWatcher::serviceRegistered, this, &PluginSidecar::serviceRegistered);

}

void PluginSidecar::invokeDetachAllMountedDevices()
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->DetachAllMountedDevices();
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid ";
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

void PluginSidecar::invokeDetachAllMountedDevicesForced()
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->DetachAllMountedDevicesForced();
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid ";
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

bool PluginSidecar::invokeIsMonotorWorking()
{
    bool ret = false;
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->IsMonotorWorking();
        reply.waitForFinished();
        if (reply.isValid() && reply.value())
            ret = true;
        else
            qCritical() << "D-Bus reply is invalid ";
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
    return ret;
}

QStringList PluginSidecar::invokeBlockDevicesIdList(const QVariantMap &opt)
{
    QStringList ret;

    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->GetBlockDevicesIdList(opt);
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else
            qCritical() << "D-Bus reply is invalid ";
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }

    return ret;
}

QStringList PluginSidecar::invokeProtolcolDevicesIdList(const QVariantMap &opt)
{
    QStringList ret;

    // TODO(zhangs): opt
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->GetProtolcolDevicesIdList();
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else
            qCritical() << "D-Bus reply is invalid ";
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }

    return ret;
}

QVariantMap PluginSidecar::invokeQueryBlockDeviceInfo(const QString &id)
{
    QVariantMap ret;
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->QueryBlockDeviceInfo(id, false);
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else
            qCritical() << "D-Bus reply is invalid ";
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
    return ret;
}

QVariantMap PluginSidecar::invokeQueryProtocolDeviceInfo(const QString &id)
{
    QVariantMap ret;
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->QueryProtocolDeviceInfo(id, false);
        reply.waitForFinished();
        if (reply.isValid())
            ret = reply.value();
        else
            qCritical() << "D-Bus reply is invalid ";
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
    return ret;
}

void PluginSidecar::invokeDetachBlockDevice(const QString &id)
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->DetachBlockDevice(id);
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid ";
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

void PluginSidecar::invokeDetachBlockDeviceForced(const QString &id)
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->DetachBlockDeviceForced(id);
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid ";
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

void PluginSidecar::invokeUnmountBlockDeviceForced(const QString &id)
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->UnmountBlockDeviceForced(id);
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid ";
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

void PluginSidecar::invokeDetachProtocolDevice(const QString &id)
{
    if (deviceInterface) {
        qInfo() << "Start call dbus: " << __PRETTY_FUNCTION__;
        auto &&reply = deviceInterface->DetachProtocolDevice(id);
        if (!reply.isValid())
            qCritical() << "D-Bus reply is invalid ";
        qInfo() << "End call dbus: " << __PRETTY_FUNCTION__;
    }
}

PluginSidecar::PluginSidecar(QObject *parent)
    : QObject(parent)
{

}

PluginSidecar::~PluginSidecar()
{

}
