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
#include <QWidget>

#include "about.h"
#include "ui_about.h"

#include <QDebug>

About::About()
{
    ui = new Ui::About;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("about");
    pluginType = MESSAGES_TASK;

//    qDebug() << "--------start------->";
//    qDebug() << QSysInfo::kernelType();
//    qDebug() << QSysInfo::kernelVersion();
//    qDebug() << "---------1---------->";
//    qDebug() << QSysInfo::machineHostName();
//    qDebug() << QSysInfo::currentCpuArchitecture();
//    qDebug() << "----------2----------->";
//    qDebug() << QSysInfo::prettyProductName();
//    qDebug() << QSysInfo::productType();
//    qDebug() << QSysInfo::productVersion();
//    qDebug() << "-------end---------->";
    _call_dbus_get_computer_info();
    _data_init();

    initUI();
}

About::~About()
{
    delete ui;
//    delete interface;
}

QString About::get_plugin_name(){
    return pluginName;
}

int About::get_plugin_type(){
    return pluginType;
}

CustomWidget *About::get_plugin_ui(){
    return pluginWidget;
}

void About::plugin_delay_control(){

}

void About::_data_init(){
    QStringList infoList = computerinfo.split("\n\n");
    QString available;
    if (infoList.length() > 1){
        available = infoList.at(1);
    }
    else {
        available = "";
    }
    if (available != ""){
        for (QString line : available.split("\n")){
            if (!line.contains(":"))
                continue;
            QStringList lineList = line.split(":");
            infoMap.insert(lineList.at(0).simplified(), lineList.at(1).simplified());
        }
    }
}

void About::initUI(){
    QLabel * manufacturers = new QLabel(tr("UNKNOWN"));
    if (infoMap.contains(MANUFACTURER))
        manufacturers->setText(QString(infoMap.find(MANUFACTURER).value()));

    QLabel * productname = new QLabel("UNKNOWN");
    if (infoMap.contains(PRODUCTNAME))
        productname->setText(QString(infoMap.find(PRODUCTNAME).value()));

    QLabel * version = new QLabel("UNKNOWN");
    if (infoMap.contains(VERSION))
        version->setText(QString(infoMap.find(VERSION).value()));

    QLabel * serialnumber = new QLabel("UNKNOWN");
    if (infoMap.contains(SERIALNUMBER))
        serialnumber->setText(QString(infoMap.find(SERIALNUMBER).value()));

    ui->sysinfoFormLayout->setHorizontalSpacing(70);
    ui->sysinfoFormLayout->addRow(tr("Manufacturers:"), manufacturers);
    ui->sysinfoFormLayout->addRow(tr("Product Name:"), productname);
    ui->sysinfoFormLayout->addRow(tr("Version:"), version);
    ui->sysinfoFormLayout->addRow(tr("Serial Number:"), serialnumber);
//    ui->sysinfoFormLayout->addRow(tr("hostname:"), "hostname");
//    ui->sysinfoFormLayout->addRow(tr("running time:"), "running time");
//    ui->sysinfoFormLayout->addRow(tr("os type:"), "os type");
//    ui->sysinfoFormLayout->addRow(tr("os version"), "os version");
//    ui->sysinfoFormLayout->addRow(tr("system bit"), "system bit");
//    ui->sysinfoFormLayout->addRow(tr("kernel version"), "kernel version");
//    ui->sysinfoFormLayout->addRow(tr("architecture"), "architecture");

    //设置桌面环境logo
    envlogoLabel = new QLabel(pluginWidget);
    envlogoLabel->setAutoFillBackground(true);
    envlogoLabel->setScaledContents(true);
    QPixmap envlogoPixmap("://logo.svg");
    envlogoLabel->setPixmap(envlogoPixmap);
    envlogoLabel->setFixedSize(envlogoPixmap.size());
    envlogoLabel->setGeometry(QRect(pluginWidget->width() - 70 - envlogoLabel->width(), ui->summaryLabel->geometry().bottom() + 25, envlogoLabel->width(), envlogoLabel->height()));

    //
    logoLabel = new QLabel(pluginWidget);
    logoLabel->setAutoFillBackground(true);
    logoLabel->setScaledContents(true); //自动缩放，显示图像大小自动调整为QLabel大小

    QPixmap logopixmap;
    if (infoMap.contains(MANUFACTURER)){
        logopixmap = QPixmap(QString("://manufacturers/%1.jpg").arg(QString(infoMap.find(MANUFACTURER).value())));
    }
    else{
        logopixmap = QPixmap("://manufacturers/UBUNTUKYLIN.jpg");
    }
    logoLabel->setPixmap(logopixmap);
    logoLabel->setFixedSize(logopixmap.size());
    logoLabel->setGeometry(QRect(pluginWidget->width() - 70 - logoLabel->width(), ui->infoLabel->geometry().bottom() + 25, logoLabel->width(), logoLabel->height()));
//    logoLabel->setMask(logopixmap.mask());
}

void About::_call_dbus_get_computer_info(){
    interface = new QDBusInterface("com.control.center.qt.systemdbus",
                                     "/",
                                     "com.control.center.interface",
                                     QDBusConnection::systemBus());

    if (!interface->isValid()){
        qCritical() << "Create Client Interface Failed: " << QDBusConnection::systemBus().lastError();
        return;
    }

//    QDBusConnection::systemBus().connect("com.control.center.qt.systemdbus",
//                                         "/",
//                                         "com.control.center.interface",
//                                         "computerinfo", this,
//                                         SLOT(get(QString)));

    QDBusReply<QString> reply =  interface->call("GetComputerInfo");
    if (reply.isValid()){
        computerinfo =  reply.value();
    }
    else {
        qCritical() << "Call 'GetComputerInfo' Failed!" << reply.error().message();
    }


    //async
//    QDBusPendingCall async = interface->asyncCall("GetComputerInfo");
//    QDBusPendingCallWatcher * watcher = new QDBusPendingCallWatcher(async, this);

//    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(call_finished_slot(QDBusPendingCallWatcher*)));
}

//void About::call_finished_slot(QDBusPendingCallWatcher * call){
//    qDebug() << "----------------start------------>";
//    QDBusPendingReply<QString> reply = *call;
//    if (!reply.isError()){
//        QString info = reply.argumentAt<0>();
//        qDebug() << "-----------0--->" << "\n" << info;
//    }
//    else{
//        qDebug() << reply.error().message();
//    }
//    call->deleteLater();
//}
