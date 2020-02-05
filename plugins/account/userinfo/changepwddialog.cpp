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
#include "changepwddialog.h"
#include "ui_changepwddialog.h"

#define PWD_LOW_LENGTH 6

ChangePwdDialog::ChangePwdDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangePwdDialog)
{
    ui->setupUi(this);

    ui->pwdtypeComboBox->addItem(tr("password"));
    ui->pwdtypeComboBox->addItem(tr("pin"));
    // 默认普通密码
    ui->pwdtypeComboBox->setCurrentIndex(0);

    ui->pwdLineEdit->setEchoMode(QLineEdit::Password);
    ui->pwdsureLineEdit->setEchoMode(QLineEdit::Password);

    ui->pwdLineEdit->setPlaceholderText(tr("New Password"));
    ui->pwdsureLineEdit->setPlaceholderText(tr("New Password Identify"));

    confirmbtn_status_refresh();

    connect(ui->pwdLineEdit, SIGNAL(textChanged(QString)), this, SLOT(legality_check_slot(QString)));
    connect(ui->pwdsureLineEdit, SIGNAL(textChanged(QString)), this, SLOT(make_sure_slot(QString)));

    connect(ui->cancelPushBtn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->canfirmPushBtn, SIGNAL(clicked()), this, SLOT(confirm_slot()));
}

ChangePwdDialog::~ChangePwdDialog()
{
    delete ui;
}

void ChangePwdDialog::set_faceLabel(QString iconfile){
    ui->faceLabel->setPixmap(QPixmap(iconfile).scaled(QSize(80, 80)));

}

void ChangePwdDialog::set_usernameLabel(QString username){
    ui->usernameLabel->setText(username);
}

void ChangePwdDialog::set_pwd_type(QString type){
    ui->pwdtypeComboBox->setCurrentText(type);
}

void ChangePwdDialog::confirmbtn_status_refresh(){
    if (ui->pwdLabel->text() != "" || ui->pwdsureLabel->text() != "" || \
            ui->pwdLineEdit->text().isEmpty() || ui->pwdLineEdit->text() == tr("New Password") || \
            ui->pwdsureLineEdit->text().isEmpty() || ui->pwdsureLineEdit->text() == tr("New Password Identify"))
        ui->canfirmPushBtn->setEnabled(false);
    else
        ui->canfirmPushBtn->setEnabled(true);
}

void ChangePwdDialog::confirm_slot(){
    this->accept();
    emit passwd_send(ui->pwdLineEdit->text(), ui->usernameLabel->text());
}

void ChangePwdDialog::legality_check_slot(QString text){
    if (text.length() < PWD_LOW_LENGTH)
        ui->pwdLabel->setText(tr("Password length needs to more than 5 character!"));
    else
        ui->pwdLabel->setText("");

    //防止先输入确认密码，再输入密码后pwdsureLabel无法刷新
    if (ui->pwdLineEdit->text() == ui->pwdsureLineEdit->text())
        ui->pwdsureLabel->setText("");

    confirmbtn_status_refresh();
}

void ChangePwdDialog::make_sure_slot(QString text){
    if (text != ui->pwdLineEdit->text())
        ui->pwdsureLabel->setText(tr("Inconsistency with pwd"));
    else
        ui->pwdsureLabel->setText("");

    confirmbtn_status_refresh();
}
