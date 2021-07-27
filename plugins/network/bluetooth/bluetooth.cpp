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
#include "bluetooth.h"
#include "ui_bluetooth.h"

#include <QProcess>

#include <QDebug>
#include <QMouseEvent>

#define ITEMFIXEDHEIGH 58
#define THEME_QT_SCHEMA                  "org.ukui.style"
#define MODE_QT_KEY                      "style-name"
Bluetooth::Bluetooth()
{
    pluginName = tr("Bluetooth");
    //~ contents_path /bluetooth/Bluetooth
    pluginType = NETWORK;
    ui = new Ui::Bluetooth;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);
    blueToothBtn = new SwitchButton(pluginWidget);

    pluginName = tr("Bluetooth");
    ui->label->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    //~ contents_path /bluetooth/Bluetooth
    pluginType = NETWORK;

    const QByteArray idd(THEME_QT_SCHEMA);
    if  (QGSettings::isSchemaInstalled(idd)){
        qtSettings = new QGSettings(idd);
    }
    //~ contents_path /bluetooth/BlueTooth
    ui->label->setText(tr("BlueTooth"));
    //~ contents_path /bluetooth/Open Bluetooth
    ui->titleLabel->setText(tr("Open Bluetooth"));

    ui->horizontalLayout->addWidget(blueToothBtn);

    pTimer = new QTimer(this);
    pTimer->setInterval(1000);
    a = new bluetooththread();
    QObject::connect(a,SIGNAL(result(bool)),this,SLOT(bluetoothservice(bool)));
    initComponent();
    a->start();
}

Bluetooth::~Bluetooth()
{
    delete ui;
}

QString Bluetooth::get_plugin_name(){
    return pluginName;
}

int Bluetooth::get_plugin_type(){
    return pluginType;
}

QWidget *Bluetooth::get_plugin_ui(){

    return pluginWidget;
}

void Bluetooth::plugin_delay_control(){

}

const QString Bluetooth::name() const {

    return QStringLiteral("bluetooth");
}

/* 初始化dbus接口 */
bool Bluetooth::initBluetoothDbus()
{
    m_pServiceInterface = new QDBusInterface(KYLIN_BLUETOOTH_SERVER_NAME,
                                             KYLIN_BLUETOOTH_SERVER_PATH,
                                             KYLIN_BLUETOOTH_SERVER_INTERFACE,
                                             QDBusConnection::sessionBus());
    if (!m_pServiceInterface->isValid()) {
        qDebug()<<"false";
        return false;
    }
    connect(m_pServiceInterface, SIGNAL(BluetoothStatusChanged(bool)), this, SLOT(BluetoothStatusChangedSlots(bool)));
    connect(blueToothBtn, SIGNAL(checkedChanged(bool)), this, SLOT(bluetoothButtonClickSlots(bool)));
    return true;
}
/* 初始化第一次蓝牙状态 */
void Bluetooth::initBluetoothStatus()
{
    QDBusMessage msg = m_pServiceInterface->call("GetBluetoothStatus");
    m_bbluetoothStatus = msg.arguments().at(0).toBool();
    blueToothBtn->setChecked(m_bbluetoothStatus);
}
/* 监听dbus信号，当蓝牙状态改变时，将会触发此信号 */
void Bluetooth::BluetoothStatusChangedSlots(bool status)
{
    bool m_bluetoothStatus = status;
    blueToothBtn->setChecked(m_bluetoothStatus);
    return;
}
// 点击按钮槽函数
void Bluetooth::bluetoothButtonClickSlots(bool status)
{
    bool bluetoothStatus = status;
    if (!bluetoothStatus) {
        m_pServiceInterface->call("SetBluetoothStatus", false);
    } else {
        m_pServiceInterface->call("SetBluetoothStatus", true);
    }
    return;
}


void Bluetooth::initComponent(){

    addWgt = new HoverWidget("");
    addWgt->setObjectName("addwgt");
    addWgt->setMinimumSize(QSize(580, 64));
    addWgt->setMaximumSize(QSize(16777215, 64));
    addWgt->setStyleSheet("HoverWidget#addwgt{background: palette(base); border-radius: 4px;}HoverWidget:hover:!pressed#addwgt{background: #2FB3E8; border-radius: 4px;}");

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel * iconLabel = new QLabel();
    //~ contents_path /bluetooth/Add Bluetooths
    QLabel * textLabel = new QLabel(tr("Add Bluetooths"));
    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
    iconLabel->setPixmap(pixgray);
    addLyt->addItem(new QSpacerItem(8,10,QSizePolicy::Fixed));
    addLyt->addWidget(iconLabel);
    addLyt->addItem(new QSpacerItem(16,10,QSizePolicy::Fixed));
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    addWgt->setLayout(addLyt);

    //图标跟随主题变化
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
        iconLabel->setPixmap(pixgray);
    } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
        iconLabel->setPixmap(pixgray);
    }
    connect(qtSettings,&QGSettings::changed,this,[=](const QString &key){
        if (key == "styleName") {
            QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
            if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
                QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
                iconLabel->setPixmap(pixgray);
            } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
                QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
                iconLabel->setPixmap(pixgray);
            }
        }
    });
    // 还原状态
    connect(addWgt, &HoverWidget::leaveWidget, this, [=](){
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
            QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
            iconLabel->setPixmap(pixgray);
        } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
            QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
            iconLabel->setPixmap(pixgray);
        }
        textLabel->setStyleSheet("color: palette(windowText);");
    });
    // 悬浮改变Widget状态
    connect(addWgt, &HoverWidget::enterWidget, this, [=](){
        QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
        if ("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode || "ukui-light" == currentThemeMode || "ukui-white-unity" == currentThemeMode) {
            QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
            iconLabel->setPixmap(pixgray);
        } else if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode || "ukui-black-unity" == currentThemeMode){
            QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
            iconLabel->setPixmap(pixgray);
        }
        textLabel->setStyleSheet("color: palette(base);");

    });

    connect(addWgt, &HoverWidget::widgetClicked, this, [=](QString mname){
        runExternalApp();
    });
    ui->addLyt->addWidget(addWgt);

    pTimer->start();
}

void Bluetooth::runExternalApp(){
    QString cmd = "/usr/bin/python3 /usr/bin/blueman-manager";

    QProcess process(this);
    process.startDetached(cmd);
}
void Bluetooth::bluetoothservice(bool status)
{
    a->quit();
    delete a;
    qDebug()<<"service="<<status;
    if (status){
        initBluetoothDbus();
        initBluetoothStatus();
    }
}
