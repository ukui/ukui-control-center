#include "bluetooth.h"
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDBusConnection>

#include <QDebug>

Bluetooth::Bluetooth() : mFirstLoad(true) {
    pluginName = tr("Bluetooth");
    pluginType = DEVICES;
}

Bluetooth::~Bluetooth() {
    bool value    = false ;
    bool ccIsOpen = false ;
    //qDebug() <<Q_FUNC_INFO << value << ccIsOpen <<__LINE__;
    QDBusMessage m = QDBusMessage::createMethodCall("org.ukui.bluetooth","/org/ukui/bluetooth","org.ukui.bluetooth","bluetoothAdapterDisconvery");
    m << value << ccIsOpen;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<bool>() <<__LINE__;
    qDebug() << Q_FUNC_INFO << m.arguments().at(1).value<bool>() <<__LINE__;
    // 发送Message
    QDBusMessage response = QDBusConnection::sessionBus().call(m);
    qDebug() <<  response.errorMessage();
    if (!mFirstLoad) {
//        delete pluginWidget;
    }
}

QString Bluetooth::get_plugin_name() {
    return pluginName;
}

int Bluetooth::get_plugin_type() {
    return pluginType;
}

QWidget *Bluetooth::get_plugin_ui() {

    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new BlueToothMain;
    }
    return pluginWidget;
}

void Bluetooth::plugin_delay_control() {

}

const QString Bluetooth::name() const {
    return QStringLiteral("bluetooth");
}

void Bluetooth::plugin_enter()
{

    if (pluginWidget)
        pluginWidget->releaseBluetoothDiscovery(true,true);
//    bool value    = true ;
//    bool ccIsOpen = true ;
//    //qDebug() <<Q_FUNC_INFO << value << ccIsOpen <<__LINE__;
//    QDBusMessage m = QDBusMessage::createMethodCall("org.ukui.bluetooth","/org/ukui/bluetooth","org.ukui.bluetooth","bluetoothAdapterDisconvery");
//    m << value << ccIsOpen;
//    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<bool>() <<__LINE__;
//    qDebug() << Q_FUNC_INFO << m.arguments().at(1).value<bool>() <<__LINE__;
//    // 发送Message
//    QDBusMessage response = QDBusConnection::sessionBus().call(m);
//    qDebug() <<  response.errorMessage();
}


void Bluetooth::plugin_leave()
{
    if (pluginWidget)
        pluginWidget->releaseBluetoothDiscovery(false,false);
//    bool value    = false ;
//    bool ccIsOpen = false ;
//    //qDebug() <<Q_FUNC_INFO << value << ccIsOpen <<__LINE__;
//    QDBusMessage m = QDBusMessage::createMethodCall("org.ukui.bluetooth","/org/ukui/bluetooth","org.ukui.bluetooth","bluetoothAdapterDisconvery");
//    m << value << ccIsOpen;
//    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<bool>() <<__LINE__;
//    qDebug() << Q_FUNC_INFO << m.arguments().at(1).value<bool>() <<__LINE__;
//    // 发送Message
//    QDBusMessage response = QDBusConnection::sessionBus().call(m);
//    qDebug() <<  response.errorMessage();
}
