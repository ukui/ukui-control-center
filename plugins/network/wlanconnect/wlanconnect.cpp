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
        refreshTimer = new QTimer();
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

}

void WlanConnect::initComponent() {
    wifiBtn = new SwitchButton(pluginWidget);
    ui->openWIifLayout_3->addWidget(wifiBtn);
    kdsDbus = new QDBusInterface("org.ukui.kds", \
                                 "/", \
                                 "org.ukui.kds.interface", \
                                 QDBusConnection::systemBus());

    // 无线网络断开或连接时刷新可用网络列表
    connect(m_interface, SIGNAL(getWifiListFinished()), this, SLOT(refreshNetInfoTimerSlot()));
    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(getNetList()));
    // 网络配置信息发生变化时刷新可用网络列表
    connect(m_interface,SIGNAL(configurationChanged()), this, SLOT(getNetList()));
    if (getwifiisEnable()) {
        wifiBtn->setChecked(getInitStatus());
        ui->widget->setVisible(getInitStatus());
    }
    connect(wifiBtn, &SwitchButton::checkedChanged, this,[=](bool checked) {
        wifiBtn->blockSignals(true);
        wifiSwitchSlot(checked);
        ui->widget->setVisible(checked);
        wifiBtn->blockSignals(false);
        kdsDbus->call("emitRfkillStatusChanged");
        if (m_interface) {
            m_interface->call("requestRefreshWifiList");
        }
        getNetList();
    });
    connect(ui->detailBtn, &QPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        runExternalApp();
    });

    m_interface->call("requestRefreshWifiList");
    ui->verticalLayout_4->setContentsMargins(0, 0, 32, 0);
}

void WlanConnect::wifiSwitchSlot(bool status) {

    QString wifiStatus = status ? "on" : "off";
    QString program = "nmcli";
    QStringList arg;
    arg << "radio" << "wifi" << wifiStatus;
    QProcess *nmcliCmd = new QProcess(this);
    nmcliCmd->start(program, arg);
    nmcliCmd->waitForFinished();
}

