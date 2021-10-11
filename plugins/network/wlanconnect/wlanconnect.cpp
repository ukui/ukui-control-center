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

#define WIRELESS_TYPE 1
#define WIFI_ENABLE 1
#define EMPTY_ERROR 2
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

bool intThan(int sign1, int sign2)
{
    return sign1 < sign2;
}


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
    //~ contents_path /wlanconnect/Advanced settings"
    ui->detailBtn->setText(tr("Advanced settings"));
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
    // 有线网络断开或连接时刷新可用网络列表
    connect(m_interface, SIGNAL(wlanactiveConnectionStateChanged(QString, QString, QString, int)), this, SLOT(updateOneWlanFrame(QString, QString, QString, int)));
    //无线网络新增时添加网络
    connect(m_interface, SIGNAL(wlanAdd(QString, QStringList)), this, SLOT(updateOneWlanFrame(QString, QStringList)));
    //删除无线网络
    connect(m_interface, SIGNAL(wlanRemove(QString, QString)), this, SLOT(updateOneWlanFrame(QString, QString)));
    //网卡插拔处理
    connect(m_interface, SIGNAL(deviceStatusChanged()), this, SLOT(updateWlanListWidget()));
    //信号更新处理
    connect(m_interface, SIGNAL(signalStrengthChange(QString, QString, int)), this, SLOT(updateStrengthList(QString, QString, int)));

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
        wifiSwtch->blockSignals(true);
        wifiSwtch->setChecked(true);
        wifiSwtch->blockSignals(false);
        initNet();
        qDebug()<<"[netconnect] org.ukui.kylin-nm.switch is not installed!";
    }

    connect(ui->detailBtn, &QPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        runExternalApp();
    });

    m_scanTimer = new QTimer(this);
    m_scanTimer->start(20 * 1000);
    connect(m_scanTimer, &QTimer::timeout, this, [=](){
        qDebug() << "time to rescan wifi";
        if (m_interface->isValid()) {
            m_interface->call("rescan");
        }
    });
}

void WlanConnect::updateOneWlanFrame(QString deviceName, QString wlannName)
{
    if(!wifiSwtch->isChecked()) {
        return;
    }
    qDebug()<<"删除WIFI"<<deviceName<<wlannName;
    QMap<QString, ItemFrame *>::iterator iters;
    for (iters = deviceWlanlistInfo.deviceLayoutMap.begin(); iters != deviceWlanlistInfo.deviceLayoutMap.end(); iters++) {
        qDebug()<<iters.key()<<iters.value();
        if (iters.value()->lanItemLayout->layout() != NULL) {
            WlanItem* item;
            QMap<QString, WlanItem*>::iterator iter;
            for (iter = deviceWlanlistInfo.wlanItemMap.begin(); iter != deviceWlanlistInfo.wlanItemMap.end(); iter++) {
                if (iter.key() == wlannName) {
                    item = iter.value();
                }
            }
            iters.value()->lanItemLayout->removeWidget(item);
            deviceWlanlistInfo.wlanItemMap.erase(iter);
        }
    }
}

void WlanConnect::updateStrengthList(QString deviceName, QString ssid, int strength)
{
    if(!wifiSwtch->isChecked() || !isFinished) {
        return;
    }
    qDebug()<<"更新无线网络信号强度(Update wireless network signal strength)："<<deviceName<<ssid<<strength;
    bool isLock = false;
    QMap<QString, ItemFrame *>::iterator iters;
    for (iters = deviceWlanlistInfo.deviceLayoutMap.begin(); iters != deviceWlanlistInfo.deviceLayoutMap.end(); iters++) {
        qDebug()<<iters.key()<<iters.value();
        if (iters.key() == deviceName) {
            if (iters.value()->lanItemLayout->layout() != NULL) {
                WlanItem* item;
                QMap<QString, WlanItem*>::iterator iter;
                for (iter = deviceWlanlistInfo.wlanItemMap.begin(); iter != deviceWlanlistInfo.wlanItemMap.end(); iter++) {
                    if (iter.key() == ssid) {
                        item = iter.value();
                    }
                }
                isLock = item->isLock;
                //remove the item from layout
                iters.value()->lanItemLayout->removeWidget(item);
                //get position
                int index = sortWlanNet(deviceName,ssid, QString::number(strength));
                //add the item to new position
                qDebug()<<"更新后位置:"<<index;
                int sign = setSignal(QString::number(strength));
                QString iconamePath = wifiIcon(isLock, sign);
                QIcon searchIcon = QIcon::fromTheme(iconamePath);
                if (iconamePath != KLanSymbolic && iconamePath != NoNetSymbolic) {
                    item->iconLabel->setProperty("useIconHighlightEffect", 0x10);
                }
                item->iconLabel->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(24, 24))));

                iters.value()->lanItemLayout->insertWidget(index, item);
            }
        }
    }
}

