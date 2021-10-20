#include "bluetooth.h"

#include <QDebug>

Bluetooth::Bluetooth() : mFirstLoad(true) {
    pluginName = tr("Bluetooth");
    bool intel = QFile::exists("/etc/apt/ota_version");
    if (intel)
        pluginType = NETWORK;
    else
        pluginType = DEVICES;
}

Bluetooth::~Bluetooth() {
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
//        pluginWidget = new BlueToothMainWindow;
    }
    return pluginWidget;
}

void Bluetooth::plugin_delay_control() {

}

const QString Bluetooth::name() const {
    return QStringLiteral("bluetooth");
}


