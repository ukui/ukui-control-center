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
#include "../shell/utils/utils.h"

#include <QDBusContext>
#include <QDBusConnectionInterface>

struct displayInfo {
    bool   _DDC;           //是否采用DDC处理，当DDC失败时使用I2C
    bool   _getI2C;        //是否从参数中获取I2C的bus号
    DDCA_Display_Handle ddca_dh_loc;   //显示器句柄
    QString edidHash;      //edid信息的hash值(md5)

    QString I2C_busType;       //兼容I2C处理亮度
    int     I2C_brightness;
};

class SysdbusRegister : public QObject,QDBusContext
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.control.center.interface")

public:
    explicit SysdbusRegister();
    ~SysdbusRegister();

public:
    bool authoriyLogin(qint64 id);
    bool checkAuthorization(qint64 id);
    bool checkCreateAuthorization(qint64 id);
    bool authoriySetPid(qint64 id);
    bool authoriyAutoLogin(qint64 id);
    bool authoriyPasswdAging(qint64 id);

private:
    void _getDisplayInfoThread();
    // 通过I2C调节外接台式屏幕亮度
    void _setI2CBrightness(QString brightness, QString type);

    // 通过I2C获取外接台式屏幕亮度
    int _getI2CBrightness(QString type);
    struct displayInfo _createDisplayInfo(QString edidHash, QString busType, bool ddc, bool getI2C);

private:
    QString mHibernateFile;
    QSettings *mHibernateSet;
    QSettings *aptSettings;
    QSettings *advanceSettings;

    volatile bool exitFlag;
    volatile bool toGetDisplayInfo;
    QVector<struct displayInfo> displayInfo_V;

    qint64 _id;
    bool onlyI2C;
    QString cpuInfo;
    bool forceToGet;

private:
    int _changeOtherUserPasswd(QString username, QString pwd);
    QMap<QString, QVariant> getJsonInfo(const  QString &confFile);
    bool isBacklight();

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
    Q_SCRIPTABLE void getDisplayInfo(bool m_forceToGet = false);
    Q_SCRIPTABLE QString showDisplayInfo();
    Q_SCRIPTABLE void setDisplayBrightness(QString brightness, QString edidHash,  QString i2cBus = "-1");
    Q_SCRIPTABLE int getDisplayBrightness(QString edidHash, QString i2cBus = "-1");

    // dmidecode解析id屏蔽电源功能
    Q_SCRIPTABLE QString getVerionsId();

    //设置apt代理
    Q_SCRIPTABLE bool setaptproxy(QString ip , QString port ,bool open);

    //获取apt代理信息
    Q_SCRIPTABLE QHash<QString , QVariant> getaptproxy();

    //获取内存大小
    Q_SCRIPTABLE QString getMemory();

    //获取模块信息
    Q_SCRIPTABLE QVariantMap getModuleHideStatus();

    //设置模块状态
    Q_SCRIPTABLE bool setModuleStatus(QMap <QString, QVariant> pluginStatus);

    //获取高级配置口令
    Q_SCRIPTABLE QString getAdvancePwd();

    //修改高级配置口令
    Q_SCRIPTABLE void setAdvancePwd(QString pwd);
};

#endif // SYSDBUSREGISTER_H
