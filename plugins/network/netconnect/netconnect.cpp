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

#include "kylin_network_interface.h"


#include <QGSettings>
#include <QProcess>
#include <QTimer>
#include <QDBusReply>
#include <QDir>
#include <QDebug>
#include <QPair>
#include <QtAlgorithms>
#include <QMap>
#include "netlistitem.h"

#define ITEMHEIGH 50

#define CONTROL_CENTER_WIFI "org.ukui.control-center.wifi.switch"
#define KYLIN_NM_SCHEMA "org.kylinnm.settings"
#define ACTIVATE_WIFI "wifiname"
#define LAN_CONNECTED "lanconn"

bool sortByVal(const QPair<QPair<QString, bool>, int> &l, const QPair<QPair<QString, bool>, int> &r) {
    return (l.second < r.second);
}

NetConnect::NetConnect():m_wifiList(new Wifi),mFirstLoad(true)
{
    pluginName = tr("Network");
    pluginType = NETWORK;
}

NetConnect::~NetConnect()
{
    if (!mFirstLoad) {
        delete ui;
        delete m_gsettings;
        if (m_nmgsettings) {
            delete m_nmgsettings;
            m_nmgsettings = NULL;
        }
        if (m_kylinnm_interface) {
            delete m_kylinnm_interface;
            m_kylinnm_interface = NULL;
        }
    }
}

QString NetConnect::get_plugin_name(){
    return pluginName;
}

int NetConnect::get_plugin_type(){
    return pluginType;
}

QWidget *NetConnect::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::NetConnect;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
        ui->title2Label->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");

        wifiBtn = new SwitchButton(pluginWidget);
        pNetWorker = new NetconnectWork;

        ui->openWIifLayout->addWidget(wifiBtn);
        ui->openWifiFrame->setAutoFillBackground(false);
        ui->lanLayout->setSpacing(0);

        initSearchText();
        initComponent();
        m_kylinnm_interface = new QDBusInterface("org.ukui.KylinNM",
                                                       "/MainWindow",
                                                       "org.ukui.KylinNM",
                                                       QDBusConnection::sessionBus(),
                                                       NULL);
        if (!m_kylinnm_interface->isValid()) {
            qCritical() << "Create DBus Interface Failed: " << QDBusConnection::sessionBus().lastError();
        }
    }
    return pluginWidget;
}

void NetConnect::plugin_delay_control(){

}

//把判断列表是否已刷新的bool值is_refreshed重置为true
void NetConnect::reset_bool_is_refreshed(){
    this->is_refreshed = true;
}

void NetConnect::properties_changed_refresh(){
    if (!handle_open_wifi_switch) {
        //手动打开wifi开关时，已刷新，不需要再次执行getNetList
        getNetList();
    }
    /*等待一段时间后把is_refreshed重置，等待是为了避免在dbus接收属性更改时收到
    多条信号并连续执行槽函数refreshed_signal_changed()并更改is_refreshed导致冲突*/
    QTimer::singleShot(1000, this, SLOT(reset_bool_is_refreshed()));
}
const QString NetConnect::name() const {

    return QStringLiteral("netconnect");
}

void NetConnect::initSearchText() {
    //~ contents_path /netconnect/LAN
    ui->titleLabel->setText(tr("LAN"));
    //~ contents_path /netconnect/WIFI
    ui->title2Label->setText(tr("WIFI"));
    //~ contents_path /netconnect/open wifi
    ui->openLabel->setText(tr("open wifi"));
}

