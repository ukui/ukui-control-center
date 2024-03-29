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

#include "dbusutils.h"

DBusUtils::DBusUtils(QObject *parent) : QObject(parent)
{

}

QString DBusUtils::callMethod(const QString &methodName, const QList<QVariant> &argList) {
    QVariant ret;
    QDBusMessage message = QDBusMessage::createMethodCall(QString(DBUSNAME),
                           QString(PATH),
                           QString(INTERFACE),
                           methodName);
    if (argList.isEmpty() == false) {
        message.setArguments(argList);
    }
    QDBusMessage response = QDBusConnection::sessionBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage)
    {
        if (response.arguments().isEmpty() == false) {
            ret = response.arguments().takeFirst();;
        }
    } else {
        qDebug()<<methodName<<"called failed";
    }

    //qDebug() << methodName;
    if (methodName == "checkLogin") {
        emit infoFinished(ret.toString());
    } else if (methodName == "querryUploaded") {
        emit querryFinished(ret.toStringList());
    } else {
        emit taskFinished(methodName,ret.toInt());
    }
    return ret.toString();
}

int DBusUtils::connectSignal(const QString &signal, QObject *object,const char *slot) {
    QDBusConnection::sessionBus().connect(QString(), QString(PATH), QString(INTERFACE), signal, object, slot);
    return 0;
}

QVariant DBusUtils::GetProperty(const QString &dbusname,const QString &interface, const QString &path, const QString &property,const int &flag) {
    QVariant variant = "qwer";

    QDBusMessage message = QDBusMessage::createMethodCall(dbusname,path,
                                                          QString(PROPERTYINTERFACE),
                                                          QString(GETMETHOD));
    message << interface << property;
    QDBusMessage response = flag == 1 ? QDBusConnection::sessionBus().call(message) : QDBusConnection::systemBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage) {
        if (response.arguments().isEmpty() == false) {
            QDBusVariant value = qvariant_cast<QDBusVariant>(response.arguments().takeFirst());
            variant = value.variant();
        }
    } else {
        qDebug() <<interface << property << "call failed";
    }
    return variant;
}

QVariant DBusUtils::ValueRet(const QString &dbusname, const QString &interface, const QString &path, const QString &method,
                                   const int &flag, const QString &value, const QList<QVariant> &argList) {
    QVariant variant = "qwer";
    QDBusMessage message = QDBusMessage::createMethodCall(dbusname,path,
                                                          interface,
                                                          method);
    QList<QVariant> list ;
    if (value != "")
        list << value;
    if (argList.isEmpty() == false) {
        list.append(argList);
    }
    if (list.isEmpty() == false) {
        message.setArguments(list);
    }
    //qDebug() << dbusname <<interface << method << "  ==========================";
    QDBusMessage response = flag == 1 ? QDBusConnection::sessionBus().call(message) : QDBusConnection::systemBus().call(message);
    if (response.type() == QDBusMessage::ReplyMessage) {
        if (response.arguments().isEmpty() == false) {
            QDBusVariant value = qvariant_cast<QDBusVariant>(response.arguments().takeFirst());
            variant = value.variant();
            if (variant.isValid() == false) {
                variant = response.arguments().takeFirst();
            }
        }
    } else {
        qDebug() << "call failed";
    }
    //qDebug()<<variant<<"=============";
    return variant;
}
