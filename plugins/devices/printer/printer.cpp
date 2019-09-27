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
#include "printer.h"
#include "ui_printer.h"

#include <QDebug>

Printer::Printer(){
    ui = new Ui::Printer;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("printer");
    pluginType = DEVICES;


    ui->addBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->addBtn->setIcon(QIcon(":/printer/add.png"));
    ui->addBtn->setIconSize(QSize(48,48));

    ui->listWidget->setStyleSheet("border: none; background-color: #f5f6f7");

    QStringList printer = QPrinterInfo::availablePrinterNames();

    for (int num = 0; num < printer.count(); num++){

        QWidget * printerdevWidget = new QWidget();
        printerdevWidget->setAttribute(Qt::WA_DeleteOnClose);
        QVBoxLayout * printerdevVerLayout = new QVBoxLayout(printerdevWidget);
        QToolButton * printerdevToolBtn = new QToolButton(printerdevWidget);
        printerdevToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        printerdevToolBtn->setIcon(QIcon("://printer/printerdev.png"));
        printerdevToolBtn->setIconSize(QSize(48,48));
        printerdevToolBtn->setText(printer.at(num));

        printerdevVerLayout->addWidget(printerdevToolBtn);
        printerdevWidget->setLayout(printerdevVerLayout);

        QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
        item->setSizeHint(QSize(180, 64));
        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, printerdevWidget);
    }

    connect(ui->addBtn, SIGNAL(clicked()), this, SLOT(run_external_app_slot()));
}

Printer::~Printer()
{
//    delete pluginWidget;
    delete ui;
}

QString Printer::get_plugin_name(){
    return pluginName;
}

int Printer::get_plugin_type(){
    return pluginType;
}

CustomWidget *Printer::get_plugin_ui(){
    return pluginWidget;
}

void Printer::plugin_delay_control(){

}

void Printer::run_external_app_slot(){
    QString cmd = "system-config-printer";

    QProcess process(this);
    process.startDetached(cmd);
}