void WlanConnect::updateWlanListWidget()
{
    qDebug()<<"网卡插拔处理";
    QEventLoop eventloop;
    QTimer::singleShot(300, &eventloop, SLOT(quit()));
    eventloop.exec();
    getDeviceList();
    initNet();
}

void WlanConnect::updateOneWlanFrame(QString deviceName, QString ssid, QString uuid, int status)
{
    if (!wifiSwtch->isChecked() || !isFinished) {
        return;
    }
    qDebug() << deviceName << ssid << uuid << status;
    QMap<QString, WlanItem*>::iterator iter;
    for (iter = deviceWlanlistInfo.wlanItemMap.begin(); iter != deviceWlanlistInfo.wlanItemMap.end(); iter++) {
        bool isFind = false;
        if (ssid.isEmpty()) {
            if (iter.value()->uuid == uuid) {
                qDebug() <<  "find " << iter.value()->titileLabel->text();
                isFind = true;
            }
        } else {
            if (ssid ==iter.key()) {
                isFind = true;
            }
        }
        if(isFind) {
            if (status == 1) {
                iter.value()->setCountCurrentTime(0);
                iter.value()->setWaitPage(1);
                iter.value()->startLoading();
            }
            if (status == 2) {
                iter.value()->stopLoading();
                iter.value()->statusLabel->setStyleSheet("");
                iter.value()->statusLabel->setMinimumSize(36,36);
                iter.value()->statusLabel->setMaximumSize(16777215,16777215);
                iter.value()->statusLabel->setText(tr("connected"));
                iter.value()->isAcitve = true;
                iter.value()->uuid = uuid;
                QMap<QString, ItemFrame *>::iterator iters;
                for (iters = deviceWlanlistInfo.deviceLayoutMap.begin(); iters != deviceWlanlistInfo.deviceLayoutMap.end(); iters++) {
                    if (iters.key() == deviceName) {
                        iters.value()->lanItemLayout->removeWidget(iter.value());
                        iters.value()->lanItemLayout->insertWidget(0,iter.value());
                    }
                }
            }
            if (status == 3) {
                iter.value()->setCountCurrentTime(0);
                iter.value()->setWaitPage(1);
                iter.value()->startLoading();
            }
            if (status == 4) {
                iter.value()->stopLoading();
                iter.value()->statusLabel->setStyleSheet("");
                iter.value()->statusLabel->setMinimumSize(36,36);
                iter.value()->statusLabel->setMaximumSize(16777215,16777215);
                iter.value()->statusLabel->setText("");
                iter.value()->isAcitve = false;
                iter.value()->uuid = "";
            }
        }
    }
}

