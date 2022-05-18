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
#include "sysdbusregister.h"

#include <QDebug>
#include <QSharedPointer>
#include <QRegExp>
#include <QProcess>
#include <stdlib.h>
#include <polkit-qt5-1/polkitqt1-authority.h>
#include <QtConcurrent/QtConcurrent>
#include <QDBusMessage>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>

}


SysdbusRegister::SysdbusRegister()
    : QDBusContext()
{
    mHibernateFile = "/etc/systemd/sleep.conf";
    mHibernateSet = new QSettings(mHibernateFile, QSettings::IniFormat, this);
    mHibernateSet->setIniCodec("UTF-8");
    runThreadFlag = false;
    getBrightnessInfo();
}

SysdbusRegister::~SysdbusRegister()
{
}

void SysdbusRegister::exitService() {
    qApp->exit(0);
}

QString SysdbusRegister::GetComputerInfo() {
    QByteArray ba;
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    snprintf(cmd, 128, "dmidecode -t system");

    if ((fp = popen(cmd, "r")) != NULL){
        rewind(fp);
        while (!feof(fp)) {
            fgets(buf, sizeof (buf), fp);
            ba.append(buf);
        }
        pclose(fp);
        fp = NULL;
    }
    return QString(ba);
}

//获取免密登录状态
QString SysdbusRegister::getNoPwdLoginStatus(){
    QByteArray ba;
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    snprintf(cmd, 128, "cat /etc/group |grep nopasswdlogin");
    if ((fp = popen(cmd, "r")) != NULL){
        rewind(fp);
        fgets(buf, sizeof (buf), fp);
        ba.append(buf);
        pclose(fp);
        fp = NULL;
    }else{
        qDebug()<<"popen文件打开失败"<<endl;
    }
    return QString(ba);
}

//设置免密登录状态
int SysdbusRegister::setNoPwdLoginStatus(bool status,QString username)
{
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    if (!authoriyLogin(conn.interface()->servicePid(msg.service()).value())){
        return 0;
    }

    QString cmd;
    if(true == status){
         cmd = QString("gpasswd  -a %1 nopasswdlogin").arg(username);
    } else{
        cmd = QString("gpasswd  -d %1 nopasswdlogin").arg(username);
    }
    QProcess::execute(cmd);

    return 1;
}

// 设置自动登录状态
int SysdbusRegister::setAutoLoginStatus(QString username)
{
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    if (!authoriyAutoLogin(conn.interface()->servicePid(msg.service()).value())){
        return 0;
    }

    QString filename = "/etc/lightdm/lightdm.conf";
    QSharedPointer<QSettings>  autoSettings = QSharedPointer<QSettings>(new QSettings(filename, QSettings::IniFormat));
    autoSettings->beginGroup("SeatDefaults");

    autoSettings->setValue("autologin-user", username);

    autoSettings->endGroup();
    autoSettings->sync();

    return 1;
}

QString SysdbusRegister::getSuspendThenHibernate() {
    mHibernateSet->beginGroup("Sleep");

    QString time = mHibernateSet->value("HibernateDelaySec").toString();

    mHibernateSet->endGroup();
    mHibernateSet->sync();

    return time;
}

void SysdbusRegister::setSuspendThenHibernate(QString time) {
    mHibernateSet->beginGroup("Sleep");

    mHibernateSet->setValue("HibernateDelaySec", time);

    mHibernateSet->endGroup();
    mHibernateSet->sync();
}

int SysdbusRegister::setPasswdAging(int days, QString username)
{
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    if (!authoriyPasswdAging(conn.interface()->servicePid(msg.service()).value())){
        return 0;
    }

    QString cmd;

    cmd = QString("chage -M %1 %2").arg(days).arg(username);
    QProcess::execute(cmd);

    return 1;
}

int SysdbusRegister::changeOtherUserPasswd(QString username, QString pwd)
{
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    if (!checkAuthorization(conn.interface()->servicePid(msg.service()).value())){
        return 0;
    }

    std::string str1 = username.toStdString();
    const char * user_name = str1.c_str();

    std::string str2 = pwd.toStdString();
    const char * passwd = str2.c_str();

    QString output;

    char * cmd = g_strdup_printf("/usr/bin/changeotheruserpwd '%s' '%s'", user_name, passwd);

    FILE   *stream;
    char buf[256];

    if ((stream = popen(cmd, "r" )) == NULL){
        return -1;
    }

    while(fgets(buf, 256, stream) != NULL){
        output = QString(buf).simplified();
    }

    pclose(stream);

    return 1;

}

