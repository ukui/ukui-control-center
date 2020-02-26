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
#include "mousecontrol.h"
#include "ui_mousecontrol.h"

#include <QDebug>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <X11/Xlib.h>
}

MouseControl::MouseControl()
{
    ui = new Ui::MouseControl;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("mouse");
    pluginType = DEVICES;

    pluginWidget->setStyleSheet("background: #ffffff;");

    ui->handWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->pointerSpeedWidget->setStyleSheet("QWidget{background: #F4F4F4; border-top-left-radius: 6px; border-top-right-radius: 6px;}");
    ui->sensitivityWidget->setStyleSheet("QWidget{background: #F4F4F4;}");
    ui->visibilityWidget->setStyleSheet("QWidget{background: #F4F4F4;}");
    ui->pointerSizeWidget->setStyleSheet("QWidget{background: #F4F4F4;  border-bottom-left-radius: 6px; border-bottom-right-radius: 6px;}");

    ui->cursorWeightWidget->setStyleSheet("QWidget{background: #F4F4F4; border-top-left-radius: 6px; border-top-right-radius: 6px;}");
    ui->cursorSpeedWidget->setStyleSheet("QWidget{background: #F4F4F4;}");
    ui->flashingWidget->setStyleSheet("QWidget{background: #F4F4F4;  border-bottom-left-radius: 6px; border-bottom-right-radius: 6px;}");

    InitDBusMouse();

    setupComponent();

    initHandHabitStatus();
    initPointerStatus();
}

MouseControl::~MouseControl()
{
    delete ui;
    DeInitDBusMouse();
}

QString MouseControl::get_plugin_name(){
    return pluginName;
}

int MouseControl::get_plugin_type(){
    return pluginType;
}

QWidget *MouseControl::get_plugin_ui(){
    return pluginWidget;
}

void MouseControl::plugin_delay_control(){

}

void MouseControl::setupComponent(){

    //设置左手右手鼠标控件
    ui->handHabitComBox->addItem(tr("Lefthand"), true);
    ui->handHabitComBox->addItem(tr("Righthand"), false);

    //设置指针可见性控件
    visiblityBtn = new SwitchButton(pluginWidget);
    ui->visibilityHorLayout->addWidget(visiblityBtn);

    //设置指针大小
    ui->pointerSizeComBox->addItem(tr("Default(Recommended)"), 24); //100%
    ui->pointerSizeComBox->addItem(tr("Medium"), 32); //125%
    ui->pointerSizeComBox->addItem(tr("Large"), 48); //150%

    //设置启用光标闪烁
    flashingBtn = new SwitchButton(pluginWidget);
    ui->enableFlashingHorLayout->addWidget(flashingBtn);

    connect(ui->handHabitComBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
        Q_UNUSED(index)
        kylin_hardware_mouse_set_lefthanded(ui->handHabitComBox->currentData().toBool());
    });

    connect(ui->pointerSpeedSlider, &QSlider::valueChanged, [=](int value){
        kylin_hardware_mouse_set_motionacceleration(static_cast<double>(value)/ui->pointerSpeedSlider->maximum()*10);
    });

    connect(ui->pointerSensitivitySlider, &QSlider::valueChanged, [=](int value){
        kylin_hardware_mouse_set_motionthreshold(10*value/ui->pointerSensitivitySlider->maximum());
    });

    connect(visiblityBtn, &SwitchButton::checkedChanged, [=](bool checked){
        kylin_hardware_mouse_set_locatepointer(checked);
    });

    connect(ui->pointerSizeComBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
        Q_UNUSED(index)
        kylin_hardware_mouse_set_cursorsize(ui->pointerSizeComBox->currentData().toInt());
    });

}

void MouseControl::initHandHabitStatus(){

    int handHabitIndex = ui->handHabitComBox->findData(kylin_hardware_mouse_get_lefthanded());
    ui->handHabitComBox->blockSignals(true);
    ui->handHabitComBox->setCurrentIndex(handHabitIndex);
    ui->handHabitComBox->blockSignals(false);
}

void MouseControl::initPointerStatus(){

    //当前系统指针加速值，-1为系统默认
    double mouse_acceleration = kylin_hardware_mouse_get_motionacceleration();

    //当前系统指针灵敏度，-1为系统默认
    int mouse_threshold =  kylin_hardware_mouse_get_motionthreshold();

    //当从接口获取的是-1,则代表系统默认值，真实值需要从底层获取
    if (mouse_threshold == -1 || static_cast<int>(mouse_acceleration) == -1){
        // speed sensitivity
        int accel_numerator, accel_denominator, threshold;  //当加速值和灵敏度为系统默认的-1时，从底层获取到默认的具体值

        XGetPointerControl(QX11Info::display(), &accel_numerator, &accel_denominator, &threshold);

        kylin_hardware_mouse_set_motionacceleration(static_cast<double>(accel_numerator/accel_denominator));

        kylin_hardware_mouse_set_motionthreshold(threshold);
    }

    //初始化指针速度控件
    ui->pointerSpeedSlider->blockSignals(true);
    ui->pointerSpeedSlider->setValue(static_cast<int>(kylin_hardware_mouse_get_motionacceleration()*100));
    ui->pointerSpeedSlider->blockSignals(false);

    //初始化指针敏感度控件
    ui->pointerSensitivitySlider->blockSignals(true);
    ui->pointerSensitivitySlider->setValue(kylin_hardware_mouse_get_motionthreshold()*100);
    ui->pointerSensitivitySlider->blockSignals(false);

    //初始化可见性控件
    visiblityBtn->blockSignals(true);
    visiblityBtn->setChecked(kylin_hardware_mouse_get_locatepointer());
    visiblityBtn->blockSignals(false);

    //初始化指针大小
    int sizeIndex = ui->pointerSizeComBox->findData(kylin_hardware_mouse_get_cursorsize());
    ui->pointerSizeComBox->blockSignals(true);
    ui->pointerSizeComBox->setCurrentIndex(sizeIndex);
    ui->pointerSizeComBox->blockSignals(false);
}

void MouseControl::initCursorStatus(){

}
