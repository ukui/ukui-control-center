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
#include "keyboardcontrol.h"
#include "ui_keyboardcontrol.h"

#include "kylin-keyboard-interface.h"
#include "kylin-interface-interface.h"
#include <QGSettings/QGSettings>

#include <QDebug>

#define KBD_LAYOUTS_SCHEMA "org.mate.peripherals-keyboard-xkb.kbd"
#define KBD_LAYOUTS_KEY "layouts"

KeyboardControl::KeyboardControl()
{
    ui = new Ui::KeyboardControl;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("keyboardcontrol");
    pluginType = DEVICES;

    pluginWidget->setStyleSheet("background: #ffffff;");

    //隐藏未开发功能
    ui->repeatWidget_4->hide();
    ui->repeatWidget_5->hide();

    ui->repeatWidget_0->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->repeatWidget_1->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->repeatWidget_2->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");

    ui->repeatWidget_3->setStyleSheet("QWidget#repeatWidget_3{background: #F4F4F4; border-radius: 6px;}");
    ui->repeatLabel_3->setStyleSheet("QLabel{background: #F4F4F4; font-size: 14px; color: #D9000000;}");

    ui->repeatWidget_4->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->repeatWidget_5->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");


    ui->layoutWidget_0->setStyleSheet("QWidget#layoutWidget_0{background: #F4F4F4; border-radius: 6px;}");
    ui->layoutLabel_0->setStyleSheet("QLabel{background: #F4F4F4; font-size: 14px; color: #D9000000;}");

    ui->layoutWidget_1->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");

    //初始化键盘布局GSettings
    const QByteArray id(KBD_LAYOUTS_SCHEMA);
    kbdsettings = new QGSettings(id);

    //构建布局管理器对象
    layoutmanagerObj = new KbdLayoutManager(kbdsettings->get(KBD_LAYOUTS_KEY).toStringList());

    //设置按键
    InitDBusKeyboard();

    initComponent();
    initGeneralStatus();

    rebuildLayoutsComBox();

}

KeyboardControl::~KeyboardControl()
{
    delete ui;
    delete kbdsettings;

    DeInitDBusKeyboard();
}

QString KeyboardControl::get_plugin_name(){
    return pluginName;
}

int KeyboardControl::get_plugin_type(){
    return pluginType;
}

QWidget *KeyboardControl::get_plugin_ui(){
    return pluginWidget;
}

void KeyboardControl::plugin_delay_control(){

}

void KeyboardControl::initComponent(){
    //重复输入开关按钮
    keySwitchBtn = new SwitchButton(pluginWidget);
    ui->enableHorLayout->addWidget(keySwitchBtn);

    //大写锁定开关按钮
    capsLockSwitchBtn = new SwitchButton(pluginWidget);
    ui->capsLockHorLayout->addWidget(capsLockSwitchBtn);

    //小键盘开关按钮
    numLockSwitchBtn = new SwitchButton(pluginWidget);
    ui->numLockHorLayout->addWidget(numLockSwitchBtn);

    ui->addBtn->setIcon(QIcon("://img/plugins/keyboardcontrol/add.png"));
    ui->addBtn->setIconSize(QSize(48, 48));

    connect(keySwitchBtn, &SwitchButton::checkedChanged, this, [=](bool checked){
        kylin_hardware_keyboard_set_repeat(checked);
    });

    connect(ui->delayHorSlider, &QSlider::valueChanged, this, [=](int value){
        kylin_hardware_keyboard_set_delay(value);
    });

    connect(ui->speedHorSlider, &QSlider::valueChanged, this, [=](int value){
        kylin_hardware_keyboard_set_rate(value);
    });


    connect(ui->addBtn, &QPushButton::clicked, this, [=]{
        layoutmanagerObj->exec();
    });

}

void KeyboardControl::initGeneralStatus(){
    //设置按键重复状态
    keySwitchBtn->setChecked(kylin_hardware_keyboard_get_repeat());

    //设置按键重复的延时
    ui->delayHorSlider->setValue(kylin_hardware_keyboard_get_delay());

    //设置按键重复的速度
    ui->speedHorSlider->setValue(kylin_hardware_keyboard_get_rate());

}

void KeyboardControl::rebuildLayoutsComBox(){
    QStringList layouts = kbdsettings->get(KBD_LAYOUTS_KEY).toStringList();
    ui->layoutsComBox->blockSignals(true);
    //清空键盘布局下拉列表
    ui->layoutsComBox->clear();

    //重建键盘布局下拉列表
    for (QString layout : layouts){
        ui->layoutsComBox->addItem(layoutmanagerObj->kbd_get_description_by_id(const_cast<const char *>(layout.toLatin1().data())), layout);
    }

    ui->layoutsComBox->blockSignals(false);
}
