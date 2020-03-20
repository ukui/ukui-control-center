#include "netconnectwork.h"

NetconnectWork::NetconnectWork()
{

}

NetconnectWork::~NetconnectWork() {

}

void NetconnectWork::run() {
    Wifi *wifi = new Wifi();
    QStringList  wifiList= wifi->getWifiList();
    emit wifiGerneral(wifiList);
}
