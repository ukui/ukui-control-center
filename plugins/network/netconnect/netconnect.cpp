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
#include "netconnect.h"
#include "ui_netconnect.h"

#include "../shell/utils/utils.h"

#include <QGSettings>
#include <QProcess>
#include <QTimer>
#include <QtDBus>
#include <QDir>
#include <QDebug>
#include <QtAlgorithms>

#define ITEMHEIGH           50
#define WLAN_TYPE           0
#define CONTROL_CENTER_WIFI              "org.ukui.control-center.wifi.switch"

const QString KLanSymbolic      = ":/img/plugins/netconnect/eth.svg";
const QString NoNetSymbolic     = ":/img/plugins/netconnect/nonet.svg";

const QString WIRED_SWITCH = "wiredswitch";
const QByteArray GSETTINGS_SCHEMA = "org.ukui.kylin-nm.switch";

bool sortByVal(const QPair<QString, int> &l, const QPair<QString, int> &r) {
    return (l.second < r.second);
}
NetConnect::NetConnect() :  mFirstLoad(true) {
    pluginName = tr("WiredConnect");
    pluginType = NETWORK;
}

NetConnect::~NetConnect() {
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
    }
    delete m_interface;
    delete m_switchGsettings;
}

QString NetConnect::get_plugin_name() {
    return pluginName;
}

int NetConnect::get_plugin_type() {
    return pluginType;
}

QWidget *NetConnect::get_plugin_ui() {
    if (mFirstLoad) {
        mFirstLoad = false;

        ui = new Ui::NetConnect;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        qDBusRegisterMetaType<QVector<QStringList>>();
        m_interface = new QDBusInterface("com.kylin.network",
                                         "/com/kylin/network",
                                         "com.kylin.network",
                                         QDBusConnection::sessionBus());
        if(!m_interface->isValid()) {
            qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }
        initSearchText();
        initComponent();
    }
    return pluginWidget;
}

void NetConnect::plugin_delay_control() {

}

const QString NetConnect::name() const {

    return QStringLiteral("netconnect");
}

void NetConnect::initSearchText() {
    //~ contents_path /netconnect/Advanced settings"
    ui->detailBtn->setText(tr("Advanced settings"));
    //~ contents_path /netconnect/Netconnect Status
    ui->titleLabel->setText(tr("Wired Network"));
    //~ contents_path /netconnect/open
    ui->openLabel->setText(tr("open"));
}

bool NetConnect::eventFilter(QObject *w, QEvent *e) {
    if (e->type() == QEvent::Enter) {
        if (w->findChild<QWidget*>())
            w->findChild<QWidget*>()->setStyleSheet("QWidget{background: palette(button);border-radius:4px;}");
    } else if (e->type() == QEvent::Leave) {
        if (w->findChild<QWidget*>())
            w->findChild<QWidget*>()->setStyleSheet("QWidget{background: palette(base);border-radius:4px;}");
    }
    return QObject::eventFilter(w,e);
}

