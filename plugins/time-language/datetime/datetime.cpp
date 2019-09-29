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

#include <QDebug>

DateTime::DateTime()
{
    ui = new Ui::DateTime;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("datetime");
    pluginType = TIME_LANGUAGE;

    QTimer * itimer = new QTimer();
    itimer->start(1000); //一秒
    connect(itimer, SIGNAL(timeout()), this, SLOT(datetime_update_slot()));

    chtimer = new QTimer();
    chtimer->setInterval(1000);
    connect(chtimer, SIGNAL(timeout()), this, SLOT(sub_time_update_slot()));

    //初始化gsettings
    const QByteArray id(FORMAT_SCHEMA);
    formatsettings = new QGSettings(id);

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
}

DateTime::~DateTime()
{
    delete ui;
    delete formatsettings;

    delete datetimeiface;
    delete datetimeiproperties;
    datetimeiface = NULL;
    datetimeiproperties = NULL;

}

QString DateTime::get_plugin_name(){
    return pluginName;
}

int DateTime::get_plugin_type(){
    return pluginType;
}

CustomWidget *DateTime::get_plugin_ui(){
    return pluginWidget;
}

void DateTime::plugin_delay_control(){

}

void DateTime::component_init(){

    ntpSwitchBtn = new SwitchButton;
    ntpSwitchBtn->setAttribute(Qt::WA_DeleteOnClose);
    ui->ntpHLayout->insertWidget(1, ntpSwitchBtn);

    longtimeSwitchBtn = new SwitchButton;
    longtimeSwitchBtn->setAttribute(Qt::WA_DeleteOnClose);
    ui->longtimeHLayout->insertWidget(1, longtimeSwitchBtn);

    //网络时间同步
    QDBusReply<QVariant> canNTP = datetimeiproperties->call("Get", "org.freedesktop.timedate1", "CanNTP");
    ntpSwitchBtn->setEnabled(canNTP.value().toBool());


    //因为ntpd和systemd的网络时间同步会有冲突，所以安装了ntp的话，禁止使用控制面板设置网络时间同步
    QFileInfo fileinfo("/usr/sbin/ntpd");
    if (fileinfo.exists()){
        ntpSwitchBtn->setChecked(false);
        ntpSwitchBtn->setEnabled(false);
    }
    else
        ui->ntptipLabel->hide();

//    QFile tzfile(TZ_DATA_FILE);
    QFile tzfile("://zoneUtc");
    if (!tzfile.open(QIODevice::ReadOnly | QIODevice::Text))
        qDebug("TZ File Open Failed");
    else{
        QTextStream txt(&tzfile);
        int index = 0;
        while(!tzfile.atEnd()){
            QStringList lineList = txt.readLine().split("\t");
            ui->tzComboBox->insertItem(index, lineList.at(1));
            ui->tzComboBox->setItemData(index, QVariant(lineList.at(0)), Qt::UserRole);
            tzindexMap.insert(lineList.at(0), index);
            index++;
        }
    }
    tzfile.close();

    //设置时间制式在group中的id
    ui->buttonGroup->setId(ui->h12RadioBtn, 0);
    ui->buttonGroup->setId(ui->h24RadioBtn, 1);

    //一周的第一天
    ui->firstdayComboBox->addItem(tr("Monday"));
    ui->firstdayComboBox->addItem(tr("Sunday"));


    //时间
    hour_combobox_setup();
    for (int m = 0; m < 60; m++)
        ui->minComboBox->addItem(QString::number(m));

    for (int s = 0; s < 60; s++)
        ui->secComboBox->addItem(QString::number(s));

}

void DateTime::status_init(){
    //网络时间同步
    QDBusReply<QVariant> ntp = datetimeiproperties->call("Get", "org.freedesktop.timedate1", "NTP");
    ntpSwitchBtn->setChecked(ntp.value().toBool());
    ui->changePushButton->setEnabled(!ntp.value().toBool());

    //时区
    QDBusReply<QVariant> tz = datetimeiface->call("Get", "org.freedesktop.timedate1", "Timezone");
    QMap<QString, int>::iterator it =  tzindexMap.find(tz.value().toString());
    if (it != tzindexMap.end()){
        ui->tzComboBox->setCurrentIndex((int)it.value());
    }
    else{
        QMap<QString, int>::iterator defaultit =  tzindexMap.find(DEFAULT_TZ);
        ui->tzComboBox->setCurrentIndex((int)defaultit.value());
    }

    //时间制式
    bool use = formatsettings->get(TIME_FORMAT_KEY).toBool();
    if (use)
        ui->h24RadioBtn->setChecked(use);
    else
        ui->h12RadioBtn->setChecked(!use);

    //长时间
    bool showsecond = formatsettings->get(SHOW_SECOND).toBool();
    if (showsecond)
        longtimeSwitchBtn->setChecked(true);
    else
        longtimeSwitchBtn->setChecked(false);

    //一周第一天
    bool sundayfirst = formatsettings->get(WEEK_FORMAT_KEY).toBool();
    if (sundayfirst)
        ui->firstdayComboBox->setCurrentIndex(1);
    else
        ui->firstdayComboBox->setCurrentIndex(0);

    //二级页面当前时间
//    qDebug() << "***************************";
    QDateTime current = QDateTime::currentDateTime();
    QString currenthourStr = current.toString("hh");
    QString currentminStr = current.toString("mm");
//    qDebug() << currenthourStr << currentminStr;
    ui->hourComboBox->setCurrentIndex(currenthourStr.toInt());
    ui->minComboBox->setCurrentIndex(currentminStr.toInt());

    chtimer->start();

    connect(ui->changePushButton, &QPushButton::clicked, this, [=]{ui->StackedWidget->setCurrentIndex(1);});

    connect(ui->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(time_format_clicked_slot(int)));
    connect(ntpSwitchBtn, SIGNAL(checkedChanged(bool)), this, SLOT(rsync_with_network_slot(bool)));
    connect(ui->tzComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(tz_combobox_changed_slot(int)));
    connect(longtimeSwitchBtn, SIGNAL(checkedChanged(bool)), this, SLOT(longdt_changed_slot(bool)));

    connect(ui->applyPushBtn, SIGNAL(clicked(bool)), this, SLOT(apply_btn_clicked_slot()));

//    connect(ui->hourComboBox, SIGNAL(), this, SLOT(dt_change_delay_stop_slot()));
//    connect(ui->minComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(dt_change_delay_stop_slot()));
//    connect(ui->secComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(dt_change_delay_stop_slot()));
}