void NetConnect::initComponent(){

    ui->RefreshBtn->hide();

    //把判断列表是否已刷新的bool值初始化为true
    this->is_refreshed = true;

    //接收到系统创建网络连接的信号时刷新可用网络列表
    QDBusConnection::systemBus().connect(QString(), QString("/org/freedesktop/NetworkManager/Settings"), "org.freedesktop.NetworkManager.Settings", "NewConnection", this, SLOT(getNetList(void)));
    //接收到系统删除网络连接的信号时刷新可用网络列表
    QDBusConnection::systemBus().connect(QString(), QString("/org/freedesktop/NetworkManager/Settings"), "org.freedesktop.NetworkManager.Settings", "ConnectionRemoved", this, SLOT(getNetList(void)));
    //接收到系统更改网络连接属性的信号时刷新可用网络列表
//    QDBusConnection::systemBus().connect(QString(), QString("/org/freedesktop/NetworkManager"), "org.freedesktop.NetworkManager", "PropertiesChanged", this, SLOT(getNetList(void)));

    const QByteArray id(CONTROL_CENTER_WIFI);
    if(QGSettings::isSchemaInstalled(id)) {
//        qDebug()<<"isSchemaInstalled"<<endl;
        m_gsettings = new QGSettings(id);

        //  监听key的value是否发生了变化
        connect(m_gsettings, &QGSettings::changed, this, [=] (const QString &key) {
//            qDebug()<<"status changed ------------>"<<endl;
            if (key == "switchor") {
                bool judge = getSwitchStatus(key);
                wifiBtn->setChecked(judge);
            }
        });
        QGSettings * wifiSwitch =  new QGSettings(CONTROL_CENTER_WIFI, QByteArray(),this);
//        bool is_wifi_open = wifiSwitch->get("switch").toBool();
//        if(is_wifi_open){
//            wifiBtn->setChecked(true);
//        } else {
//            wifiBtn->setChecked(false);
//        }

        connect(wifiSwitch, &QGSettings::changed, this, [=](){
            bool is_wifi_open = wifiSwitch->get("switch").toBool();
            if(is_wifi_open){
                wifiBtn->setChecked(true);
            } else {
                wifiBtn->setChecked(false);
            }
        });
    }
    const QByteArray iid(KYLIN_NM_SCHEMA);
    if(QGSettings::isSchemaInstalled(iid)) {
        m_nmgsettings = new QGSettings(iid);
        if (m_nmgsettings->keys().contains(ACTIVATE_WIFI)) {
            m_activateWifi = m_nmgsettings->get(ACTIVATE_WIFI).toString();
            connect(m_nmgsettings, &QGSettings::changed, this, [ = ](const QString &key) {
                if (QString::compare(key, ACTIVATE_WIFI) == 0) {
                    m_activateWifi = m_nmgsettings->get(ACTIVATE_WIFI).toString();
                    if (wifiBtn->isChecked()) //当wifi开关未打开时，wifi列表不显示，就不必去刷新列表了
                        wifiChangedSlot(m_activateWifi);
                } else if (QString::compare(key, LAN_CONNECTED) == 0) {
                    getNetList();
                }
            });
        }
    }

    connect(ui->RefreshBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
//        clearContent();
//        ui->waitLabel->setVisible(true);
//        ui->statuswaitLabel->setVisible(true);
//        ui->RefreshBtn->setEnabled(false);
        wifiBtn->setEnabled(false);
        QTimer::singleShot(1*1000,this,SLOT(getNetList()));
    });

    if (getwifiisEnable()) {
        wifiBtn->setChecked(getInitStatus());
    }
    connect(wifiBtn, &SwitchButton::checkedChanged, this,[=](bool checked){
        handle_open_wifi_switch = true;
//        clearContent();
//        ui->waitLabel->setVisible(true);
//        ui->statuswaitLabel->setVisible(true);
        ui->RefreshBtn->setEnabled(false);
        wifiBtn->setEnabled(false);

        wifiBtn->blockSignals(true);
        wifiSwitchSlot(checked);
        wifiBtn->blockSignals(false);
        getNetList();
    });

    ui->RefreshBtn->setEnabled(false);
    wifiBtn->setEnabled(false);

    emit ui->RefreshBtn->clicked(true);
    ui->verticalLayout_2->setContentsMargins(0,0,9,0);
}

void NetConnect::rebuildNetStatusComponent(QString iconPath, QString netName){
    ////构建Widget
    NetListItem * baseWidget = new NetListItem(iconPath, netName);
    baseWidget->setObjectName(QString("Lan%1").arg(netName));
    baseWidget->installEventFilter(this);

    ui->lanLayout->addWidget(baseWidget);
}

