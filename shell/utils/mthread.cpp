#include "mthread.h"
#include <unistd.h>
#include <QTime>

MThread::MThread(QObject *parent) : QObject(parent)
{

}

MThread::~MThread()
{

}

void MThread::run()
{
    qDebug()<<QThread::currentThreadId();
    QTime timedebuge;//声明一个时钟对象
    timedebuge.start();//开始计时
    m_cloudInterface = new QDBusInterface("org.kylinssoclient.dbus",
                                          "/org/kylinssoclient/path",
                                          "org.freedesktop.kylinssoclient.interface",
                                          QDBusConnection::sessionBus());
    if (!m_cloudInterface->isValid()) {
        qDebug() << "fail to connect to service";
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        return;
    }
    QDBusConnection::sessionBus().connect(QString(), QString("/org/kylinssoclient/path"),
                                          QString(
                                              "org.freedesktop.kylinssoclient.interface"), "keyChanged", this,
                                          SLOT(keychanged(QString)));
    // 将以后所有DBus调用的超时设置为 milliseconds
    m_cloudInterface->setTimeout(2147483647); // -1 为默认的25s超时
     qDebug()<<"NetWorkAcount"<<"  线程耗时: "<<timedebuge.elapsed()<<"ms";
}

void MThread::keychanged(QString str)
{
    emit keychangedsignal(str);
}
