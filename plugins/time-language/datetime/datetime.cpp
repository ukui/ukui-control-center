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
#include "datetime.h"
#include "ui_datetime.h"
#include "SwitchButton/switchbutton.h"
#include <QHBoxLayout>

#include <QDebug>

DateTime::DateTime()
{
    ui = new Ui::DateTime;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    pluginWidget->setStyleSheet("background: #ffffff;");
    ui->setupUi(pluginWidget);

    pluginName = tr("datetime");
    pluginType = DATETIME;

    itimer = new QTimer();
    itimer->start(1000);
    connect(itimer,SIGNAL(timeout()), this, SLOT(datetime_update_slot()));

    formTimeBtn = new SwitchButton;
    formTimeLabel = new QLabel(tr("24-hour clock"));


    //初始化dbus
    datetimeiface = new QDBusInterface("org.freedesktop.timedate1",
                                       "/org/freedesktop/timedate1",
                                       "org.freedesktop.timedate1",
                                       QDBusConnection::systemBus());

    datetimeiproperties = new QDBusInterface("org.freedesktop.timedate1",
                                             "/org/freedesktop/timedate1",
                                             "org.freedesktop.DBus.Properties",
                                             QDBusConnection::systemBus());

    component_init();
    status_init();

    connect(ui->chgtimebtn,SIGNAL(clicked()),this,SLOT(changetime_slot()));

}

DateTime::~DateTime()
{
    delete ui;
}

QString DateTime::get_plugin_name(){
    return pluginName;
}

int DateTime::get_plugin_type(){
    return pluginType;
}

QWidget *DateTime::get_plugin_ui(){
    return pluginWidget;
}

void DateTime::plugin_delay_control(){

}

void DateTime::component_init(){
    ui->tilteLable->setContentsMargins(0,0,0,16);
    ui->timeLable->setContentsMargins(0,0,0,16);

    ui->synsystimeBtn->setStyleSheet("QPushButton{background-color:#E9E9E9;border-radius:4px}"
                                   "QPushButton:hover{background-color: #3D6BE5;};border-radius:4px");

    ui->synsystimeBtn->setText(tr("Sync system time"));

    ui->chgtimebtn->setStyleSheet("QPushButton{background-color:#E5E7E9;border-radius:4px}"
                                   "QPushButton:hover{background-color: #3D6BE5;};border-radius:4px");

    ui->chgtimebtn->setText(tr("Change time"));

    ui->chgzonebtn->setStyleSheet("QPushButton{background-color:#E5E7E9;border-radius:4px}"
                                   "QPushButton:hover{background-color: #3D6BE5;};border-radius:4px");
    ui->chgzonebtn->setText(tr("Change time zone"));



    ui->chgLayout->setSpacing(16);

    ui->hourWidget->setStyleSheet("background-color:#E5E7E9;border-radius:6px");

    QHBoxLayout *hourLayout = new QHBoxLayout(ui->hourWidget);

    hourLayout->addWidget(formTimeLabel);
    hourLayout->addWidget(formTimeBtn);


    //ui->hourwidget->addWidget(formTimeLabel);


    QDateTime currentime = QDateTime::currentDateTime();
    QString timeAndWeek = currentime.toString("yyyy/MM/dd ddd");
    ui->dateLabel->setText(timeAndWeek);

    QFile tzfile("://zoneUtc");
    if(!tzfile.open(QIODevice::ReadOnly | QIODevice::Text)){
         qDebug("TZ File Open Failed");
    } else {
        QTextStream txt(&tzfile);
        int index = 0;
        qDebug()<<"TODO------->language problem"<<endl;
        while (!tzfile.atEnd()) {
            QStringList lineList = txt.readLine().split("\t");
            tzindexMapEn.insert(lineList.at(0),index);
            tzindexMapCN.insert(lineList.at(1),index);
            index++;
        }
    }
    tzfile.close();

}

void DateTime::status_init(){    

    //时区
    QDBusReply<QVariant> tz = datetimeiface->call("Get", "org.freedesktop.timedate1", "Timezone");
    QMap<QString, int>::iterator it = tzindexMapEn.find(tz.value().toString());
    qDebug()<<"TODO-------》"<<tz.value().toString()<<endl;
    if(it != tzindexMapEn.end()){
        for(QMap<QString,int>::iterator itc = tzindexMapCN.begin();itc!=tzindexMapCN.end();itc++)
        {
            if(itc.value() == it.value()){
                ui->timezoneLabel->setText(itc.key());
                break;
            }
        }
    } else {
        QMap<QString, int>::iterator defaultit =  tzindexMapEn.find(DEFAULT_TZ);
        ui->timezoneLabel->setText(defaultit.key());
    }
}



void DateTime::datetime_update_slot(){
    QFont ft;
    ft.setPointSize(15);
    ft.setBold(true);
    //当前时间    

    QDateTime current = QDateTime::currentDateTime();    
    QString currentsecStr = current.toString("hh : mm : ss");

    ui->timeLable->setText(currentsecStr);
    ui->timeLable->setFont(ft);

}

void DateTime::changetime_slot(){
    ChangtimeDialog *dialog = new ChangtimeDialog;


    dialog->setWindowTitle(tr("change time"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();



}

