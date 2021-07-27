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
#include <QLineEdit>

#include <QTimeZone>
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <locale.h>
#include <libintl.h>
#include <QProcess>
#include <sys/timex.h>
#include <qmath.h>
#include <polkit-qt5-1/polkitqt1-authority.h>
#include "ImageUtil/imageutil.h"
#include "clock.h"
#include <QButtonGroup>
#include <QCalendarWidget>

const char kTimezoneDomain[] = "installer-timezones";
const char kDefaultLocale[]  = "en_US.UTF-8";

const QString kcnBj =           "北京";
const QString kenBj =           "Asia/Beijing";

#define FORMAT_SCHEMA           "org.ukui.control-center.panel.plugins"
#define TIME_FORMAT_KEY         "hoursystem"
#define DATE_KEY                "date"
//#define SYNC_TIME_KEY           "synctime"
#define NTP_KEY                 "ntp"
#define TIMEZONES_KEY           "timezones"
#define MAX_TIMES               500

volatile bool syncThreadFlag =  false;

const QStringList ntpAddressList = {
                            "pool.ntp.org",
                            "cn.ntp.org.cn",
                            "cn.pool.ntp.org",
                            "ntp.aliyun.com",

                            "0.debian.pool.ntp.org",
                            "1.debian.pool.ntp.org",

                            "0.arch.pool.ntp.org",
                            "1.arch.pool.ntp.org",

                            "0.fedora.pool.ntp.org",
                            "1.fedora.pool.ntp.org",

                          };

DateTime::DateTime() : mFirstLoad(true)
{
    ui = new Ui::DateTime;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);
    pluginName = tr("Date");
    pluginType = DATETIME;
}

DateTime::~DateTime()
{
    if (!mFirstLoad) {
        delete ui;
        delete m_zoneinfo;
        ui         = nullptr;
        m_zoneinfo = nullptr;
    }
}

QString DateTime::get_plugin_name()
{
    return pluginName;
}

int DateTime::get_plugin_type()
{
    return pluginType;
}

QWidget *DateTime::get_plugin_ui()
{

    if (mFirstLoad) {

        mFirstLoad = false;

        initUI();
        initTitleLabel();
        initStatus();
        initComponent();
        initConnect();
        connectToServer();
        initTimeShow();
    }
    return pluginWidget;
}

void DateTime::plugin_delay_control()
{

}

const QString DateTime::name() const
{

    return QStringLiteral("date");
}

void DateTime::initTitleLabel()
{
    QGSettings *m_fontSetting = new QGSettings("org.ukui.style");
    QFont font;
    ui->titleLabel_2->adjustSize();
    //~ contents_path /date/Other Timezone
    ui->titleLabel_2->setText(tr("Other Timezone"));
    ui->timeClockLable->setObjectName("timeClockLable");
    font.setPixelSize(m_fontSetting->get("systemFontSize").toInt() * 23 / 11);
    font.setWeight(QFont::Medium);
    ui->timeClockLable->setFont(font);
    delete m_fontSetting;
    m_fontSetting = nullptr;
}

