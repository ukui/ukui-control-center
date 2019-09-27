/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "systemdbusdispatcher.h"

#include <QDebug>

SystemDbusDispatcher::SystemDbusDispatcher(QObject *parent) :
    QObject(parent)
{
    systemiface = new QDBusInterface("org.freedesktop.Accounts",
                                     "/org/freedesktop/Accounts",
                                     "org.freedesktop.Accounts",
                                     QDBusConnection::systemBus());
    connect(systemiface, SIGNAL(UserAdded(QDBusObjectPath)), this, SLOT(create_user_success(QDBusObjectPath)));
    connect(systemiface, SIGNAL(UserDeleted(QDBusObjectPath)), this, SLOT(delete_user_success(QDBusObjectPath)));
}

SystemDbusDispatcher::~SystemDbusDispatcher()
{
    delete systemiface;
    systemiface = NULL;
}

QStringList SystemDbusDispatcher::list_cached_users(){
    QStringList users;
    QDBusReply<QList<QDBusObjectPath> > reply = systemiface->call("ListCachedUsers");
    if (reply.isValid()){
        for (QDBusObjectPath op : reply.value())
            users << op.path();
    }
    return users;
}

void SystemDbusDispatcher::create_user(QString name, QString fullname, int accounttype){
    QDBusReply<QDBusObjectPath> repley = systemiface->call("CreateUser", name, fullname, accounttype);
    if (!repley.isValid())
        qDebug() << "Create User Error" << repley.error().message();
//        QMessageBox::information(0, "Create User Error", repley.error().message());
}

void SystemDbusDispatcher::delete_user(qint64 uid, bool removefile){
    systemiface->call("DeleteUser", uid, removefile);
}

void SystemDbusDispatcher::create_user_success(QDBusObjectPath objpath){
    emit createuserdone(objpath.path());
}

void SystemDbusDispatcher::delete_user_success(QDBusObjectPath objpath){
    emit deleteuserdone(objpath.path());
}
