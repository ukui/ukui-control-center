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
#include <QHBoxLayout>
#include <QDebug>
#include <QMovie>
#include <QDir>
#include <QDesktopWidget>

#include <locale.h>
#include <libintl.h>

const char kTimezoneDomain[] = "installer-timezones";
const char kDefaultLocale[] = "en_US.UTF-8";

#define FORMAT_SCHEMA "org.ukui.control-center.panel.plugins"
#define TIME_FORMAT_KEY "hoursystem"
#define DATE_KEY "date"

#define STYLE_FONT_SCHEMA "org.ukui.style"
#define SYSTEM_FONT_EKY "system-font-size"
#define SYSTEM_NAME_KEY "system-font"
#define PANEL_GSCHEMAL   "org.ukui.control-center.panel.plugins"
#define CALENDAR_KEY     "calendar"
#define DATE_FORMATE_KEY "date"
#define TIME_KEY         "hoursystem"

DateTime::DateTime() : mFirstLoad(true)
{
    pluginName = tr("Date & Time");
    pluginType = SYSTEM;
}

DateTime::~DateTime()
{
    if (!mFirstLoad) {
        delete ui;
        delete m_formatsettings;
        delete m_datetimeiface;
        delete m_datetimeiproperties;
    }
}

QString DateTime::get_plugin_name(){
    return pluginName;
}

int DateTime::get_plugin_type(){
    return pluginType;
}

QWidget *DateTime::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        m_initPath =  QString("%1/%2/%3").arg(QDir::homePath()).arg(".config/ukui").arg("timeZone.ini");//构造函数中初始化设置信息
        ui = new Ui::DateTime;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    //    pluginWidget->setStyleSheet("background: #ffffff;");
        ui->setupUi(pluginWidget);

        ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
        m_zoneinfo = new ZoneInfo;
        m_timezone = new TimeZoneChooser;
        m_itimer = new QTimer();
        m_itimer->start(50);
        connect(m_itimer,SIGNAL(timeout()), this, SLOT(datetime_update_slot()));

        m_formTimeBtn = new SwitchButton;
        //~ contents_path /datetime/24-hour clock
        m_formTimeLabel = new QLabel(tr("24-hour clock"));
        ui->label->close();
        m_ntpBtn = new SwitchButton;
        m_ntpLabel = new QLabel(tr("Sync system time"));
        QPixmap pixmap_1=(QString("://img/plugins/datetime/Time.png"));
        ui->label->setStyleSheet("background:transparent");
        ui->label->setFixedSize(32,32);
        ui->label->setPixmap(pixmap_1);
        QPixmap pixmap_2=(QString("://img/plugins/datetime/time-zone.png"));
        ui->chgzonebtn_label->setStyleSheet("background:transparent");
        ui->chgzonebtn_label->setFixedSize(32,32);
        ui->chgzonebtn_label->setPixmap(pixmap_2);
        //隐藏原来的时间同步按钮
        ui->synsystimeBtn->hide();
        //隐藏调整时间按钮
        ui->chgtimebtn->hide();
        //隐藏24小时制调整按钮
        ui->frame_2->hide();
        //隐藏时区选择功能
        ui->chgzonebtn->hide();
        ui->chgzonebtn_label->hide();
        //初始化gsettings
        const QByteArray id(FORMAT_SCHEMA);
        const QByteArray idd(PANEL_GSCHEMAL);
        const QByteArray styleID(STYLE_FONT_SCHEMA);
        if(QGSettings::isSchemaInstalled(id) && QGSettings::isSchemaInstalled(styleID) && QGSettings::isSchemaInstalled(idd)) {
            m_formatsettings = new QGSettings(id);
            stylesettings = new QGSettings(styleID);
            m_gsettings = new QGSettings(idd);
        }
        connectGSetting();
        //初始化dbus
        m_datetimeiface = new QDBusInterface("org.freedesktop.timedate1",
                                           "/org/freedesktop/timedate1",
                                           "org.freedesktop.timedate1",
                                           QDBusConnection::systemBus());

        m_datetimeiproperties = new QDBusInterface("org.freedesktop.timedate1",
                                                 "/org/freedesktop/timedate1",
                                                 "org.freedesktop.DBus.Properties",
                                                 QDBusConnection::systemBus());
        QDBusInterface * iproperty = new QDBusInterface("org.freedesktop.timedate1",
                                                "/org/freedesktop/timedate1",
                                                "org.freedesktop.DBus.Properties",
                                                QDBusConnection::systemBus());
        QDBusReply<QMap<QString, QVariant> > reply = iproperty->call("GetAll", "org.freedesktop.timedate1");
        if (reply.isValid()){
            QMap<QString, QVariant> propertyMap;
            propertyMap = reply.value();
            QString is_sync = propertyMap.find("NTP").value().toString();
            qDebug()<<"是否同步: "<<is_sync;
            if(is_sync == QString::fromLocal8Bit("true")){
                m_ntpBtn->setChecked(true);
            } else {
                m_ntpBtn->setChecked(false);
            }
        } else {
            qDebug() << "reply failed";
        }
        ui->calendarComBox->addItem(tr("solar calendar"));

        QString locale = QLocale::system().name();
        //系统语言为中文时加入农历选项
        if ("zh_CN" == locale){
            ui->calendarComBox->addItem(tr("lunar"));
        }
        if ("en_US" == locale){
            ui->calendarComBox->setCurrentIndex(0);
            ui->calendarComBox->removeItem(1);
        }

        QString currentsecStr;
        QDateTime current = QDateTime::currentDateTime();

        currentsecStr = current.toString("yyyy/MM/dd ");
        ui->dateComBox->addItem(currentsecStr);

        currentsecStr = current.toString("yyyy-MM-dd ");
        ui->dateComBox->addItem(currentsecStr);

        QString currentsecStr_1 ;
        currentsecStr_1 = current.toString("hh:mm:ss");
        ui->timeComBox->addItem(currentsecStr_1);
        currentsecStr_1 = current.toString("hh:mm:ss AP");
        ui->timeComBox->addItem(currentsecStr_1);

        component_init();
        status_init();
        initConnection();
    }
    return pluginWidget;
}