void NetConnect::getNetList() {
    bool wifiSt = getwifiisEnable();
    if (!wifiSt) {
        wifiBtn->setChecked(wifiSt);
    }
    wifiBtn->setEnabled(wifiSt);

    this->TlanList =  execGetLanList();
//    pThread = new QThread;
//    pNetWorker = new NetconnectWork;

    connect(pNetWorker, &NetconnectWork::wifiGerneral,this,[&](QStringList list){
        handle_open_wifi_switch = false;
        this->TwifiList = list;
        getWifiListDone(this->TwifiList, this->TlanList);
        QMap<QPair<QString, bool>, int>::iterator iter = this->wifiList.begin();
        QVector<QPair<QPair<QString, bool>, int>> vec;
        QString iconamePah;

        while(iter != this->wifiList.end()) {

            vec.push_back(qMakePair(iter.key(), iter.value()));
            iter++;
        }
        qSort(vec.begin(), vec.end(), sortByVal);
        for (int i = 0; i < vec.size(); i++) {
            if (!wifiBtn->isChecked()){
                break;
            }
            if (QString::compare(m_activateWifi, vec[i].first.first) == 0) {
                if (vec[i].first.second) { //加密wifi
                    iconamePah= ":/img/plugins/netconnect/wifionpwd" + QString::number(vec[i].second)+".svg";
                } else { //非加密wifi
                    iconamePah= ":/img/plugins/netconnect/wifion" + QString::number(vec[i].second)+".png";
                }
            } else {
                if (vec[i].first.second) { //加密wifi
                    iconamePah= ":/img/plugins/netconnect/wifipwd" + QString::number(vec[i].second)+".svg";
                } else { //非加密wifi
                    iconamePah= ":/img/plugins/netconnect/wifi" + QString::number(vec[i].second)+".png";
                }
            }
            rebuildAvailComponent(iconamePah , vec[i].first.first);
        }

        for(int i = 0; i < this->lanList.length(); i++) {
            if (!this->actLanName.isEmpty() && lanList.at(i) == this->actLanName) {
               iconamePah =  ":/img/plugins/netconnect/pb-network-online.png";
            } else {
//                iconamePah = ":/img/plugins/netconnect/pb-network-offline.png";
                iconamePah = ":/img/plugins/netconnect/pb-top-network-offline.png";
            }
            rebuildNetStatusComponent(iconamePah , lanList.at(i));
        }
        if (lanList.length() == 0) {
            rebuildNetStatusComponent(":/img/plugins/netconnect/pb-top-network-offline.png" , "No Net");
            ui->frame_2->setFixedHeight(56);
        } else {
            ui->frame_2->setFixedHeight(56 * lanList.length() + lanList.length() - 1);
        }

    });
    connect(pNetWorker, &NetconnectWork::workerComplete,this, [=]{
       pNetWorker->quit();
       pNetWorker->wait();
    });
//    pNetWorker->moveToThread(pThread);
//    connect(pThread, &QThread::started, pNetWorker, &NetconnectWork::run);
//    connect(pThread, &QThread::started, this, [=](){

//    });
    connect(pNetWorker, &QThread::finished, this, [=]{
        bool wifiSt = getwifiisEnable();
        wifiBtn->setEnabled(wifiSt);
        ui->RefreshBtn->setEnabled(true);

//        ui->waitLabel->setVisible(false);
//        ui->statuswaitLabel->setVisible(false);
    });
//    connect(pNetWorker, &QThread::finished, pNetWorker, &NetconnectWork::deleteLater);
//    pThread->start();
    pNetWorker->is_wifi_open = this->wifiBtn->isChecked();
    pNetWorker->start();
    //    qDebug() << "getNetList end" <<endl;
}

/**
 * @brief NetConnect::wifiChangedSlot 当wifi开关已开启且已连接wifi发生变化时，执行此槽函数
 * @param active_wifi 当前激活的wifi名，可能为空，为空时指没有wifi被连接
 */
void NetConnect::wifiChangedSlot(const QString &active_wifi)
{
//    if (!active_wifi.isEmpty() || active_wifi != "") {
    //连接了新的wifi，找到此wifi并置顶
    foreach(NetListItem* item, ui->frame->findChildren<NetListItem*>()) {
//            qDebug() << item->objectName() << item->isActive();
        QString obj_name = QString("Wifi%1").arg(active_wifi);
        if (item->isActive() && item->objectName() != obj_name) {
            item->setActive(false);
        } else if (!item->isActive() && item->objectName() == obj_name) {
            ui->wifiLayout->insertWidget(0, item);
            item->setActive(true);
        }
    }
//    }
}

void NetConnect::rebuildAvailComponent(QString iconPath, QString netName){

    ////构建Widget
    NetListItem * item = new NetListItem(iconPath, netName, ui->frame);
    item->setObjectName(QString("Wifi%1").arg(netName));
    item->installEventFilter(this);
    if (QString::compare(netName, m_activateWifi) == 0) {
        ui->wifiLayout->insertWidget(0, item);
    } else {
        ui->wifiLayout->addWidget(item);
    }
}

/**
 * @brief NetConnect::runExternalApp 打开Kylin-nm主界面
 * @param type 打开的界面类型，0为wifi页，1为lan页
 * @param netname 点击的网络名称
 */
