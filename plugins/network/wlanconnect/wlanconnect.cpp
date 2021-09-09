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
#include "wlanconnect.h"
#include "ui_wlanconnect.h"

#include "../shell/utils/utils.h"

#include <QGSettings>
#include <QProcess>
#include <QTimer>
#include <QtDBus>
#include <QDir>
#include <QDebug>
#include <QtAlgorithms>


const QString WIRELESS_SWITCH = "wirelessswitch";
const QByteArray GSETTINGS_SCHEMA = "org.ukui.kylin-nm.switch";

const QString KWifiSymbolic     = "network-wireless-signal-excellent";
const QString KWifiLockSymbolic = "network-wireless-secure-signal-excellent";
const QString KWifiGood         = "network-wireless-signal-good";
const QString KWifiLockGood     = "network-wireless-secure-signal-good";
const QString KWifiOK           = "network-wireless-signal-ok";
const QString KWifiLockOK       = "network-wireless-secure-signal-ok";
const QString KWifiLow          = "network-wireless-signal-low";
const QString KWifiLockLow      = "network-wireless-secure-signal-low";
const QString KWifiNone         = "network-wireless-signal-none";
const QString KWifiLockNone     = "network-wireless-secure-signal-none";

const QString KLanSymbolic      = ":/img/plugins/netconnect/eth.svg";
const QString NoNetSymbolic     = ":/img/plugins/netconnect/nonet.svg";
WlanConnect::WlanConnect() :  mFirstLoad(true) {
    pluginName = tr("WlanConnect");
    pluginType = NETWORK;
}

WlanConnect::~WlanConnect()
{
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
    }
    delete m_interface;
    delete m_switchGsettings;
}

QString WlanConnect::get_plugin_name() {
    return pluginName;
}

int WlanConnect::get_plugin_type() {
    return pluginType;
}

QWidget *WlanConnect::get_plugin_ui() {
    if (mFirstLoad) {
        mFirstLoad = false;

        ui = new Ui::WlanConnect;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        qDBusRegisterMetaType<QVector<QStringList>>();
        m_interface = new QDBusInterface("com.kylin.network", "/com/kylin/network",
                                         "com.kylin.network",
                                         QDBusConnection::sessionBus());
        if(!m_interface->isValid()) {
            qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }
        initComponent();
    }
    return pluginWidget;
}

void WlanConnect::plugin_delay_control() {

}

const QString WlanConnect::name() const {

    return QStringLiteral("wlanconnect");
}

void WlanConnect::initSearchText() {
    //~ contents_path /wlanconnect/Network settings"
    ui->detailBtn->setText(tr("Network settings"));
    //~ contents_path /wlanconnect/WLAN
    ui->titleLabel->setText(tr("WLAN"));
    //~ contents_path /wlanconnect/open
    ui->openLabel->setText(tr("open"));
}

bool WlanConnect::eventFilter(QObject *w, QEvent *e) {
    if (e->type() == QEvent::Enter) {
        if (w->findChild<QWidget*>())
            w->findChild<QWidget*>()->setStyleSheet("QWidget{background: palette(button);border-radius:4px;}");
    } else if (e->type() == QEvent::Leave) {
        if (w->findChild<QWidget*>())
            w->findChild<QWidget*>()->setStyleSheet("QWidget{background: palette(base);border-radius:4px;}");
    }
    return QObject::eventFilter(w,e);
}

