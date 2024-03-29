/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2020 KYLINOS Information Technology Co., Ltd.
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

#ifndef CHANGEGROUPDIALOG_H
#define CHANGEGROUPDIALOG_H

#include <QDialog>
#include <QPainter>
#include <QPainterPath>
#include <QListWidget>
#include <QDebug>
#include <QPushButton>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QDBusReply>
#include <QMessageBox>
#include <QDateTime>
#include <QCoreApplication>
#include <polkit-qt5-1/polkitqt1-authority.h>

#include "HoverWidget/hoverwidget.h"

struct custom_struct
{
    QString groupname;
    QString passphrase;
    QString groupid;
    QString usergroup;

    friend QDBusArgument &operator<<(QDBusArgument &argument, const custom_struct&mystruct)
    {
        argument.beginStructure();
        argument << mystruct.groupname << mystruct.passphrase << mystruct.groupid << mystruct.usergroup;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, custom_struct&mystruct)
    {
        argument.beginStructure();
        argument >> mystruct.groupname >> mystruct.passphrase >> mystruct.groupid >> mystruct.usergroup;
        argument.endStructure();
        return argument;
    }

};

Q_DECLARE_METATYPE(custom_struct)

namespace Ui {
class ChangeGroupDialog;
}

class ChangeGroupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeGroupDialog(QWidget *parent = nullptr);
    ~ChangeGroupDialog();

public:
    void connectToServer();
    void initNewGroupBtn();
    void loadGroupInfo();
    void loadPasswdInfo();
    void loadAllGroup();
    bool polkitEdit();
    bool polkitDel();
    bool polkitAdd();
    void refreshList();

public:
    QDBusInterface *serviceInterface;
    QList<custom_struct *>  *groupList;
    QList<custom_struct *>  *passwdList;

protected:
    void paintEvent(QPaintEvent * event);

private:
    Ui::ChangeGroupDialog *ui;

    HoverWidget *addWgt;

    void setupInit();
    void signalsBind();

private slots:
    void needRefreshSlot();
};

#endif // CHANGEGROUPDIALOG_H
