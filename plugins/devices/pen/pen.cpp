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
#include "pen.h"
#include "ui_pen.h"

#include <QProcess>

#include <QDebug>
#include <QMouseEvent>

#define ITEMFIXEDHEIGH 58

Pen::Pen() : mFirstLoad(true)
{
    pluginName = tr("Pen");
    pluginType = DEVICES;
}

Pen::~Pen()
{
    if (!mFirstLoad)
        delete ui;
}

QString Pen::get_plugin_name(){
   return pluginName;
}

int Pen::get_plugin_type(){
   return pluginType;
}

QWidget *Pen::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Pen;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        //~ contents_path /pen/Stylus settings
        ui->label->setText(tr("Stylus settings"));
        ui->label->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
        //~ contents_path /pen/Use the side button 2 of the stylus to simulate Erase
        ui->label_5->setText(tr("Use the side button 2 of the stylus to simulate Erase"));
        ui->label_6->setText(tr("Handwriting anti touch"));
        ui->label_2->setPixmap(QPixmap(":/img/plugins/pen/setting-illus-pen_small.png"));
        ui->label_3->setText(tr("Please further configure the side button function of the stylus through other software"));
        ui->label_3->setEnabled(false);
        ui->moreFrame->hide();
        initComponent();
    }
   return pluginWidget;
}
void Pen::plugin_delay_control(){

}

const QString Pen::name() const {

   return QStringLiteral("pen");
}


void Pen::initComponent(){

//    rightBtn = new SwitchButton(pluginWidget);
    rightBtn = new QLabel(pluginWidget);
    rightBtn->setPixmap(QPixmap(":/img/plugins/pen/open_inuse.png"));
    ui->righthorLayout->addWidget(rightBtn);

    moreBtn = new SwitchButton(pluginWidget);
    ui->morhorLayout->addWidget(moreBtn);

    const QByteArray id("org.ukui.control-center.pen");
    if (QGSettings::isSchemaInstalled(id)){
        m_settings = new QGSettings(id);
        connect(m_settings, &QGSettings::changed, this, &Pen::getGsettingChageSlots);
    }
//    connect(rightBtn, &SwitchButton::checkedChanged, this, &Pen::setRight);
    connect(moreBtn, &SwitchButton::checkedChanged, this, &Pen::setMore);
//    bool status1 = m_settings->get("right-click").toBool();
    bool status2 = false;
    if (m_settings && m_settings->keys().contains("multi-touch")) {
        status2 = m_settings->get("multi-touch").toBool();
    }
//    rightBtn->setChecked(status1);
    moreBtn->setChecked(status2);

}
void Pen::setRight(bool isOpen)
{
    if (m_settings && m_settings->keys().contains("right-click"))
        m_settings->set("right-click",isOpen);
}
void Pen::setMore(bool isOpen)
{
    if (m_settings && m_settings->keys().contains("multi-touch"))
        m_settings->set("multi-touch",isOpen);
}
void Pen::getGsettingChageSlots()
{
//    bool status1 = m_settings->get("right-click").toBool();
    bool status2 = false;
    if (m_settings && m_settings->keys().contains("multi-touch"))
        status2 = m_settings->get("multi-touch").toBool();
//    rightBtn->setChecked(status1);
    moreBtn->setChecked(status2);
}

