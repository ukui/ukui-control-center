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
#include "changetypedialog.h"
#include "ui_changetypedialog.h"

#include <QDebug>

ChangeTypeDialog::ChangeTypeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeTypeDialog)
{
    ui->setupUi(this);

    autologinSwitchBtn = new SwitchButton;
    autologinSwitchBtn->setAttribute(Qt::WA_DeleteOnClose);
    ui->autologinVLayout->addWidget(autologinSwitchBtn);


    ui->buttonGroup->setId(ui->standardRadioButton, 0);
    ui->buttonGroup->setId(ui->adminRadioButton, 1);

    ui->confirmPushBtn->setEnabled(false);

    connect(ui->cancelPushBtn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->confirmPushBtn, SIGNAL(clicked()), this, SLOT(confirm_slot()));
}

ChangeTypeDialog::~ChangeTypeDialog()
{
    delete ui;
}

void ChangeTypeDialog::set_face_label(QString iconfile){
    ui->faceLabel->setPixmap(QPixmap(iconfile).scaled(QSize(80, 80)));
}

void ChangeTypeDialog::set_username_label(QString username){
    ui->usernameLabel->setText(username);
}

void ChangeTypeDialog::set_account_type_label(QString atype){
    ui->typeLabel->setText(atype);
}

void ChangeTypeDialog::set_current_account_type(int id){
    currenttype = id;
    if (id == 0)
        ui->standardRadioButton->setChecked(true);
    else
        ui->adminRadioButton->setChecked(true);

    connect(ui->buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(radioBtn_clicked_slot(int)));
}

void ChangeTypeDialog::set_autologin_status(bool status){
    currentloginstatus = status;
    autologinSwitchBtn->setChecked(status);

    connect(autologinSwitchBtn, SIGNAL(checkedChanged(bool)), this, SLOT(autologin_status_changed_slot(bool)));
}

void ChangeTypeDialog::autologin_status_changed_slot(bool status){
    if (status != currentloginstatus)
        ui->confirmPushBtn->setEnabled(true);
    else
        ui->confirmPushBtn->setEnabled(false);
}

void ChangeTypeDialog::radioBtn_clicked_slot(int id){
    if (id != currenttype)
        ui->confirmPushBtn->setEnabled(true);
    else
        ui->confirmPushBtn->setEnabled(false);
}

void ChangeTypeDialog::confirm_slot(){
    this->accept();
    emit type_send(ui->buttonGroup->checkedId(), ui->usernameLabel->text(), autologinSwitchBtn->isChecked());
}
