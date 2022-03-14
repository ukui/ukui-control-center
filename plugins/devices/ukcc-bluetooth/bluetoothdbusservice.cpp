#include "bluetoothdbusservice.h"

QDBusInterface BlueToothDBusService::interface(DBUSSERVICE,DBUSPATH,DBUSINTERFACE);

BlueToothDBusService::BlueToothDBusService(QObject *parent)
{

}

BlueToothDBusService::~BlueToothDBusService()
{

}

bool BlueToothDBusService::getDefaultAdapterPower()
{
    QDBusReply<bool> reply = interface.call("getDefaultAdapterPower");
    if (reply.isValid()) {
        return reply.value();
    } else {
        return false;
    }
}

bool BlueToothDBusService::getBluetoothBlock()
{
    QDBusReply<bool> reply = interface.call("getBluetoothBlock");
    if (reply.isValid()) {
        return reply.value();
    } else {
        return false;
    }
}

bool BlueToothDBusService::getAdapterDiscoverable()
{
    QDBusReply<bool> reply = interface.call("getDefaultAdapterDiscoverable");
    if (reply.isValid()) {
        return reply.value();
    } else {
        return false;
    }
}

QStringList BlueToothDBusService::getAdapterList()
{
    QDBusReply<QStringList> reply = interface.call("getAdapterDevAddressList");
    if (reply.isValid()) {
        return reply.value();
    } else {
        return QStringList();
    }
}

QString BlueToothDBusService::getDefaultAdapterAddr()
{
    QDBusReply<QString> reply = interface.call("getDefaultAdapterAddress");
    if (reply.isValid()) {
        return reply.value();
    } else {
        return "";
    }
}

QString BlueToothDBusService::getAdapterNameByAddr(QString addr)
{
    QDBusReply<QString> reply = interface.call("getAdapterNameByAddr",addr);
    if (reply.isValid()) {
        return reply.value();
    } else {
        return "";
    }
}

QString BlueToothDBusService::getDevNameByAddr(QString addr)
{
    QDBusReply<QString> reply = interface.call("getDevName",addr);
    if (reply.isValid()) {
        return reply.value();
    } else {
        return "";
    }
}

void BlueToothDBusService::setAdapterPower(bool v)
{
    interface.call("setDefaultAdapterPower",v);
}

void BlueToothDBusService::setAdapterDiscoverable(bool v)
{
    interface.call("setDefaultAdapterDiscoverable",v);
}

bool BlueToothDBusService::getDevPairedByAddr(QString addr)
{
    QDBusReply<bool> reply = interface.call("getDevPairStatus",addr);
    if (reply.isValid()) {
        return reply.value();
    } else {
        return false;
    }
}

QString BlueToothDBusService::getDevTypeByAddr(QString)
{

}
