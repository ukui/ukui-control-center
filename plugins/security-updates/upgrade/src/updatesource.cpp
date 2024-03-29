/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "updatesource.h"
#include "updatesource.h"

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
/*
 * 调用源管理器更新源缓存接口
*/
void UpdateSource::callDBusUpdateSource(QString symbol)
{
   QDBusPendingCall call = serviceInterface->asyncCall("updateSourcePackages",symbol);
   qDebug() << "源管理器：" <<"Call updateSourcePackages" ;

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
                callDBusUpdateSource(Symbol);
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