void DateTime::initUI()
{
    m_formTimeBtn       = new SwitchButton(pluginWidget);
    //~ contents_path /date/24-hour clock
    m_formTimeLabel     = new QLabel(tr("24-hour clock"), pluginWidget);
    ui->syncLabel->setText(tr("Sync from network"));
    syncNetworkRetLabel = new QLabel(pluginWidget);
    syncNetworkRetLabel->setStyleSheet("QLabel{font-size: 15px; color: #D9F82929;}");

    m_zoneinfo          = new ZoneInfo;
    m_timezone          = new TimeZoneChooser(pluginWidget);
    m_itimer            = new QTimer(this);
    m_itimer->start(1000);

    ui->showFrame->adjustSize();

    ui->frame_3->adjustSize();
    ui->frame_3->setObjectName("baseFrame");
    ui->frame_3->setStyleSheet("QFrame#baseFrame{background-color:palette(base);}");
    Clock *m_clock = new Clock();
    //ui->clockFrame->setFrameShape(QFrame::Shape::Box);
    ui->clockLayout->addWidget(m_clock);
//    ui->dateEdit->calendarWidget()->setVisible(false);
   // ui->dateEdit->adjustSize();
//    ui->dateEdit->setStyleSheet("QCalendarWidget QWidget#qt_calendar_navigationbar{\
//                                background-color: palette(window);\
//                                }");


    //~ contents_path /date/Manual Time
    ui->timeLabel->setText(tr("Manual Time"));
    for (int m = 0; m < 60; m++) {
        ui->minComboBox->addItem(QString::number(m));
    }

    for (int s = 0; s < 60; s++) {
        ui->secComboBox->addItem(QString::number(s));
    }

    for (int h = 0; h < 24; h++){
        ui->hourComboBox->addItem(QString::number(h));
    }

    const QByteArray id(FORMAT_SCHEMA);
    if (QGSettings::isSchemaInstalled(id)) {
        const QByteArray id(FORMAT_SCHEMA);
        m_formatsettings = new QGSettings(id, QByteArray(), this);
        connect(m_formatsettings, &QGSettings::changed, this, [=](QString key) {
            Q_UNUSED(key);
            QString hourFormat = m_formatsettings->get(TIME_FORMAT_KEY).toString();
            bool status = ("24" == hourFormat ? false : true);
            timeFormatClickedSlot(status, true);
        });
    }

    // 初始化dbus
    m_datetimeiface       = new QDBusInterface("org.freedesktop.timedate1",
                                       "/org/freedesktop/timedate1",
                                       "org.freedesktop.timedate1",
                                       QDBusConnection::systemBus(), this);

    m_datetimeiproperties = new QDBusInterface("org.freedesktop.timedate1",
                                             "/org/freedesktop/timedate1",
                                             "org.freedesktop.DBus.Properties",
                                             QDBusConnection::systemBus(), this);

    initNtp();
}

void DateTime::initComponent()
{
    ui->titleLabel->setContentsMargins(0,0,0,0);
    ui->timeClockLable->setContentsMargins(0,0,0,0);

    //~ contents_path /date/Change time zone
    ui->chgzonebtn->setText(tr("Change time zone"));

    ui->hourFrame->setVisible(false);  //移到area里面了
    QHBoxLayout *hourLayout = new QHBoxLayout(ui->hourFrame);

    hourLayout->addWidget(m_formTimeLabel);
    hourLayout->addWidget(m_formTimeBtn);

    ui->radioButton->adjustSize();
    ui->radioButton_2->adjustSize();
    //~ contents_path /date/Sync Time
    ui->radioButton->setText(tr("Sync Time"));
    //~ contents_path /date/Manual Time
    ui->radioButton_2->setText(tr("Manual Time"));

    ui->syncHintFrame->adjustSize();
    ui->hintLayout->addWidget(syncNetworkRetLabel);

    QButtonGroup *timeGroupBtn = new QButtonGroup(this);
    timeGroupBtn->addButton(ui->radioButton, 0);
    timeGroupBtn->addButton(ui->radioButton_2, 1);

    connect(timeGroupBtn, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [=](int id){
        if (id == 0) {
            synctimeFormatSlot(true, true);
        } else {
            synctimeFormatSlot(false, true);
            syncNetworkRetLabel->clear();
        }
    });



    QDateTime currentime = QDateTime::currentDateTime();
    QString timeAndWeek = currentime.toString("yyyy/MM/dd  ddd").replace("周","星期");
    ui->dateLabel->setText(timeAndWeek + "     " + localizedTimezone);

    QFile tzfile("://zoneUtc");
    if (!tzfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
         qDebug("TZ File Open Failed");
    } else {
        QTextStream txt(&tzfile);
        int index = 0;
        while (!tzfile.atEnd()) {
            QStringList lineList = txt.readLine().split("\t");
            tzindexMapEn.insert(lineList.at(0),index);
            tzindexMapCN.insert(lineList.at(1),index);
            index++;
        }
    }
    tzfile.close();
}

