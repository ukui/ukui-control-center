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
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->frame->setStyleSheet("QFrame{background: #ffffff; border: none; border-radius: 6px;}");
    ui->closeBtn->setStyleSheet("QPushButton{background: #ffffff; border: none;}");


    ui->closeBtn->setIcon(QIcon("://img/titlebar/close.png"));


    setupComonpent();

}

ChangeTypeDialog::~ChangeTypeDialog()
{
    delete ui;
}

void ChangeTypeDialog::setupComonpent(){
    ui->buttonGroup->setId(ui->standardRadioButton, 0);
    ui->buttonGroup->setId(ui->adminRadioButton, 1);

    ui->confirmPushBtn->setEnabled(false);

    connect(ui->closeBtn, &QPushButton::clicked, [=]{
        close();
    });
    connect(ui->cancelPushBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        reject();
    });
    connect(ui->confirmPushBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        this->accept();
        emit type_send(ui->buttonGroup->checkedId(), ui->usernameLabel->text());
    });
}

void ChangeTypeDialog::setFace(QString faceFile){
    ui->faceLabel->setPixmap(QPixmap(faceFile));
}

void ChangeTypeDialog::setUsername(QString username){
    ui->usernameLabel->setText(username);
}

void ChangeTypeDialog::setCurrentAccountTypeLabel(QString atype){
    ui->typeLabel->setText(atype);
}

void ChangeTypeDialog::setCurrentAccountTypeBtn(int id){
    currenttype = id;
    if (id == 0)
        ui->standardRadioButton->setChecked(true);
    else
        ui->adminRadioButton->setChecked(true);

    connect(ui->buttonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), [=](int id){
        if (id != currenttype)
            ui->confirmPushBtn->setEnabled(true);
        else
            ui->confirmPushBtn->setEnabled(false);
    });
}