void DateTime::plugin_delay_control(){

}

const QString DateTime::name() const {

    return QStringLiteral("datetime");
}

void DateTime::initConnection(){
    connect(ui->chgtimebtn,SIGNAL(clicked()),this,SLOT(changetime_slot()));
    connect(ui->chgzonebtn,SIGNAL(clicked()),this,SLOT(changezone_slot()));
    connect(m_formTimeBtn, SIGNAL(checkedChanged(bool)),this,SLOT(time_format_clicked_slot(bool)));
    connect(ui->calendarComBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_calender_slot(int)));
    connect(ui->dateComBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_date_slot(int)));
    connect(ui->timeComBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_time_slot(int)));
    connect(m_timezone, &TimeZoneChooser::confirmed, this, [this] (const QString &timezone) {
//        qDebug()<<"timezone is---------->"<<timezone<<endl;
        m_datetimeiface->call("SetTimezone", timezone, true);
        m_timezone->hide();
        const QString locale = QLocale::system().name();
        QDBusReply<QMap<QString, QVariant> > reply = m_datetimeiproperties->call("GetAll","org.freedesktop.timedate1");
        if (reply.isValid()) {
            QMap<QString, QVariant> propertyMap;
            propertyMap = reply.value();
            currentZone = propertyMap.find("Timezone").value().toString();
        }
        if (currentZone == timezone) {
            QString localizedTimezone = QString(tr("timezone: %1")).arg(getLocalTimezoneName(timezone, locale));
//            ui->timezoneLabel->setText(localizedTimezone);
            ui->timezoneLabel->setText(tr("timezone: Beijing"));
        }
        QSettings settings(m_initPath, QSettings::IniFormat);
        int currentZoneIndex = settings.value("timeZone_Chooser/zone").toInt();
        m_timezone->timeZoneBox->setCurrentIndex(currentZoneIndex);
        qDebug()<<"currentZoneIndex"<<currentZoneIndex;
    });
    connect(ui->synsystimeBtn,SIGNAL(clicked()),this,SLOT(rsync_with_network_slot()));
    connect(m_ntpBtn, SIGNAL(checkedChanged(bool)), this, SLOT(synctime_slot(bool)));
}

