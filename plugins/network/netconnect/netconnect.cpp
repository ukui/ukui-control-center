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
    connect(m_interface, SIGNAL(lanActiveConnectionStateChanged(QString, QString, int)), this, SLOT(updateOneLanFrame(QString, QString, int)));
    //有线网络新增时添加网络
    connect(m_interface, SIGNAL(lanAdd(QString, QStringList)), this, SLOT(updateOneLanFrame(QString, QStringList)));
    //删除有线网络
    connect(m_interface, SIGNAL(lanRemove(QString)), this, SLOT(updateOneLanFrame(QString)));
    //删除有线网络
    connect(m_interface, SIGNAL(lanUpdate(QString, QStringList)), this, SLOT(updateLanInfo(QString, QStringList)));

    connect(m_interface, SIGNAL(deviceStatusChanged()), this, SLOT(updateLanListWidget()));

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
    deviceListMap.clear();
    QDBusMessage result = m_interface->call(QStringLiteral("getDeviceListAndEnabled"),0);
    if(result.type() == QDBusMessage::ErrorMessage)
    {
        qWarning() << "getWiredDeviceList error:" << result.errorMessage();
    }
    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    dbusArg >> deviceListMap;
}

void NetConnect::updateLanInfo(QString deviceName, QStringList lanInfo)
{
    bool needMove = false; //是否更改网卡配置
    QMap<QString, QString>::iterator iterator;
    for (iterator = ssidDeviceMap.begin(); iterator != ssidDeviceMap.end(); iterator++) {
        if (lanInfo.at(1) == iterator.key()) {
            if (iterator.value() != deviceName) {
                needMove = true;
            } else {
                needMove = false;
            }
        }
    }
    if (needMove) {
        deleteOneLan(lanInfo.at(1));
        QMap<QString, ItemFrame *>::iterator iter;
        for (iter = deviceLanlistInfo.deviceLayoutMap.begin(); iter != deviceLanlistInfo.deviceLayoutMap.end(); iter++) {
            if (deviceName == iter.key()) {
                rebuildAvailComponent(iter.value(), KLanSymbolic, deviceName, lanInfo.at(0), lanInfo.at(1), lanInfo.at(2), false, WLAN_TYPE);
            }
        }
    } else {
        QMap<QString, LanItem*>::iterator iter;
        for (iter = deviceLanlistInfo.lanItemMap.begin(); iter != deviceLanlistInfo.lanItemMap.end(); iter++) {
            if (iter.key() == lanInfo.at(1)) {
                iter.value()->titileLabel->setText(lanInfo.at(0));
            }
        }
    }
}

void NetConnect::updateLanListWidget()
{
    qDebug()<<"网卡插拔处理";
    QEventLoop eventloop;
    QTimer::singleShot(300, &eventloop, SLOT(quit()));
    eventloop.exec();
    getDeviceList();
    initNet();
}

void NetConnect::updateOneLanFrame(QString deviceName, QString uuid, int status)
{
    if (!wiredSwitch->isChecked()) {
        return;
    }
    QMap<QString, LanItem*>::iterator iter;
    for (iter = deviceLanlistInfo.lanItemMap.begin(); iter != deviceLanlistInfo.lanItemMap.end(); iter++) {
        if(uuid == iter.key()) {
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
                iter.value()->statusLabel->setText(tr("no connected"));
                iter.value()->isAcitve = false;
            }
        }
    }
}

void NetConnect::updateOneLanFrame(QString devicePath)
{
    qDebug()<<"删除网络";
    QString ssid;
    QMap<QString, QString>::iterator iter;
    for (iter = pathSsidMap.begin(); iter != pathSsidMap.end(); iter++) {
        if (iter.value() == devicePath) {
            ssid = iter.key();
        }
    }
    deleteOneLan(ssid);
}