void NetConnect::initComponent() {
    wiredSwitch = new SwitchButton(pluginWidget);
    ui->openWIifLayout->addWidget(wiredSwitch);

    kdsDbus = new QDBusInterface("org.ukui.kds", \
                                 "/", \
                                 "org.ukui.kds.interface", \
                                 QDBusConnection::systemBus());
    // 接收到系统创建网络连接的信号时刷新可用网络列表
    QDBusConnection::systemBus().connect(QString(),
                                         QString("/org/freedesktop/NetworkManager/Settings"),
                                         "org.freedesktop.NetworkManager.Settings",
                                         "NewConnection",
                                         this,
                                         SLOT(getNetList(void)));

    // 接收到系统删除网络连接的信号时刷新可用网络列表
    QDBusConnection::systemBus().connect(QString(),
                                         QString("/org/freedesktop/NetworkManager/Settings"),
                                         "org.freedesktop.NetworkManager.Settings",
                                         "ConnectionRemoved",
                                         this,
                                         SLOT(getNetList(void)));

    // 接收到系统更改网络连接属性时把判断是否已刷新的bool值置为false
    QDBusConnection::systemBus().connect(QString(),
                                         QString("/org/freedesktop/NetworkManager"),
                                         "org.freedesktop.NetworkManager", "PropertiesChanged",
                                         this,
                                         SLOT(netPropertiesChangeSlot(QMap<QString,QVariant>)));

    // 有线网络断开或连接时刷新可用网络列表
    connect(m_interface, SIGNAL(wiredActivating(QString,QString)), this, SLOT(setItemStartLoading(QString,QString)));
    connect(m_interface, SIGNAL(listUpdate(QString)), this, SLOT(setItemStopLoading(QString)));

    connect(ui->detailBtn, &QPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        runExternalApp();
    });

    getDeviceList();
    if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA)) {
        m_switchGsettings = new QGSettings(GSETTINGS_SCHEMA);
        connect(wiredSwitch, &SwitchButton::checkedChanged, this, [=] (bool checked) {
            qDebug()<<"wiredSwitch status:"<<checked<<__LINE__;
            m_interface->call(QStringLiteral("setWiredSwitchEnable"),checked);
            m_switchGsettings->set(WIRED_SWITCH,checked);
        });
        setSwitchStatus();
        connect(m_switchGsettings, &QGSettings::changed, this, [=] (const QString &key) {
            if (key == WIRED_SWITCH) {
                setSwitchStatus();
            }
        });
    } else {
        wiredSwitch->blockSignals(true);
        wiredSwitch->setChecked(true);
        wiredSwitch->blockSignals(false);
        initNet();
        qDebug()<<"[netconnect] org.ukui.kylin-nm.switch is not installed!";
    }

}

void NetConnect::getDeviceList()
{
    QDBusMessage result = m_interface->call(QStringLiteral("getDeviceListAndEnabled"),0);
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "getWiredDeviceList error:" << result.errorMessage();
    }
    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    dbusArg >> deviceListMap;
}

void NetConnect::setItemStartLoading(QString devName, QString ssid)
{
    QMap<QString, LanItem*>::iterator iter;
    for (iter =  deviceLanlistInfo.lanItemMap.begin(); iter !=  deviceLanlistInfo.lanItemMap.end(); iter++) {
        if (iter.key() == ssid) {
            iter.value()->setCountCurrentTime(0);
            iter.value()->setWaitPage(1);
            iter.value()->startLoading();
        }
        qDebug()<<iter.key();
    }
}

void NetConnect::setItemStopLoading(QString devName)
{
    QMap<QString, LanItem*>::iterator iterFir;
    for (iterFir =  deviceLanlistInfo.lanItemMap.begin(); iterFir !=  deviceLanlistInfo.lanItemMap.end(); iterFir++) {
        if (iterFir.value()->loading) {
            iterFir.value()->stopLoading();
        }
    }
    QMap<QString, ItemFrame*>::iterator iterSec;
    for (iterSec =  deviceLanlistInfo.deviceLayoutMap.begin(); iterSec !=  deviceLanlistInfo.deviceLayoutMap.end(); iterSec++) {
        if (iterSec.key() == devName) {
            ItemFrame * frame = new ItemFrame;
            frame = iterSec.value();
            rebuildOneFrame(devName,frame);
            break;
        }
    }
}