void DateTime::component_init(){
    ui->titleLabel->setContentsMargins(0,0,0,16);
//    ui->timeClockLable->setContentsMargins(0,0,0,16);
    ui->pointLabel_1->setContentsMargins(0,0,0,6);
    ui->pointLabel_2->setContentsMargins(0,0,0,6);

    ui->ntpFrame->setAutoFillBackground(false);
    ui->hourFrame->setAutoFillBackground(false);

    ////~ contents_path /datetime/Sync system time
    ui->synsystimeBtn->setText(tr("Sync system time"));
    ui->chgtimebtn->setText(tr("Change time"));
    ////~ contents_path /datetime/Change time zone
    ui->chgzonebtn->setText(tr("Change time..."));
    ui->chgzonebtn->setToolTip(tr("Change time zone"));
    //~ contents_path /datetime/calendar
    ui->calendarlabel->setText(tr("calendar"));
    //~ contents_path /datetime/date
    ui->datelabel->setText(tr("date"));
    const QByteArray id("org.ukui.style");
    if (QGSettings::isSchemaInstalled(id)){
        themeSetting = new QGSettings(id);
        QString m_themeMode = themeSetting->get("style-name").toString();
        if ("ukui-white" == m_themeMode || "ukui-default" == m_themeMode || "ukui-light" == m_themeMode || "ukui-white-unity" == m_themeMode) {
            ui->chgzonebtn->setStyleSheet("QPushButton#chgzonebtn{background-color:#F6F6F6;border: 1px;"
                                       "border-radius: 12px;font-size: 16px; color: palette(windowText);}"
                                       "QPushButton:hover:!pressed#chgzonebtn:hover{background: #2FB3E8;"
                                       "border: 1px;border-radius: 12px;font-size: 16px; color: palette(windowText);}");
        } else if ("ukui-dark" == m_themeMode || "ukui-black" == m_themeMode || "ukui-black-unity" == m_themeMode){
            ui->chgzonebtn->setStyleSheet("QPushButton#chgzonebtn{background-color:#6F7176;border: 1px;"
                                      "border-radius: 12px;font-size: 16px; color: palette(windowText);}"
                                      "QPushButton:hover:!pressed#chgzonebtn:hover{background: #2FB3E8;"
                                      "border: 1px;border-radius: 12px;font-size: 16px; color: palette(windowText);}");
        }
        connect(themeSetting, &QGSettings::changed, this, [=](const QString &key){
            QString m_themeMode = themeSetting->get("style-name").toString();
            if ("ukui-white" == m_themeMode || "ukui-default" == m_themeMode || "ukui-light" == m_themeMode || "ukui-white-unity" == m_themeMode) {
                ui->chgzonebtn->setStyleSheet("QPushButton#chgzonebtn{background-color:#F6F6F6;border: 1px;"
                                           "border-radius: 12px;font-size: 16px; color: palette(windowText);}"
                                           "QPushButton:hover:!pressed#chgzonebtn:hover{background: #2FB3E8;"
                                           "border: 1px;border-radius: 12px;font-size: 16px; color: palette(windowText);}");
            } else if ("ukui-dark" == m_themeMode || "ukui-black" == m_themeMode || "ukui-black-unity" == m_themeMode){
                ui->chgzonebtn->setStyleSheet("QPushButton#chgzonebtn{background-color:#6F7176;border: 1px;"
                                           "border-radius: 12px;font-size: 16px; color: palette(windowText);}"
                                           "QPushButton:hover:!pressed#chgzonebtn:hover{background: #2FB3E8;"
                                           "border: 1px;border-radius: 12px;font-size: 16px; color: palette(windowText);}");
            }
        });
    }

    ui->chgLayout->setSpacing(16);
    ui->syslabel->setVisible(false);

    ui->endlabel->setVisible(false);

    QHBoxLayout *hourLayout = new QHBoxLayout(ui->hourFrame);
    hourLayout->setContentsMargins(16,8,16,8);
    hourLayout->addWidget(m_formTimeLabel);
    hourLayout->addWidget(m_formTimeBtn);

    QHBoxLayout *ntpLayout = new QHBoxLayout(ui->ntpFrame);

    ntpLayout->addWidget(m_ntpLabel);
    ntpLayout->addWidget(m_ntpBtn);

    QDateTime currentime = QDateTime::currentDateTime();
    QString timeAndWeek = currentime.toString("yyyy/MM/dd ddd");
    ui->dateLabel->setText(timeAndWeek);

    //因为ntpd和systemd的网络时间同步会有冲突，所以安装了ntp的话，禁止使用控制面板设置网络时间同步
    QFileInfo fileinfo("/usr/sbin/ntpd");
    if (fileinfo.exists()){
        ui->synsystimeBtn->setVisible(false);
    }

    QFile tzfile("://zoneUtc");
    if(!tzfile.open(QIODevice::ReadOnly | QIODevice::Text)){
         qDebug("TZ File Open Failed");
    } else {
        QTextStream txt(&tzfile);
        int index = 0;
        qDebug()<<"TODO------->language problem"<<endl;
//        while (!tzfile.atEnd()) {
        //由于tzfile.atEnd只能读取到第338行，无法加载全部时区，故改用qtextstream读取全部行数
        while (!txt.atEnd()) {
            QStringList lineList = txt.readLine().split("\t");
            tzindexMapEn.insert(lineList.at(0),index);
            tzindexMapCN.insert(lineList.at(1),index);
            index++;
//            qDebug()<<index<<":"<<lineList.at(0)<<" "<<lineList.at(1);
        }
    }

    //初始化区域数据格式Combox控件的值
    QString clac = m_gsettings->get(CALENDAR_KEY).toString();
    QString locale = QLocale::system().name();
    //系统语言为中文时加入农历选项
    if ("zh_CN" == locale){
        if ("solarlunar" == clac) {
            ui->calendarComBox->setCurrentIndex(0);
        } else {
            ui->calendarComBox->setCurrentIndex(1);
        }
    }

    QString dateFormat = m_gsettings->get(DATE_FORMATE_KEY).toString();
    if ("cn" == dateFormat) {
       ui->dateComBox->setCurrentIndex(0);
    } else {
       ui->dateComBox->setCurrentIndex(1);
    }
    QString timeformate = m_gsettings->get(TIME_KEY).toString();
    if ("24" == timeformate) {
       ui->timeComBox->setCurrentIndex(0);
    } else {
       ui->timeComBox->setCurrentIndex(1);
    }

    tzfile.close();

}

