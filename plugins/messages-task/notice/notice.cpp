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
#include "notice.h"
#include "ui_notice.h"
#include "appdetail.h"
#include "commonComponent/HoverWidget/hoverwidget.h"

#define NOTICE_SCHEMA "org.ukui.control-center.notice"
#define NEW_FEATURE_KEY "show-new-feature"
#define ENABLE_NOTICE_KEY "enable-notice"
#define SHOWON_LOCKSCREEN_KEY "show-on-lockscreen"

#define NOTICE_ORIGIN_SCHEMA "org.ukui.control-center.noticeorigin"
#define NOTICE_ORIGIN_PATH "/org/ukui/control-center/noticeorigin/"

#define DESKTOPPATH "/usr/share/applications/"

Notice::Notice()
{
    ui = new Ui::Notice;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    appsName<<"kylin-video"<<"ubuntu-kylin-software-center"<<"kylin-assistant";
    appsKey<<"kylinVideo"<<"ubuntuKylinSoftwareCenter"<<"kylinAssistant";

    ui->setupUi(pluginWidget);

    pluginName = tr("notice");
    pluginType = NOTICEANDTASKS;

    pluginWidget->setStyleSheet("background: #ffffff;");

    ui->newfeatureWidget->setVisible(false);
    ui->newfeatureWidget->setStyleSheet("QWidget{background: #F4F4F4; border: none; border-top-left-radius: 6px; border-top-right-radius: 6px;}");


    ui->enableWidget->setStyleSheet("QWidget{background: #F4F4F4; border: none;}");

    ui->lockscreenWidget->setVisible(false);
    ui->lockscreenWidget->setStyleSheet("QWidget{background: #F4F4F4; border: none; border-bottom-left-radius: 6px; border-bottom-right-radius: 6px;}");

    ui->title2Label->setContentsMargins(0,0,0,16);
    ui->applistWidget->setStyleSheet("QListWidget#applistWidget{border: none;}");
    ui->applistWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->applistWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setupGSettings();
    setupComponent();
    initNoticeStatus();
    initOriNoticeStatus();
}

Notice::~Notice()
{
    delete ui;

    delete nSetting;
}

QString Notice::get_plugin_name(){
    return pluginName;
}

int Notice::get_plugin_type(){
    return pluginType;
}

QWidget * Notice::get_plugin_ui(){
    return pluginWidget;
}

void Notice::plugin_delay_control(){

}

void Notice::setupComponent(){
    newfeatureSwitchBtn = new SwitchButton(pluginWidget);
    enableSwitchBtn = new SwitchButton(pluginWidget);
    lockscreenSwitchBtn =  new SwitchButton(pluginWidget);

    ui->newfeatureHorLayout->addWidget(newfeatureSwitchBtn);
    ui->enableHorLayout->addWidget(enableSwitchBtn);
    ui->lockscreenHorLayout->addWidget(lockscreenSwitchBtn);



    connect(newfeatureSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked){        
        nSetting->set(NEW_FEATURE_KEY, checked);
    });
    connect(enableSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked){        
//        qDebug()<<"checked is-------->"<<endl;
        nSetting->set(ENABLE_NOTICE_KEY, checked);        
    });
    connect(lockscreenSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked){

        nSetting->set(SHOWON_LOCKSCREEN_KEY, checked);
    });
}

void Notice::setupGSettings() {

    if(QGSettings::isSchemaInstalled(NOTICE_SCHEMA) &&
            QGSettings::isSchemaInstalled(NOTICE_ORIGIN_SCHEMA)) {

        QByteArray id(NOTICE_SCHEMA);
        nSetting = new QGSettings(id);

        QByteArray orid(NOTICE_ORIGIN_SCHEMA);
        oriSettings = new QGSettings(orid);
    }
}

void Notice::initAppSwitchbtn(SwitchButton *appBtn,QString appName) {
    if (oriSettings) {
        QStringList appKeys = oriSettings->keys();
//        qDebug()<<"key is------->"<<appKeys<<endl;
        bool judge = oriSettings->get(appName).toBool();
        appBtn->setChecked(judge);
    } else {
        qDebug()<<" qgsettings is not installed "<<endl;
    }
}

void Notice::initNoticeStatus(){
    newfeatureSwitchBtn->blockSignals(true);
    enableSwitchBtn->blockSignals(true);
    lockscreenSwitchBtn->blockSignals(true);
    newfeatureSwitchBtn->setChecked(nSetting->get(NEW_FEATURE_KEY).toBool());
    enableSwitchBtn->setChecked(nSetting->get(ENABLE_NOTICE_KEY).toBool());
    lockscreenSwitchBtn->setChecked(nSetting->get(SHOWON_LOCKSCREEN_KEY).toBool());
    newfeatureSwitchBtn->blockSignals(false);
    enableSwitchBtn->blockSignals(false);
    lockscreenSwitchBtn->blockSignals(false);
}