void DateTime::initStatus()
{
    // 时区
    const QString locale = QLocale::system().name();
    QDBusReply<QVariant> tz = m_datetimeiproperties->call("Get", "org.freedesktop.timedate1", "Timezone");
    localizedTimezone = getLocalTimezoneName(tz.value().toString(), locale);
    loadHour();
}


void DateTime::initTimeShow()
{
    ui->summaryLabel->setObjectName("summaryText");
    ui->summaryLabel->setText(tr("Add time zones to display the time,only 5 can be added"));
    ui->summaryLabel->setVisible(false);

    HoverWidget *addTimeWgt = new HoverWidget("");
    addTimeWgt->setObjectName("addTimeWgt");
    addTimeWgt->setMinimumSize(QSize(580, 50));

    QPalette pal;
    QBrush brush = pal.highlight();  //获取window的色值
    QColor highLightColor = brush.color();

    QString stringColor = QString("rgba(%1,%2,%3)") //叠加20%白色
           .arg(highLightColor.red()*0.8 + 255*0.2)
           .arg(highLightColor.green()*0.8 + 255*0.2)
           .arg(highLightColor.blue()*0.8 + 255*0.2);

    addTimeWgt->setStyleSheet(QString("HoverWidget#addTimeWgt{background: palette(button); \
                                                      border-radius: 4px;}\
                              HoverWidget:hover:!pressed#addTimeWgt{background: %1; \
                                                                    border-radius: 4px;}").arg(stringColor));

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel * iconLabel = new QLabel();
    QLabel * textLabel = new QLabel(tr("Add Timezone"));
    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
    iconLabel->setPixmap(pixgray);
    iconLabel->setProperty("useIconHighlightEffect", true);
    iconLabel->setProperty("iconHighlightEffectMode", 1);

    addLyt->addStretch();
    addLyt->addWidget(iconLabel);
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    addTimeWgt->setLayout(addLyt);
    ui->addLayout->addWidget(addTimeWgt);

    connect(addTimeWgt, &HoverWidget::widgetClicked, this, [=](QString mname) {
        Q_UNUSED(mname);
        changeZoneFlag = false;
        changezoneSlot(1);
    });

    // 悬浮改变Widget状态
    connect(addTimeWgt, &HoverWidget::enterWidget, this, [=](){
        if (!ui->addFrame->isEnabled())
            return;
        iconLabel->setProperty("useIconHighlightEffect", false);
        iconLabel->setProperty("iconHighlightEffectMode", 0);
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
        iconLabel->setPixmap(pixgray);
        textLabel->setStyleSheet("color: white;");
    });

    // 还原状态
    connect(addTimeWgt, &HoverWidget::leaveWidget, this, [=](){
        if (!ui->addFrame->isEnabled())
            return;
        iconLabel->setProperty("useIconHighlightEffect", true);
        iconLabel->setProperty("iconHighlightEffectMode", 1);
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
        iconLabel->setPixmap(pixgray);
        textLabel->setStyleSheet("color: palette(windowText);");
    });

    if (m_formatsettings->keys().contains(TIMEZONES_KEY)) {
        timezonesList = m_formatsettings->get(TIMEZONES_KEY).toStringList();

        int timesNum = timezonesList.size();
        if (timezonesList.size() >= MAX_TIMES) {
            timesNum = MAX_TIMES;
            ui->addFrame->setEnabled(false);
            for (int i = MAX_TIMES; i < timezonesList.size(); ++i) {
                timezonesList.removeLast();
            }
            m_formatsettings->set(TIMEZONES_KEY, timezonesList);
        }

        for (int i = 0; i < timesNum; ++i) {
            newTimeshow(timezonesList[i]);
        }
     }
}