void WlanConnect::updateOneWlanFrame(QString deviceName, QStringList wlanInfo)
{
    qDebug()<<"新增无线网络"<<wlanInfo;
    if(!wifiSwtch->isChecked() || !isFinished) {
        return;
    }
    bool isLock = true;
    if (wlanInfo.at(2) == "") {
        isLock = false;
    } else {
        isLock = true;
    }
    if (deviceName == nullptr) {
        QMap<QString, ItemFrame *>::iterator iter;
        for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
            addOneWlanFrame(iter.value(), iter.key(), wlanInfo.at(0), wlanInfo.at(1), isLock, false, WIRELESS_TYPE);
        }
    } else {
        QMap<QString, ItemFrame *>::iterator iter;
        for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
            if (deviceName == iter.key()) {
                addOneWlanFrame(iter.value(), deviceName, wlanInfo.at(0), wlanInfo.at(1), isLock, false, WIRELESS_TYPE);
            }
        }
    }
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
    if (deviceListMap.size() == 0) {
        wifiSwtch->blockSignals(true);
        wifiSwtch->setChecked(false);
        wifiSwtch->blockSignals(false);
        return;
    }
    bool status = m_switchGsettings->get(WIRELESS_SWITCH).toBool();
    wifiSwtch->blockSignals(true);
    wifiSwtch->setChecked(status);
    wifiSwtch->blockSignals(false);
    if (!wifiSwtch->isChecked()) {
        clearLayout(ui->availableLayout);
    } else {
        initNet();
    }
}

void WlanConnect::initNet() {
    clearLayout(ui->availableLayout);
    QMap<QString, bool>::iterator iter;
    int count = 1;
    for (iter = deviceListMap.begin(); iter != deviceListMap.end(); iter++) {
        if (getNetListFromDevice(iter.key(), iter.value(), ui->availableLayout, count) == 2) {
            QEventLoop eventloop;
            QTimer::singleShot(300, &eventloop, SLOT(quit()));
            eventloop.exec();
            initNet();
        }
        count ++;
    }
}

int WlanConnect::getNetListFromDevice(QString deviceName, bool deviceStatus, QVBoxLayout *layout, int count)
{
    if (!wifiSwtch->isChecked()) {
        return WIFI_ENABLE;
    }
    QDBusMessage result = m_interface->call(QStringLiteral("getWirelessList"));
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "getWirelessList error:" << result.errorMessage();
    }
    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    QMap<QString, QVector<QStringList>> variantList;
    dbusArg >> variantList;
    if (variantList.size() == 0) {
        isFinished = false;
        return EMPTY_ERROR;
    }
    QMap<QString, QVector<QStringList>>::iterator iter;

    for (iter = variantList.begin(); iter != variantList.end(); iter++) {
        if (deviceName == iter.key()) {
            ItemFrame *deviceFrame = new ItemFrame(pluginWidget);
            ui->availableLayout->addWidget(deviceFrame);
            dropDownMap.insert(deviceName,false);
            rebuildDeviceComponent(deviceFrame, iter.key(), count);
            QVector<QStringList> wlanListInfo = iter.value();
            deviceWlanlistInfo.deviceLayoutMap.insert(iter.key(),deviceFrame);
            qDebug()<<iter.value();
            bool isLock = true;
            QString uuid = "";
            if (wlanListInfo.at(0).at(0) == "--") {
                for (int i = 1; i < wlanListInfo.length(); i++) {
                    if (wlanListInfo.at(i).at(2) == "") {
                        isLock = false;
                    } else {
                        isLock = true;
                    }
                    rebuildAvailComponent(deviceFrame, deviceName, wlanListInfo.at(i).at(0), wlanListInfo.at(i).at(1), uuid, isLock, false, WIRELESS_TYPE);
                }
            } else {
                if (wlanListInfo.at(0).at(2) == "") {
                    isLock = false;
                } else {
                    isLock = true;
                }
                uuid = wlanListInfo.at(0).at(3);
                rebuildAvailComponent(deviceFrame, deviceName, wlanListInfo.at(0).at(0), wlanListInfo.at(0).at(1), uuid, isLock, true, WIRELESS_TYPE);
                for (int i = 1; i < wlanListInfo.length(); i++) {
                    if (wlanListInfo.at(i).at(2) == "") {
                        isLock = false;
                    } else {
                        isLock = true;
                    }
                    rebuildAvailComponent(deviceFrame, deviceName, wlanListInfo.at(i).at(0), wlanListInfo.at(i).at(1), uuid, isLock, false, WIRELESS_TYPE);
                }
            }
        }
    }
    isFinished = true;
    return 0;
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
//下拉按钮逻辑处理
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

