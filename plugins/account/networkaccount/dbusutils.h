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

#ifndef DBUSUTILS_H
#define DBUSUTILS_H

#include <QObject>
#include <QtDBus>
#include <QDBusMessage>

#define PATH "/org/kylinssoclient/path"
#define INTERFACE "org.freedesktop.kylinssoclient.interface"
#define DBUSNAME "org.kylinssoclient.dbus"
#define PROPERTYINTERFACE "org.freedesktop.DBus.Properties"
#define GETMETHOD "Get"
#define SETMETHOD "Set"

class DBusUtils : public QObject
{
    Q_OBJECT
public:
    explicit DBusUtils(QObject *parent = nullptr);
    QString callMethod(const QString &methodName,const QList<QVariant> &argList);
    int connectSignal(const QString &signal, QObject *obejct,const char *slot);
    QVariant GetProperty(const QString &dbusname,const QString &interface,const QString &path,const QString &property,const int &flag);
    QVariant ValueRet(const QString &dbusname, const QString &interface, const QString &path, const QString &method,
                            const int &flag, const QString &value, const QList<QVariant> &argList);

Q_SIGNALS:
    void taskFinished(const QString &taskName,int ret);
    void infoFinished(const QString &ret);
    void querryFinished(const QStringList &list);
};

#endif // DBUSUTILS_H