void DateTime::addTimezone(const QString &timezone)
{
    for (int i = 0; i < timezonesList.size(); ++i) {
        if (timezonesList[i] == timezone) {
            return;
        } else if (i == timezonesList.size() - 1) {
            break;
        }
    }
    timezonesList.append(timezone);
    if (timezonesList.size() >= MAX_TIMES) {
        ui->addFrame->setEnabled(false);
    }
     if (m_formatsettings->keys().contains(TIMEZONES_KEY)) {
        m_formatsettings->set(TIMEZONES_KEY, timezonesList);
     }
    newTimeshow(timezone);
}

void DateTime::newTimeshow(const QString &timezone)
{
    TimeBtn *timeBtn = new TimeBtn(timezone);
    ui->showLayout->addWidget(timeBtn);
    timeBtn->updateTime(m_formTimeBtn->isChecked());
    connect(timeBtn->deleteBtn, &QPushButton::clicked, this, [=](){
       timezonesList.removeOne(timezone);
       if (m_formatsettings->keys().contains(TIMEZONES_KEY)) {
           m_formatsettings->set(TIMEZONES_KEY, timezonesList);
       }
       timeBtn->close();
    });

    connect(m_itimer, &QTimer::timeout, this, [=](){
        timeBtn->updateTime(m_formTimeBtn->isChecked());
    });
}

void DateTime::initNtp()
{
    QLabel      *ntpLabel  = new QLabel(ui->ntpFrame);
    QHBoxLayout *ntpLayout = new QHBoxLayout(ui->ntpFrame);
                 ntpCombox = new QComboBox(ui->ntpFrame);
    ntpLabel->setFixedWidth(135);
    ntpLayout->setContentsMargins(16,8,26,8);
    ui->ntpFrame->setLayout(ntpLayout);
    ntpLayout->addWidget(ntpLabel);
    ntpLayout->addWidget(ntpCombox);
    ntpLabel->setText(tr("Time Server"));
    ntpCombox->setFixedHeight(36);
    ntpCombox->addItem(tr("Default"));
    ntpCombox->addItems(ntpAddressList);
    ntpCombox->addItem(tr("Customize"));

    /*自定义*/
    QLabel *ntpLabel_2 = new QLabel(ui->ntpFrame_2);
    QHBoxLayout *ntpLayout_2 = new QHBoxLayout(ui->ntpFrame_2);
    QLineEdit *ntpLineEdit = new QLineEdit();
    QPushButton *saveBtn = new QPushButton(ui->ntpFrame_2);

    ntpLayout_2->setContentsMargins(16,8,26,8);
    ntpLineEdit->setParent(ui->ntpFrame_2);
    ntpLabel_2->setText(tr("Server Address"));
    ntpLayout_2->addWidget(ntpLabel_2);
    ntpLabel_2->setFixedWidth(135);
    ntpLayout_2->addWidget(ntpLineEdit);
    ntpLayout_2->addWidget(saveBtn);
    ntpLineEdit->setPlaceholderText(tr("Required"));
    saveBtn->setText(tr("Save"));

    if (m_formatsettings->keys().contains(NTP_KEY))
        ntpLineEdit->setText(m_formatsettings->get(NTP_KEY).toString());

    connect(ntpLineEdit, &QLineEdit::textChanged, this, [=](){
        saveBtn->setEnabled(!ntpLineEdit->text().isEmpty());   //为空时不允许保存
    });

    connect(saveBtn, &QPushButton::clicked, this, [=](){
        QString setAddr = ntpLineEdit->text();
        if (!setNtpAddr(setAddr)) {   //失败or不修改
            if (m_formatsettings->keys().contains(NTP_KEY))
                ntpLineEdit->setText(m_formatsettings->get(NTP_KEY).toString());
        } else {
            if (m_formatsettings->keys().contains(NTP_KEY))
                m_formatsettings->set(NTP_KEY, setAddr);
        }
    });


    const QString ntpFileName = "/etc/systemd/timesyncd.conf.d/kylin.conf";
    QFile ntpFile(ntpFileName);
    if (!ntpFile.exists()) {    //默认
        ntpCombox->setCurrentIndex(0);
        ui->ntpFrame_2->setVisible(false);
    } else {
        QSettings readFile(ntpFileName, QSettings::IniFormat);
        QString initAddress = readFile.value("Time/NTP").toString();
        for (int i = 0; i < ntpCombox->count(); ++i) {
            if (initAddress == ntpCombox->itemText(i)) {   //是选中的
                ntpCombox->setCurrentIndex(i);
                ui->ntpFrame_2->setVisible(false);
                break;
            } else if (i == ntpCombox->count() - 1) {     //是自定义的
                ntpCombox->setCurrentIndex(i);
                ntpLineEdit->setText(initAddress);
                ui->ntpFrame_2->setVisible(true);
                break;
            }
        }
    }

    ntpComboxPreId = ntpCombox->currentIndex();

    connect(ntpCombox, &QComboBox::currentTextChanged, this, [=](){
        QString setAddr;
        if (m_formatsettings->keys().contains(NTP_KEY))
            setAddr = m_formatsettings->get(NTP_KEY).toString();
        if (ntpCombox->currentIndex() == (ntpCombox->count() - 1) && setAddr == "") { //自定义且为空
            ui->ntpFrame_2->setVisible(true);  //需要添加地址并点击保存再授权
        } else {
            if (ntpCombox->currentIndex() == 0) {  //默认
                setAddr = "default";
                ui->ntpFrame_2->setVisible(false);
            } else if (ntpCombox->currentIndex() != ntpCombox->count() - 1) { //选择系统
                setAddr = ntpCombox->currentText();
                ui->ntpFrame_2->setVisible(false);
            } else { //自定义且不为空
                ui->ntpFrame_2->setVisible(true);
            }
            if (!setNtpAddr(setAddr)) {   //失败or不修改
                ntpCombox->blockSignals(true);
                ntpCombox->setCurrentIndex(ntpComboxPreId);
                ntpCombox->blockSignals(false);
                if (ntpComboxPreId == ntpCombox->count() - 1) {
                    ui->ntpFrame_2->setVisible(true);
                } else {
                    ui->ntpFrame_2->setVisible(false);
                }
            } else {
                ntpComboxPreId = ntpCombox->currentIndex();
                if (m_formatsettings->keys().contains(NTP_KEY)) //防止未保存的内容一直存在
                    ntpLineEdit->setText(m_formatsettings->get(NTP_KEY).toString());
            }
        }
    });
}

