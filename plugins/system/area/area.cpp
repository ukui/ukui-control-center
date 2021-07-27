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
#include "area.h"
#include "ui_area.h"
#include "dataformat.h"

#include <QDebug>
#include <QFile>
#include <QString>
#include <QDateTime>
#include <QTimer>
#include <QGSettings>
#include <KMessageBox>

#define PANEL_GSCHEMAL   "org.ukui.control-center.panel.plugins"
#define CALENDAR_KEY     "calendar"
#define DAY_KEY          "firstday"
#define DATE_FORMATE_KEY "date"
#define TIME_KEY         "hoursystem"

Area::Area() : mFirstLoad(true)
{
    pluginName = tr("Language & Region");
    pluginType = SYSTEM;
}

Area::~Area()
{
    if (!mFirstLoad) {
        delete ui;
        delete m_itimer;
        delete m_areaInterface;
    }
}

QString Area::get_plugin_name() {
    return pluginName;
}

int Area::get_plugin_type() {
    return pluginType;
}

QWidget *Area::get_plugin_ui() {
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Area;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        messagedialog = new MessageDialog;

        unsigned int uid = getuid();
        objpath = objpath +"/org/freedesktop/Accounts/User"+QString::number(uid);


        m_areaInterface = new QDBusInterface("org.freedesktop.Accounts",
                                             objpath,
                                             "org.freedesktop.Accounts.User",
                                             QDBusConnection::systemBus());
        const QByteArray id(PANEL_GSCHEMAL);

        if(QGSettings::isSchemaInstalled(id)) {
            m_gsettings = new QGSettings(id);
        }

        m_itimer = new QTimer();
        m_itimer->start(1000);

        initUI();
        initComponent();
        initConnection();
    }
    return pluginWidget;
}

void Area::plugin_delay_control() {

}

const QString Area::name() const {
    return QStringLiteral("area");

}
void Area::initConnection() {
    connect(m_gsettings,SIGNAL(changed(QString)),this,SLOT(hour_time_slot()));
    connect(m_itimer,SIGNAL(timeout()), this, SLOT(datetime_update_slot()));
    connect(ui->langcomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_language_slot(int)));
    connect(ui->countrycomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_area_slot(int)));
//    connect(ui->chgformButton,SIGNAL(clicked()),this,SLOT(changeform_slot()));
    connect(ui->countrycomboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=]{
//        messagedialog->show();
        messagedialog->exec();
//        KMessageBox::information(ui->languageframe_2, tr("Need to log off to take effect"));
    });
    connect(ui->calendarComBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_calender_slot(int)));
    connect(ui->dateComBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_date_slot(int)));
    connect(ui->timeComBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_time_slot(int)));
}
void Area::run_external_app_slot() {
    QString cmd = "gnome-language-selector";

    QProcess process(this);
    process.startDetached(cmd);
}
void Area::hour_time_slot() {
    QStringList keys = m_gsettings->keys();
    QString format;
    if (keys.contains("hoursystem")) {
        format = m_gsettings->get(TIME_KEY).toString();
        if (format == "24") {
            ui->timeComBox->setCurrentIndex(0);
        } else {
            ui->timeComBox->setCurrentIndex(1);
        }
    }
}
void Area::initUI() {
    ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    ui->title2Label->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    ui->title3Label->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");

    ////~ contents_path /area/current area
    ui->titleLabel->setText(tr("current area"));
    ////~ contents_path /area/display format area
    ui->countrylabel->setText(tr("display format area"));
    ////~ contents_path /area/format of area
    ui->title2Label->setText(tr("format of area"));
    ////~ contents_path /area/calendar
    ui->calendarlabel->setText(tr("calendar"));
    ////~ contents_path /area/date
    ui->datelabel->setText(tr("date"));
    ////~ contents_path /area/time
    ui->timelabel->setText(tr("time"));
//    ui->chgformButton->setText(tr("change format of data"));
    ////~ contents_path /area/first language
    ui->title3Label->setText(tr("first language"));
    ////~ contents_path /area/system language
    ui->languagelabel->setText(tr("system language"));

    //给Combox控件插入选项
    ui->countrycomboBox->addItem(tr("US"));
    ui->countrycomboBox->addItem(tr("中国(默认)"));
    ui->langcomboBox->addItem(tr("English"));
    ui->langcomboBox->addItem(tr("简体中文(默认)"));

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
//    currentsecStr_1.replace("上午", "AM");
//    currentsecStr_1.replace("下午", "PM");
    ui->timeComBox->addItem(currentsecStr_1);
    //根据设计稿去掉添加首语言功能
//    addWgt = new HoverWidget("");
//    addWgt->setObjectName(tr("addwgt"));
//    addWgt->setMinimumSize(QSize(580, 50));
//    addWgt->setMaximumSize(QSize(960, 50));
//    addWgt->setStyleSheet("HoverWidget#addwgt{background: palette(button); border-radius: 4px;}HoverWidget:hover:!pressed#addwgt{background: #3D6BE5; border-radius: 4px;}");

//    QHBoxLayout *addLyt = new QHBoxLayout;

//    QLabel * iconLabel = new QLabel();
//    QLabel * textLabel = new QLabel(tr("Add main language"));
//    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
//    iconLabel->setPixmap(pixgray);
//    addLyt->addWidget(iconLabel);
//    addLyt->addWidget(textLabel);
//    addLyt->addStretch();
//    addWgt->setLayout(addLyt);

//    // 悬浮改变Widget状态
//    connect(addWgt, &HoverWidget::enterWidget, this, [=](QString mname) {
//        Q_UNUSED(mname);
//        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
//        iconLabel->setPixmap(pixgray);
//        textLabel->setStyleSheet("color: palette(base);");

//    });
//    // 还原状态
//    connect(addWgt, &HoverWidget::leaveWidget, this, [=](QString mname) {
//        Q_UNUSED(mname);
//        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
//        iconLabel->setPixmap(pixgray);
//        textLabel->setStyleSheet("color: palette(windowText);");
//    });

//    connect(addWgt, &HoverWidget::widgetClicked, this, [=](QString mname) {
//        Q_UNUSED(mname);
//        add_lan_btn_slot();
//    });

//    ui->addLyt->addWidget(addWgt);
}