bool SysdbusRegister::authoriyLogin(qint64 id)
{
    _id = id;

    if (_id == 0)
        return false;

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.login",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::No){
        _id = 0;
        return false;
    } else {
        _id = 0;
        return true;
    }
}

bool SysdbusRegister::authoriyAutoLogin(qint64 id)
{
    _id = id;

    if (_id == 0)
        return false;

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.autologin",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::No){
        _id = 0;
        return false;
    } else {
        _id = 0;
        return true;
    }
}

bool SysdbusRegister::authoriyPasswdAging(qint64 id)
{
    _id = id;

    if (_id == 0)
        return false;

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.passwdaging",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::No){
        _id = 0;
        return false;
    } else {
        _id = 0;
        return true;
    }
}

bool SysdbusRegister::checkAuthorization(qint64 id)
{
    _id = id;

    if (_id == 0)
        return false;

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::No){
        _id = 0;
        return false;
    } else {
        _id = 0;
        return true;
    }
}

void SysdbusRegister::setDDCBrightness(QString brightness, QString type) {
    QString program = "/usr/sbin/i2ctransfer";
    QStringList arg;
    int br=brightness.toInt();
    QString light = "0x" + QString::number(br,16);
    QString c = "0x" + QString::number(168^br,16);
    arg << "-f" << "-y" << type << "w7@0x37" << "0x51" << "0x84" << "0x03"
        << "0x10" << "0x00" << light << c;
    QProcess *vcpPro = new QProcess(this);
//    vcpPro->start(program, arg);
//    vcpPro->waitForStarted();
//    vcpPro->waitForFinished();
    vcpPro->startDetached(program, arg);
}

int SysdbusRegister::getDDCBrightness(QString type) {
    QString program = "/usr/sbin/i2ctransfer";
    QStringList arg;
    arg<<"-f"<<"-y"<<type<<"w5@0x37"<<"0x51"<<"0x82"<<"0x01"<<"0x10"<<"0xac";
    QProcess *vcpPro = new QProcess();
    vcpPro->start(program, arg);
    vcpPro->waitForStarted();
    vcpPro->waitForFinished();
    arg.clear();
    arg<<"-f"<<"-y"<<type<<"r16@0x37";
    usleep(40000);
    vcpPro->start(program, arg);
    vcpPro->waitForStarted();
    vcpPro->waitForFinished();
    QString result = vcpPro->readAllStandardOutput().trimmed();
    if (result == "")
        return -1;

    QString bri=result.split(" ").at(9);
    bool ok;
    int bright=bri.toInt(&ok,16);
    if(ok && bright >= 0 && bright <= 100)
        return bright;

    return -1;
}

int SysdbusRegister::changeRTC() {
    QString cmd = "hwclock -w";
    return system(cmd.toLatin1().data());
}

void SysdbusRegister::getBrightnessInfo()
{
    if (true == runThreadFlag)
        return;

    QtConcurrent::run([=] {
        runThreadFlag = true;
        QString program = "/usr/bin/ddcutil";
        QStringList arg;
        arg << "detect";
        QProcess *vcpPro = new QProcess();
        vcpPro->start(program, arg);
        vcpPro->waitForStarted();
        vcpPro->waitForFinished();
        QByteArray arr=vcpPro->readAll();

        char *re=arr.data();
        char *p;
        QList<QString> l;
        while (*re) {
            p=strpbrk(re,"\n");
            *p=0;
            QString s=re;
            s=s.trimmed();
            l.append(s);
            re=++p;
            if(*re=='\n')
                re++;
        }

        for (int i=0; i < l.count(); i=i+9) {
            if (l.at(i).startsWith("Display") || l.at(i).startsWith("Invalid display")) {
                if (i+5 >= l.count())
                    break;
                QString bus=l.at(i+1).split(":").at(1).trimmed();
                QString serial=l.at(i+5).split(":").at(1).trimmed();
                QString busType = bus.split("-").at(1);
                bool existFlag = false;
                for (int i = 0; i < brightInfo_V.size(); i++) {
                    if (brightInfo_V[i].serialNum == serial && brightInfo_V[i].busType == busType) { //990存在多个bus号的情况，但内核只链接到一个，所以都需要存留
                        brightInfo_V[i].brightness = getDDCBrightness(busType);
                        existFlag = true;
                        break;
                    }
                }
                if (false == existFlag) {
                    struct brightInfo  mBrightInfo;
                    mBrightInfo.serialNum  = serial;
                    mBrightInfo.busType    = busType;
                    for (int i = 0; i < 4; i++) {
                        mBrightInfo.brightness = getDDCBrightness(busType);
                        if (mBrightInfo.brightness < 10) {//<10时确认一下,大多数是插拔时异常导致<10
                            sleep(2);
                            continue;
                        } else {
                            break;
                        }
                    }
                    brightInfo_V.push_back(mBrightInfo);
                }
            }
        }
        runThreadFlag = false;
    });
    return;
}

