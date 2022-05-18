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
#ifndef SYSDBUSREGISTER_H
#define SYSDBUSREGISTER_H

#include <QObject>
#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QSettings>
#include <QVector>
#include <ddcutil_c_api.h>
#include <ddcutil_types.h>
#include <QDBusContext>
#include <QDBusConnectionInterface>

struct brightInfo {
    QString serialNum;
    QString busType;
    int     brightness;
};

struct displayInfo {
    DDCA_Display_Handle ddca_dh_loc;   //显示器句柄
    QString             edidHash;      //edid信息的hash值(md5)
};

class SysdbusRegister : public QObject,QDBusContext
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.control.center.interface")

public:
    explicit SysdbusRegister();
    ~SysdbusRegister();

public:
    bool checkCreateAuthorization(qint64 id);
    bool checkAuthorization(qint64 id);
    bool authoriyLogin(qint64 id);
    bool authoriySetPid(qint64 id);
    bool authoriyAutoLogin(qint64 id);
    bool authoriyPasswdAging(qint64 id);

private:
    QString mHibernateFile;
    QSettings *mHibernateSet;

    volatile bool exitFlag;
    volatile bool toGetDisplayInfo;
    qint64 _id;
    QVector<struct displayInfo> displayInfo_V;

private:
    int _changeOtherUserPasswd(QString username, QString pwd);
    void _getDisplayInfoThread();

signals:
    Q_SCRIPTABLE void nameChanged(QString);
    Q_SCRIPTABLE void computerinfo(QString);

public slots:

    Q_SCRIPTABLE void exitService();
    Q_SCRIPTABLE QString GetComputerInfo();
    // 设置进程id
    Q_SCRIPTABLE int setPid(qint64 id);

    // 设置免密登录状态
    Q_SCRIPTABLE int setNoPwdLoginStatus(bool status,QString username);

    // 获取免密登录状态
    Q_SCRIPTABLE QString getNoPwdLoginStatus();

    // 设置自动登录状态
    Q_SCRIPTABLE int setAutoLoginStatus(QString username);

    // 获取挂起到休眠时间
    Q_SCRIPTABLE QString getSuspendThenHibernate();

    // 设置挂起到休眠时间
    Q_SCRIPTABLE void setSuspendThenHibernate(QString time);

    // 设置密码时效
    Q_SCRIPTABLE int setPasswdAging(int days, QString username);

    // 提权修改其他用户密码
    Q_SCRIPTABLE int changeOtherUserPasswd(QString username, QString pwd);

    // 提权创建用户，避免两次验证弹窗
    Q_SCRIPTABLE int createUser(QString name, QString fullname, int accounttype, QString faceicon, QString pwd);

    // 修改硬件时间
    Q_SCRIPTABLE int changeRTC();

    // 设置NTP授时服务器
    Q_SCRIPTABLE bool setNtpSerAddress(QString serverAddress);

    //新亮度相关的接口
    Q_SCRIPTABLE void getDisplayInfo();
    Q_SCRIPTABLE QString showDisplayInfo();
    Q_SCRIPTABLE void setDisplayBrightness(QString brightness, QString edidHash);
    Q_SCRIPTABLE int getDisplayBrightness(QString edidHash);
};

#endif // SYSDBUSREGISTER_H
