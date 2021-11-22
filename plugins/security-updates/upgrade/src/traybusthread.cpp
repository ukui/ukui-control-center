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

#include "traybusthread.h"
#include <QSqlDatabase>
#include <QDateTime>

void traybusthread::run()
{

//    QTime dieTime = QTime::currentTime().addMSecs(2000);
//    while( QTime::currentTime() < dieTime )
//    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    m_pServiceInterface = new QDBusInterface("com.scorpio.test",
                                             "/test/objectsd",
                                             "com.scorpio.test.value",
                                             QDBusConnection::sessionBus());

    QDBusConnection::sessionBus().connect(QString("com.scorpio.test"), QString("/test/objectsd"),
                                          QString("com.scorpio.test.value"),
                                          QString("ready"), this, SLOT(getInameAndCnameList(QString)));
    QDBusReply<QStringList> reply1 = m_pServiceInterface->call("connectSuccessslots");
}

void traybusthread::getInameAndCnameList(QString arg)
{
    qDebug()<<"getsignal";
    qDebug()<<arg;
    //调用远程的value方法

    QDBusReply<QStringList> reply = m_pServiceInterface->call("getImportant");
    if (reply.isValid()) {
        inameList = reply.value();
        qDebug() << inameList;

    } else {
        qDebug() << "value method called failed!";
    }
    emit result(inameList);
    QDBusReply<QString> rep = m_pServiceInterface->call("quitslots");

}
