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

#ifndef UPDATESOURCE_H
#define UPDATESOURCE_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDebug>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusReply>
const QString Symbol = "control";
class UpdateSource : public QObject
{
    Q_OBJECT
public:
    explicit UpdateSource(QObject *parent = nullptr);

    void callDBusUpdateTemplate();
    void callDBusUpdateSource(QString symbol);
//    QDBusReply<QString> replyStr;   //string类型的返回值
    QDBusInterface *serviceInterface = nullptr;
    void getReply(QDBusPendingCallWatcher *call);
    QString getFailInfo(int statusCode);
    QString getOrSetConf(QString type, QStringList name);
    void killProcessSignal(int pid, int signal);
signals:
    void getReplyFalseSignal();
    void startDbusFinished();
    void sigReconnTimes(int);
public slots:
    void startDbus();
};

#endif // UPDATESOURCE_H
