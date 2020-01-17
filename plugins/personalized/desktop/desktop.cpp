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
#include "desktop.h"
#include "ui_desktop.h"

#include <QGSettings/QGSettings>

#include "SwitchButton/switchbutton.h"

#include <QDebug>

#define DESKTOP_SCHEMA "org.ukui.peony.desktop"

#define COMPUTER_ICON "computer-icon-visible"
#define HOME_ICON "home-icon-visible"
#define NETWORK_ICON "network-icon-visible"
#define TRASH_ICON "trash-icon-visible"
#define VOLUMES_ICON "volumes-visible"

Desktop::Desktop()
{
    ui = new Ui::Desktop;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("desktop");
    pluginType = PERSONALIZED;

    pluginWidget->setStyleSheet("background: #ffffff;");

    ui->deskComputerWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->deskTrashWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->deskHomeWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->deskVolumeWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->deskNetworkWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");

    ui->menuComputerWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->menuTrashWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->menuHomeWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->menuSettingWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->menuNetworkWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");


    const QByteArray id(DESKTOP_SCHEMA);
    deskGSettings = new QGSettings(id);

    initComponent();
    initDesktopStatus();
}

Desktop::~Desktop()
{
    delete ui;

    delete deskGSettings;
}

QString Desktop::get_plugin_name(){
    return pluginName;
}

int Desktop::get_plugin_type(){
    return pluginType;
}

QWidget *Desktop::get_plugin_ui(){
    return pluginWidget;
}

void Desktop::plugin_delay_control(){

}

void Desktop::initComponent(){

    ui->deskComputerLabel->setPixmap(QPixmap("://img/plugins/desktop/computer.png"));
    ui->deskHomeLabel->setPixmap(QPixmap("://img/plugins/desktop/homefolder.png"));
    ui->deskTrashLabel->setPixmap(QPixmap("://img/plugins/desktop/trash.png"));
    ui->deskNetworkLabel->setPixmap(QPixmap("://img/plugins/desktop/default.png"));
    ui->deskVolumeLabel->setPixmap(QPixmap("://img/plugins/desktop/default.png"));


    deskComputerSwitchBtn = new SwitchButton(pluginWidget);
    ui->deskComputerHorLayout->addWidget(deskComputerSwitchBtn);
    connect(deskComputerSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked){deskGSettings->set(COMPUTER_ICON, checked);});

    deskTrashSwitchBtn = new SwitchButton(pluginWidget);
    ui->deskTrashHorLayout->addWidget(deskTrashSwitchBtn);
    connect(deskTrashSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked){deskGSettings->set(TRASH_ICON, checked);});


    deskHomeSwitchBtn = new SwitchButton(pluginWidget);
    ui->deskHomeHorLayout->addWidget(deskHomeSwitchBtn);
    connect(deskHomeSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked){deskGSettings->set(HOME_ICON, checked);});

    deskVolumeSwitchBtn = new SwitchButton(pluginWidget);
    ui->deskVolumeHorLayout->addWidget(deskVolumeSwitchBtn);
    connect(deskVolumeSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked){deskGSettings->set(VOLUMES_ICON, checked);});

    deskNetworkSwitchBtn = new SwitchButton(pluginWidget);
    ui->deskNetworkHorLayout->addWidget(deskNetworkSwitchBtn);
    connect(deskNetworkSwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked){deskGSettings->set(NETWORK_ICON, checked);});



    menuComputerSwitchBtn = new SwitchButton(pluginWidget);
    ui->menuComputerHorLayout->addWidget(menuComputerSwitchBtn);

    menuTrashSwitchBtn = new SwitchButton(pluginWidget);
    ui->menuTrashHorLayout->addWidget(menuTrashSwitchBtn);

    menuHomeSwitchBtn = new SwitchButton(pluginWidget);
    ui->menuHomeHorLayout->addWidget(menuHomeSwitchBtn);

    menuSettingSwitchBtn = new SwitchButton(pluginWidget);
    ui->menuSettingHorLayout->addWidget(menuSettingSwitchBtn);

    menuNetworkSwitchBtn = new SwitchButton(pluginWidget);
    ui->menuNetworkHorLayout->addWidget(menuNetworkSwitchBtn);

}

void Desktop::initDesktopStatus(){
    deskComputerSwitchBtn->blockSignals(true);
    deskHomeSwitchBtn->blockSignals(true);
    deskTrashSwitchBtn->blockSignals(true);
    deskVolumeSwitchBtn->blockSignals(true);
    deskNetworkSwitchBtn->blockSignals(true);

    deskComputerSwitchBtn->setChecked(deskGSettings->get(COMPUTER_ICON).toBool());
    deskHomeSwitchBtn->setChecked(deskGSettings->get(HOME_ICON).toBool());
    deskTrashSwitchBtn->setChecked(deskGSettings->get(TRASH_ICON).toBool());
    deskVolumeSwitchBtn->setChecked(deskGSettings->get(VOLUMES_ICON).toBool());
    deskNetworkSwitchBtn->setChecked(deskGSettings->get(NETWORK_ICON).toBool());

    deskComputerSwitchBtn->blockSignals(false);
    deskHomeSwitchBtn->blockSignals(false);
    deskTrashSwitchBtn->blockSignals(false);
    deskVolumeSwitchBtn->blockSignals(false);
    deskNetworkSwitchBtn->blockSignals(false);
}
