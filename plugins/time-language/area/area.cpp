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
#include "area.h"
#include "ui_area.h"

Area::Area()
{
    ui = new Ui::Area;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("area");
    pluginType = TIME_LANGUAGE;

    PublicData * publicdata = new PublicData();
    QStringList tmpList = publicdata->subfuncList[DEVICES];
    connect(ui->kbdBtn, &QPushButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList.at(2), DEVICES, 0);});
    connect(ui->setPushButton, SIGNAL(clicked(bool)), this, SLOT(run_external_app_slot()));

    delete publicdata;
}

Area::~Area()
{
    delete ui;
}

QString Area::get_plugin_name(){
    return pluginName;
}

int Area::get_plugin_type(){
    return pluginType;
}

CustomWidget *Area::get_plugin_ui(){
    return pluginWidget;
}

void Area::plugin_delay_control(){

}

void Area::run_external_app_slot(){
    QString cmd = "gnome-language-selector";

    QProcess process(this);
    process.startDetached(cmd);
}

