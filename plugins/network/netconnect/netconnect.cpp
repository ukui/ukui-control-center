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

#include <QProcess>

NetConnect::NetConnect()
{
    ui = new Ui::NetConnect;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("netconnect");
    pluginType = NETWORK;

    pluginWidget->setStyleSheet("background: #ffffff;");


    ui->statusListWidget->setStyleSheet("QListWidget#statusListWidget{border: none;}");
    ui->availableListWidget->setStyleSheet("QListWidget#availableListWidget{border: none;}");

    ui->statusListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->statusListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->availableListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->availableListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->statusListWidget->setSpacing(0);
    ui->availableListWidget->setSpacing(0);
    ui->statusListWidget->setFocusPolicy(Qt::NoFocus);
    ui->availableListWidget->setFocusPolicy(Qt::NoFocus);
    ui->statusListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    ui->availableListWidget->setSelectionMode(QAbstractItemView::NoSelection);

    ui->detailBtn->setStyleSheet("QPushButton{border: none;}");

    //构建网络配置对象
    nmg  = new QNetworkConfigurationManager();

    initComponent();
}

NetConnect::~NetConnect()
{
    delete ui;
    delete nmg;
}

QString NetConnect::get_plugin_name(){
    return pluginName;
}

int NetConnect::get_plugin_type(){
    return pluginType;
}

QWidget *NetConnect::get_plugin_ui(){
    return pluginWidget;
}

void NetConnect::plugin_delay_control(){

}

void NetConnect::initComponent(){

    //构建网络状态组件
    rebuildNetStatusComponent();

    //网络配置变化回调
    connect(nmg, &QNetworkConfigurationManager::configurationChanged, this, [=](const QNetworkConfiguration &config){
        Q_UNUSED(config)
        rebuildNetStatusComponent();

    });


    //详细设置按钮connect
    connect(ui->detailBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        runExternalApp();
    });
}

void NetConnect::rebuildNetStatusComponent(){
    //获取网卡信息
    _acquireCardInfo();

    ui->statusListWidget->blockSignals(true);
    //清空Item
    ui->statusListWidget->clear();

    //初始化网络设备信息
    for (int num = 0; num < cardinfoQList.count(); num++){
        CardInfo current = cardinfoQList.at(num);
        QString iconPath;
        QString statusTip;
        if (current.type == ETHERNET)
            if (current.status){
                iconPath = "://netconnect/eth.png";
                statusTip = tr("Connect");
            }
            else{
                iconPath = "://netconnect/eth_disconnect.png";
                statusTip = tr("Disconnect");
            }
        else
            if (current.status){
                iconPath = "://netconnect/wifi.png";
                statusTip = tr("Connect");
            }
            else{
                iconPath = "://netconnect/wifi_disconnect.png";
                statusTip = tr("Disconnect");
            }

        ////构建Widget
        QWidget * baseWidget = new QWidget();
        baseWidget->setAttribute(Qt::WA_DeleteOnClose);

        QVBoxLayout * baseVerLayout = new QVBoxLayout(baseWidget);
        baseVerLayout->setSpacing(0);
        baseVerLayout->setContentsMargins(0, 0, 0, 2);

        QWidget * devWidget = new QWidget(baseWidget);
        devWidget->setMinimumWidth(550);
        devWidget->setMaximumWidth(960);
        devWidget->setMinimumHeight(50);
        devWidget->setMaximumHeight(50);

//        devWidget->setFixedHeight(50);
        devWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
        QHBoxLayout * devHorLayout = new QHBoxLayout(devWidget);
        devHorLayout->setSpacing(8);
        devHorLayout->setContentsMargins(16, 0, 0, 0);

        QLabel * iconLabel = new QLabel(devWidget);
        QSizePolicy iconSizePolicy = iconLabel->sizePolicy();
        iconSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
        iconSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        iconLabel->setSizePolicy(iconSizePolicy);
        iconLabel->setScaledContents(true);
        iconLabel->setPixmap(QPixmap(iconPath));

        QLabel * nameLabel = new QLabel(devWidget);
        QSizePolicy nameSizePolicy = nameLabel->sizePolicy();
        nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
        nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        nameLabel->setSizePolicy(nameSizePolicy);
        nameLabel->setScaledContents(true);
        nameLabel->setText(current.name);

        QLabel * statusLabel = new QLabel(devWidget);
        QSizePolicy statusSizePolicy = statusLabel->sizePolicy();
        statusSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
        statusSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        statusLabel->setSizePolicy(statusSizePolicy);
        statusLabel->setScaledContents(true);
        statusLabel->setText(statusTip);

        devHorLayout->addWidget(iconLabel);
        devHorLayout->addWidget(nameLabel);
        devHorLayout->addWidget(statusLabel);
        devHorLayout->addStretch();

        devWidget->setLayout(devHorLayout);

        baseVerLayout->addWidget(devWidget);
        baseVerLayout->addStretch();

        baseWidget->setLayout(baseVerLayout);

        QListWidgetItem * item = new QListWidgetItem(ui->statusListWidget);
        item->setSizeHint(QSize(502, 52));

        ui->statusListWidget->setItemWidget(item, baseWidget);

    }

    ui->statusListWidget->blockSignals(false);
}

void NetConnect::_acquireCardInfo(){
    QList<QNetworkInterface> network = QNetworkInterface::allInterfaces();
    for (QList<QNetworkInterface>::const_iterator it = network.constBegin(); it != network.constEnd(); it++){
        CardInfo ci;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
        if ((*it).type() == QNetworkInterface::Loopback)
            continue;

        if ((*it).type() == QNetworkInterface::Ethernet)
            ci.type = ETHERNET;
        else if ((*it).type() == QNetworkInterface::Wifi)
            ci.type = WIFI;
#else
        if ((*it).flags().testFlag(QNetworkInterface::IsLoopBack))
            continue;
       ci.type =ETHERNET;
#endif
        ci.name = (*it).humanReadableName();

        QList<QNetworkAddressEntry> addressList = (*it).addressEntries();
        if (addressList.count() == 0)
            ci.status = false;
        else
            ci.status = true;

        cardinfoQList.append(ci);
    }
}

void NetConnect::runExternalApp(){
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}

