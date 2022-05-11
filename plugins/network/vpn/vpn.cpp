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
#include "vpn.h"
#include "ui_vpn.h"

#include <QProcess>
#include <QMouseEvent>

Vpn::Vpn() : mFirstLoad(true)
{
    pluginName = tr("Vpn");
    pluginType = NETWORK;
}

Vpn::~Vpn()
{
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
    }
}

QString Vpn::plugini18nName(){
    return pluginName;
}

int Vpn::pluginTypes(){
    return pluginType;
}

QWidget *Vpn::pluginUi(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Vpn;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        initComponent();
    }
    return pluginWidget;
}

const QString Vpn::name() const {

    return QStringLiteral("Vpn");
}

bool Vpn::isShowOnHomePage() const
{
    return true;
}

QIcon Vpn::icon() const
{
    return QIcon::fromTheme("ukui-vpn-symbolic");
}

bool Vpn::isEnable() const
{
    return true;
}

void Vpn::initComponent(){
    mAddBtn = new AddBtn(pluginWidget);

    connect(mAddBtn, &AddBtn::clicked, this, [=]() {
        runExternalApp();
    });

    ui->addLyt->addWidget(mAddBtn);
}

void Vpn::runExternalApp(){
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}
