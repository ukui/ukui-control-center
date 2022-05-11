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

#include <QDebug>
#include <QFile>
#include <QString>
#include <QDateTime>
#include <QTimer>
#include <QGSettings>
#include <QMessageBox>
#include "languageFrame.h"
//#include "Frame/hlineframe.h"
#include <ukcc/widgets/hlineframe.h>

#define PANEL_GSCHEMAL   "org.ukui.control-center.panel.plugins"
#define CALENDAR_KEY     "calendar"
#define DAY_KEY          "firstday"
#define DATE_FORMATE_KEY "date"
#define TIME_KEY         "hoursystem"


Area::Area() : mFirstLoad(true)
{
    pluginName = tr("Area");
    pluginType = DATETIME;
}

Area::~Area()
{
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
    }
}

void Area::cloudChangedSlot(const QString &key) {
    if(key == "area") {
        initComponent();
    }
}

void Area::connectToServer(){
    QThread *NetThread = new QThread;
    MThread *NetWorker = new MThread;
    NetWorker->moveToThread(NetThread);
    connect(NetThread, &QThread::started, NetWorker, &MThread::run);
    connect(NetWorker,&MThread::keychangedsignal,this,&Area::cloudChangedSlot);
    connect(NetThread, &QThread::finished, NetWorker, &MThread::deleteLater);
    NetThread->start();
}

QString Area::plugini18nName() {
    return pluginName;
}

int Area::pluginTypes() {
    return pluginType;
}

QWidget *Area::pluginUi() {
    if (mFirstLoad) {

        mFirstLoad = false;

        ui = new Ui::Area;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        ui->countrylabel->adjustSize();
        ui->formframe->adjustSize();

        const QByteArray id(PANEL_GSCHEMAL);

        if(QGSettings::isSchemaInstalled(id)) {
            m_gsettings = new QGSettings(id, QByteArray(), pluginWidget);
            mDateFormat = m_gsettings->get(DATE_FORMATE_KEY).toString();
        }

        unsigned int uid = getuid();
        objpath = objpath +"/org/freedesktop/Accounts/User"+QString::number(uid);


        m_areaInterface = new QDBusInterface("org.freedesktop.Accounts",
                                             objpath,
                                             "org.freedesktop.Accounts.User",
                                             QDBusConnection::systemBus());

        initUI();
        initFormFrame();
        initComponent();
        connectToServer();
        initConnect();
    } else {
        ui->dateBox->blockSignals(true);
        int index = ui->dateBox->currentIndex();
        ui->dateBox->clear();

        QString currentsecStr;
        QDateTime current = QDateTime::currentDateTime();
        currentsecStr = current.toString("yyyy/MM/dd ");
        ui->dateBox->addItem(currentsecStr);
        currentsecStr = current.toString("yyyy-MM-dd ");
        ui->dateBox->addItem(currentsecStr);

        ui->dateBox->setCurrentIndex(index);
        ui->dateBox->blockSignals(false);
    }
    return pluginWidget;
}

const QString Area::name() const {
    return QStringLiteral("Area");

}

bool Area::isShowOnHomePage() const
{
    return true;
}

QIcon Area::icon() const
{
    return QIcon::fromTheme("ukui-area-symbolic");
}

bool Area::isEnable() const
{
    return true;
}

void Area::run_external_app_slot() {
    QString cmd = "gnome-language-selector";

    QProcess process(this);
    process.startDetached(cmd);
}

void Area::initUI() {
    //~ contents_path /Area/Regional Format
    ui->titleLabel->setText(tr("Language Format"));
    //~ contents_path /Area/system language
    ui->title3Label->setText(tr("System Language"));

    ui->summaryLabel->setContentsMargins(16,0,0,0);
    ui->summaryLabel->setText(tr("Language for system windows,menus and web pages"));
    ui->summaryLabel->setVisible(true);

    initLanguage();

    AddBtn *addBtn = new AddBtn;
    addBtn->setFixedHeight(60);
    connect(addBtn,&AddBtn::clicked,this,[=](){
       add_lan_btn_slot();
    });

    ui->languageFrame->layout()->addWidget(addBtn);
}

