#ifndef BLUETOOTHDBUSSERVICE_H
#define BLUETOOTHDBUSSERVICE_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>

#define DBUSSERVICE "com.ukui.bluetooth"
#define DBUSPATH "/com/ukui/bluetooth"
#define DBUSINTERFACE "com.ukui.bluetooth"

class BlueToothDBusService : public QObject
{
    Q_OBJECT
public:
    BlueToothDBusService(QObject *parent = nullptr);
    ~BlueToothDBusService();

    static QDBusInterface interface;
    static bool getDefaultAdapterPower();
    static bool getBluetoothBlock();
    static bool getAdapterDiscoverable();
    static QStringList getAdapterList();
    static QString getDefaultAdapterAddr();
    static QString getAdapterNameByAddr(QString);

    static void setAdapterPower(bool);
    static void setAdapterDiscoverable(bool);

    static bool getDevPairedByAddr(QString);
    static QString getDevTypeByAddr(QString);
    static QString getDevNameByAddr(QString);
private:


};

#endif // BLUETOOTHDBUSSERVICE_H
