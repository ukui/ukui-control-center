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
#include <QCryptographicHash>
#include <polkit-qt5-1/polkitqt1-authority.h>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

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

    QString filename = "/usr/share/ukui-control-center/shell/res/apt.ini";
    aptSettings = new QSettings(filename, QSettings::IniFormat, this);
}

SysdbusRegister::~SysdbusRegister()
{
    exitFlag = true;

}

void SysdbusRegister::exitService() {
    exitFlag = true;
    qApp->exit(0);
}

int SysdbusRegister::setPid(qint64 id){
    _id = id;

    return 1;
}

bool SysdbusRegister::isSudoGroupNumber(QString uname)
{
    QString cmd = QString("cat /etc/group | grep sudo | awk -F: '{ print $NF}'");
    QString output;

    FILE   *stream;
    char buf[256];

    if ((stream = popen(cmd.toLatin1().data(), "r" )) == NULL){
        return false;
    }

    while(fgets(buf, 256, stream) != NULL){
        output = QString(buf).simplified();
    }

    pclose(stream);

    QStringList users = output.split(",");

    if (users.contains(uname)){
        return true;
    } else {
        return false;
    }
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
int SysdbusRegister::setAutoLoginStatus(QString username) {
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

int SysdbusRegister::setPasswdAging(int days, QString username) {
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

int SysdbusRegister::changeOtherUserPasswd(QString username, QString pwd){

    //密码校验
    if (!checkAuthorization()){
        return 0;
    }

    _changeOtherUserPasswd(username, pwd);

    return 1;

}

int SysdbusRegister::createUser(QString name, QString fullname, int accounttype, QString faceicon, QString pwd){

    //密码校验
    if (!checkCreateAuthorization()){
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

bool SysdbusRegister::checkCreateAuthorization()
{

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

bool SysdbusRegister::checkAuthorization(){

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

void SysdbusRegister::_setI2CBrightness(QString brightness, QString type) {
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

int SysdbusRegister::_getI2CBrightness(QString type) {
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
    if(ok && bright > 0 && bright <= 100)  // == 0 maybe means failed
        return bright;

    return -1;
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

bool SysdbusRegister::setaptproxy(QString ip, QString port, bool open)
{
    QStringList keys = aptSettings->childGroups();
    aptSettings->beginGroup("Info");
    aptSettings->setValue("open", open);
    aptSettings->setValue("ip", ip);
    aptSettings->setValue("port", port);
    aptSettings->endGroup();
    QString content_http = QString("%1%2%3%4%5%6").arg("Acquire::http::Proxy ").arg("\"http://").arg(ip).arg(":").arg(port).arg("\";\n");
    QString content_https = QString("%1%2%3%4%5%6").arg("Acquire::https::Proxy ").arg("\"https://").arg(ip).arg(":").arg(port).arg("\";\n");
    QString profile_http = QString("%1%2%3%4%5").arg("export http_proxy=\"http://").arg(ip).arg(":").arg(port).arg("\"\n");
    QString profile_https = QString("%1%2%3%4%5").arg("export https_proxy=\"https://").arg(ip).arg(":").arg(port).arg("\"\n");

    QString dirName  = "/etc/apt/apt.conf.d/";
    QString fileName = "/etc/apt/apt.conf.d/80apt-proxy";
    QString dirName_1  = "/etc/profile.d/";
    QString fileName_1 = "/etc/profile.d/80apt-proxy.sh";
    QDir AptDir(dirName);
    QDir ProDir(dirName_1);
    QFile AptProxyFile(fileName);
    QFile AptProxyProFile(fileName_1);

    if (AptDir.exists() && ProDir.exists()) {
        if (open) {    //开关开启则创建对应文件，未开启则删掉对应文件
            if (AptProxyFile.exists() && AptProxyProFile.exists()) {
               AptProxyFile.remove();
               AptProxyProFile.remove();
            }
            AptProxyFile.open(QIODevice::ReadWrite | QIODevice::Text);
            AptProxyProFile.open(QIODevice::ReadWrite | QIODevice::Text);
            //写入内容,这里需要转码，否则报错
            QByteArray str = content_http.toUtf8();
            QByteArray str_1 = content_https.toUtf8();
            QByteArray str_2 = profile_http.toUtf8();
            QByteArray str_3 = profile_https.toUtf8();
            //写入QByteArray格式字符串
            AptProxyFile.write(str);
            AptProxyFile.write(str_1);
            AptProxyProFile.write(str_2);
            AptProxyProFile.write(str_3);
        } else {
            if (AptProxyFile.exists() && AptProxyProFile.exists()) {
               AptProxyFile.remove();
               AptProxyProFile.remove();
            }
        }
    }else {
           return false;
    }
    return true;
}

QHash<QString, QVariant> SysdbusRegister::getaptproxy()
{
    QHash<QString, QVariant> mAptInfo;
    aptSettings->beginGroup("Info");
    mAptInfo.insert("open" ,  aptSettings->value("open").toBool());
    mAptInfo.insert("ip" ,  aptSettings->value("ip").toString());
    mAptInfo.insert("port" ,  aptSettings->value("port").toString());
    aptSettings->endGroup();
    return mAptInfo;
}

void SysdbusRegister::sethostname(QString hostname)
{
    QString fileName = "/etc/hosts";
    QString strAll;
    QFile readFile(fileName);
    if(readFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream stream_1(&readFile);
        int count = 0;
        while(!stream_1.atEnd())
        {
            if (count != 0)
                strAll.append("\n");
            count++;
            QString oneLine = stream_1.readLine();  //读取一行
            if(oneLine.contains("127.0.1.1")) {
                QString tempStr = QString("%1%2").arg("127.0.1.1       ").arg(hostname);
                strAll.append(tempStr);
                continue;
            }
            strAll.append(oneLine);
        }
    }
    readFile.close();
    QFile writeFile(fileName);
    if(writeFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
            QTextStream stream_2(&writeFile);
            stream_2<<strAll;
    }
    writeFile.close();
}

QString SysdbusRegister::getMemory()
{
    float memorysize(0.0);
    // 设置系统环境变量
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG","en_US");
    QProcess *process = new QProcess;
    process->setProcessEnvironment(env);
    process->start("bash" , QStringList() << "-c" << "dmidecode -t memory | grep Size ");
    process->waitForFinished();
    QByteArray ba = process->readAllStandardOutput();
    delete process;

    QString sizeinfo = QString(ba.data());
    QStringList size_list = sizeinfo.split('\n');
    for (QString str : size_list) {
        str.remove(QRegExp("\\s"));
        if (str.split(':').at(0) == "Size") {
            QString res = str.split(':').at(1);
            QRegExp rx("^\\d");
            if (rx.indexIn(res) == 0){
                QRegExp rx_1("^(.*)MB$");
                QRegExp rx_2("^(.*)GB$");
                int pos_1 = rx_1.indexIn(res);
                int pos_2 = rx_2.indexIn(res);
                if (pos_1 > -1) {
                    qDebug()<<rx_1.cap(1);
                    memorysize +=  ceil(rx_1.cap(1).toFloat()/1024);
                }
                if (pos_2 > -1) {
                    qDebug()<<rx_2.cap(1);
                    memorysize +=  rx_2.cap(1).toFloat();
                }
            }
        }
    }
    qDebug()<<"memory : "<<memorysize;
    return QString::number(memorysize);
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
            bool include_invalid_displays = true;
            DDCA_Display_Info_List*  dlist_loc = nullptr;
            ddca_get_display_info_list2(include_invalid_displays, &dlist_loc);
            for(int i = 0; i < dlist_loc->ct; i++) {
                QCryptographicHash Hash(QCryptographicHash::Md5);
                Hash.reset();
                Hash.addData(reinterpret_cast<const char *>(dlist_loc->info[i].edid_bytes), 128);
                QByteArray md5 = Hash.result().toHex();
                QString edidHash = QString(md5);

                if (dlist_loc->info[i].dispno < 0) {  //this display is invalid for DDC.
                    bool edidExist = false;
                    for (int j = 0; j < displayInfo_V.size(); j++) {
                        if (edidHash == displayInfo_V[j].edidHash) {
                            if (false == displayInfo_V[j]._DDC) {
                                edidExist = true;
                                displayInfo_V[j].I2C_brightness = _getI2CBrightness(displayInfo_V[j].I2C_busType); //重新获取亮度
                            } else { //有的显示器刚开始是valid
                                displayInfo_V.remove(j);
                                edidExist = false;
                            }
                            break;
                        }
                    }
                    if (false == edidExist) {
                        struct displayInfo display;
                        display.edidHash = edidHash;
                        display._DDC = false;
                        display.I2C_busType = QString::number(dlist_loc->info[i].path.path.i2c_busno);
                        display.I2C_brightness = _getI2CBrightness(display.I2C_busType);
                        displayInfo_V.append(display);
                    }
                } else {  //this display is valid for DDC.
                    bool edidExist = false;
                    for (int j = 0; j < displayInfo_V.size(); j++) {
                        if (edidHash == displayInfo_V[j].edidHash) {
                            if (true == displayInfo_V[j]._DDC) {
                                edidExist = true;
                            } else { //有的显示器刚开始是invalid
                                displayInfo_V.remove(j);
                                edidExist = false;
                            }
                            break;
                        }
                    }
                    if (!edidExist) {
                        struct displayInfo display;
                        DDCA_Display_Identifier did;
                        DDCA_Display_Ref ddca_dref;
                        display._DDC = true;
                        display.edidHash = edidHash;
                        display.I2C_busType = QString::number(dlist_loc->info[i].path.path.i2c_busno);
                        ddca_create_edid_display_identifier(dlist_loc->info[i].edid_bytes,&did);
                        ddca_create_display_ref(did,&ddca_dref);
                        ddca_open_display2(ddca_dref,false,&display.ddca_dh_loc);
                        displayInfo_V.append(display);
                    }
                }
            }
            ddca_free_display_info_list(dlist_loc);
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
            if (true == displayInfo_V[j]._DDC) {
                uint8_t new_sh = brightness.toUInt() >> 8;
                uint8_t new_sl = brightness.toUInt() & 0xff;
                ddca_set_non_table_vcp_value(displayInfo_V[j].ddca_dh_loc,0x10,new_sh,new_sl);
            } else {
                _setI2CBrightness(brightness, displayInfo_V[j].I2C_busType);
                displayInfo_V[j].I2C_brightness = brightness.toInt();
            }
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
            if (true == displayInfo_V[j]._DDC) {
                DDCA_Non_Table_Vcp_Value  valrec;
                if (ddca_get_non_table_vcp_value(displayInfo_V[j].ddca_dh_loc,0x10,&valrec) == 0) {
    //                uint16_t max_val = valrec.mh << 8 | valrec.ml; 暂未使用
                    uint16_t cur_val = valrec.sh << 8 | valrec.sl;
                    return cur_val;
                } else {
                    getDisplayInfo();
                    return -2;
                }
            } else {
                if (displayInfo_V[j].I2C_brightness >=0 && displayInfo_V[j].I2C_brightness <= 100) {
                    return displayInfo_V[j].I2C_brightness;
                } else {
                    getDisplayInfo();
                    return -2;
                }
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
        if (true == displayInfo_V[j]._DDC) {
            retString = retString + "<DDC>" + " bus=" + displayInfo_V[j].I2C_busType;
        } else {
            retString = retString + "<I2C>" + " bus=" + displayInfo_V[j].I2C_busType + "("+QString::number(displayInfo_V[j].I2C_brightness)+")";
        }
        retString = retString + " edidHash=" + displayInfo_V[j].edidHash + "\r\n";
    }
    return retString;
}