void Area::initLanguage()
{
    LanguageFrame *chineseFrame = new LanguageFrame("简体中文");
    LanguageFrame *englishFrame = new LanguageFrame("English");
    HLineFrame *lineF = new HLineFrame();
    HLineFrame *lineF_2 = new HLineFrame();

    QStringList res = getUserDefaultLanguage();
    QString lang = res.at(1);
    int langIndex = lang.split(':').at(0) == "zh_CN" ? 0 : 1;

    if (0 == langIndex) {
        chineseFrame->showSelectedIcon(true);
        englishFrame->showSelectedIcon(false);
    } else {
        chineseFrame->showSelectedIcon(false);
        englishFrame->showSelectedIcon(true);
    }

    ui->languageFrame->layout()->addWidget(chineseFrame);
    ui->languageFrame->layout()->addWidget(lineF);
    ui->languageFrame->layout()->addWidget(englishFrame);
    ui->languageFrame->layout()->addWidget(lineF_2);

    connect(chineseFrame, &LanguageFrame::clicked, this, [=](){
        englishFrame->showSelectedIcon(false);
        m_areaInterface->call("SetLanguage","zh_CN");
        showMessageBox(2);
    });

    connect(englishFrame, &LanguageFrame::clicked, this, [=](){
        chineseFrame->showSelectedIcon(false);
        m_areaInterface->call("SetLanguage","en_US");
        showMessageBox(2);
    });
}

void Area::initComponent() {
    ui->countrycomboBox->addItem(tr("US"));
    ui->countrycomboBox->addItem(tr("CN"));

    QStringList res = getUserDefaultLanguage();
    QString lang = res.at(1);
    int langIndex = lang.split(':').at(0) == "zh_CN" ? 1 : 0;
    int formatIndex = res.at(0) == "zh_CN.UTF-8" ? 1 : 0;
   // ui->langcomboBox->setCurrentIndex(langIndex);
    ui->countrycomboBox->setCurrentIndex(formatIndex);

    if (ui->countrycomboBox->currentIndex() == 1) {
        ui->dayBox->addItem("星期一");
        ui->dayBox->addItem("星期日");
    } else {
        ui->dayBox->addItem("monday");
        ui->dayBox->addItem("sunday");
    }

    initFormComponent(0b1111);
}

void Area::initFormComponent(int8_t value)
{
    const QStringList list = m_gsettings->keys();

    if (!list.contains(CALENDAR_KEY) || !list.contains(DAY_KEY)
            || !list.contains(DATE_FORMATE_KEY) || !list.contains(TIME_KEY)){
        return ;
    }

    if (value >> 0 & 1) {
        QString clac = m_gsettings->get(CALENDAR_KEY).toString();
        if (ui->calendarBox->count() <= 1) {
            ui->calendarBox->setCurrentIndex(0);
        } else {
            if ("solarlunar" == clac) {
                ui->calendarBox->setCurrentIndex(0);
            } else {
                ui->calendarBox->setCurrentIndex(1);
            }
        }
    }

    if (value >> 1 & 1) {
        QString day = m_gsettings->get(DAY_KEY).toString();
        if ("monday" == day) {
            ui->dayBox->setCurrentIndex(0);
        } else {
            ui->dayBox->setCurrentIndex(1);
        }
    }

    if (value >> 2 & 1) {
        QString dateFormat = m_gsettings->get(DATE_FORMATE_KEY).toString();
        if ("cn" == dateFormat) {
           ui->dateBox->setCurrentIndex(0);
        } else {
           ui->dateBox->setCurrentIndex(1);
        }
    }

    if (value >> 3 & 1) {
        QString hourFormat = m_gsettings->get(TIME_KEY).toString();
        if ("24" == hourFormat) {
            ui->timeBox->setCurrentIndex(1);
        } else {
            ui->timeBox->setCurrentIndex(0);
        }
    }
}

