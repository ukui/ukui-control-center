#ifndef USBTHREAD_H
#define USBTHREAD_H



#include <QObject>

class UsbThread : public QObject
{
    Q_OBJECT
public:
    explicit UsbThread();
    ~UsbThread();

    void run();

private:
    void usbDeviceIdentify(const QString &str);
    int init_sock();
    QString getDeivceTypeFromPath(const QString &path);
    QString getRetFromCommand(const QStringList &command);

signals:
    void addsignal();
    void removesignal();
};

#endif // USBTHREAD_H