void Notice::initOriNoticeStatus() {

    for(int i = 0; i < appsName.length(); i++){
        QByteArray ba = QString(DESKTOPPATH + appsName.at(i) + ".desktop").toUtf8();
        GDesktopAppInfo * audioinfo = g_desktop_app_info_new_from_filename(ba.constData());
        QString appname = g_app_info_get_name(G_APP_INFO(audioinfo));

//        qDebug()<<"notify appname is------------->"<<appname<<endl;

        ////构建Widget
        QWidget * baseWidget = new QWidget();
        baseWidget->setAttribute(Qt::WA_DeleteOnClose);

        QVBoxLayout * baseVerLayout = new QVBoxLayout(baseWidget);
        baseVerLayout->setSpacing(0);
        baseVerLayout->setContentsMargins(0, 0, 0, 2);

        HoverWidget * devWidget = new HoverWidget(appname,baseWidget);
        devWidget->setMinimumWidth(550);
        devWidget->setMaximumWidth(960);
        devWidget->setMinimumHeight(50);
        devWidget->setMaximumHeight(50);
        devWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
        connect(devWidget, &HoverWidget::enterWidget, this, [=](QString name){
            Q_UNUSED(name)
            devWidget->setStyleSheet("background: #EEF2FD;");

        });

        connect(devWidget, &HoverWidget::leaveWidget, this, [=](QString name){
            Q_UNUSED(name)
            devWidget->setStyleSheet("background: #F4F4F4;");
        });

        connect(devWidget, &HoverWidget::widgetClicked, this, [=](QString name){
//            qDebug()<<"clicked widget--------->"<<endl;
            AppDetail *app= new AppDetail(name,appsName.at(i));
            app->show();
        });
        QHBoxLayout * devHorLayout = new QHBoxLayout();
        devHorLayout->setSpacing(8);
        devHorLayout->setContentsMargins(16, 0, 16, 0);

        QPushButton * iconBtn = new QPushButton();
        QSizePolicy iconSizePolicy = iconBtn->sizePolicy();
        iconSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
        iconSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        iconBtn->setSizePolicy(iconSizePolicy);
        iconBtn->setIcon(QIcon::fromTheme(appsName.at(i)));


        QLabel * nameLabel = new QLabel();
        QSizePolicy nameSizePolicy = nameLabel->sizePolicy();
        nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
        nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        nameLabel->setSizePolicy(nameSizePolicy);
        nameLabel->setScaledContents(true);
        nameLabel->setText(appname);

        SwitchButton *appSwitch = new SwitchButton();
        initAppSwitchbtn(appSwitch, appsName.at(i));

        if (oriSettings) {
            connect(oriSettings, &QGSettings::changed, [=](QString key){
//                qDebug()<<"change key is--->"<<key<<endl;
                if (key == appsKey.at(i)) {
                    bool judge = oriSettings->get(appsKey.at(i)).toBool();
                    appSwitch->setChecked(judge);
                }
            });
        }

//        appsBtn->append(appSwitch);
        connect(enableSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked) {
//            qDebug()<<"checked is-------->"<<checked<<endl;
            changeAppstatus(checked, appname, appSwitch);
        });

//        if (enableSwitchBtn->isChecked()){
//            bool value =  oriSettings->get(appsName.at(i)).toBool();
//            appSwitch->setChecked(value);
//        } else {
//            appSwitch->setChecked(false);
//        }

        connect(appSwitch, &SwitchButton::checkedChanged, [=](bool checked){
            qDebug()<<"key and bool is------>"<<appsName.at(i)<<appSwitch->isChecked()<<endl;

            if (oriSettings){
                oriSettings->set(appsName.at(i), checked);
            }
            if (checked) {

                enableSwitchBtn->setChecked(true);
                appSwitch->setChecked(true);
            }
        });

        devHorLayout->addWidget(iconBtn);
        devHorLayout->addWidget(nameLabel);
        devHorLayout->addStretch();

        devHorLayout->addWidget(appSwitch);

        devWidget->setLayout(devHorLayout);

        baseVerLayout->addWidget(devWidget);
        baseVerLayout->addStretch();

        baseWidget->setLayout(baseVerLayout);

        QListWidgetItem * item = new QListWidgetItem(ui->applistWidget);
        item->setSizeHint(QSize(502, 52));

        ui->applistWidget->setItemWidget(item, baseWidget);
    }
}

void Notice::changeAppstatus(bool checked, QString name, SwitchButton *appBtn) {
    // if master swtich is off, record app's pre status
//    qDebug()<<"changeAppstatus------------->"<<checked<<endl;
    bool judge;
    if (!checked) {
        judge = appBtn->isChecked();
        appMap.insert(name,judge);
        appBtn->setChecked(checked);
    } else {
        judge =appMap.value(name);
        appBtn->setChecked(judge);
    }
}


