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
#include "update.h"
#include "ui_update.h"

Update::Update()
{
    ui = new Ui::Update;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("update");
    pluginType = SECURITY_UPDATES;

    ui_init();

    connect(ui->checkBtn, SIGNAL(clicked()), this, SLOT(update_btn_clicked()));
    connect(ui->changesettingsBtn, SIGNAL(clicked()), this, SLOT(update_settings_btn_clicked()));
}

Update::~Update()
{
    delete ui;
}

QString Update::get_plugin_name(){
    return pluginName;
}

int Update::get_plugin_type(){
    return pluginType;
}

CustomWidget *Update::get_plugin_ui(){
    return pluginWidget;
}

void Update::plugin_delay_control(){

}

void Update::ui_init(){
    ui->pushButton_2->setEnabled(false);

    QPixmap pixmap("://update/logo.svg");
    ui->logoLabel->setPixmap(pixmap.scaled(QSize(64,64)));

    QDateTime current =QDateTime::currentDateTime();
    QString current_date_time =current.toString(QString("yyyy-MM-dd hh:mm:ss"));
    ui->updatetimeLabel->setText(current_date_time);
}

void Update::update_btn_clicked(){
    QString cmd = "/usr/bin/update-manager";

    QProcess process(this);
    process.startDetached(cmd);
}

void Update::update_settings_btn_clicked(){
    QString cmd = "software-properties-gtk";

    QProcess process(this);
    process.startDetached(cmd);
}