void DateTime::connectToServer()
{
    m_cloudInterface = new QDBusInterface("org.kylinssoclient.dbus",
                                          "/org/kylinssoclient/path",
                                          "org.freedesktop.kylinssoclient.interface",
                                          QDBusConnection::sessionBus());
    if (!m_cloudInterface->isValid()) {
        qDebug() << "fail to connect to service";
        qDebug() << qPrintable(QDBusConnection::systemBus().lastError().message());
        return;
    }
    QDBusConnection::sessionBus().connect(QString(), QString("/org/kylinssoclient/path"), QString("org.freedesktop.kylinssoclient.interface"), "keyChanged", this, SLOT(keyChangedSlot(QString)));
    // 将以后所有DBus调用的超时设置为 milliseconds
    m_cloudInterface->setTimeout(2147483647); // -1 为默认的25s超时
}

void DateTime::keyChangedSlot(const QString &key)
{
    if (key == "datetime") {
        initStatus();
    }
}

bool DateTime::fileIsExits(const QString &filepath)
{
    QFile file(filepath);
    if(file.exists()) {
        return true;
    } else {
        return false;
    }
}

void DateTime::datetimeUpdateSlot()
{
    setCurrentTime();
    QString timeAndWeek = getTimeAndWeek(current);
    ui->dateLabel->setText(timeAndWeek + "     " + localizedTimezone);
}