bool WlanConnect::getInitStatus() {
    QDBusInterface interface( "org.freedesktop.NetworkManager",
                              "/org/freedesktop/NetworkManager",
                              "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus() );
    //　获取当前wifi是否打开
    QDBusReply<QVariant> m_result = interface.call("Get", "org.freedesktop.NetworkManager", "WirelessEnabled");

    if (m_result.isValid()) {
        bool status = m_result.value().toBool();
        return status;
    } else {
        qDebug()<<"org.freedesktop.NetworkManager get invalid"<<endl;
        return false;
    }
}

void WlanConnect::runExternalApp() {
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}

void WlanConnect::refreshNetInfoTimerSlot() {
    refreshTimer->start(200);
}

void WlanConnect::getNetList() {
    refreshTimer->stop();
    wifiBtn->blockSignals(true);
    getInitStatus();
    wifiBtn->blockSignals(false);
    this->wifilist.clear();
    QDBusReply<QVector<QStringList>> reply = m_interface->call("getWifiList");
    if (!reply.isValid()) {
        qWarning() << "value method called failed!";
    }
    if (getWifiStatus() && reply.value().length() == 1 && getHasWirelessCard()) {
        QElapsedTimer time;
        time.start();
        while (time.elapsed() < 300) {
            QCoreApplication::processEvents();
        }
        if (m_interface) {
            m_interface->call("requestRefreshWifiList");
        }
        getNetList();
    } else {
        connectWifi.clear();
        if (reply.value().length() != 0) {
            if (reply.value().at(0).at(0) != "--") {
                connectWifi = reply.value().at(0).at(0);
            } else {
                connectWifi = "--";
            }
        } else {
            connectWifi = "--";
        }
        getWifiListDone(reply);
        for (int i = 1; i < reply.value().length(); i++) {
            QString wifiName;
            wifiName = reply.value().at(i).at(0);
            if (reply.value().at(i).at(2) != NULL && reply.value().at(i).at(2) != "--") {
                wifiName += "lock";
            }
            QString signal = reply.value().at(i).at(1);
            int sign = this->setSignal(signal);
            wifilist.append(wifiName + QString::number(sign));
        }
        QString iconamePath;
        for (int i = 0; i < wifilist.size(); i++) {
            if (!wifiBtn->isChecked()) {
                break;
            }
            QString wifiInfo = wifilist.at(i);
            bool isLock = wifiInfo.contains("lock");
            QString wifiName = wifiInfo.left(wifiInfo.size() - 1);
            int wifiStrength = wifiInfo.right(1).toInt();
            wifiName = isLock ? wifiName.remove("lock") : wifiName;
            iconamePath = wifiIcon(isLock, wifiStrength);
            rebuildAvailComponent(iconamePath, wifiName, "wifi");
        }
    }
}

void WlanConnect::getWifiListDone(QVector<QStringList> getwifislist) {
    clearContent();
    mActiveInfo.clear();
    getActiveConInfo(mActiveInfo);
    if (!getwifislist.isEmpty() && getwifislist.length() != 1) {
        connectedWifi.clear();
        wifiList.clear();
        QString actWifiName;

        int index = 0;
        while (index < mActiveInfo.size()) {
            if (mActiveInfo[index].strConType == "wifi"
                    || mActiveInfo[index].strConType == "802-11-wireless") {
                actWifiName = QString(mActiveInfo[index].strConName);
                break;
            }
            index++;
        }
        QString wname;
        for (int i = 0; i < getwifislist.size(); ++i) {
            if (getwifislist.at(i).at(0) == actWifiName) {
                wname = getwifislist.at(i).at(0);
                if (getwifislist.at(i).at(2) != NULL && getwifislist.at(i).at(2) != "--") {
                    wname += "lock";
                }
                connectedWifi.insert(wname, this->setSignal(getwifislist.at(i).at(1)));
            } else if (connectWifi != "--" && getwifislist.at(i).at(0) == connectWifi && getwifislist.at(i).at(0) != actWifiName) {
                wname = actWifiName;
                if (getwifislist.at(i).at(2) != NULL && getwifislist.at(i).at(2) != "--") {
                    wname += "lock";
                }
                connectedWifi.insert(wname, this->setSignal(getwifislist.at(i).at(1)));
            }
        }
    }
    if (!this->connectedWifi.isEmpty()) {
        QMap<QString, int>::iterator iter = this->connectedWifi.begin();

        QString connectedWifiName = iter.key();
        int strength = iter.value();

        bool isLock = connectedWifiName.contains("lock");
        connectedWifiName = isLock ? connectedWifiName.remove("lock") : connectedWifiName;
        QString iconamePah;
        iconamePah = wifiIcon(isLock, strength);
        actWifiNames.append(connectedWifiName);
        rebuildWifiActComponent(iconamePah, actWifiNames);
    }
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

void WlanConnect::clearContent() {
    if (ui->availableLayout_3->layout() != NULL) {
        QLayoutItem* item;
        while ((item = ui->availableLayout_3->layout()->takeAt(0)) != NULL ) {
            delete item->widget();
            delete item;
            item = nullptr;
        }
    }

    if (ui->detailLayOut_3->layout() != NULL) {
        QLayoutItem* item;
        while ((item = ui->detailLayOut_3->layout()->takeAt(1)) != NULL) {
            delete item->widget();
            delete item;
            item = nullptr;
        }
    }

    this->connectedWifi.clear();
    this->actWifiNames.clear();
    this->wifiList.clear();
    this->lanList.clear();
    this->TwifiList.clear();

}

void WlanConnect::rebuildWifiActComponent(QString iconPath, QStringList netNameList) {
    bool hasNet = false;
    for (int i = 0; i < netNameList.size(); ++i) {
        if (netNameList.at(i) == "无连接" || netNameList.at(i) == "No net") {
            hasNet = true;
        }
        QWidget *frame = new QWidget;
        frame->setContentsMargins(0,0,0,0);

        QVBoxLayout * vLayout = new QVBoxLayout;
        vLayout->setContentsMargins(0,0,0,0);

        QString wifiName;
        if (connectWifi != "--" && connectWifi != netNameList.at(i)) {
            wifiName = connectWifi;
        } else {
            wifiName = netNameList.at(i);
        }

        HoverBtn * deviceItem;
        if (!hasNet) {
            deviceItem = new HoverBtn(netNameList.at(i), false, pluginWidget);
        } else {
            deviceItem = new HoverBtn(netNameList.at(i), true, pluginWidget);
        }
        deviceItem->mPitLabel->setText(wifiName);

        if (!hasNet) {
            deviceItem->mDetailLabel->setText(tr("Connected"));
        } else {
            deviceItem->mDetailLabel->setText("");
        }

        QIcon searchIcon = QIcon::fromTheme(iconPath);
        deviceItem->mPitIcon->setProperty("useIconHighlightEffect", 0x10);
        deviceItem->mPitIcon->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(24, 24))));

        deviceItem->mAbtBtn->setMinimumWidth(100);
        deviceItem->mAbtBtn->setText(tr("Detail"));

        vLayout->addWidget(deviceItem);
        frame->setLayout(vLayout);
        ui->detailLayOut_3->addWidget(frame);
    }
}