void NetConnect::runExternalApp(const int& type, const QString& netname){
    if (!m_kylinnm_interface || !m_kylinnm_interface->isValid()) return;
    if (type) {
        QDBusMessage msg = m_kylinnm_interface->call(QStringLiteral("ShowPb"),true,"lan", netname);
    } else {
        QDBusMessage msg = m_kylinnm_interface->call(QStringLiteral("ShowPb"),true,"wifi", netname);
    }
}

bool NetConnect::getwifiisEnable()
{
    QDBusInterface m_interface( "org.freedesktop.NetworkManager",
                                "/org/freedesktop/NetworkManager",
                                "org.freedesktop.NetworkManager",
                                QDBusConnection::systemBus() );

    QDBusReply<QList<QDBusObjectPath>> obj_reply = m_interface.call("GetAllDevices");
    if (!obj_reply.isValid()) {
        qDebug()<<"execute dbus method 'GetAllDevices' is invalid in func getObjectPath()";
    }

    QList<QDBusObjectPath> obj_paths = obj_reply.value();

    foreach (QDBusObjectPath obj_path, obj_paths){
        QDBusInterface interface( "org.freedesktop.NetworkManager",
                                  obj_path.path(),
                                  "org.freedesktop.DBus.Introspectable",
                                  QDBusConnection::systemBus() );

        QDBusReply<QString> reply = interface.call("Introspect");
        if (!reply.isValid()) {
            qDebug()<<"execute dbus method 'Introspect' is invalid in func getObjectPath()";
        }

        if(reply.value().indexOf("org.freedesktop.NetworkManager.Device.Wired") != -1){
        } else if (reply.value().indexOf("org.freedesktop.NetworkManager.Device.Wireless") != -1){
            return true;
        }
    }
    return false ;
}

bool NetConnect::eventFilter(QObject *w, QEvent *e)
{
    if (e->type() == QEvent::MouseButtonRelease) {
        if (w->objectName().startsWith("Lan")) {
            runExternalApp(1, w->objectName().mid(3));
        } else runExternalApp(0, w->objectName().mid(4));
    } else if (e->type() == QEvent::Enter) {
        if (w->findChild<QWidget*>())
            w->findChild<QWidget*>()->setStyleSheet("QWidget{background: palette(button);border-radius:4px;}");
    } else if (e->type() == QEvent::Leave) {
        if (w->findChild<QWidget*>())
            w->findChild<QWidget*>()->setStyleSheet("QWidget{background: palette(base);}");
    }
    return QObject::eventFilter(w,e);
}


QStringList NetConnect::execGetLanList(){
    QProcess *lanPro = new QProcess();
    QString shellOutput = "";
    lanPro->start("nmcli -f type,device,name connection show");
    lanPro->waitForFinished();
    QString output = lanPro->readAll();
    shellOutput += output;
    QStringList slist = shellOutput.split("\n");

//    qDebug()<<"lanslist--------------->"<<slist<<endl;
    return slist;
}