void DateTime::changetimeSlot()
{
    ChangtimeDialog *dialog = new ChangtimeDialog(m_formTimeBtn->isChecked(),pluginWidget);
    dialog->setWindowTitle(tr("change time"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    m_itimer->stop();
    m_itimer->start();
    dialog->exec();
}

void DateTime::changezoneSlot(int flag)
{
    m_timezone->setFixedSize(1000,720);
    if (flag == 1) {
        m_timezone->setTitle(tr("Add Timezone"));
    } else {
        m_timezone->setTitle(tr("Change Timezone"));
    }
    m_timezone->setWindowModality(Qt::ApplicationModal);
    m_timezone->show();

    m_timezone->setMarkedTimeZoneSlot(m_zoneinfo->getCurrentTimzone());
}

void DateTime::changezoneSlot(QString zone)
{
    m_datetimeiface->call("SetTimezone", zone, true);
}

void DateTime::timeFormatClickedSlot(bool flag, bool outChange)
{
    if (!m_formatsettings) {
        qDebug()<<"org.ukui.control-center.panel.plugins not installed"<<endl;
        return;
    }
    QStringList keys = m_formatsettings->keys();
    if (keys.contains("hoursystem") && !outChange) {
        if (flag == true) {
            m_formatsettings->set(TIME_FORMAT_KEY, "24");
        } else {
            m_formatsettings->set(TIME_FORMAT_KEY, "12");
        }
    }
    //重置时间格式
    m_itimer->stop();
    m_itimer->start(1000);
}

QDBusMessage DateTime::rsyncWithNetworkSlot(bool status)
{
    return m_datetimeiface->call("SetNTP", status, true);
}

void DateTime::loadHour()
{
    if (!m_formatsettings) {
        qDebug()<<"org.ukui.control-center.panel.plugins not installed";
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

    setCurrentTime();

    QDBusReply<QVariant> ret = m_datetimeiproperties->call("Get", "org.freedesktop.timedate1", "NTP");
    bool syncFlag = ret.value().toBool();

    if (syncFlag != false) {
        setNtpFrame(true);
        ui->setTimeFrame->setVisible(false);
        ui->radioButton->blockSignals(true);
        ui->radioButton->setChecked(true);
        ui->radioButton->blockSignals(false);

    } else {
        setNtpFrame(false);
        initSetTime();
        ui->setTimeFrame->setVisible(true);
        ui->radioButton_2->blockSignals(true);
        ui->radioButton_2->setChecked(true);
        ui->radioButton_2->blockSignals(false);
    }

}

QString  DateTime::getTimeAndWeek(const QDateTime timeZone)
{
    QString dateformat;
    if (m_formatsettings) {
        QStringList keys = m_formatsettings->keys();
        if(keys.contains("date")) {
            dateformat = m_formatsettings->get(DATE_KEY).toString();
        }
    }
    QString timeAndWeek;
    if ("cn" == dateformat) {
       timeAndWeek = timeZone.toString("yyyy/MM/dd  ddd").replace("周","星期");
    } else {
       timeAndWeek = timeZone.toString("yyyy-MM-dd  ddd");
    }
    return timeAndWeek;
}



void DateTime::setCurrentTime()
{
    initSetTime();
    current = QDateTime::currentDateTime();
    QString currentsecStr ;
    if (m_formTimeBtn->isChecked()) {
        currentsecStr = current.toString("hh : mm : ss");
    } else {
        currentsecStr = current.toString("AP hh: mm : ss");
    }
    ui->timeClockLable->setText(currentsecStr);
}

void DateTime::initConnect()
{
    connect(ui->chgzonebtn, &QPushButton::clicked, this, [=](){
        changeZoneFlag = true;
        changezoneSlot();
    });

    connect(m_formTimeBtn, &SwitchButton::checkedChanged, this, [=](bool status) {
        timeFormatClickedSlot(status, false);
    });

    connect(m_timezone, &TimeZoneChooser::confirmed, this, [this] (const QString &timezone) {
        if (changeZoneFlag) {
            changezoneSlot(timezone);
            const QString locale = QLocale::system().name();
            localizedTimezone = m_zoneinfo->getLocalTimezoneName(timezone, locale);
        } else {
            addTimezone(timezone);
        }
        m_timezone->hide();
    });

    connect(m_itimer,SIGNAL(timeout()), this, SLOT(datetimeUpdateSlot()));

    connect(m_formatsettings, &QGSettings::changed, this, [=] (const QString &key) {
        if (key == "hoursystem") {
            QString value = m_formatsettings->get(TIME_FORMAT_KEY).toString();
            bool checked = (value == "24" ? true : false);
            m_formTimeBtn->setChecked(checked);
        }
    });

    connect(ui->dateEdit, &QDateEdit::dateChanged, this, [=]() {
        setTime();
    });

    connect(ui->hourComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [=]() {
        setTime();
    });

    connect(ui->minComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [=]() {
        setTime();
    });

    connect(ui->secComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, [=]() {
        setTime();
    });




}

QString DateTime::getLocalTimezoneName(QString timezone, QString locale)
{
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

void DateTime::synctimeFormatSlot(bool status,bool outChange)
{
    Q_UNUSED(outChange);
    if (!m_formatsettings) {
        qDebug()<<"org.ukui.control-center.panel.plugins not installed"<<endl;
        return;
    }

    QDBusMessage retDBus =  rsyncWithNetworkSlot(status);
    if (status != false) {
        ui->setTimeFrame->setVisible(false);
        setNtpFrame(true);
        if (retDBus.type() == QDBusMessage::ReplyMessage) {
            QString successMSG = tr("  ");
            QString failMSG = tr("Sync from network failed");
            CGetSyncRes *syncThread = new CGetSyncRes(this,successMSG,failMSG);
            connect(syncThread, &CGetSyncRes::finished, this, [=](){
                syncThread->deleteLater();
                ui->radioButton_2->setEnabled(true);
            });
            syncThread->start();
            ui->radioButton_2->setEnabled(false);
        } else {
            syncNetworkRetLabel->setText(tr("Sync from network failed"));
        }
    } else {
        initSetTime();
        ui->setTimeFrame->setVisible(true);
        setNtpFrame(false);
    }
}
/*同步硬件时钟*/
void DateTime::syncRTC()
{
    QDBusInterface * changeRTCinterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                                             "/",
                                                             "com.control.center.interface",
                                                             QDBusConnection::systemBus());

    if (!changeRTCinterface->isValid()) {
        qCritical() << "Create Client Interface Failed When execute gpasswd: " << QDBusConnection::systemBus().lastError();
        return;
    }
    changeRTCinterface->call("changeRTC");
    delete changeRTCinterface;
    changeRTCinterface = nullptr;
}

bool DateTime::setNtpAddr(QString address)
{
    PolkitQt1::Authority::Result result;
    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.ntp",
                PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::Yes) {
        QDBusInterface *setNtpAddrDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                                                 "/",
                                                                 "com.control.center.interface",
                                                                 QDBusConnection::systemBus());

        QDBusReply<bool> reply = setNtpAddrDbus->call("setNtpSerAddress", address);
        delete setNtpAddrDbus;
        setNtpAddrDbus = nullptr;
        return reply;
    }
    return false;
}

void DateTime::setNtpFrame(bool visiable)
{
    ui->ntpFrame->setVisible(visiable);
    if (visiable && ntpCombox != nullptr) {
        ui->ntpFrame_2->setVisible(ntpCombox->currentIndex() == ntpCombox->count() - 1);
    } else {
        ui->ntpFrame_2->setVisible(visiable);
    }
}

void DateTime::initSetTime() {
    QDateTime m_time = QDateTime::currentDateTime();

    ui->dateEdit->blockSignals(true);
    ui->hourComboBox->blockSignals(true);
    ui->minComboBox->blockSignals(true);
    ui->secComboBox->blockSignals(true);

    ui->dateEdit->setDate(m_time.date());
    ui->hourComboBox->setCurrentIndex(m_time.time().hour());
    ui->minComboBox->setCurrentIndex(m_time.time().minute());
    ui->secComboBox->setCurrentIndex(m_time.time().second());

    ui->dateEdit->blockSignals(false);
    ui->hourComboBox->blockSignals(false);
    ui->minComboBox->blockSignals(false);
    ui->secComboBox->blockSignals(false);
}

void DateTime::setTime() {
    QDate tmpdate(ui->dateEdit->date());
    QTime tmptime(ui->hourComboBox->currentIndex(), ui->minComboBox->currentIndex(),ui->secComboBox->currentIndex());
    QDateTime setdt(tmpdate,tmptime);

    m_datetimeiface->call("SetTime", QVariant::fromValue(setdt.toSecsSinceEpoch() * G_TIME_SPAN_SECOND), false, true);

}

bool DateTime::getSyncStatus() {
    return ui->radioButton->isChecked();
}

CGetSyncRes::CGetSyncRes(DateTime *dataTimeUI,QString successMSG,QString failMSG)
{
    this -> dataTimeUI = dataTimeUI;
    this -> successMSG = successMSG;
    this -> failMSG = failMSG;
}

CGetSyncRes::~CGetSyncRes()
{

}
void CGetSyncRes::run()
{
    for(qint8 i = 0; i < 80; ++i) {
        if (this->dataTimeUI->getSyncStatus() == false) {
            this->dataTimeUI->syncNetworkRetLabel->clear();
            return;
        }
        struct timex txc = {};
        if (adjtimex(&txc) < 0 || txc.maxerror >= 16000000) {  //未能同步时间
            int picNum = i - qFloor(i/8)*8; //限制在0~7
            QString pixName = QString(":/img/plugins/upgrade/loading%1.svg").arg(picNum+10);
            QPixmap pix(pixName);
            qApp->processEvents();
            this->dataTimeUI->syncNetworkRetLabel->setPixmap(pix);
            msleep(70);
            continue;
        } else {                                               //同步时间成功
            DateTime::syncRTC();
            this->dataTimeUI->syncNetworkRetLabel->setText(successMSG);
            return;
        }
    }
    this->dataTimeUI->syncNetworkRetLabel->setText(failMSG);
    if (syncThreadFlag == false) { //创建线程一直查时间同步是否成功
        CSyncTime *syncTimeThread = new CSyncTime(this->dataTimeUI,successMSG,failMSG);
        connect(syncTimeThread,SIGNAL(finished()),syncTimeThread,SLOT(deleteLater()));
        syncTimeThread->start();
        syncThreadFlag = true;
    }
    return;
}

CSyncTime::CSyncTime(DateTime *dataTimeUI,QString successMSG,QString failMSG)
{
    this -> dataTimeUI = dataTimeUI;
    this -> successMSG = successMSG;
    this -> failMSG = failMSG;
}

CSyncTime::~CSyncTime()
{

}
void CSyncTime::run()
{
    QDBusInterface *r_datetimeiface = new QDBusInterface("org.freedesktop.timedate1",
                                           "/org/freedesktop/timedate1",
                                           "org.freedesktop.timedate1",
                                           QDBusConnection::systemBus(), this);
    while (true) {
        if (this->dataTimeUI->getSyncStatus() == false) {
            syncThreadFlag = false;
            delete r_datetimeiface;
            return;
        }
        r_datetimeiface->call("SetNTP", true, true);
        struct timex txc = {};
        if (adjtimex(&txc) > 0 && txc.maxerror < 16000000) { //同步时间成功
            DateTime::syncRTC();
            this->dataTimeUI->syncNetworkRetLabel->setText(successMSG);
            syncThreadFlag = false;
            delete r_datetimeiface;
            return;
        }
        sleep(2);
    }
}