void Area::change_area_slot(int index) {
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

void Area::add_lan_btn_slot() {
    QString cmd = "gnome-language-selector";

    QProcess process(this);
    process.startDetached(cmd);
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
    QString formats;
    QString language;
    QStringList result;

    QDBusInterface iproperty("org.freedesktop.Accounts",
                             objpath,
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply = iproperty.call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()){
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        if (propertyMap.keys().contains("FormatsLocale")) {
            formats = propertyMap.find("FormatsLocale").value().toString();
        }
        if(language.isEmpty() && propertyMap.keys().contains("Language")) {
            language = propertyMap.find("Language").value().toString();
        }
    } else {
        qDebug() << "reply failed";
    }
    result.append(formats);
    result.append(language);
    return result;
}


void Area::initFormFrame()
{
    //~ contents_path /Area/Current Region
    ui->countrylabel->setText(tr("Current Region"));
    //~ contents_path /Area/Calendar
    ui->calendarLabel->setText(tr("Calendar"));
    //~ contents_path /Area/First Day Of Week
    ui->dayLabel->setText(tr("First Day Of Week"));
    //~ contents_path /Area/Date
    ui->dateLabel->setText(tr("Date"));
    //~ contents_path /Area/Time
    ui->timelabel->setText(tr("Time"));

    ui->calendarBox->addItem(tr("solar calendar"));
    QString locale = QLocale::system().name();
    if ("zh_CN" == locale){
        ui->calendarBox->addItem(tr("lunar"));
    }

    QString currentsecStr;
    QDateTime current = QDateTime::currentDateTime();

    currentsecStr = current.toString("yyyy/MM/dd ");
    ui->dateBox->addItem(currentsecStr);

    currentsecStr = current.toString("yyyy-MM-dd ");
    ui->dateBox->addItem(currentsecStr);

    ui->timeBox->addItem(tr("12 Hours"));
    ui->timeBox->addItem(tr("24 Hours"));
}

void Area::initConnect()
{
    connect(ui->countrycomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_area_slot(int)));
    connect(ui->countrycomboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=](int index){
        showMessageBox(1);
        int dayValue = ui->dayBox->currentIndex();
        ui->dayBox->clear();
        if (index == 1) {
            ui->dayBox->addItem("星期一");
            ui->dayBox->addItem("星期日");
        } else {
            ui->dayBox->addItem("monday");
            ui->dayBox->addItem("sunday");
        }
        ui->dayBox->setCurrentIndex(dayValue);
    });
    connect(ui->timeBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=]() {
        bool flag_24;
        if (0 == ui->timeBox->currentIndex()) {
            flag_24 = false;
        } else {
            flag_24 = true;
        }
        timeFormatClicked(flag_24);
    });

    connect(ui->dayBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=]{
        QString dayValue;
        if (0 == ui->dayBox->currentIndex()) {
            dayValue = "monday";
        } else {
            dayValue = "sunday";
        }
        writeGsettings(DAY_KEY, dayValue);
    });

    connect(ui->calendarBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=]{
        QString calendarValue;
        if ( 0 == ui->calendarBox->currentIndex()) {
            calendarValue = "solarlunar";
        } else {
            calendarValue = "lunar";
        }
        writeGsettings(CALENDAR_KEY, calendarValue);
    });

    connect(ui->dateBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=]{
        QString dateValue;
        if ( 0 == ui->dateBox->currentIndex()) {
            dateValue = "cn";
        } else {
            dateValue = "en";
        }
        writeGsettings(DATE_FORMATE_KEY, dateValue);
    });

    connect(m_gsettings, &QGSettings::changed, this, [=] (const QString &key) {
        if (key == CALENDAR_KEY ) {
            ui->calendarBox->blockSignals(true);
            initFormComponent(0b0001);
            ui->calendarBox->blockSignals(false);
        } else if(key == DAY_KEY ) {
            ui->dayBox->blockSignals(true);
            initFormComponent(0b0010);
            ui->dayBox->blockSignals(false);
        } else if(key == DATE_FORMATE_KEY) {
            ui->dateBox->blockSignals(true);
            initFormComponent(0b0100);
            ui->dateBox->blockSignals(false);
        } else if(key == TIME_KEY) {
            ui->timeBox->blockSignals(true);
            initFormComponent(0b1000);
            ui->timeBox->blockSignals(false);
        }
    });

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

void Area::timeFormatClicked(bool flag)
{
    if (!m_gsettings) {
        qDebug()<<"org.ukui.control-center.panel.plugins not installed"<<endl;
        return;
    }
    QStringList keys = m_gsettings->keys();
    if (keys.contains(TIME_KEY)) {
        if (flag == true) {
            m_gsettings->set(TIME_KEY, "24");
        } else {
            m_gsettings->set(TIME_KEY, "12");
        }
    }
}

void Area::showMessageBox(int flag)
{
    QMessageBox msg(qApp->activeWindow());
    msg.setIcon(QMessageBox::Warning);

    if (flag == 1) {
        msg.setText(tr("Modify the current region need to logout to take effect, whether to logout?"));
        msg.addButton(tr("Logout later"), QMessageBox::NoRole);
        msg.addButton(tr("Logout now"), QMessageBox::ApplyRole);
    } else if(flag == 2) {
        msg.setText(tr("Modify the first language need to reboot to take effect, whether to reboot?"));
        msg.addButton(tr("Reboot later"), QMessageBox::NoRole);
        msg.addButton(tr("Reboot now"), QMessageBox::ApplyRole);
    }
    int ret = msg.exec();

    if (ret == 1) {
        if (flag == 1) {
            system("ukui-session-tools --logout");
        } else if (flag == 2) {
            system("ukui-session-tools --reboot");
        }
    }
    return;
}
