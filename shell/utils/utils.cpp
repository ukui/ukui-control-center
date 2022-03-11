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
#include "utils.h"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QProcess>
#include <QFile>
#include <QRegularExpression>

#ifdef WITHKYSEC
#include <kysec/libkysec.h>
#include <kysec/status.h>
#endif

void Utils::centerToScreen(QWidget* widget) {
    if (!widget)
      return;
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = widget->width();
    int y = widget->height();
    widget->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
}

QVariantMap Utils::getModuleHideStatus() {
    QDBusInterface m_interface( "org.ukui.ukcc.session",
                                "/",
                                "org.ukui.ukcc.session.interface",
                                QDBusConnection::sessionBus());

    QDBusReply<QVariantMap> obj_reply = m_interface.call("getModuleHideStatus");
    if (!obj_reply.isValid()) {
        qDebug()<<"execute dbus method getModuleHideStatus failed";
    }
    return obj_reply.value();
}

QString Utils::getCpuInfo() {
    QFile file("/proc/cpuinfo");

    if (file.open(QIODevice::ReadOnly)) {
        QString buffer = file.readAll();
        QStringList modelLine = buffer.split('\n').filter(QRegularExpression("^model name"));
        QStringList modelLineWayland = buffer.split('\n').filter(QRegularExpression("^Hardware"));
        QStringList lines = buffer.split('\n');

        if (modelLine.isEmpty()) {
            if (modelLineWayland.isEmpty()) {
                return "Unknown";
            }
            modelLine = modelLineWayland;
        }


        int count = lines.filter(QRegularExpression("^processor")).count();

        QString result;
        result.append(modelLine.first().split(':').at(1));
        result = result.trimmed();

        return result;
    }

    return QString();
}


QString Utils::getCpuArchitecture()
{
    QString cpuArchitecture;
    // 设置系统环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","en_US");
    QProcess *process = new QProcess;
    process->setProcessEnvironment(env);
    process->start("lscpu");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();

    delete process;
    QString cpuinfo = QString(ba.data());
    QStringList cpuinfo_list = cpuinfo.split("\n");
    for (int i = 0; i < cpuinfo_list.count(); i++) {
        QString mstring = cpuinfo_list.at(i);
        if (mstring.contains("Architecture")) {
            // 去除空格
            mstring = mstring.remove(QRegExp("\\s"));
            QStringList list = mstring.split(":");
            cpuArchitecture = list.at(1);
            break;
        }
    }
    return cpuArchitecture;
}

bool Utils::isExistEffect() {
    QString filename = QDir::homePath() + "/.config/ukui-kwinrc";
    QSettings kwinSettings(filename, QSettings::IniFormat);

    QStringList keys = kwinSettings.childGroups();

    kwinSettings.beginGroup("Plugins");
    bool kwin = kwinSettings.value("blurEnabled", kwin).toBool();

    if (!kwinSettings.childKeys().contains("blurEnabled")) {
        kwin = true;
    }
    kwinSettings.endGroup();

    QFileInfo dir(filename);
    if (!dir.isFile()) {
        return true;
    }

    if (keys.contains("Compositing")) {
        kwinSettings.beginGroup("Compositing");
        QString xder;
        bool kwinOG = false;
        bool kwinEN = true;
        xder = kwinSettings.value("Backend", xder).toString();
        kwinOG = kwinSettings.value("OpenGLIsUnsafe", kwinOG).toBool();
        kwinEN = kwinSettings.value("Enabled", kwinEN).toBool();
        if ("XRender" == xder || kwinOG || !kwinEN) {
            return false;
        } else {
            return true;
        }
        kwinSettings.endGroup();
    }
    return true;
}

void Utils::setKwinMouseSize(int size) {

    QString filename = QDir::homePath() + "/.config/kcminputrc";
    QSettings *mouseSettings = new QSettings(filename, QSettings::IniFormat);

    mouseSettings->beginGroup("Mouse");
    mouseSettings->setValue("cursorSize", size);
    mouseSettings->endGroup();

    delete mouseSettings;
    mouseSettings = nullptr;

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(5);
    args.append(0);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
}

bool Utils::isWayland() {
    QString sessionType = getenv("XDG_SESSION_TYPE");

    if (!sessionType.compare("wayland", Qt::CaseSensitive)) {
       return true;
    } else {
        return false;
    }
}

bool Utils::isCommunity()
{
    QString filename = "/etc/os-release";
    QSettings osSettings(filename, QSettings::IniFormat);

    QString versionID = osSettings.value("VERSION_ID").toString();

    if (versionID.compare("20.04", Qt::CaseSensitive)) {
        return false;
    }
    return true;
}

QRect Utils::sizeOnCursor()
{
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    return desk_rect;
}

bool Utils::isTablet()
{
    return QFile::exists("/etc/apt/ota_version");
}

bool Utils::isExitBattery()
{
    /* 默认机器没有电池 */
    bool hasBat = false;
    QDBusInterface *brightnessInterface = new QDBusInterface("org.freedesktop.UPower",
                                     "/org/freedesktop/UPower/devices/DisplayDevice",
                                     "org.freedesktop.DBus.Properties",
                                     QDBusConnection::systemBus());
    if (!brightnessInterface->isValid()) {
        qDebug() << "Create UPower Interface Failed : " << QDBusConnection::systemBus().lastError();
        return false;
    }

    QDBusReply<QVariant> briginfo;
    briginfo  = brightnessInterface ->call("Get", "org.freedesktop.UPower.Device", "PowerSupply");

    if (briginfo.value().toBool()) {
        hasBat = true ;
    }
    delete brightnessInterface;

    return hasBat;
}


QString Utils::getHostName()
{
    QString hostname;
    // 设置系统环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","en_US");
    QProcess *process = new QProcess;
    process->setProcessEnvironment(env);
    process->start("hostname");
    process->waitForFinished();

    QByteArray ba = process->readAllStandardOutput();

    delete process;
    hostname = ba.data();

    hostname.replace(QString("\n"),QString(""));
    return hostname;
}