void SysdbusRegister::setDDCBrightnessUkui(QString brightness, QString serialNum, QString busNum)
{
    if (busNum != "-1") {
        for (int i = 0; i < brightInfo_V.size(); i++) {
            if (brightInfo_V[i].busType == busNum) {
                setDDCBrightness(brightness, brightInfo_V[i].busType);
                brightInfo_V[i].brightness = brightness.toInt();
                return;
            }
        }
        return;
    }

    if (serialNum.contains("HDMI") || serialNum.contains("VGA")) {
        if (serialNum.contains("HDMI")) {
            for (int i = 0; i < brightInfo_V.size(); i++) {
                if (brightInfo_V[i].busType == "8") {
                    setDDCBrightness(brightness, brightInfo_V[i].busType);
                    brightInfo_V[i].brightness = brightness.toInt();
                    return;
                }
            }
        } else if (serialNum.contains("VGA")) {
            for (int i = 0; i < brightInfo_V.size(); i++) {
                if (brightInfo_V[i].busType == "4") {
                    setDDCBrightness(brightness, brightInfo_V[i].busType);
                    brightInfo_V[i].brightness = brightness.toInt();
                    return;
                }
            }
        }
        //未能通过8/4匹配成功，根据序列号去匹配
        QString getSerial = serialNum.split("/").at(1);
        for (int i = 0; i < brightInfo_V.size(); i++) {
            if (brightInfo_V[i].serialNum == getSerial) {
                setDDCBrightness(brightness, brightInfo_V[i].busType);
                brightInfo_V[i].brightness = brightness.toInt();
                return;
            }
        }
    } else {
        for (int i = 0; i < brightInfo_V.size(); i++) {
            if (brightInfo_V[i].serialNum == serialNum) {
                setDDCBrightness(brightness, brightInfo_V[i].busType);
                brightInfo_V[i].brightness = brightness.toInt();
                return;
            }
        }
    }
}

int SysdbusRegister::getDDCBrightnessUkui(QString serialNum, QString busNum)
{
    if (serialNum.contains("RE")) {
        brightInfo_V.clear();
        getBrightnessInfo();
        return -2;
    }
    if (busNum != "-1") {  //发来了bus号，直接用
        for (int i = 0; i < brightInfo_V.size(); i++) {
            if (brightInfo_V[i].busType == busNum && brightInfo_V[i].brightness >= 0 && brightInfo_V[i].brightness <= 100) {
                return brightInfo_V[i].brightness;
            }
        }
        getBrightnessInfo();
        return -2;
    }

    if (serialNum.contains("HDMI") || serialNum.contains("VGA")) {
        if (serialNum.contains("HDMI")) {
            for (int i = 0; i < brightInfo_V.size(); i++) {
                if (brightInfo_V[i].busType == "8" && brightInfo_V[i].brightness >= 0 && brightInfo_V[i].brightness <= 100) {
                    return brightInfo_V[i].brightness;
                }
            }
        } else if (serialNum.contains("VGA")) {
            for (int i = 0; i < brightInfo_V.size(); i++) {
                if (brightInfo_V[i].busType == "4" && brightInfo_V[i].brightness >= 0 && brightInfo_V[i].brightness <= 100) {
                    return brightInfo_V[i].brightness;
                }
            }
        }
        //未能通过8/4匹配成功，根据序列号去匹配
        QString getSerial = serialNum.split("/").at(1);
        for (int i = 0; i < brightInfo_V.size(); i++) {
            if (brightInfo_V[i].serialNum == getSerial && brightInfo_V[i].brightness >= 0 && brightInfo_V[i].brightness <= 100) {
                return brightInfo_V[i].brightness;
            }
        }

    } else {
        for (int i = 0; i < brightInfo_V.size(); i++) {
            if (brightInfo_V[i].serialNum == serialNum && brightInfo_V[i].brightness >= 0 && brightInfo_V[i].brightness <= 100) {
                return brightInfo_V[i].brightness;
            }
        }
    }

    getBrightnessInfo();
    return -2;   //表示没有信息，需要隔3～5秒再次获取
}
