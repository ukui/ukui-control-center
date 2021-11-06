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
#include "networkaccount.h"

networkaccount::networkaccount() : mFirstLoad(true)
{
    pluginName = tr("Cloud Account");
    pluginType = ACCOUNT;
}

QString networkaccount::plugini18nName() {
    return pluginName;
}

int networkaccount::pluginTypes() {
    return pluginType;
}

QWidget * networkaccount::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new MainWidget();
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    }
    return pluginWidget;
}

const QString networkaccount::name() const {

    return QStringLiteral("Cloud Account");
}

bool networkaccount::isShowOnHomePage() const
{
    return true;
}

QIcon networkaccount::icon() const
{
    return QIcon();
}

bool networkaccount::isEnable() const
{
    return true;
}
