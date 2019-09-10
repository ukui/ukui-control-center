#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>

#include <QDebug>

#include "sysdbusregister.h"

int main(int argc, char *argv[]){

    QCoreApplication app(argc, argv);
    app.setOrganizationName("Kylin Team");
    app.setApplicationName("ukcc-service");


    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.registerService("com.control.center.qt.systemdbus")){
        qCritical() << "QDbus register service failed reason:" << systemBus.lastError();
        exit(1);
    }

    if (!systemBus.registerObject("/", new SysdbusRegister(), QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)){
        qCritical() << "QDbus register object failed reason:" << systemBus.lastError();
        exit(2);
    }

    return app.exec();
}
