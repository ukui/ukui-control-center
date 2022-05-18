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
#include <QDir>

#include <QDBusInterface>
#include <QDBusReply>
#include<QCryptographicHash>
#include <polkit-qt5-1/polkitqt1-authority.h>

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
#include <QtConcurrent/QtConcurrent>

#include <QDBusMessage>

QStringList ddcProIdList;

SysdbusRegister::SysdbusRegister()
    : QDBusContext()
{
    mHibernateFile = "/etc/systemd/sleep.conf";
    mHibernateSet = new QSettings(mHibernateFile, QSettings::IniFormat, this);
    mHibernateSet->setIniCodec("UTF-8");
    exitFlag = false;
    toGetDisplayInfo = true;
    _getDisplayInfoThread();
    _id = 0;
}

SysdbusRegister::~SysdbusRegister()
{
    exitFlag = true;
}

void SysdbusRegister::exitService() {
    exitFlag = true;
    qApp->exit(0);
}

int SysdbusRegister::setPid(qint64 id)
{
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();

    if (!authoriySetPid(conn.interface()->servicePid(msg.service()).value())){
        return 0;
    }

    _id = id;

    return 1;
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

int SysdbusRegister::_changeOtherUserPasswd(QString username, QString pwd){

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

int SysdbusRegister::changeOtherUserPasswd(QString username, QString pwd)
{
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    if (!checkAuthorization(conn.interface()->servicePid(msg.service()).value())){
        return 0;
    }

    _changeOtherUserPasswd(username, pwd);

    return 1;
}

int SysdbusRegister::createUser(QString name, QString fullname, int accounttype, QString faceicon, QString pwd){
    //密码校验
    QDBusConnection conn = connection();
    QDBusMessage msg = message();
    if (!checkCreateAuthorization(conn.interface()->servicePid(msg.service()).value())){
        return 0;
    }

    QDBusInterface iface("org.freedesktop.Accounts",
                         "/org/freedesktop/Accounts",
                         "org.freedesktop.Accounts",
                         QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> reply = iface.call("CreateUser", name, fullname, accounttype);

    if (reply.isValid()){
        QString op = reply.value().path();
        if (!op.isEmpty()){

            QDBusInterface ifaceUser("org.freedesktop.Accounts",
                                     op,
                                     "org.freedesktop.Accounts.User",
                                     QDBusConnection::systemBus());
            // 设置头像
            ifaceUser.call("SetIconFile", faceicon);

            // 设置密码
            _changeOtherUserPasswd(name, pwd);

        }
    }

    return 1;
}

bool SysdbusRegister::checkCreateAuthorization(qint64 id)
{
    _id = id;

    if (_id == 0)
        return false;

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.create",
                PolkitQt1::UnixProcessSubject(_id),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::No) {
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

bool SysdbusRegister::authoriySetPid(qint64 id)
{
    _id = id;

    if (_id == 0)
        return false;

    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.pid",
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


int SysdbusRegister::changeRTC() {
    QString cmd = "hwclock -w";
    return system(cmd.toLatin1().data());
}

bool SysdbusRegister::setNtpSerAddress(QString serverAddress)
{
    if (serverAddress == "default") {
        system("rm -rf /etc/systemd/timesyncd.conf.d/");
        system("timedatectl set-ntp false");
        system("timedatectl set-ntp true");
        return true;
    }

    QString dirName  = "/etc/systemd/timesyncd.conf.d/";
    QString fileName = "/etc/systemd/timesyncd.conf.d/kylin.conf";

    QDir  dir(dirName);
    QFile file(fileName);
    if (!dir.exists()) {
        if (dir.mkdir(dirName) == false) {
            return false;
        }
    }
    if (file.open(QIODevice::WriteOnly) == false) {
        return false;
    }
    file.write("[Time]\n");
    file.write("NTP = ");
    file.write(serverAddress.toLatin1().data());
    file.write("\n");
    file.close();
    system("timedatectl set-ntp false");
    system("timedatectl set-ntp true");
    return true;

}

void SysdbusRegister::getDisplayInfo()
{
    toGetDisplayInfo = true;
    return;
}

void SysdbusRegister::_getDisplayInfoThread()
{
    QtConcurrent::run([=] {  //运行独立线程去获取ddc信息，不能每次重新运行run，会导致获取的信息不对
        while (true) {
            if (exitFlag)
                return;
            if (!toGetDisplayInfo) {
                sleep(1);
                continue;
            }
            bool include_invalid_displays = false;
            DDCA_Display_Info_List*  dlist_loc = nullptr;
            ddca_get_display_info_list2(include_invalid_displays, &dlist_loc);
            QCryptographicHash Hash(QCryptographicHash::Md5);
            for(int i = 0; i < dlist_loc->ct; i++) {
                Hash.reset();
                Hash.addData(reinterpret_cast<const char *>(dlist_loc->info[i].edid_bytes), 128);
                QByteArray md5 = Hash.result().toHex();
                QString edidHash = QString(md5);
                bool edidExist = false;
                for (int j = 0; j < displayInfo_V.size(); j++) {
                    if (edidHash == displayInfo_V[j].edidHash) {
                        edidExist = true;
                        break;
                    }
                }
                if (!edidExist) {
                    struct displayInfo display;
                    DDCA_Display_Identifier did;
                    DDCA_Display_Ref ddca_dref;
                    display.edidHash = edidHash;
                    ddca_create_edid_display_identifier(dlist_loc->info[i].edid_bytes,&did);
                    ddca_create_display_ref(did,&ddca_dref);
                    ddca_open_display2(ddca_dref,false,&display.ddca_dh_loc);
                    displayInfo_V.append(display);
                }
            }
            toGetDisplayInfo = false;
        }
    });
}

void SysdbusRegister::setDisplayBrightness(QString brightness, QString edidHash)
{
    bool edidExist = false;
    for (int j = 0; j < displayInfo_V.size(); j++) {
        if (displayInfo_V[j].edidHash == edidHash) {
            edidExist = true;
            uint8_t new_sh = brightness.toUInt() >> 8;
            uint8_t new_sl = brightness.toUInt() & 0xff;
            ddca_set_non_table_vcp_value(displayInfo_V[j].ddca_dh_loc,0x10,new_sh,new_sl);
        }
    }
    if (!edidExist) {
        getDisplayInfo();
    }
    return;
}

int SysdbusRegister::getDisplayBrightness(QString edidHash)
{
    bool edidExist = false;
    for (int j = 0; j < displayInfo_V.size(); j++) {
        if (displayInfo_V[j].edidHash == edidHash) {
            edidExist = true;
            DDCA_Non_Table_Vcp_Value  valrec;
            if (ddca_get_non_table_vcp_value(displayInfo_V[j].ddca_dh_loc,0x10,&valrec) == 0) {
//                uint16_t max_val = valrec.mh << 8 | valrec.ml; 暂未使用
                uint16_t cur_val = valrec.sh << 8 | valrec.sl;
                return cur_val;
            } else {
                getDisplayInfo();
                return -2;
            }
        }
    }
    if (!edidExist) {
        getDisplayInfo();
    }
    return -2;
}

QString SysdbusRegister::showDisplayInfo()
{
    QString retString = "";
    for (int j = 0; j < displayInfo_V.size(); j++) {
        retString = retString + "edidHash:" + displayInfo_V[j].edidHash + "\r\n";
    }
    return retString;
}