void DateTime::hour_combobox_setup(){
    ui->hourComboBox->clear();

    //获取时间制式，设置时间combobox
    bool use = formatsettings->get(TIME_FORMAT_KEY).toBool();
    if (use){
        for (int h = 0; h < 24; h++)
            ui->hourComboBox->addItem(QString::number(h));
    }
    else{
        for (int h = 0; h < 12; h++)
            ui->hourComboBox->addItem(QString::number(h));
    }
}

void DateTime::time_format_clicked_slot(int id){
    if (id)
        formatsettings->set(TIME_FORMAT_KEY, true);
    else
        formatsettings->set(TIME_FORMAT_KEY, false);

    //停止计时器，重置小时的combobox
    chtimer->stop();
    hour_combobox_setup();
    chtimer->start(1000);
}

void DateTime::datetime_update_slot(){
    //当前时间
    QDateTime current = QDateTime::currentDateTime();
    QString currentfix = current.toString("yyyy-MM-dd, hh:mm:ss ");
    ui->currentLabel->setText(currentfix);
}

void DateTime::sub_time_update_slot(){
    //当前时间
    QDateTime current = QDateTime::currentDateTime();
    QString currenthourStr = current.toString("hh");
    QString currentminStr = current.toString("mm");
    QString currentsecStr = current.toString("ss");
//    qDebug() << currenthourStr << ":" << currentminStr << ":" << currentsecStr;

    ui->secComboBox->setCurrentIndex(currentsecStr.toInt());
    if (currentsecStr.toInt() == 0)
        ui->minComboBox->setCurrentIndex(currentminStr.toInt());
    if (currentsecStr.toInt() == 0 && currentminStr.toInt() == 0)
        ui->hourComboBox->setCurrentIndex(currenthourStr.toInt());
}

void DateTime::rsync_with_network_slot(bool status){
    if (status){
        datetimeiface->call("SetNTP", true, true);
        ui->changePushButton->setEnabled(false);
    }
    else{
        datetimeiface->call("SetNTP", false, true);
        ui->changePushButton->setEnabled(true);
    }
}

void DateTime::tz_combobox_changed_slot(int index){
    QString tzString = ui->tzComboBox->itemData(index, Qt::UserRole).toString();
    datetimeiface->call("SetTimezone", tzString, true);
}

void DateTime::longdt_changed_slot(bool status){
    if (status)
        formatsettings->set(SHOW_SECOND, true);
    else
        formatsettings->set(SHOW_SECOND, false);
}

void DateTime::apply_btn_clicked_slot(){
//    qDebug() << ui->hourComboBox->currentIndex() << ui->minComboBox->currentIndex() << ui->secComboBox->currentIndex();

    QDate selected = ui->calendarWidget->selectedDate();
//    qDebug() << selected.year() << selected.month() << selected.day();

    QDate tmpdate(selected.year(), selected.month(), selected.day());
    QTime tmptime(ui->hourComboBox->currentIndex(), ui->minComboBox->currentIndex(), ui->secComboBox->currentIndex());
    QDateTime setdt(tmpdate, tmptime);

//    GDateTime *now;
//    gint64 unixdt;
//    now = g_date_time_new_local(selected.year(), selected.month(), selected.day(), ui->hourComboBox->currentIndex(),
//                                ui->minComboBox->currentIndex(), (gdouble)ui->secComboBox->currentIndex());
//    unixdt = g_date_time_to_unix(now);
//    qDebug() << unixdt << "***" << setdt.toSecsSinceEpoch() << "---" << (unixdt * G_TIME_SPAN_SECOND);

    // G_TIME_SPAN_SECOND来自glib，设置时间的逻辑来自gtk控制面板

    datetimeiface->call("SetTime", QVariant::fromValue(setdt.toSecsSinceEpoch() * G_TIME_SPAN_SECOND), false, true);
}
