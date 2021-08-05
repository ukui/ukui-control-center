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
#define CONTROL_CENTER_WIFI              "org.ukui.control-center.wifi.switch"
#define THEME_QT_SCHEMA                  "org.ukui.style"
#define MODE_QT_KEY                      "style-name"

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
        delete qtSettings;
    }
    delete m_interface;
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
        m_interface = new QDBusInterface("com.kylin.network", "/com/kylin/network",
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
    //~ contents_path /netconnect/Network settings"
    ui->detailBtn->setText(tr("Network settings"));
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
    initHoverWidget();
    //添加有线网络按钮
    connect(addLanWidget, &HoverWidget::widgetClicked, this, [=](){

    });

    ui->verticalLayout_4->addWidget(addLanWidget);
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
    connect(m_interface,SIGNAL(actWiredConnectionChanged()), this, SLOT(getNetList()));
    connect(ui->detailBtn, &QPushButton::clicked, this, [=](bool checked) {
        Q_UNUSED(checked)
        runExternalApp();
    });
    getNetList();
}

void NetConnect::initHoverWidget() {
    const QByteArray idd(THEME_QT_SCHEMA);
    if  (QGSettings::isSchemaInstalled(idd)){
        qtSettings = new QGSettings(idd);
    }
    addLanWidget = new HoverWidget;
    addLanWidget->setObjectName("addLanWidget");
    addLanWidget->setMinimumSize(QSize(550, 60));
    addLanWidget->setMaximumSize(QSize(16777215, 60));
    addLanWidget->setStyleSheet("HoverWidget#addLanWidget{background: palette(base); border-radius: 4px;}"
                                "HoverWidget:hover:!pressed#addLanWidget{background: palette(button); border-radius: 4px;}");

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel * iconLabel = new QLabel;

    //~ contents_path /netconnect/Add Wired Network
    QLabel * addWiredNetLabel = new QLabel(tr("Add Wired Network"));

    QPixmap pixgray = ImageUtil::loadSvg(":/img/plugins/netconnect/add.svg", "black", 15);
    iconLabel->setPixmap(pixgray);
    addLyt->addStretch();
    addLyt->addWidget(iconLabel);
    addLyt->addItem(new QSpacerItem(15,10,QSizePolicy::Fixed));
    addLyt->addWidget(addWiredNetLabel);
    addLyt->addStretch();
    addLanWidget->setLayout(addLyt);

    //图标跟随主题变化
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
        QPixmap pixgray = ImageUtil::loadSvg(":/img/plugins/netconnect/add.svg", "black", 15);
        iconLabel->setPixmap(pixgray);
    } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
        QPixmap pixgray = ImageUtil::loadSvg(":/img/plugins/netconnect/add.svg", "white", 15);
        iconLabel->setPixmap(pixgray);
    }
    connect(qtSettings,&QGSettings::changed,this,[=](const QString &key){
        if (key == "styleName") {
            QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
                QPixmap pixgray = ImageUtil::loadSvg(":/img/plugins/netconnect/add.svg", "black", 15);
                iconLabel->setPixmap(pixgray);
            } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
                QPixmap pixgray = ImageUtil::loadSvg(":/img/plugins/netconnect/add.svg", "white", 15);
                iconLabel->setPixmap(pixgray);
            }
        }
    });
    // 还原状态
    connect(addLanWidget, &HoverWidget::leaveWidget, this, [=](){
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
            QPixmap pixgray = ImageUtil::loadSvg(":/img/plugins/netconnect/add.svg", "black", 15);
            iconLabel->setPixmap(pixgray);
        } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
            QPixmap pixgray = ImageUtil::loadSvg(":/img/plugins/netconnect/add.svg", "white", 15);
            iconLabel->setPixmap(pixgray);
        }
    });
    // 悬浮改变Widget状态
    connect(addLanWidget, &HoverWidget::enterWidget, this, [=](){
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
            QPixmap pixgray = ImageUtil::loadSvg(":/img/plugins/netconnect/add.svg", "black", 15);
            iconLabel->setPixmap(pixgray);
        } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
            QPixmap pixgray = ImageUtil::loadSvg(":/img/plugins/netconnect/add.svg", "white", 15);
            iconLabel->setPixmap(pixgray);
        }
    });
}

void NetConnect::rebuildNetStatusComponent(QString iconPath, QMap<QString, bool> netNameMap) {
    bool hasNet = false;
    QMap<QString,bool>::ConstIterator iter = netNameMap.constBegin();
    while(iter != netNameMap.constEnd())
    {
        QVBoxLayout * vLayout = new QVBoxLayout;
        vLayout->setContentsMargins(0,0,0,0);

        QWidget *frame = new QWidget;
        frame->setContentsMargins(0,0,0,0);
        if (iter.key() == "无连接" || iter.key() == "No net") {
            hasNet = true;
        }
        HoverBtn * deviceItem;
        if (!hasNet) {
            deviceItem = new HoverBtn(iter.key(), false, pluginWidget);
        } else {
            deviceItem = new HoverBtn(iter.key(), true, pluginWidget);
        }
        deviceItem->mPitLabel->setText(iter.key());

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

        deviceItem->installEventFilter(this);
        vLayout->addWidget(deviceItem);
        frame->setLayout(vLayout);
        ui->detailLayOut->addWidget(frame);
        ++iter;
    }
}

