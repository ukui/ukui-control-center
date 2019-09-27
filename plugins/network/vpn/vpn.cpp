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

Vpn::Vpn()
{
    ui = new Ui::Vpn;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("vpn");
    pluginType = NETWORK;

    component_init();

    connect(ui->addBtn, SIGNAL(clicked()), this, SLOT(run_external_app_slot()));
}

Vpn::~Vpn()
{
    delete ui;
}

QString Vpn::get_plugin_name(){
    return pluginName;
}

int Vpn::get_plugin_type(){
    return pluginType;
}

CustomWidget *Vpn::get_plugin_ui(){
    return pluginWidget;
}

void Vpn::plugin_delay_control(){

}

void Vpn::component_init(){
    ui->addBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->addBtn->setIcon(QIcon(":/vpn/add.png"));
    ui->addBtn->setIconSize(QSize(48,48));
    ui->addBtn->setText(tr("Add VPN Connect"));

    PublicData * publicdata = new PublicData();
    QStringList tmpList = publicdata->subfuncList[NETWORK];
    connect(ui->availablenetBtn, &QPushButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList.at(0), NETWORK, 0);});

    delete publicdata;
}

void Vpn::run_external_app_slot(){
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}
