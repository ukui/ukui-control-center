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
    //qDebug() <<Q_FUNC_INFO << value << ccIsOpen <<__LINE__;
    QDBusMessage m = QDBusMessage::createMethodCall("org.ukui.bluetooth",
                                                    "/org/ukui/bluetooth",
                                                    "org.ukui.bluetooth",
                                                    "bluetoothAdapterDisconvery");


    m << false << false;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<bool>() <<__LINE__;
    qDebug() << Q_FUNC_INFO << m.arguments().at(1).value<bool>() <<__LINE__;
    // 发送Message
    QDBusMessage response = QDBusConnection::sessionBus().call(m);
    qDebug() <<  response.errorMessage();
//    while("Not connected to D-Bus server" == response.errorMessage())
//    {
//        response = QDBusConnection::sessionBus().call(m);
//        qDebug() <<  response.errorMessage();
//    }

//    pluginWidget->~BlueToothMain();
    //sleep(1);
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


