#ifndef BLUETOOTHTHREAD_H
#define BLUETOOTHTHREAD_H
#include <QThread>
#include <QtDBus>
#define KYLIN_BLUETOOTH_SERVER_NAME      "org.blueman.Applet"
#define KYLIN_BLUETOOTH_SERVER_PATH      "/org/blueman/applet"
#define KYLIN_BLUETOOTH_SERVER_INTERFACE "org.blueman.Applet"
class bluetooththread : public QThread
{
    Q_OBJECT
signals:
    void result(bool bluestatus);
private:
    void run();
};

#endif // BLUETOOTHTHREAD_H
