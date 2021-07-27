#ifndef NETCONNECTWORK_H
#define NETCONNECTWORK_H

#include <QObject>
#include <QThread>
#include <QStringList>

#include "wifi.h"

class NetconnectWork : public QThread
{
    Q_OBJECT
public:
    explicit NetconnectWork();
    ~NetconnectWork();

public:
    void run();
    bool is_wifi_open;

Q_SIGNALS:
    void wifiGerneral(QStringList wifiList);
    void workerComplete();
};

#endif // NETCONNECTWORK_H
