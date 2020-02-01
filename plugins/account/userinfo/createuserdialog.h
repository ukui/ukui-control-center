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
#ifndef CREATEUSERDIALOG_H
#define CREATEUSERDIALOG_H

#include <QDialog>
#include <QProcess>

#include "SwitchButton/switchbutton.h"

namespace Ui {
class CreateUserDialog;
}

class CreateUserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateUserDialog(QStringList userlist, QWidget *parent = 0);
    ~CreateUserDialog();

    void set_face_label(QString iconfile);

    void confirm_btn_status_refresh();
    void tiplabel_status_refresh();

private:
    Ui::CreateUserDialog *ui;

    SwitchButton * autologinSwitchBtn;

    int ostype;

    QProcess * process;
    bool back;
    QStringList usersStringList;

private slots:
    void confirm_btn_clicked_slot();
    void name_legality_check_slot(QString username);
    void pwd_legality_check_slot(QString pwd);
    void pwdsure_legality_check_slot(QString pwdsure);
    void pin_legality_check_slot(QString pin);
    void pinsure_legality_check_slot(QString pinsure);

    void name_conflict_group_slot();

Q_SIGNALS:
    void user_info_send(QString username, QString pwd, QString pin, int atype, bool autologin);
};

#endif // CREATEUSERDIALOG_H
