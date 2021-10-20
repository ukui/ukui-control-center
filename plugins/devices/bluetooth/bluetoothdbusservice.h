#ifndef BLUETOOTHDBUSSERVICE_H
#define BLUETOOTHDBUSSERVICE_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>

#define DBUSNAME "com.ukui.bluetooth"

class BlueToothDBusService : public QObject
{
    Q_OBJECT
public:
    BlueToothDBusService(QObject *parent = nullptr);
    ~BlueToothDBusService();

signals:


private:


};

#endif // BLUETOOTHDBUSSERVICE_H
