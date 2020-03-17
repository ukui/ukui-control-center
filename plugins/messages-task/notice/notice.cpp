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

#define NOTICE_SCHEMA "org.ukui.control-center.notice"
#define NEW_FEATURE_KEY "show-new-feature"
#define ENABLE_NOTICE_KEY "enable-notice"
#define SHOWON_LOCKSCREEN_KEY "show-on-lockscreen"

#define NOTICE_ORIGIN_SCHEMA "org.ukui.control-center.noticeorigin"
#define NOTICE_ORIGIN_PATH "/org/ukui/control-center/noticeorigin/"

Notice::Notice()
{
    ui = new Ui::Notice;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("notice");
    pluginType = NOTICEANDTASKS;

    pluginWidget->setStyleSheet("background: #ffffff;");

    ui->newfeatureWidget->setStyleSheet("QWidget{background: #F4F4F4; border: none; border-top-left-radius: 6px; border-top-right-radius: 6px;}");

    ui->enableWidget->setStyleSheet("QWidget{background: #F4F4F4; border: none;}");

    ui->lockscreenWidget->setStyleSheet("QWidget{background: #F4F4F4; border: none; border-bottom-left-radius: 6px; border-bottom-right-radius: 6px;}");

    QByteArray id(NOTICE_SCHEMA);
    nSetting = new QGSettings(id);

    setupComponent();
    initNoticeStatus();
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
        nSetting->set(ENABLE_NOTICE_KEY, checked);
    });
    connect(lockscreenSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked){
        nSetting->set(SHOWON_LOCKSCREEN_KEY, checked);
    });
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