void WlanConnect::rebuildAvailComponent(QString iconPath, QString netName, QString type) {
    HoverBtn * wifiItem = new HoverBtn(netName, false, pluginWidget);
    wifiItem->mPitLabel->setText(netName);

    QIcon searchIcon = QIcon::fromTheme(iconPath);
    if (iconPath != KLanSymbolic && iconPath != NoNetSymbolic) {
        wifiItem->mPitIcon->setProperty("useIconHighlightEffect", 0x10);
    }
    wifiItem->mPitIcon->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(24, 24))));
    wifiItem->mAbtBtn->setMinimumWidth(100);
    wifiItem->mAbtBtn->setText(tr("Connect"));

    connect(wifiItem->mAbtBtn, &QPushButton::clicked, this, [=] {
        runKylinmApp(netName,type);
    });

    ui->availableLayout_3->addWidget(wifiItem);
}

void WlanConnect::runKylinmApp(QString netName, QString type) {
    m_interface->call("showPb",type,netName);
}

bool WlanConnect::getWifiStatus() {

    QDBusInterface interface( "org.freedesktop.NetworkManager",
                              "/org/freedesktop/NetworkManager",
                              "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus() );
    // 获取当前wifi是否打开
    QDBusReply<QVariant> m_result = interface.call("Get", "org.freedesktop.NetworkManager", "WirelessEnabled");

    if (m_result.isValid()) {
        bool status = m_result.value().toBool();
        return status;
    } else {
        qDebug()<<"org.freedesktop.NetworkManager get invalid"<<endl;
        return false;
    }
}

//获取当前机器是否有无线网卡设备
bool WlanConnect::getHasWirelessCard(){
    QProcess *wirlessPro = new QProcess(this);
    wirlessPro->start("nmcli device");
    wirlessPro->waitForFinished();
    QString output = wirlessPro->readAll();
    if (output.contains("wifi")) {
        return true;
    } else {
        return false;
    }
}

bool WlanConnect::getwifiisEnable() {
    QDBusInterface m_interface( "org.freedesktop.NetworkManager",
                                "/org/freedesktop/NetworkManager",
                                "org.freedesktop.NetworkManager",
                                QDBusConnection::systemBus() );

    QDBusReply<QList<QDBusObjectPath>> obj_reply = m_interface.call("GetAllDevices");
    if (!obj_reply.isValid()) {
        qDebug()<<"execute dbus method 'GetAllDevices' is invalid in func getObjectPath()";
    }

    QList<QDBusObjectPath> obj_paths = obj_reply.value();

    foreach (QDBusObjectPath obj_path, obj_paths) {
        QDBusInterface interface( "org.freedesktop.NetworkManager",
                                  obj_path.path(),
                                  "org.freedesktop.DBus.Introspectable",
                                  QDBusConnection::systemBus() );

        QDBusReply<QString> reply = interface.call("Introspect");
        if (!reply.isValid()) {
            qDebug()<<"execute dbus method 'Introspect' is invalid in func getObjectPath()";
        }

        if(reply.value().indexOf("org.freedesktop.NetworkManager.Device.Wired") != -1) {

        } else if (reply.value().indexOf("org.freedesktop.NetworkManager.Device.Wireless") != -1) {
            return true;
        }
    }
    return false ;
}

void WlanConnect::getActiveConInfo(QList<ActiveConInfo>& qlActiveConInfo) {
    ActiveConInfo activeNet;
    QDBusInterface interface( "org.freedesktop.NetworkManager",
                              "/org/freedesktop/NetworkManager",
                              "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus() );
    QDBusMessage result = interface.call("Get", "org.freedesktop.NetworkManager", "ActiveConnections");
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusVariant dbvFirst = first.value<QDBusVariant>();
    QVariant vFirst = dbvFirst.variant();
    const QDBusArgument &dbusArgs = vFirst.value<QDBusArgument>();

    QDBusObjectPath objPath;
    dbusArgs.beginArray();

    while (!dbusArgs.atEnd()) {
        dbusArgs >> objPath;
        QDBusInterface interfacePro("org.freedesktop.NetworkManager",
                                    objPath.path(),
                                    "org.freedesktop.NetworkManager.Connection.Active",
                                    QDBusConnection::systemBus());
        QVariant replyType = interfacePro.property("Type");
        QVariant replyUuid = interfacePro.property("Uuid");
        QVariant replyId   = interfacePro.property("Id");

        activeNet.strConName = replyId.toString();
        activeNet.strConType = replyType.toString();
        activeNet.strConUUID = replyUuid.toString();
        qlActiveConInfo.append(activeNet);
    }
    dbusArgs.endArray();
}

