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
#ifndef CERTIFICATIONDIALOG_H
#define CERTIFICATIONDIALOG_H

#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>

#define HTTP_PROXY_SCHEMA         "org.gnome.system.proxy.http"
#define HTTP_AUTH_KEY         "use-authentication"
#define HTTP_AUTH_USER_KEY        "authentication-user"
#define HTTP_AUTH_PASSWD_KEY      "authentication-password"



namespace Ui {
class CertificationDialog;
}

class SwitchButton;
class QGSettings;

class CertificationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CertificationDialog(QWidget *parent = 0);
    ~CertificationDialog();

public:
    void component_init();
    void status_init();

protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::CertificationDialog *ui;

    QGSettings * cersettings;

    SwitchButton * activeSwitchBtn;
    QFrame * titleFrame;
    QHBoxLayout * titleLayout;
    QLabel * titleLabel;
    QFrame * iconFrame;
    QHBoxLayout * iconLayout;
    QLabel * iconLabel;
    QLineEdit * pwdLineEdit;
    QLineEdit * userLineEdit;
    QFrame * btnFrame;
    QHBoxLayout * btnLayout;
    QPushButton * closePushBtn;
    QPushButton * confirmPushBtn;

private slots:
    void active_status_changed_slot(bool status);

    void user_edit_changed_slot(QString edit);
    void pwd_edit_changed_slot(QString edit);
};

#endif // CERTIFICATIONDIALOG_H