void NetConnect::rebuildOneFrame(QString deviceName, ItemFrame *frame)
{
    clearLayout(frame->lanItemLayout);
    if (!wiredSwitch->isChecked()) {
        return;
    }
    QDBusMessage result = m_interface->call(QStringLiteral("getWiredList"));
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "getWiredList error:" << result.errorMessage();
    }
    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    QMap<QString, QVector<QStringList>> variantList;
    dbusArg >> variantList;
    QMap<QString, QVector<QStringList>>::iterator iter;

    for (iter = variantList.begin(); iter != variantList.end(); iter++) {
        if (deviceName == iter.key()) {
            rebuildDeviceComponent(frame, iter.key(), 1);
            QVector<QStringList> lanListInfo = iter.value();
            if (lanListInfo.at(0).at(0) == "--") {
                for (int i = 1; i < lanListInfo.length(); i++) {
                    rebuildAvailComponent(frame, KLanSymbolic, iter.key(), lanListInfo.at(i).at(0), lanListInfo.at(i).at(1), false, WLAN_TYPE);
                }
            } else {
                rebuildAvailComponent(frame, KLanSymbolic, iter.key(), lanListInfo.at(0).at(0), lanListInfo.at(0).at(1), true, WLAN_TYPE);
                for (int i = 1; i < lanListInfo.length(); i++) {
                    rebuildAvailComponent(frame, KLanSymbolic, iter.key(), lanListInfo.at(i).at(0), lanListInfo.at(i).at(1), false, WLAN_TYPE);
                }
            }
            rebuildAddComponent(frame, iter.key());
        }
    }
}

void NetConnect::getNetListFromDevice(QString deviceName, bool deviceStatus, QVBoxLayout *layout, int count)
{
    if (!wiredSwitch->isChecked()) {
        return;
    }
    QDBusMessage result = m_interface->call(QStringLiteral("getWiredList"));
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "getWiredList error:" << result.errorMessage();
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
            QVector<QStringList> lanListInfo = iter.value();
            deviceLanlistInfo.deviceLayoutMap.insert(iter.key(),deviceFrame);
            if (lanListInfo.at(0).at(0) == "--") {
                for (int i = 1; i < lanListInfo.length(); i++) {
                    rebuildAvailComponent(deviceFrame, KLanSymbolic, iter.key(), lanListInfo.at(i).at(0), lanListInfo.at(i).at(1), false, WLAN_TYPE);
                }
            } else {
                rebuildAvailComponent(deviceFrame, KLanSymbolic, iter.key(), lanListInfo.at(0).at(0), lanListInfo.at(0).at(1), true, WLAN_TYPE);
                for (int i = 1; i < lanListInfo.length(); i++) {
                    rebuildAvailComponent(deviceFrame, KLanSymbolic, iter.key(), lanListInfo.at(i).at(0), lanListInfo.at(i).at(1), false, WLAN_TYPE);
                }
            }
            rebuildAddComponent(deviceFrame, iter.key());
        }
    }
}

void NetConnect::clearLayout(QVBoxLayout *layout)
{
    if (layout->layout() != NULL) {
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != NULL) {
            delete item->widget();
            delete item;
            item = nullptr;
        }
    }
}

void NetConnect::setSwitchStatus()
{
    if (QGSettings::isSchemaInstalled(GSETTINGS_SCHEMA)) {
        bool status = m_switchGsettings->get(WIRED_SWITCH).toBool();
        wiredSwitch->blockSignals(true);
        wiredSwitch->setChecked(status);
        wiredSwitch->blockSignals(false);
        if (!wiredSwitch->isChecked()) {
            clearLayout(ui->availableLayout);
        } else {
            initNet();
        }
    } else {

        qDebug()<<"[netconnect] org.ukui.kylin-nm.switch is not installed!";
    }

}

void NetConnect::initNet()
{
    clearLayout(ui->availableLayout);
    QMap<QString, bool>::iterator iter;
    int count = 1;
    for (iter = deviceListMap.begin(); iter != deviceListMap.end(); iter++) {
        getNetListFromDevice(iter.key(), iter.value(), ui->availableLayout, count);
        count ++;
    }
}

void NetConnect::netPropertiesChangeSlot(QMap<QString, QVariant> property) {
    if (property.keys().contains("WirelessEnabled")) {
        if (m_interface) {
            m_interface->call("requestRefreshWifiList");
        }
    }
}

void NetConnect::rebuildAddComponent(ItemFrame *frame, QString deviceName)
{
    connect(frame->addLanWidget, &AddBtn::clicked, this, [=]() {
        qDebug()<<"add lan connect";
    });

}