void WlanConnect::addOneWlanFrame(ItemFrame *frame, QString deviceName, QString name, QString signal, bool isLock, bool status, int type)
{
    qDebug()<<name<<signal;
    int sign = setSignal(signal);
    QString iconamePath = wifiIcon(isLock, sign);
    WlanItem * wlanItem = new WlanItem(status, isLock, pluginWidget);
    QIcon searchIcon = QIcon::fromTheme(iconamePath);
    if (iconamePath != KLanSymbolic && iconamePath != NoNetSymbolic) {
        wlanItem->iconLabel->setProperty("useIconHighlightEffect", 0x10);
    }
    wlanItem->iconLabel->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(24, 24))));
    wlanItem->titileLabel->setText(name);
    if (status) {
        wlanItem->statusLabel->setText(tr("connected"));
    } else {
        wlanItem->statusLabel->setText("");
    }

    connect(wlanItem->infoLabel, &InfoButton::clicked, this, [=]{
        // open landetail page
        m_interface->call(QStringLiteral("showPropertyWidget"), deviceName, name);
    });

    connect(wlanItem, &QPushButton::clicked, this, [=] {
        if (status) {
            deActiveConnect(name, deviceName, type);
        } else {
            activeConnect(name, deviceName, type);
        }
    });
    deviceWlanlistInfo.wlanItemMap.insert(name, wlanItem);
    deviceFrameMap.insert(deviceName,frame);
    int index = sortWlanNet(deviceName,name, signal);
    qDebug()<<"位置："<<index;
    frame->lanItemLayout->insertWidget(index, wlanItem);
}

int WlanConnect::sortWlanNet(QString deviceName, QString name, QString signal)
{
    qDebug()<<name<<signal;
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
            QVector<QStringList> wlanListInfo = iter.value();
            for (int i = 0; i < wlanListInfo.size(); i++) {
                if (name == wlanListInfo.at(i).at(0)) {
                    return i;
                }
            }
        }
    }
    return 0;
}

void WlanConnect::rebuildAvailComponent(ItemFrame *frame, QString deviceName, QString name, QString signal, QString uuid, bool isLock, bool status, int type) {
    qDebug()<<name<<signal;
    int sign = setSignal(signal);
    QString iconamePath = wifiIcon(isLock, sign);
    WlanItem * wlanItem = new WlanItem(status, isLock, pluginWidget);
    QIcon searchIcon = QIcon::fromTheme(iconamePath);
    if (iconamePath != KLanSymbolic && iconamePath != NoNetSymbolic) {
        wlanItem->iconLabel->setProperty("useIconHighlightEffect", 0x10);
    }
    wlanItem->isLock = isLock;
    wlanItem->iconLabel->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(24, 24))));
    wlanItem->titileLabel->setText(name);
    if (status) {
        wlanItem->statusLabel->setText(tr("connected"));
        wlanItem->uuid = uuid;
    } else {
        wlanItem->statusLabel->setText("");
    }

    connect(wlanItem->infoLabel, &InfoButton::clicked, this, [=]{
        // open landetail page
        m_interface->call(QStringLiteral("showPropertyWidget"), deviceName, name);

    });

    connect(wlanItem, &QPushButton::clicked, this, [=] {
        if (wlanItem->isAcitve) {
            deActiveConnect(name, deviceName, type);
        } else {
            activeConnect(name, deviceName, type);
        }
    });
    QStringList list;
    list.append(name);
    list.append(signal);
    wlanSignalList.push_back(list);

    deviceWlanlistInfo.wlanItemMap.insert(name, wlanItem);
    deviceFrameMap.insert(deviceName,frame);
    frame->lanItemLayout->addWidget(wlanItem);
}

void WlanConnect::activeConnect(QString netName, QString deviceName, int type) {
    qDebug() << "try to connect "<< netName << " in " << deviceName;
    m_interface->call("activateConnect",type, deviceName, netName);
}

void WlanConnect::deActiveConnect(QString netName, QString deviceName, int type) {
    qDebug() << "try to disconnect "<< netName << " in " << deviceName;
    m_interface->call("deActivateConnect",type, deviceName, netName);
}

