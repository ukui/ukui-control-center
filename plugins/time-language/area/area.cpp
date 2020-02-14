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

#include <QDebug>


Area::Area()
{
    ui = new Ui::Area;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    pluginWidget->setStyleSheet("background: #ffffff;");
    ui->setupUi(pluginWidget);

    pluginName = tr("area");
    pluginType = DATETIME;

    unsigned int uid = getuid();
    objpath = objpath +"/org/freedesktop/Accounts/User"+QString::number(uid);


    m_areaInterface = new QDBusInterface("org.freedesktop.Accounts",
                                         objpath,
                                         "org.freedesktop.Accounts.User",
                                         QDBusConnection::systemBus());
    initUI();
    connect(ui->langcomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_language_slot(int)));
    connect(ui->countrycomboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(change_area_slot(int)));
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

QWidget *Area::get_plugin_ui(){
    return pluginWidget;
}

void Area::plugin_delay_control(){

}

void Area::run_external_app_slot(){
    QString cmd = "gnome-language-selector";

    QProcess process(this);
    process.startDetached(cmd);
}


void Area::initUI(){
    ui->arealabel->setText(tr("current area"));

    ui->countrylabel->setText(tr("display format area"));
    ui->showformwidget->setStyleSheet("QWidget#showformwidget{background-color:#E5E7E9;border-radius:6px}");
    ui->countrylabel->setStyleSheet("QLabel#countrylabel{background: #E5E7E9;}");

    ui->countrycomboBox->addItem(tr("US"));
    ui->countrycomboBox->addItem(tr("CN"));

    ui->areaformlabel->setText(tr("format of area"));
    ui->calendarlabel->setText(tr("calendar"));
    ui->weeklabel->setText(tr("first day of week"));
    ui->datelabel->setText(tr("date"));
    ui->timelabel->setText(tr("time"));

    ui->chgformButton->setStyleSheet("QPushButton{background-color:#E5E7E9;border-radius:4px}"
                                   "QPushButton:hover{background-color: #3D6BE5;};border-radius:4px");

    ui->chgformButton->setText(tr("change format of data"));

    ui->formwidget->setStyleSheet("background-color:#E5E7E9;border-radius:6px");

    ui->mainLagLabel->setText(tr("first language"));
    ui->languagewidget->setStyleSheet("QWidget#languagewidget{background-color:#E5E7E9;border-radius:6px}");
    ui->languagelabel->setText(tr("system language"));
    ui->languagelabel->setStyleSheet("QLabel#languagelabel{background: #E5E7E9;}");

    ui->addlanwidget->setStyleSheet("QWidget#addlanwidget{background-color:#E5E7E9;border-radius:6px}");

    ui->langcomboBox->addItem(tr("English"));
    ui->langcomboBox->addItem(tr("Chinese"));

    ui->addlanlabel->setStyleSheet("QLabel{background-color:#E5E7E9}");
    ui->addlanlabel->setText(tr("add main language"));

    ui->addlanBtn->setIcon(QIcon("://img/plugins/printer/add.png"));
    ui->addlanBtn->setIconSize(QSize(48, 48));
}

void Area::change_language_slot(int index){
    QDBusReply<bool> res;
    switch (index) {
    case 0:
        res = m_areaInterface->call("SetLanguage","en_US");
        break;
    case 1:
        res = m_areaInterface->call("SetLanguage","zh_CN");
        break;
    }
}

void Area::change_area_slot(int index){
    qDebug()<<"area----------->"<<endl;
    QDBusReply<bool> res;
    switch (index) {
    case 0:
        res = m_areaInterface->call("SetFormatsLocale","en_US");
        break;
    case 1:
        res = m_areaInterface->call("SetFormatsLocale","zh_CN");
        break;
    }
}