void DateTime::writeGsettings(const QString &key, const QString &value) {
    if(!m_gsettings) {
        return ;
    }

    const QStringList list = m_gsettings->keys();
    if (!list.contains(key)) {
        return ;
    }
    m_gsettings->set(key,value);
}

void DateTime::change_calender_slot(int index) {
    switch (index) {
    case 0:
        writeGsettings("calendar", "solarlunar");
        break;
    case 1:
        writeGsettings("calendar", "lunar");
        break;
    }
}

void DateTime::change_date_slot(int index) {
    switch (index) {
    case 0:
        writeGsettings("date", "cn");
        break;
    case 1:
        writeGsettings("date", "en");
        break;
    }
}

void DateTime::change_time_slot(int index) {
    switch (index) {
    case 0:
        writeGsettings("hoursystem", "24");
        break;
    case 1:
        writeGsettings("hoursystem", "12");
        break;
    }
}

void DateTime::status_init(){

    //时区
    const QString locale = QLocale::system().name();
    QDBusReply<QVariant> tz = m_datetimeiproperties->call("Get", "org.freedesktop.timedate1", "Timezone");
    QMap<QString, int>::iterator it = tzindexMapEn.find(tz.value().toString());
    qDebug()<<"时区："<<tz.value();
    if(it != tzindexMapEn.end()){
        for(QMap<QString,int>::iterator itc = tzindexMapCN.begin();itc!=tzindexMapCN.end();itc++)
        {
            if(itc.value() == it.value()){
//                ui->timezoneLabel->setText(getLocalTimezoneName(itc.key(), locale));
//                ui->timezoneLabel->setText(QString(tr("timezone: %1")).arg(getLocalTimezoneName(it.key(), locale)));
                ui->timezoneLabel->setText(tr("timezone: Beijing"));
                break;
            }
        }
    } else {
        QMap<QString, int>::iterator defaultit =  tzindexMapEn.find(DEFAULT_TZ);
//        ui->timezoneLabel->setText(QString(tr("timezone: %1")).arg(getLocalTimezoneName(defaultit.key(), locale)));
        ui->timezoneLabel->setText(tr("timezone: Beijing"));
    }


    loadHour();
}

bool DateTime::fileIsExits(const QString &filepath) {
    QFile file(filepath);
    if(file.exists()) {
        return true;
    } else {
        return false;
    }
}

