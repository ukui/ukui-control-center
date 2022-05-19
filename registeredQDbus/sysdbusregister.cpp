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
#include <QSettings>
#include <QSharedPointer>

SysdbusRegister::SysdbusRegister()
{
}

SysdbusRegister::~SysdbusRegister()
{
}

//QString SysdbusRegister::name () const{
//    return m_name;
//}

//void SysdbusRegister::SetName(QString name){
//    m_name = name;
//}

void SysdbusRegister::exitService(){
    qApp->exit(0);
}

QString SysdbusRegister::GetComputerInfo(){
    QByteArray ba;
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    sprintf(cmd, "dmidecode -t system");

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

void SysdbusRegister::systemRun(QString cmd){
    QProcess::execute(cmd);
}

//获取免密登录状态
QString SysdbusRegister::getNoPwdLoginStatus(){
    QByteArray ba;
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    sprintf(cmd, "cat /etc/group |grep nopasswdlogin");
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

//设置电源模式
void SysdbusRegister::setPowerStatus(int is_saving)
{
//    QString filename = "/etc/tlp.d/00-template.conf";
//    QSharedPointer<QSettings>  autoSettings = QSharedPointer<QSettings>(new QSettings(filename, QSettings::IniFormat));
//    if(autoSettings.get()->contains("TLP_ENABLE")) {
//        qDebug()<<"Found!"<<QString::number(is_saving);
//        autoSettings.get()->setValue("TLP_ENABLE", QString::number(is_saving));
//        autoSettings.get()->endGroup();
//    }
//    autoSettings.get()->sync();
//由于电源管理提供的配置文件不存在分组，故选择另一种写入方式
    QStringList strList;
    QString strAll;
    QString powerFile = "/etc/tlp.d/00-template.conf";
    QFile readFile(powerFile);
    if(readFile.open((QIODevice::ReadOnly|QIODevice::Text)))
    {
        QTextStream stream(&readFile);
        strAll=stream.readAll();
    }
    readFile.close();
    QFile writeFile(powerFile);
    if(writeFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
            QTextStream stream(&writeFile);
            strList=strAll.split("\n");
            for(int i=0;i<strList.count();i++)
            {
                if(strList.at(i).contains("TLP_ENABLE"))
                {
                    //省电模式TLP_ENABLE=1，其他模式TLP_ENABLE=0
                    qDebug()<<"Found!"<<QString("TLP_ENABLE=%1").arg(is_saving);
                    QString tempStr=strList.at(i);
                    tempStr.replace(0,tempStr.length(),QString("TLP_ENABLE=%1").arg(is_saving));
                    stream<<tempStr<<'\n';
                    continue;
                }
                if (i==strList.count()-1) {
                    //最后一行不需要换行
                    stream<<strList.at(i);
                } else {
                    stream<<strList.at(i)<<'\n';
                }
            }
    }
    writeFile.close();
}