void NetConnect::rebuildDeviceComponent(ItemFrame *frame, QString deviceName, int count)
{
    frame->deviceFrame->deviceLabel->setText(tr("card")+QString("%1").arg(count)+"："+deviceName);
    QMap<QString, bool>::iterator iter;
    for (iter = deviceListMap.begin(); iter != deviceListMap.end(); iter++) {
        if (iter.key() == deviceName) {
            frame->deviceFrame->deviceSwitch->setChecked(iter.value());
        }
    }
    connect(frame->deviceFrame->deviceSwitch, &SwitchButton::checkedChanged, this, [=] (bool checked) {
        m_interface->call(QStringLiteral("setDeviceEnable"), deviceName, checked);
    });

    connect(frame->deviceFrame->dropDownLabel, &DrownLabel::labelClicked, this, [=] () {
        QMap<QString, bool>::iterator iters;
        for (iters =  dropDownMap.begin(); iters !=  dropDownMap.end(); iters++) {
            if (frame->deviceFrame->deviceLabel->text().contains(iters.key())) {
                iters.value() =!iters.value();
                frame->deviceFrame->dropDownLabel->setDropDownStatus(iters.value());
                qDebug()<<iters.key()<<iters.value();
            }
        }
        dropDownAnimation(frame->deviceFrame, deviceName, deviceListMap);
    });
}

void NetConnect::dropDownAnimation(DeviceFrame * deviceFrame, QString deviceName, QMap<QString, bool> deviceListMap)
{
    QMap<QString, bool>::iterator iters;
    for (iters =  dropDownMap.begin(); iters !=  dropDownMap.end(); iters++) {
        if (deviceFrame->deviceLabel->text().contains(iters.key())) {
            qDebug()<<deviceFrame->deviceLabel->text()<<"dropdown status:"<<iters.value();
            if (iters.value()) {
                QMap<QString, ItemFrame*>::iterator iter;
                for (iter =  deviceLanlistInfo.deviceLayoutMap.begin(); iter !=  deviceLanlistInfo.deviceLayoutMap.end(); iter++) {
                    if (iter.key() == deviceName) {
                        iter.value()->lanItemFrame->hide();
                    }
                }
            } else {
                QMap<QString, ItemFrame*>::iterator iter;
                for (iter =  deviceLanlistInfo.deviceLayoutMap.begin(); iter !=  deviceLanlistInfo.deviceLayoutMap.end(); iter++) {
                    if (iter.key() == deviceName) {
                        iter.value()->lanItemFrame->show();
                    }
                }
            }
        }
    }
}


void NetConnect::rebuildAvailComponent(ItemFrame *frame, QString iconPath, QString deviceName, QString name, QString ssid, bool status, int type) {
    qDebug()<<name<<ssid;
    LanItem * lanItem = new LanItem(pluginWidget);
    QIcon searchIcon = QIcon::fromTheme(iconPath);
    if (iconPath != KLanSymbolic && iconPath != NoNetSymbolic) {
        lanItem->iconLabel->setProperty("useIconHighlightEffect", 0x10);
    }
    lanItem->iconLabel->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(24, 24))));
    lanItem->titileLabel->setText(name);
    if (status) {
        lanItem->statusLabel->setText(tr("connected"));
    } else {
        lanItem->statusLabel->setText(tr("no connected"));
    }

    connect(lanItem->infoLabel, &InfoButton::clicked, this, [=]{
        // open landetail page
        m_interface->call(QStringLiteral("showPropertyWidget"), deviceName, ssid);
    });

    connect(lanItem, &QPushButton::clicked, this, [=] {
        if (status) {
            deActiveConnect(name, deviceName, type);
        } else {
            activeConnect(name, deviceName, type);
        }
    });
    deviceLanlistInfo.lanItemMap.insert(ssid,lanItem);
    frame->lanItemLayout->addWidget(lanItem);
}

void NetConnect::runExternalApp() {
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}

void NetConnect::activeConnect(QString netName, QString deviceName, int type) {
    m_interface->call("activateConnect",type, deviceName, netName);
}

void NetConnect::deActiveConnect(QString netName, QString deviceName, int type) {
    m_interface->call("deActivateConnect",type, deviceName, netName);
}