void NetConnect::getWifiListDone(QStringList getwifislist, QStringList getlanList) {
//    qDebug()<<"getwifiListDone--------->"<<getwifislist<<" \n" <<getlanList<<endl;
    clearContent();

    QString lockPath = QDir::homePath() + "/.config/control-center-net";
    activecon *act = kylin_network_get_activecon_info(lockPath.toUtf8().data());

    //if is wifi list
    if(!getwifislist.isEmpty()){
        connectedWifi.clear();
        wifiList.clear();

        QString actWifiName = "--";


        int index = 0;
        while(act[index].con_name != NULL){
            if(QString(act[index].type) == "wifi" ||QString(act[index].type) == "802-11-wireless"){
                actWifiName = QString(act[index].con_name);
                break;
            }
            index ++;
        }
//        qDebug()<<"now wifi is----->"<<actWifiName<<endl;

        // 填充可用网络列表
        QString headLine = getwifislist.at(0);
        headLine = headLine.trimmed();
        int indexRate = headLine.indexOf("SIGNAL");
        int indexName = headLine.indexOf("SSID");
        int indexSecurity = headLine.indexOf("SECURITY");

        QStringList wnames;
        int count = 0;
        for(int i = 1; i < getwifislist.size(); i ++) {
            QString line = getwifislist.at(i);
            QString wsignal = line.mid(0, indexName).trimmed();
            QString wname = line.mid(indexName, indexSecurity - indexName).trimmed();
            QString wsecurity = line.mid(indexSecurity).trimmed();

            bool isContinue = false;
            foreach (QString addName, wnames) {
                // 重复的网络名称，跳过不处理
                if(addName == wname){ isContinue = true; }
            }
            if(isContinue){ continue; }

            if(wname != "" && wname != "--"){
                int strength = this->setSignal(wsignal);
                QPair<QString, bool> pair;
                pair.first = wname;
//                pair.second = true;
                if (wsecurity == "--" || wsecurity == "") {
                    pair.second = false;
                } else {
                    pair.second = true;
                }
//                wifiList.insert(wname, strength);
                wifiList.insert(pair, strength);

                if(wname == actWifiName) {
                    connectedWifi.insert(wname, strength);
                }
                wnames.append(wname);
            }
        }
    }
    if(getlanList.isEmpty()){
        getlanList = execGetLanList();
    }
    if(!getlanList.isEmpty()){
        lanList.clear();
        connectedLan.clear();

        int indexLan = 0;
        while(act[indexLan].con_name != NULL){
            if (QString(act[indexLan].type) == "ethernet" || QString(act[indexLan].type) == "802-3-ethernet"){
                actLanName = QString(act[indexLan].con_name);
                break;
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
        for(int i =1 ;i < getlanList.length(); i++)
        {
            QString line = getlanList.at(i);
            QString ltype = line.mid(0, indexDevice).trimmed();
            QString nname = line.mid(indexName).trimmed();
            if(ltype  != "wifi" && ltype != "" && ltype != "--"){
                this->lanList << nname;
            }
        }
    } else {
        lanList.clear();
        connectedLan.clear();
        this->actLanName.clear();
        rebuildNetStatusComponent(":/img/plugins/netconnect/pb-top-network-offline.png" , "No Net");
        ui->frame_2->setFixedHeight(56);
    }
}

bool NetConnect::getSwitchStatus(QString key){
//    qDebug()<<"key is------------->"<<key<<endl;
    if (!m_gsettings) {
        return true;
    }
    const QStringList list = m_gsettings->keys();
    if (!list.contains(key)) {
        return true;
    }
    bool res = m_gsettings->get(key).toBool();
    return res;
}

bool NetConnect::getInitStatus()
{

    QDBusInterface interface( "org.freedesktop.NetworkManager",
                              "/org/freedesktop/NetworkManager",
                              "org.freedesktop.DBus.Properties",
                              QDBusConnection::systemBus() );
    //获取当前wifi是否连接
    QDBusReply<QVariant> m_result = interface.call("Get", "org.freedesktop.NetworkManager", "WirelessEnabled");

    if (m_result.isValid()){
        bool status = m_result.value().toBool();
        return status;
    } else {
        qDebug()<<"org.freedesktop.NetworkManager get invalid"<<endl;
        return false;
    }
}

void NetConnect::clearContent()
{
    if (ui->wifiLayout->layout() != NULL) {
        QLayoutItem* item;
        while ((item = ui->wifiLayout->layout()->takeAt( 0 )) != NULL )
        {
            delete item->widget();
            delete item;
        }
//        delete ui->availableLayout->layout();
    }

    if (ui->lanLayout->layout() != NULL) {
        QLayoutItem* item;
        while ((item = ui->lanLayout->layout()->takeAt( 0 )) != NULL )
        {
            delete item->widget();
            delete item;
        }
//        delete ui->availableLayout->layout();
    }
//    ui->statusListWidget->clear();

    this->connectedLan.clear();
    this->connectedWifi.clear();
    this->actLanName.clear();
    this->wifiList.clear();
    this->lanList.clear();
    this->TlanList.clear();
    this->TwifiList.clear();
}

//get wifi's strength
int NetConnect::setSignal(QString lv) {
    int signal = lv.toInt();
//    qDebug()<<"signal is---------->"<<lv<<endl;
    int signalLv;

    if(signal > 75){
        signalLv = 1;
    }
    if(signal > 55 && signal <= 75){
        signalLv = 2;
    }
    if(signal > 35 && signal <= 55){
        signalLv = 3;
    }
    if(signal > 15 && signal <= 35){
        signalLv = 4;
    }
    if(signal <= 15){
        signalLv = 4;
    }

    return signalLv;
}

void NetConnect::wifiSwitchSlot(bool signal){
    if(!m_gsettings) {
        return ;
    }
    const QStringList list = m_gsettings->keys();
    if (!list.contains("switch")) {
        return ;
    }
    m_gsettings->set("switch",signal);
    if (!list.contains("switchor")) {
        return ;
    }
    m_gsettings->set("switchor",signal);

//    QTimer::singleShot(2*1000,this,SLOT(getNetList()));
}
