#include "updatesource.h"
#include "updatedbus.h"

UpdateSource::UpdateSource(QObject *parent) : QObject(parent)
{

}

void UpdateSource::startDbus()
{
    serviceInterface = new QDBusInterface("com.kylin.software.properties",
                                                          "/com/kylin/software/properties",
                                                          "com.kylin.software.properties.interface",
                                                          QDBusConnection::systemBus());
    if(!serviceInterface->isValid())
    {
        qDebug() << "源管理器：" <<"Service Interface: " << qPrintable(QDBusConnection::systemBus().lastError().message());
        return;
    }
    emit startDbusFinished();
}

QString UpdateSource::getOrSetConf(QString type, QStringList name)
{
    QVariantList args;
    args << QVariant::fromValue(type);
    args << QVariant::fromValue(name);
    QDBusPendingReply<QString > reply = serviceInterface->call("getOrSetAutoUpgradeconf", args);
    qDebug() << args;

    if (!reply.isValid()) {
        qDebug() << "获取自动更新配置文件失败";
        return reply;
    }
    return reply;
}

void UpdateSource::killProcessSignal(int pid, int signal)
{
    QVariantList args;
    args << QVariant::fromValue(pid) << QVariant::fromValue(signal);
    serviceInterface->call("killProcessSignal", args);
}

/*
 * 调用源管理器更新源模版接口
*/
void UpdateSource::callDBusUpdateTemplate()
{
    QDBusPendingCall call = serviceInterface->asyncCall("updateSourceTemplate");
    if (!call.isValid()) {
        qDebug() << "updateSourceTemplate 成功";
    }
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call,this);
    connect(watcher,&QDBusPendingCallWatcher::finished,this,&UpdateSource::getReply);
    qDebug() <<"源管理器：" << "callDBusUpdateTemplate: " << "updateSourceTemplate";
}

QString UpdateSource::getFailInfo(int statusCode)
{
    qDebug()<< "UpdateSource::getFailInfo::::"<< statusCode;
    QDBusReply<QString> replyStr;   //string类型的返回值
    replyStr = serviceInterface->call("getFailInfo",statusCode);
    if (replyStr.isValid()) {
        qDebug() <<"源管理器：" << "getFailInfo:"<<replyStr.value();
        return replyStr.value();

    }
    else{
        qDebug() << "源管理器：" <<QString("调用失败 getFailInfo");
        return QString(tr("Connection failed, please reconnect!"));
//        return QString(tr("连接失败，请重新连接！"));
    }

}

void UpdateSource::getReply(QDBusPendingCallWatcher *call)
{
    UpdateDbus* updateMutual = nullptr;
    updateMutual = UpdateDbus::getInstance();
    /* 重连次数 */
    static int reconnTimes = 0;
    QDBusPendingReply<bool> reply = *call;

    if (!reply.isValid()) {
         qDebug() <<"源管理器：" << "getReply:" << "iserror";
    } else {
            bool status = reply.value();
            qDebug() <<"源管理器：" << "getReply:" << status;
            if (status) {
                reconnTimes = 0;
                updateMutual->callDBusUpdateSource();
            }
            else
            {
                if (reconnTimes < 5) {
                    callDBusUpdateTemplate();
                    reconnTimes++;
                    emit sigReconnTimes(reconnTimes);
                } else {
                    reconnTimes = 0;
                    emit getReplyFalseSignal();
                }
            }
    }
}