void WlanConnect::initComponent() {
    wifiSwtch = new SwitchButton(pluginWidget);
    ui->openWIifLayout->addWidget(wifiSwtch);
    kdsDbus = new QDBusInterface("org.ukui.kds", \
                                 "/", \
                                 "org.ukui.kds.interface", \
                                 QDBusConnection::systemBus());

    // 无线网络断开或连接时刷新可用网络列表
    connect(m_interface, SIGNAL(wirelessActivating(QString,QString)), this, SLOT(setItemLoading(QString,QString)));
    connect(m_interface, SIGNAL(listUpdate(QString)), this, SLOT(setItemStopLoading(QString)));

    getDeviceList();

    if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA)) {
        m_switchGsettings = new QGSettings(GSETTINGS_SCHEMA);
        connect(wifiSwtch, &SwitchButton::checkedChanged, this, [=] (bool checked) {
            qDebug()<<"wiredSwitch status:"<<checked<<__LINE__;
            m_interface->call(QStringLiteral("setWirelessSwitchEnable"),checked);
            m_switchGsettings->set(WIRELESS_SWITCH,checked);
        });
        setSwitchStatus();
        connect(m_switchGsettings, &QGSettings::changed, this, [=] (const QString &key) {
            if (key == WIRELESS_SWITCH) {
                setSwitchStatus();
            }
        });
    } else {
        qDebug()<<"[netconnect] org.ukui.kylin-nm.switch is not installed!";
    }

    connect(ui->detailBtn, &QPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        runExternalApp();
    });
}

void WlanConnect::getDeviceList()
{
    QDBusMessage result = m_interface->call(QStringLiteral("getDeviceListAndEnabled"),1);
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "getWirelessDeviceList error:" << result.errorMessage();
    }
    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    dbusArg >> deviceListMap;
}

void WlanConnect::setSwitchStatus()
{
    bool status = m_switchGsettings->get(WIRELESS_SWITCH).toBool();
    wifiSwtch->blockSignals(true);
    wifiSwtch->setChecked(status);
    wifiSwtch->blockSignals(false);
    if (!wifiSwtch->isChecked()) {
        clearLayout(ui->availableLayout);
    } else {
        clearLayout(ui->availableLayout);
        QMap<QString, bool>::iterator iter;
        int count = 1;
        for (iter = deviceListMap.begin(); iter != deviceListMap.end(); iter++) {
            getNetListFromDevice(iter.key(), iter.value(), ui->availableLayout, count);
            count ++;
        }
    }
}

void WlanConnect::setItemLoading(QString devName, QString ssid)
{
    QMap<QString, WlanItem*>::iterator iter;
    for (iter =  deviceWlanlistInfo.wlanItemMap.begin(); iter !=  deviceWlanlistInfo.wlanItemMap.end(); iter++) {
        if (iter.key() == ssid) {
            iter.value()->setCountCurrentTime(0);
            iter.value()->setWaitPage(1);
            iter.value()->startLoading();
        }
        qDebug()<<iter.key();
    }
}

void WlanConnect::setItemStopLoading(QString devName)
{
    QMap<QString, WlanItem*>::iterator iterFir;
    for (iterFir =  deviceWlanlistInfo.wlanItemMap.begin(); iterFir !=  deviceWlanlistInfo.wlanItemMap.end(); iterFir++) {
        if (iterFir.value()->loading) {
            iterFir.value()->stopLoading();
        }
    }
    QMap<QString, ItemFrame*>::iterator iterSec;
    for (iterSec =  deviceWlanlistInfo.deviceLayoutMap.begin(); iterSec !=  deviceWlanlistInfo.deviceLayoutMap.end(); iterSec++) {
        if (iterSec.key() == devName) {
            ItemFrame * frame = new ItemFrame;
            frame = iterSec.value();
            rebuildOneFrame(devName,frame);
            break;
        }
    }
}

