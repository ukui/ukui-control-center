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
#include "power.h"
#include "ui_power.h"
#include "powermacrodata.h"

#include <QDebug>

typedef enum {
    BALANCE,
    SAVING,
    CUSTDOM
}MODE;

typedef enum {
    PRESENT,
    ALWAYS
}ICONDISPLAY;

Power::Power()
{
    ui = new Ui::Power;
    itemDelege = new QStyledItemDelegate();
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("power");
    pluginType = SYSTEM;

    const QByteArray id(POWERMANAGER_SCHEMA);
    settings = new QGSettings(id);

    pluginWidget->setStyleSheet("background: #ffffff;");

    ui->balanceWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->savingWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");
    ui->customWidget->setStyleSheet("QWidget{background: #F4F4F4; border-top-left-radius: 6px; border-top-right-radius: 6px;}");
    ui->custom1Widget->setStyleSheet("QWidget{background: #F4F4F4;}");
    ui->custom2Widget->setStyleSheet("QWidget{background: #F4F4F4;}"
                                     "QWidget#custom2Widget{border-bottom-left-radius: 6px; border-bottom-right-radius: 6px;}");

    ui->acBtn->setStyleSheet("QPushButton#acBtn:checked{background: #3D6BE5; border-radius: 4px; color: #ffffff;}");
    ui->batteryBtn->setStyleSheet("QPushButton#batteryBtn:checked{background: #3D6BE5; border-radius: 4px; color: #ffffff;}");

    component_init();
    status_init();
}

Power::~Power()
{
    delete ui;
    delete settings;
}

QString Power::get_plugin_name(){
    return pluginName;
}

int Power::get_plugin_type(){
    return pluginType;
}

QWidget * Power::get_plugin_ui(){
    return pluginWidget;
}

void Power::plugin_delay_control(){

}

void Power::component_init(){
    //
    ui->powerModeBtnGroup->setId(ui->balanceRadioBtn, BALANCE);
    ui->powerModeBtnGroup->setId(ui->savingRadioBtn, SAVING);
    ui->powerModeBtnGroup->setId(ui->custdomRadioBtn, CUSTDOM);

    //电脑睡眠延迟
    sleepStringList  << tr("never") << tr("10 min") << tr("30 min") << tr("60 min") << tr("120 min") << tr("300 min");
//    ui->sleepComboBox->addItems(sleepStringList);
    ui->sleepComboBox->setItemDelegate(itemDelege);
    ui->sleepComboBox->setMaxVisibleItems(5);
    ui->sleepComboBox->insertItem(0, sleepStringList.at(0), QVariant::fromValue(0));
    ui->sleepComboBox->insertItem(1, sleepStringList.at(1), QVariant::fromValue(10));
    ui->sleepComboBox->insertItem(2, sleepStringList.at(2), QVariant::fromValue(30));
    ui->sleepComboBox->insertItem(3, sleepStringList.at(3), QVariant::fromValue(60));
    ui->sleepComboBox->insertItem(4, sleepStringList.at(4), QVariant::fromValue(120));
    ui->sleepComboBox->insertItem(5, sleepStringList.at(5), QVariant::fromValue(300));


    //显示器关闭延迟
    closeStringList  << tr("never") << tr("1 min") << tr("5 min") << tr("10 min") << tr("30 min") << tr("60 min");
//    ui->closeComboBox->addItems(closeStringList);
    ui->closeComboBox->setItemDelegate(itemDelege);
    ui->closeComboBox->setMaxVisibleItems(4);
    ui->closeComboBox->insertItem(0, closeStringList.at(0), QVariant::fromValue(0));
    ui->closeComboBox->insertItem(1, closeStringList.at(1), QVariant::fromValue(1));
    ui->closeComboBox->insertItem(2, closeStringList.at(2), QVariant::fromValue(5));
    ui->closeComboBox->insertItem(3, closeStringList.at(3), QVariant::fromValue(10));
    ui->closeComboBox->insertItem(4, closeStringList.at(4), QVariant::fromValue(30));
    ui->closeComboBox->insertItem(5, closeStringList.at(5), QVariant::fromValue(60));


    //默认电源
    ui->acBtn->setChecked(true);
    connect(ui->powerTypeBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [=]{
        status_init();
    });

    connect(ui->sleepComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
        Q_UNUSED(index)
        int value = ui->sleepComboBox->currentData(Qt::UserRole).toInt() * 60;
        if (ui->acBtn->isChecked()){
            settings->set(SLEEP_COMPUTER_AC_KEY, QVariant(value));
        }
        if (ui->batteryBtn->isChecked()){
            settings->set(SLEEP_COMPUTER_BATT_KEY, QVariant(value));
        }
    });
    connect(ui->closeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
        Q_UNUSED(index)
        int value = ui->closeComboBox->currentData(Qt::UserRole).toInt() * 60;
        if (ui->acBtn->isChecked()){
            settings->set(SLEEP_DISPLAY_AC_KEY, QVariant(value));
        }
        if (ui->batteryBtn->isChecked()){
            settings->set(SLEEP_DISPLAY_BATT_KEY, QVariant(value));
        }
    });

    //lid