void NetConnect::deleteOneLan(QString ssid)
{
    QMap<QString, ItemFrame *>::iterator iters;
    for (iters = deviceLanlistInfo.deviceLayoutMap.begin(); iters != deviceLanlistInfo.deviceLayoutMap.end(); iters++) {
        qDebug()<<iters.key()<<iters.value();
        if (iters.value()->lanItemLayout->layout() != NULL) {
            LanItem* item;
            QMap<QString, LanItem*>::iterator iter;
            for (iter = deviceLanlistInfo.lanItemMap.begin(); iter != deviceLanlistInfo.lanItemMap.end(); iter++) {
                if (iter.key() == ssid) {
                    item = iter.value();
                }
            }
            iters.value()->lanItemLayout->removeWidget(item);
            deviceLanlistInfo.lanItemMap.erase(iter);
        }
    }
}
void NetConnect::updateOneLanFrame(QString deviceName, QStringList lanInfo)
{
    qDebug()<<"新增有线网络";
    if (deviceName == nullptr) {
        QMap<QString, ItemFrame *>::iterator iter;
        for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
            rebuildAvailComponent(iter.value(), KLanSymbolic, iter.key(), lanInfo.at(0), lanInfo.at(1), lanInfo.at(2), false, WLAN_TYPE);
        }
    } else {
        QMap<QString, ItemFrame *>::iterator iter;
        for (iter = deviceFrameMap.begin(); iter != deviceFrameMap.end(); iter++) {
            if (deviceName == iter.key()) {
                rebuildAvailComponent(iter.value(), KLanSymbolic, iter.key(), lanInfo.at(0), lanInfo.at(1), lanInfo.at(2), false, WLAN_TYPE);
            }
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
            qDebug()<<iter.key()<<lanListInfo<<"开关状态:"<<deviceStatus;
            if (deviceStatus) {
                deviceFrame->lanItemFrame->show();
            } else {
                deviceFrame->lanItemFrame->hide();
            }
            qDebug()<<"网卡列表"<<iter.value();
            if (lanListInfo.at(0).at(0) == "--") {
                for (int i = 1; i < lanListInfo.length(); i++) {
                    rebuildAvailComponent(deviceFrame, KLanSymbolic, iter.key(), lanListInfo.at(i).at(0), lanListInfo.at(i).at(1), lanListInfo.at(i).at(2), false, WLAN_TYPE);
                }
            } else {
                rebuildAvailComponent(deviceFrame, KLanSymbolic, iter.key(), lanListInfo.at(0).at(0), lanListInfo.at(0).at(1), lanListInfo.at(0).at(2), true, WLAN_TYPE);
                for (int i = 1; i < lanListInfo.length(); i++) {
                    if (lanListInfo.at(i).at(0) != "") {
                        rebuildAvailComponent(deviceFrame, KLanSymbolic, iter.key(), lanListInfo.at(i).at(0), lanListInfo.at(i).at(1), lanListInfo.at(i).at(2), false, WLAN_TYPE);
                    }
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
        m_interface->call("showCreateWiredConnectWidget",deviceName);
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
        if (checked) {
            frame->lanItemFrame->show();
        } else {
            frame->lanItemFrame->hide();
        }
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


void NetConnect::rebuildAvailComponent(ItemFrame *frame, QString iconPath, QString deviceName, QString name, QString ssid, QString path, bool status, int type) {
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
    lanItem->isAcitve = status;
    qDebug()<<name<<ssid<<deviceName;
    connect(lanItem, &QPushButton::clicked, this, [=] {
        if (lanItem->isAcitve || lanItem->loading) {
            deActiveConnect(ssid, deviceName, type);
            lanItem->isAcitve = !lanItem->isAcitve;
        } else {
            activeConnect(ssid, deviceName, type);
            lanItem->isAcitve = !lanItem->isAcitve;
        }
    });

    deviceLanlistInfo.lanItemMap.insert(ssid,lanItem);
    ssidDeviceMap.insert(ssid,deviceName);
    deviceFrameMap.insert(deviceName,frame);
    pathSsidMap.insert(ssid, path);
    frame->lanItemLayout->addWidget(lanItem);
}

void NetConnect::runExternalApp() {
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}

void NetConnect::activeConnect(QString ssid, QString deviceName, int type) {
    qDebug()<<"连接";
    m_interface->call(QStringLiteral("activateConnect"),type, deviceName, ssid);
}

void NetConnect::deActiveConnect(QString ssid, QString deviceName, int type) {
    qDebug()<<"断开";
    m_interface->call(QStringLiteral("deActivateConnect"),type, deviceName, ssid);
}

