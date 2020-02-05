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
#ifndef SYSTEMDBUSDISPATCHER_H
#define SYSTEMDBUSDISPATCHER_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>

#include<QMessageBox>

class SystemDbusDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit SystemDbusDispatcher(QObject * parent = 0);
    ~SystemDbusDispatcher();

    QStringList list_cached_users();

    void create_user(QString name, QString fullname, int accounttype);
    void delete_user(qint64 uid, bool removefile);

private:
    QDBusInterface * systemiface;

private slots:
    void create_user_success(QDBusObjectPath objpath);
    void delete_user_success(QDBusObjectPath objpath);

signals:
    void createuserdone(QString path);
    void deleteuserdone(QString path);

};

#endif // SYSTEMDBUSDISPATCHER_H
