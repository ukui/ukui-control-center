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

QString Bluetooth::plugini18nName() {
    return pluginName;
}

int Bluetooth::pluginTypes() {
    return pluginType;
}

QWidget *Bluetooth::pluginUi() {

    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new BlueToothMain;
//        pluginWidget = new BlueToothMainWindow;
    }
    return pluginWidget;
}

const QString Bluetooth::name() const {
    return QStringLiteral("Bluetooth");
}

bool Bluetooth::isShowOnHomePage() const
{
    return true;
}

QIcon Bluetooth::icon() const
{
    return QIcon();
}

bool Bluetooth::isEnable() const
{
    return true;
}