void Area::initComponent() {
    QStringList res = getUserDefaultLanguage();
    QString lang = res.at(1);
    int langIndex = lang.split(':').at(0) == "en_US" ? 0 : 1;;
    int formatIndex = res.at(0) == "en_US.UTF-8" ? 0 : 1;
    ui->langcomboBox->setCurrentIndex(langIndex);
    ui->countrycomboBox->setCurrentIndex(formatIndex);

    if (!m_gsettings) {
        return ;
    }
    const QStringList list = m_gsettings->keys();

    if (!list.contains("calendar") || !list.contains("firstday")
            || !list.contains("date") || !list.contains("hoursystem")){
        return ;
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

    initFormatData();
}


void Area::initFormatData() {

    QString locale = QLocale::system().name();
    if (!m_gsettings) {
        return ;
    }
    const QStringList list = m_gsettings->keys();

    if (!list.contains("calendar") || !list.contains("firstday")){
        return ;
    }

    if ( "zh_CN" != locale) {
        m_gsettings->set(CALENDAR_KEY, "solarlunar");
    }
//    QString clac = m_gsettings->get(CALENDAR_KEY).toString();
//    if ("lunar" == clac) {
//        ui->Lunarcalendar->setText(tr("lunar"));
//    } else {
//        ui->Lunarcalendar->setText(tr("solar calendar"));
//    }

//    QString day = m_gsettings->get(DAY_KEY).toString();
//    if ("monday" == day) {
//        ui->firstDayLabel->setText(tr("monday"));
//    } else {
//        ui->firstDayLabel->setText(tr("sunday"));
//    }

//    QDateTime current = QDateTime::currentDateTime();
//    QString currentsecStr  ;
//    QString dateFormat = m_gsettings->get(DATE_FORMATE_KEY).toString();
//    if ("cn" == dateFormat) {
//       currentsecStr = current.toString("yyyy/MM/dd ");;
//    } else {
//       currentsecStr = current.toString("yyyy-MM-dd ");
//    }
//    ui->datelabelshow->setText(currentsecStr);

//    this->hourformat = m_gsettings->get(TIME_KEY).toString();
}

void Area::change_language_slot(int index) {

    QDBusReply<bool> res;
    switch (index) {
    case 0:
        res = m_areaInterface->call("SetLanguage","en_US");
        break;
    case 1:
        res = m_areaInterface->call("SetLanguage","zh_CN");
        break;
    }
    messagedialog->exec();
//    KMessageBox::information(ui->languageframe, tr("Need to log off to take effect"));
}

void Area::change_area_slot(int index) {
//    unsigned int uid = getuid();
//    objpath = objpath +"/org/freedesktop/Accounts/User"+QString::number(uid);


//    m_areaInterface = new QDBusInterface("org.freedesktop.Accounts",
//                                         objpath,
//                                         "org.freedesktop.Accounts.User",
//                                         QDBusConnection::systemBus());
    QDBusReply<bool> res;
    switch (index) {
    case 0:
        res = m_areaInterface->call("SetFormatsLocale","en_US.UTF-8");
        break;
    case 1:
        res = m_areaInterface->call("SetFormatsLocale","zh_CN.UTF-8");
        break;
    }
}
void Area::change_calender_slot(int index) {
    QDBusReply<bool> res;
    switch (index) {
    case 0:
        writeGsettings("calendar", "solarlunar");
        break;
    case 1:
        writeGsettings("calendar", "lunar");
        break;
    }
}
void Area::change_date_slot(int index) {
    QDBusReply<bool> res;
    switch (index) {
    case 0:
        writeGsettings("date", "cn");
        break;
    case 1:
        writeGsettings("date", "en");
        break;
    }
}
void Area::change_time_slot(int index) {
    QDBusReply<bool> res;
    switch (index) {
    case 0:
        writeGsettings("hoursystem", "24");
        break;
    case 1:
        writeGsettings("hoursystem", "12");
        break;
    }
}

void Area::writeGsettings(const QString &key, const QString &value) {
    if(!m_gsettings) {
        return ;
    }

    const QStringList list = m_gsettings->keys();
    if (!list.contains(key)) {
        return ;
    }
    m_gsettings->set(key,value);
}

void Area::datetime_update_slot() {
    QDateTime current = QDateTime::currentDateTime();
    QString currentsecStr ;
    currentsecStr = current.toString("hh:mm:ss");
    ui->timeComBox->setItemText(0,currentsecStr);
    currentsecStr = current.toString("hh:mm:ss AP");
//    currentsecStr.replace("上午", "AM");
//    currentsecStr.replace("下午", "PM");
    ui->timeComBox->setItemText(1,currentsecStr);
}

void Area::add_lan_btn_slot() {
    QString cmd = "gnome-language-selector";

    QProcess process(this);
    process.startDetached(cmd);
}

void Area::changeform_slot() {
    DataFormat *dialog = new DataFormat();
//    connect(dialog, SIGNAL(dataChangedSignal()),this,SLOT(initFormatData()));
    dialog->setWindowTitle(tr("change data format"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

QStringList Area::readFile(const QString& filepath) {
    QStringList res;
    QFile file(filepath);
    if(file.exists()) {
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "ReadFile() failed to open" << filepath;
            return QStringList();
        }
        QTextStream textStream(&file);
        while(!textStream.atEnd()) {
            QString line= textStream.readLine();
            line.remove('\n');
            res<<line;
        }
        file.close();
        return res;
    } else {
        qWarning() << filepath << " not found"<<endl;
        return QStringList();
    }
}

QStringList Area::getUserDefaultLanguage() {
    int pos = 0;
    QString formats;
    QString language;
    QStringList filestr;
    QStringList result;
    QString fname = getenv("HOME");
    fname += "/.pam_environment";

    filestr = this->readFile(fname);
    QRegExp re("LANGUAGE(\t+DEFAULT)?=(.*)$");
    for(int i = 0; i < filestr.length(); i++) {
        while((pos = re.indexIn(filestr.at(i), pos)) != -1) {
            language = re.cap(2);
            pos += re.matchedLength();
        }
    }

    QDBusInterface * iproperty = new QDBusInterface("org.freedesktop.Accounts",
                                            objpath,
                                            "org.freedesktop.DBus.Properties",
                                            QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply = iproperty->call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()){
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        formats = propertyMap.find("FormatsLocale").value().toString();
        if(language.isEmpty()) {
            language = propertyMap.find("Language").value().toString();
        }
    } else {
        qDebug() << "reply failed";
    }
    result.append(formats);
    result.append(language);
    return result;
}