void WlanConnect::rebuildOneFrame(QString deviceName, ItemFrame *frame)
{
    clearLayout(frame->lanItemLayout);
    if (!wifiSwtch->isChecked()) {
        return;
    }

    QDBusMessage result = m_interface->call(QStringLiteral("getWirelessList"));
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "getWirelessList error:" << result.errorMessage();
    }
    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    QMap<QString, QVector<QStringList>> variantList;
    dbusArg >> variantList;
    QMap<QString, QVector<QStringList>>::iterator iter;

    for (iter = variantList.begin(); iter != variantList.end(); iter++) {
        if (deviceName == iter.key()) {
            rebuildDeviceComponent(frame, iter.key(), 1);
            QVector<QStringList> wlanListInfo = iter.value();

            bool isLock = true;
            if (wlanListInfo.at(0).at(0) == "--") {
                for (int i = 1; i < wlanListInfo.length(); i++) {
                    if (wlanListInfo.at(i).at(2) == "") {
                        isLock = false;
                    } else {
                        isLock = true;
                    }
                    rebuildAvailComponent(frame, wlanListInfo.at(i).at(0), wlanListInfo.at(i).at(1), isLock, false, "ethernet");
                }
            } else {
                if (wlanListInfo.at(0).at(2) == "") {
                    isLock = false;
                } else {
                    isLock = true;
                }
                rebuildAvailComponent(frame, wlanListInfo.at(0).at(0), wlanListInfo.at(0).at(1), isLock, true, "ethernet");
                for (int i = 1; i < wlanListInfo.length(); i++) {
                    if (wlanListInfo.at(i).at(2) == "") {
                        isLock = false;
                    } else {
                        isLock = true;
                    }
                    rebuildAvailComponent(frame, wlanListInfo.at(i).at(0), wlanListInfo.at(i).at(1), isLock, false, "ethernet");
                }
            }
        }
    }
}

void WlanConnect::getNetListFromDevice(QString deviceName, bool deviceStatus, QVBoxLayout *layout, int count)
{
    if (!wifiSwtch->isChecked()) {
        return;
    }
    QDBusMessage result = m_interface->call(QStringLiteral("getWirelessList"));
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "getWirelessList error:" << result.errorMessage();
    }
    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    QMap<QString, QVector<QStringList>> variantList;
    dbusArg >> variantList;
    QMap<QString, QVector<QStringList>>::iterator iter;

    for (iter = variantList.begin(); iter != variantList.end(); iter++) {
        if (deviceName == iter.key()) {
            ItemFrame *deviceFrame = new ItemFrame(pluginWidget);
            ui->availableLayout->addWidget(deviceFrame);
            dropDownMap.insert(deviceName,false);
            rebuildDeviceComponent(deviceFrame, iter.key(), count);
            QVector<QStringList> wlanListInfo = iter.value();
            deviceWlanlistInfo.deviceLayoutMap.insert(iter.key(),deviceFrame);

            bool isLock = true;
            if (wlanListInfo.at(0).at(0) == "--") {
                for (int i = 1; i < wlanListInfo.length(); i++) {
                    if (wlanListInfo.at(i).at(2) == "") {
                        isLock = false;
                    } else {
                        isLock = true;
                    }
                    rebuildAvailComponent(deviceFrame, wlanListInfo.at(i).at(0), wlanListInfo.at(i).at(1), isLock, false, "ethernet");
                }
            } else {
                if (wlanListInfo.at(0).at(2) == "") {
                    isLock = false;
                } else {
                    isLock = true;
                }
                rebuildAvailComponent(deviceFrame, wlanListInfo.at(0).at(0), wlanListInfo.at(0).at(1), isLock, true, "ethernet");
                for (int i = 1; i < wlanListInfo.length(); i++) {
                    if (wlanListInfo.at(i).at(2) == "") {
                        isLock = false;
                    } else {
                        isLock = true;
                    }
                    rebuildAvailComponent(deviceFrame, wlanListInfo.at(i).at(0), wlanListInfo.at(i).at(1), isLock, false, "ethernet");
                }
            }
        }
    }
}

void WlanConnect::runExternalApp() {
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}


QString WlanConnect::wifiIcon(bool isLock, int strength) {
    switch (strength) {
    case 1:
        return isLock ? KWifiLockSymbolic : KWifiSymbolic;
    case 2:
        return isLock ? KWifiLockGood : KWifiGood;
    case 3:
        return isLock ? KWifiLockOK : KWifiOK;
    case 4:
        return isLock ? KWifiLockLow : KWifiLow;
    case 5:
        return isLock ? KWifiLockNone : KWifiNone;
    default:
        return "";
    }
}