void DateTime::datetime_update_slot(){
    QString dateformat;
    if(m_formatsettings) {
        QStringList keys = m_formatsettings->keys();
        if(keys.contains("date")) {
            dateformat =  m_formatsettings->get(DATE_KEY).toString();
        }
    }

    //当前时间
    current = QDateTime::currentDateTime();
//    qDebug()<<"current time is-------->"<<current<<endl;

    QString currentsecStr ;
    if(m_formTimeBtn->isChecked()){
        currentsecStr = current.toString("hh:mm:ss");
        currentSec = currentsecStr;
        currentHour= currentSec.split(':').at(0);
        currentMin= currentSec.split(':').at(1);
        currentSecClock= currentSec.split(':').at(2);
    } else {
        currentsecStr = current.toString("hh:mm:ss AP");
        currentSec = currentsecStr.split(' ').first();
        currentHour= currentSec.split(':').at(0);
        currentMin= currentSec.split(':').at(1);
        currentSecClock= currentSec.split(':').at(2);
    }
    QString timeAndWeek;
    if ("cn" == dateformat) {
       timeAndWeek = current.toString("yyyy/MM/dd ddd");
    } else {
       timeAndWeek = current.toString("yyyy-MM-dd ddd");
    }

    QString currentsecStr_1 ;
    if(m_formTimeBtn->isChecked()){
        currentsecStr_1 = current.toString("");
    }else{
        currentsecStr_1 = current.toString("AP");
//        currentsecStr_1.replace("上午", "AM");
//        currentsecStr_1.replace("下午", "PM");
    }
    QString timeAndWeek_1;
    if ("cn" == dateformat) {
       timeAndWeek_1 = current.toString("yyyy/MM/dd ddd");
    } else {
       timeAndWeek_1 = current.toString("yyyy-MM-dd ddd");
    }

//    qDebug()<<"year is----------->"<<timeAndWeek<<endl;
    ui->dateLabel->setText(timeAndWeek_1);

    QString sysfont = stylesettings->get(SYSTEM_NAME_KEY).toString();
    ui->HourClockLable->setFont(QFont( sysfont , 32 ,  QFont::Light) );
    ui->HourClockLable->setMinimumSize(QSize(47,47));
    ui->HourClockLable->setMaximumSize(QSize(47,47));
    ui->HourClockLable->setText(currentHour);
    ui->HourClockLable->adjustSize();
    ui->MinClockLabel->setFont(QFont( sysfont , 32 ,  QFont::Light) );
    ui->MinClockLabel->setMinimumSize(QSize(47,47));
    ui->MinClockLabel->setMaximumSize(QSize(47,47));
    ui->MinClockLabel->setText(currentMin);
    ui->SecClockLabel->setFont(QFont( sysfont , 32 ,  QFont::Light) );
    ui->SecClockLabel->setMinimumSize(QSize(47,47));
    ui->SecClockLabel->setMaximumSize(QSize(47,47));
    ui->SecClockLabel->setText(currentSecClock);

    ui->pointLabel_1->setFont(QFont( sysfont , 32 ,  QFont::Light) );
    ui->pointLabel_1->setMinimumHeight(47);
    ui->pointLabel_1->setMaximumHeight(47);
    ui->pointLabel_1->setText(":");

    ui->pointLabel_2->setFont(QFont( sysfont , 32 ,  QFont::Light) );
    ui->pointLabel_2->setMinimumHeight(47);
    ui->pointLabel_2->setMaximumHeight(47);
    ui->pointLabel_2->setText(":");

    ui->timeClockLable_2->setFont(QFont( sysfont , 32 ,  QFont::Light) );
    ui->timeClockLable_2->setMinimumHeight(47);
    ui->timeClockLable_2->setMaximumHeight(47);
    ui->timeClockLable_2->setText(currentsecStr_1);

    QDateTime current = QDateTime::currentDateTime();
    QString currentsecStr1 ;
    currentsecStr1 = current.toString("hh:mm:ss");
    ui->timeComBox->setItemText(0,currentsecStr1);
    currentsecStr1 = current.toString("hh:mm:ss AP");
//    currentsecStr.replace("上午", "AM");
//    currentsecStr.replace("下午", "PM");
    ui->timeComBox->setItemText(1,currentsecStr1);

}

