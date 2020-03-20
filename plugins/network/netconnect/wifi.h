#ifndef WIFI_H
#define WIFI_H

#include <QDebug>
#include <QStringList>
#include <QProcess>

class Wifi
{
public:
    Wifi();

    QStringList getWifiList();
};

#endif // WIFI_H
