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
#include "screenlock.h"
#include "ui_screenlock.h"

#include <QDebug>

#define BGPATH "/usr/share/backgrounds/"
#define SCREENLOCK_BG_SCHEMA "org.ukui.screensaver"
#define SCREENLOCK_BG_KEY "background"

Screenlock::Screenlock()
{
    ui = new Ui::Screenlock;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("screenlock");
    pluginType = PERSONALIZED;

    bggsettings = g_settings_new(SCREENLOCK_BG_SCHEMA);

    lockbgSize = QSize(400, 240);

    component_init();
    status_init();


}

Screenlock::~Screenlock()
{
    delete ui;
    g_object_unref(bggsettings);
}

QString Screenlock::get_plugin_name(){
    return pluginName;
}

int Screenlock::get_plugin_type(){
    return pluginType;
}

CustomWidget *Screenlock::get_plugin_ui(){
    return pluginWidget;
}

void Screenlock::plugin_delay_control(){

}

void Screenlock::component_init(){
    //背景形式
    QStringList formList;
    formList << tr("picture");
    ui->formComboBox->addItems(formList);

    //同步登录背景的控件SwitchButton
    setloginbgBtn = new SwitchButton();
    setloginbgBtn->setAttribute(Qt::WA_DeleteOnClose);
    ui->setloginbgHLayout->addWidget(setloginbgBtn);
    ui->setloginbgHLayout->addStretch();
}

void Screenlock::status_init(){
    //获取当前锁屏背景及登录背景，对比后确定按钮状态
    QString bgfilename = QString(g_settings_get_string(bggsettings, SCREENLOCK_BG_KEY));
    if (bgfilename != ""){
        QPixmap bg = QPixmap(bgfilename);
        ui->bgLabel->setPixmap(bg.scaled(lockbgSize));
    }
    else {
        ui->bgLabel->setPixmap(QPixmap("://screenlock/none.png").scaled(lockbgSize));
    }

    PublicData * publicdata = new PublicData();
    QStringList tmpList = publicdata->subfuncList[PERSONALIZED];
    connect(ui->screensaverBtn, &QPushButton::clicked, this, [=]{pluginWidget->emitting_toggle_signal(tmpList.at(4), PERSONALIZED, 0);});

    connect(ui->openPushBtn, SIGNAL(clicked()), this, SLOT(openpushbtn_clicked_slot()));

    delete publicdata;
}

void Screenlock::openpushbtn_clicked_slot(){

    QString filters = "Image files(*.png *.jpg)";
    QFileDialog fd;
    fd.setDirectory(BGPATH);
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::List);
    fd.setNameFilter(filters);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle(tr("selsect screenlock background"));
    fd.setLabelText(QFileDialog::Accept, "Select");

    if (fd.exec() != QDialog::Accepted)
        return;

    QString selectedfile;
    selectedfile = fd.selectedFiles().first();

    QPixmap bg = QPixmap(selectedfile);
    ui->bgLabel->setPixmap(bg.scaled(lockbgSize));

    //QString to char *
    QByteArray ba = selectedfile.toLatin1();

    g_settings_set_string(bggsettings, SCREENLOCK_BG_KEY, ba.data());
}
