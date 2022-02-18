#include "bluetooththread.h"


void bluetooththread::run()
{
    QDBusInterface *m_pServiceInterface = new QDBusInterface(KYLIN_BLUETOOTH_SERVER_NAME,
                                             KYLIN_BLUETOOTH_SERVER_PATH,
                                             KYLIN_BLUETOOTH_SERVER_INTERFACE,
                                             QDBusConnection::sessionBus());
    if (!m_pServiceInterface->isValid()) {
        qDebug()<<"false";
        delete m_pServiceInterface;
        emit result(false);
    }else {
        qDebug()<<"successed";
        delete m_pServiceInterface;
        emit result(true);
    }
}
