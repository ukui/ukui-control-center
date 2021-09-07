#ifndef NETCONNECTWORK_H
#define NETCONNECTWORK_H

#include <QObject>
#include <QStringList>
#include <QProcess>
#include <QDebug>

class NetconnectWork : public QObject
{
    Q_OBJECT
public:
    explicit NetconnectWork();
    ~NetconnectWork();

public:
    void run(bool status);

private:
    bool getWifiIsOpen();

Q_SIGNALS:
    void complete();
};

#endif // NETCONNECTWORK_H
