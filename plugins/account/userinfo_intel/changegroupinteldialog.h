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

#ifndef CHANGEGROUPINTELDIALOG_H
#define CHANGEGROUPINTELDIALOG_H

#include <QDialog>
#include <QPainter>
#include <QPainterPath>
#include <QListWidget>
#include <QDebug>

#include <ukcc/widgets/hoverwidget.h>

//struct custom_struct
//{
//    QString groupname;
//    QString passphrase;
//    QString groupid;
//    QString usergroup;
//};

namespace Ui {
class ChangeGroupIntelDialog;
}

class ChangeGroupIntelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangeGroupIntelDialog(QWidget *parent = nullptr);
    ~ChangeGroupIntelDialog();

public:
    void initGeneralItemsStyle();
    void initItemsStyle(QListWidget * listWidget);
    void initNewGroupBtn();
    void loadGroupInfo();
    void showCreateGroupDialog();

protected:
    void paintEvent(QPaintEvent * event);

private:
    Ui::ChangeGroupIntelDialog *ui;

    HoverWidget *addWgt;
    //QList<custom_struct *>  *value;

    void setupInit();
    void signalsBind();


};

#endif // CHANGEGROUPINTELDIALOG_H