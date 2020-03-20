#include "wifi.h"

Wifi::Wifi()
{

}

QStringList Wifi::getWifiList() {
    QProcess *wifiPro = new QProcess();
    QString shellOutput = "";
    wifiPro->start("nmcli -f signal,ssid device wifi");
    wifiPro->waitForFinished();
    QString output = wifiPro->readAll();
    shellOutput += output;
    QStringList slist = shellOutput.split("\n");

//    emit getWifiListFinished(slist);
//    qDebug()<<"wifilist--------------->"<<slist<<endl;
    return slist;
}
