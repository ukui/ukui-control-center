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
#include "vpn.h"
#include "ui_vpn.h"

#include <QProcess>
#include <QMouseEvent>
#define THEME_QT_SCHEMA                  "org.ukui.style"
#define MODE_QT_KEY                      "style-name"
Vpn::Vpn() : mFirstLoad(true)
{
    pluginName = tr("VPN");
    pluginType = NETWORK;
}

Vpn::~Vpn()
{
    if (!mFirstLoad) {
        delete ui;
    }
}

QString Vpn::get_plugin_name(){
    return pluginName;
}

int Vpn::get_plugin_type(){
    return pluginType;
}

QWidget *Vpn::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Vpn;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        const QByteArray idd(THEME_QT_SCHEMA);
        if  (QGSettings::isSchemaInstalled(idd)){
            qtSettings = new QGSettings(idd);
        }
        ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
        initComponent();
    }
    return pluginWidget;
}

void Vpn::plugin_delay_control(){

}

const QString Vpn::name() const {

    return QStringLiteral("vpn");
}

void Vpn::initComponent(){
//    ui->addBtn->setIcon(QIcon("://img/plugins/vpn/add.png"));
//    ui->addBtn->setIconSize(QSize(48, 48));
//    ui->addBtn->setStyleSheet("QPushButton{background-color:transparent;}");


    addWgt = new HoverWidget("");
    addWgt->setObjectName("addwgt");
//    addWgt->setMinimumSize(QSize(580, 50));
//    addWgt->setMaximumSize(QSize(960, 50));
    addWgt->setFixedHeight(64);
    addWgt->setStyleSheet("HoverWidget#addwgt{background: palette(base); border-radius: 4px;}HoverWidget:hover:!pressed#addwgt{background: #2FB3E8; border-radius: 4px;}");

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel * iconLabel = new QLabel();
    //~ contents_path /vpn/Add vpn connect
    QLabel * textLabel = new QLabel(tr("Add vpn"));
    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
    iconLabel->setPixmap(pixgray);
    addLyt->addWidget(iconLabel);
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

}

void Vpn::runExternalApp(){
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}

//bool Vpn::eventFilter(QObject *watched, QEvent *event)
//{
//    if (watched == ui->addFrame){
//        if (event->type() == QEvent::MouseButtonPress){
//            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
//            if (mouseEvent->button() == Qt::LeftButton){
//                runExternalApp();
//                return true;
//            } else
//                return false;
//        }
//    }
//    return QObject::eventFilter(watched, event);
//}
