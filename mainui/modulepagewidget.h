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
#ifndef MODULEPAGEWIDGET_H
#define MODULEPAGEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QHBoxLayout>

#include "interface.h"

#include "mainComponent/listwidgetitem.h"

class MainWindow;

namespace Ui {
class ModulePageWidget;
}

class ModulePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModulePageWidget(QWidget *parent = 0);
    ~ModulePageWidget();

    void initUI();
    void switch_modulepage(QObject * plugin, int page = 0);
    void update_backbtn_text(int index);
    void update_plugin_widget(CommonInterface * plu, int page);

private:
    Ui::ModulePageWidget *ui;

    MainWindow * pmainWindow;

    QMap<QString, CommonInterface*> pluginInstanceMap;

    QLabel * backiconLabel;
    QLabel * backtextLabel;

public slots:
    void itemClicked_cb(QListWidgetItem * item);

    void backBtnClicked_cb();
    void update_backbtn_text_slot(int index);
    void toggle_plugin_slot(QString pluginname, int plugintype, int page);
};

#endif // MODULEPAGEWIDGET_H