//    lidStringList << tr("nothing") << tr("blank") << tr("suspend") << tr("shutdown");
//    ui->aclidComboBox->addItems(lidStringList);
//    ui->batlidComboBox->addItems(lidStringList);

    //button
//    buttonStringList << tr("interactive") << tr("suspend") << tr("shutdown");
//    ui->powerbtnComboBox->addItems(buttonStringList);
//    ui->suspendComboBox->addItems(buttonStringList);

    //
//    ui->icondisplayBtnGroup->setId(ui->presentRadioBtn, PRESENT);
//    ui->icondisplayBtnGroup->setId(ui->alwaysRadioBtn, ALWAYS);

    //平衡和节能模式暂未开放
    ui->custdomRadioBtn->setChecked(true);
    ui->balanceRadioBtn->setCheckable(false);
    ui->savingRadioBtn->setCheckable(false);
//    refreshUI();
}

void Power::status_init(){
    //信号阻塞
    ui->sleepComboBox->blockSignals(true);
    ui->closeComboBox->blockSignals(true);

    if (ui->acBtn->isChecked()){
        //计算机睡眠延迟
        int acsleep = settings->get(SLEEP_COMPUTER_AC_KEY).toInt();
        QString acsleepString;
        if (acsleep != 0)
            acsleepString = QString("%1 min").arg(QString::number(acsleep/FIXES));
        else
            acsleepString = tr("never");
        ui->sleepComboBox->setCurrentText(acsleepString);

        //显示器关闭延迟
        int acclose = settings->get(SLEEP_DISPLAY_AC_KEY).toInt();
        QString accloseString;
        if (acclose != 0)
            accloseString = QString("%1 min").arg(QString::number(acclose/FIXES));
        else
            accloseString = tr("never");
        ui->closeComboBox->setCurrentText(accloseString);
    }

    if (ui->batteryBtn->isChecked()){

        int batsleep = settings->get(SLEEP_COMPUTER_BATT_KEY).toInt();
        QString batsleepString;
        if (batsleep != 0)
            batsleepString = QString("%1 min").arg(QString::number(batsleep/FIXES));
        else
            batsleepString = tr("never");
        ui->sleepComboBox->setCurrentText(batsleepString);

        int batclose = settings->get(SLEEP_DISPLAY_BATT_KEY).toInt();
        QString batcloseString;
        if (batclose != 0)
            batcloseString = QString("%1 min").arg(QString::number(batclose/FIXES));
        else
            batcloseString = tr("never");
        ui->closeComboBox->setCurrentText(batcloseString);
    }

    //信号阻塞解除
    ui->sleepComboBox->blockSignals(false);
    ui->closeComboBox->blockSignals(false);

    //lid 枚举类型但是toint为零只能toString
//    QString aclidString = settings->get(BUTTON_LID_AC_KEY).toString();
//    ui->aclidComboBox->setCurrentText(aclidString);
//    QString batlidString = settings->get(BUTTON_LID_BATT_KET).toString();
//    ui->batlidComboBox->setCurrentText(batlidString);

    //power button
//    QString powerbtn = settings->get(BUTTON_POWER_KEY).toString();
//    ui->powerbtnComboBox->setCurrentText(powerbtn);
//    QString suspentbtn = settings->get(BUTTON_SUSPEND_KEY).toString();
//    ui->suspendComboBox->setCurrentText(suspentbtn);

    //电池图标  枚举类型但是toint为零只能toString
//    QString ipvalue = settings->get(ICONPOLICY).toString();
//    if (ipvalue == PRESENT_VALUE)
//        ui->presentRadioBtn->setChecked(true);
//    else if (ipvalue == ALWAYS_VALUE)
//        ui->alwaysRadioBtn->setChecked(true);

//    connect(ui->powerBtnGroup, SIGNAL(buttonClicked(int)), this, SLOT(powerBtnGroup_changed_slot(int)));
////    connect(ui->icondisplayBtnGroup, SIGNAL(buttonClicked(int)), this, SLOT(icondisplayBtnGroup_changed_slot(int)));

//    connect(ui->acsleepComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(ac_sleep_changed_slot(QString)));
//    connect(ui->batsleepComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(bat_sleep_changed_slot(QString)));
//    connect(ui->accloseComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(ac_close_changed_slot(QString)));
//    connect(ui->batcloseComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(bat_close_changed_slot(QString)));
//    connect(ui->aclidComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(ac_lid_changed_slot(QString)));
//    connect(ui->batlidComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(bat_lid_changed_slot(QString)));

//    connect(ui->powerbtnComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(power_btn_changed_slot(QString)));
//    connect(ui->suspendComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(suspend_btn_changed_slot(QString)));
}