int WlanConnect::setSignal(QString lv) {
    int signal = lv.toInt();
    int signalLv = 0;

    if (signal > 75) {
        signalLv = 1;
    } else if (signal > 55 && signal <= 75) {
        signalLv = 2;
    } else if (signal > 35 && signal <= 55) {
        signalLv = 3;
    } else if (signal > 15 && signal  <= 35) {
        signalLv = 4;
    } else if (signal <= 15) {
        signalLv = 5;
    }
    return signalLv;
}

void WlanConnect::clearLayout(QVBoxLayout * layout) {
    if (layout->layout() != NULL) {
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != NULL) {
            delete item->widget();
            delete item;
            item = nullptr;
        }
    }
}

void WlanConnect::rebuildDeviceComponent(ItemFrame *frame, QString deviceName, int count)
{
    frame->deviceFrame->deviceLabel->setText(tr("card")+QString("%1").arg(count)+"："+deviceName);

    connect(frame->deviceFrame->dropDownLabel, &DrownLabel::labelClicked, this, [=] () {
        QMap<QString, bool>::iterator iters;
        for (iters =  dropDownMap.begin(); iters !=  dropDownMap.end(); iters++) {
            if (frame->deviceFrame->deviceLabel->text().contains(iters.key())) {
                iters.value() =!iters.value();
                frame->deviceFrame->dropDownLabel->setDropDownStatus(iters.value());
                qDebug()<<iters.key()<<iters.value();
            }
        }
        dropDownAnimation(frame->deviceFrame, deviceName);
    });
}

void WlanConnect::dropDownAnimation(DeviceFrame * deviceFrame, QString deviceName)
{
    QMap<QString, bool>::iterator iters;
    for (iters =  dropDownMap.begin(); iters !=  dropDownMap.end(); iters++) {
        if (deviceFrame->deviceLabel->text().contains(iters.key())) {
            qDebug()<<deviceFrame->deviceLabel->text()<<"dropdown status:"<<iters.value();
            if (iters.value()) {
                QMap<QString, ItemFrame*>::iterator iter;
                for (iter =  deviceWlanlistInfo.deviceLayoutMap.begin(); iter !=  deviceWlanlistInfo.deviceLayoutMap.end(); iter++) {
                    if (iter.key() == deviceName) {
                        iter.value()->lanItemFrame->hide();
                    }
                }
            } else {
                QMap<QString, ItemFrame*>::iterator iter;
                for (iter =  deviceWlanlistInfo.deviceLayoutMap.begin(); iter !=  deviceWlanlistInfo.deviceLayoutMap.end(); iter++) {
                    if (iter.key() == deviceName) {
                        iter.value()->lanItemFrame->show();
                    }
                }
            }
        }
    }
}

void WlanConnect::rebuildWifiActComponent(QString iconPath, QStringList netNameList) {
}

void WlanConnect::rebuildAvailComponent(ItemFrame *frame, QString name, QString signal, bool isLock, bool status, QString type) {
    qDebug()<<name<<signal;
    int sign = setSignal(signal);
    QString iconamePath = wifiIcon(isLock, sign);
    WlanItem * wlanItem = new WlanItem(pluginWidget);
    QIcon searchIcon = QIcon::fromTheme(iconamePath);
    if (iconamePath != KLanSymbolic && iconamePath != NoNetSymbolic) {
        wlanItem->iconLabel->setProperty("useIconHighlightEffect", 0x10);
    }
    wlanItem->iconLabel->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(24, 24))));
    wlanItem->titileLabel->setText(name);
    if (status) {
        wlanItem->statusLabel->setText(tr("connected"));
    } else {
        wlanItem->statusLabel->setText(tr("no connected"));
    }

    connect(wlanItem->infoLabel, &InfoButton::clicked, this, [=]{
        // open landetail page

    });

    connect(wlanItem, &QPushButton::clicked, this, [=] {
        runKylinmApp(name,type);
    });
    deviceWlanlistInfo.wlanItemMap.insert(name,wlanItem);
    frame->lanItemLayout->addWidget(wlanItem);
}