void NetConnect::getNetList() {
    this->TlanList.clear();
    this->TlanList  = execGetLanList();
    getWifiListDone(this->TlanList);
    for (int i = 0; i < this->lanList.length(); i++) {
        rebuildAvailComponent(KLanSymbolic , lanList.at(i), "ethernet");
    }
}

void NetConnect::netPropertiesChangeSlot(QMap<QString, QVariant> property) {
    if (property.keys().contains("WirelessEnabled")) {
        if (m_interface) {
            m_interface->call("requestRefreshWifiList");
        }
    }
}

void NetConnect::rebuildAvailComponent(QString iconPath, QString netName, QString type) {
    HoverBtn * lanItem = new HoverBtn(netName, false, pluginWidget);
    lanItem->mPitLabel->setText(netName);

    QIcon searchIcon = QIcon::fromTheme(iconPath);
    if (iconPath != KLanSymbolic && iconPath != NoNetSymbolic) {
        lanItem->mPitIcon->setProperty("useIconHighlightEffect", 0x10);
    }
    lanItem->mPitIcon->setPixmap(searchIcon.pixmap(searchIcon.actualSize(QSize(24, 24))));
    lanItem->mAbtBtn->setMinimumWidth(100);
    lanItem->mAbtBtn->setText(tr("Connect"));
    lanItem->installEventFilter(this);
    connect(lanItem->mAbtBtn, &QPushButton::clicked, this, [=] {
        runKylinmApp(netName,type);
    });

    ui->availableLayout->addWidget(lanItem);
}

void NetConnect::runExternalApp() {
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}

void NetConnect::runKylinmApp(QString netName, QString type) {
    m_interface->call("showPb",type,netName);
}

bool NetConnect::getwifiisEnable() {
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

QStringList NetConnect::execGetLanList() {
    QProcess *lanPro = new QProcess(this);
    QString shellOutput = "";
    lanPro->start("nmcli -f type,device,name connection show");
    lanPro->waitForFinished();
    QString output = lanPro->readAll();
    shellOutput += output;
    QStringList slist = shellOutput.split("\n");

    return slist;
}

void NetConnect::getWifiListDone(QStringList getlanList) {
    clearContent();
    mActiveInfo.clear();
    getActiveConInfo(mActiveInfo);
    if (!getlanList.isEmpty()) {
        lanList.clear();

        int indexLan = 0;
        while (indexLan < mActiveInfo.size()) {
            if (mActiveInfo[indexLan].strConType == "ethernet"
                    || mActiveInfo[indexLan].strConType == "802-3-ethernet"){
                actLanNames.insert(mActiveInfo[indexLan].strConName, false);
            }
            indexLan ++;
        }


        // 填充可用网络列表
        QString headLine = getlanList.at(0);
        int indexDevice, indexName;
        headLine = headLine.trimmed();

        bool isChineseExist = headLine.contains(QRegExp("[\\x4e00-\\x9fa5]+"));
        if (isChineseExist) {
            indexDevice = headLine.indexOf("设备") + 2;
            indexName = headLine.indexOf("名称") + 4;
        } else {
            indexDevice = headLine.indexOf("DEVICE");
            indexName = headLine.indexOf("NAME");
        }

        for (int i =1 ;i < getlanList.length(); i++) {
            QString line = getlanList.at(i);
            QString ltype = line.mid(0, indexDevice).trimmed();
            QString nname = line.mid(indexName).trimmed();
            if (ltype != "wifi" && ltype != "" && ltype != "--") {
                this->lanList << nname;
            }
        }
    }
    if (!this->actLanNames.isEmpty()) {
        QMap<QString,bool>::ConstIterator iter = actLanNames.constBegin();
        while(iter != actLanNames.constEnd())
        {
            QString actLanName = iter.key();
            this->lanList.removeOne(actLanName);
            ++iter;
        }
    }

    if (!this->actLanNames.isEmpty()) {
        QString lanIconamePah = KLanSymbolic;
        rebuildNetStatusComponent(lanIconamePah, this->actLanNames);
    }
    if (this->actLanNames.isEmpty()) {
        noneAct.insert(tr("No net"),false);
        rebuildNetStatusComponent(NoNetSymbolic , this->noneAct);
    }
}

bool NetConnect::getInitStatus() {

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

void NetConnect::clearContent() {
    if (ui->availableLayout->layout() != NULL) {
        QLayoutItem* item;
        while ((item = ui->availableLayout->layout()->takeAt(0)) != NULL ) {
            delete item->widget();
            delete item;
            item = nullptr;
        }
    }

    if (ui->detailLayOut->layout() != NULL) {
        QLayoutItem* item;
        while ((item = ui->detailLayOut->layout()->takeAt(1)) != NULL) {
            delete item->widget();
            delete item;
            item = nullptr;
        }
    }

    this->actLanNames.clear();
    this->lanList.clear();
    this->TlanList.clear();
    this->noneAct.clear();

}

void NetConnect::wifiSwitchSlot(bool status) {

    QString wifiStatus = status ? "on" : "off";
    QString program = "nmcli";
    QStringList arg;
    arg << "radio" << "wifi" << wifiStatus;
    QProcess *nmcliCmd = new QProcess(this);
    nmcliCmd->start(program, arg);
    nmcliCmd->waitForFinished();
}

void NetConnect::getActiveConInfo(QList<ActiveConInfo>& qlActiveConInfo) {
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