void Power::refreshUI(){
//    if (ui->powerBtnGroup->checkedId() != CUSTDOM)
//        ui->widget->setEnabled(false);
//    else
//        ui->widget->setEnabled(true);
}

void Power::power_btn_changed_slot(QString value){
    settings->set(BUTTON_POWER_KEY, QVariant(value));
}

void Power::suspend_btn_changed_slot(QString value){
    settings->set(BUTTON_SUSPEND_KEY, QVariant(value));
}

void Power::ac_lid_changed_slot(QString value){
    settings->set(BUTTON_LID_AC_KEY, QVariant(value));
}

void Power::bat_lid_changed_slot(QString value){
    settings->set(BUTTON_LID_BATT_KET, QVariant(value));
}

void Power::ac_sleep_changed_slot(QString value){
    qDebug() << "--->" << value;
//    int setvalue;
//    if (value == "never")
//        setvalue = 0;
//    else{
//        QString num = value.split(' ')[0];
//        setvalue = (num.toInt()) * 60;
//    }
//    settings->set(SLEEP_COMPUTER_AC_KEY, QVariant(setvalue));
}

void Power::bat_sleep_changed_slot(QString value){
    int setvalue;
    if (value == "never")
        setvalue = 0;
    else{
        QString num = value.split(' ')[0];
        setvalue = (num.toInt()) * 60;
    }
    settings->set(SLEEP_COMPUTER_BATT_KEY, QVariant(setvalue));
}

void Power::ac_close_changed_slot(QString value){
    int setvalue;
    if (value == "never")
        setvalue = 0;
    else{
        QString num = value.split(' ')[0];
        setvalue = (num.toInt()) * 60;
    }
    settings->set(SLEEP_DISPLAY_AC_KEY, QVariant(setvalue));
}

void Power::bat_close_changed_slot(QString value){
    int setvalue;
    if (value == "never")
        setvalue = 0;
    else{
        QString num = value.split(' ')[0];
        setvalue = (num.toInt()) * 60;
    }
    settings->set(SLEEP_DISPLAY_BATT_KEY, QVariant(setvalue));
}

//void Power::icondisplayBtnGroup_changed_slot(int index){
//    if (index == PRESENT)
//        settings->set(ICONPOLICY, QVariant(PRESENT_VALUE));
//    else if (index == ALWAYS)
//        settings->set(ICONPOLICY, QVariant(ALWAYS_VALUE));
//}

void Power::powerBtnGroup_changed_slot(int index){
    if (index == BALANCE){

    }
    else if (index == SAVING)
    {

    }
    else{

    }
    refreshUI();
}
