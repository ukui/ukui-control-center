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
#include "modulepagewidget.h"
#include "ui_modulepagewidget.h"

#include <QListWidgetItem>
#include <QDebug>

#include "mainwindow.h"
#include "interface.h"
#include "utils/keyvalueconverter.h"
#include "utils/functionselect.h"
#include "utils/utils.h"
#include "component/leftwidgetitem.h"
#include "component/leftmenulist.h"
#include <QScrollBar>

ModulePageWidget::ModulePageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModulePageWidget)
{
    ui->setupUi(this);
    initUI();
}

ModulePageWidget::~ModulePageWidget()
{
    delete ui;
    ui = nullptr;
}

void ModulePageWidget::initUI() {
    //设置伸缩策略

    QSizePolicy rightSizePolicy = ui->widget->sizePolicy();

    rightSizePolicy.setHorizontalStretch(5);

    ui->widget->setSizePolicy(rightSizePolicy);
    ui->widget->setObjectName("widget");
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    connect(ui->scrollArea->verticalScrollBar(),&QScrollBar::rangeChanged,this, [=](){
        if (ui->scrollArea->verticalScrollBar()->maximum() == 0) {
            ui->scrollArea->verticalScrollBar()->setHidden(true);
        } else {
            ui->scrollArea->verticalScrollBar()->setHidden(false);
        }
    });
    connect(ui->scrollArea->horizontalScrollBar(), &QScrollBar::rangeChanged, this, [=](){
        if (ui->scrollArea->horizontalScrollBar()->maximum() == 0) {
            emit hScrollBarHide();
        } else {
            emit hScrollBarShow();
        }
    });

    //设置qss之后,点击屏保再主页进入插件会卡死,原因未知
//    ui->scrollArea->setStyleSheet("QScrollArea{background-color: palette(window);}");
    ui->scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
}

void ModulePageWidget::switchPage(QObject *plugin, bool recorded){

    CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
    refreshPluginWidget(pluginInstance);
}

void ModulePageWidget::refreshPluginWidget(CommonInterface *plu){
    if (plu->pluginBtn) {
        plu->pluginBtn->setChecked(true);
    }
    prePlugin = currentPlugin;
    if (prePlugin) {
        prePlugin->plugin_leave();
    }
    currentPlugin = plu;
    ui->scrollArea->takeWidget();
    delete(ui->scrollArea->widget());

    QWidget *pluginwidget =  plu->pluginUi();
    pluginwidget->setContentsMargins(40,0,20,40);
    ui->scrollArea->setWidget(pluginwidget);
    mCurrentPluName = plu->name();

    //记录打开历史
    if (flagBit){
        FunctionSelect::pushRecordValue(plu->pluginTypes(), plu->plugini18nName());
    }

    //恢复标志位
    flagBit = true;
    if (plu->isIntel()) {
        plu->settingForIntel();
    }
}

void ModulePageWidget::pluginLeave()
{
    if (currentPlugin) {
        currentPlugin->plugin_leave();
    }
}