void DateTime::changetime_slot(){
    ChangtimeDialog *dialog = new ChangtimeDialog(m_formTimeBtn->isChecked());
    connect(dialog, SIGNAL(time_changed()), this, SLOT(close_sync()));
    dialog->setWindowTitle(tr("change time"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    m_itimer->stop();
    m_itimer->start();
    dialog->exec();

}

void DateTime::close_sync(){
    if(m_ntpBtn->isChecked()){
        m_ntpBtn->setChecked(false);
    }
}

void DateTime::changezone_slot(){
//    qDebug()<<"changezone_slot------->"<<endl;

    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = m_timezone->width();
    int y = m_timezone->height();
    m_timezone->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
    m_timezone->show();

    m_timezone->setMarkedTimeZoneSlot(m_zoneinfo->getCurrentTimzone());
}

void DateTime::synctime_slot(bool is_sync){
    if(is_sync){
        //同步系统时间
        QDBusMessage result = m_datetimeiface->call("SetNTP", true, true);
        QList<QVariant> outArgs = result.arguments();
        QString status = outArgs.at(0).value<QString>();
        if(status!="")
        {
            m_ntpBtn->disabledSig = true;
            m_ntpBtn->setChecked(false);
        }
    } else {
        //关闭同步
        QDBusMessage result = m_datetimeiface->call("SetNTP", false, true);
        QList<QVariant> outArgs = result.arguments();
        QString status = outArgs.at(0).value<QString>();
        if(status!="")
        {
            m_ntpBtn->disabledSig = true;
            m_ntpBtn->setChecked(true);
        }
    }
}


void DateTime::changezone_slot(QString zone){
    m_datetimeiface->call("SetTimezone", zone, true);
}

void DateTime::time_format_clicked_slot(bool flag){    
    if (!m_formatsettings){
        qDebug()<<"org.ukui.control-center.panel.plugins not installed"<<endl;
        return;
    }
    QStringList keys = m_formatsettings->keys();
    if (keys.contains("hoursystem")) {
        if(flag == true) {
            m_formatsettings->set(TIME_FORMAT_KEY, "24");
        } else {
            m_formatsettings->set(TIME_FORMAT_KEY, "12");
        }
    }
    //重置时间格式
    m_itimer->stop();
    m_itimer->start(1000);
}

void DateTime::showendLabel(){
    ui->syslabel->setVisible(false);
    if(ui->syslabel->isVisible()){
        ui->endlabel->setVisible(false);
    }else {
        ui->endlabel->setVisible(true);
    }
    QTimer::singleShot(2*1000,this,SLOT(hidendLabel()));
}

void DateTime::hidendLabel(){
    ui->endlabel->setVisible(false);
}

void DateTime::rsync_with_network_slot(){
//    qDebug()<<"TODO------> sleep waies?"<<endl;

    m_datetimeiface->call("SetNTP", true, true);

//    QMovie *loadgif = new QMovie(":/sys.gif");
//    loadgif->start();
//    ui->syslabel->setVisible(true);
//    ui->syslabel->setMovie(loadgif);
//    ui->syslabel->setScaledContents(true);
//    ui->syslabel->setStyleSheet("QLabel#syslabel{border-radius:4px;}");

//    QTimer::singleShot(2*1000,this,SLOT(showendLabel()));
}

void DateTime::loadHour() {
    if (!m_formatsettings) {
        qDebug()<<"org.ukui.control-center.panel.plugins not installed"<<endl;
        return;
    }    
    QStringList keys = m_formatsettings->keys();
    QString format;
    if (keys.contains("hoursystem")) {
        format = m_formatsettings->get(TIME_FORMAT_KEY).toString();
    }
    if (format == "24") {
        m_formTimeBtn->setChecked(true);
    } else {
        m_formTimeBtn->setChecked(false);
    }
}

void DateTime::connectGSetting() {
    connect(m_formatsettings, &QGSettings::changed, this, [=] (const QString &key) {
//            qDebug()<<"status changed ------------>"<<endl;
        if (key == "hoursystem") {
            QString value = m_formatsettings->get(TIME_FORMAT_KEY).toString();
            bool checked = (value == "24" ? true : false);
            m_formTimeBtn->setChecked(checked);
        }
        if (key == "date") {
            QString value = m_formatsettings->get(DATE_KEY).toString();
        }
    });
}

QString DateTime::getLocalTimezoneName(QString timezone, QString locale) {
    (void) setlocale(LC_ALL, QString(locale + ".UTF-8").toStdString().c_str());
    const QString local_name(dgettext(kTimezoneDomain,
                                      timezone.toStdString().c_str()));
    int index = local_name.lastIndexOf('/');
    if (index == -1) {
      // Some translations of locale name contains non-standard char.
      index = local_name.lastIndexOf("∕");
    }

    // Reset locale.
    (void) setlocale(LC_ALL, kDefaultLocale);

    return (index > -1) ? local_name.mid(index + 1) : local_name;
}
