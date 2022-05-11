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
#include "vino_hw.h"
#include "../../../shell/utils/utils.h"

Vino::Vino() : mFirstLoad(true)
{
    pluginName = tr("Vino");
    pluginType = SYSTEM;
}

Vino::~Vino()
{
}

QString Vino::plugini18nName()
{
    return pluginName;
}

int Vino::pluginTypes()
{
    return pluginType;
}

QWidget *Vino::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        // will delete by takewidget
        pluginWidget = new ShareMain;
    }

    return pluginWidget;
}


const QString Vino::name() const
{
    return QStringLiteral("Vino");
}

bool Vino::isShowOnHomePage() const
{
    return false;
}

QIcon Vino::icon() const
{
    return QIcon::fromTheme("folder-remote-symbolic");
}

bool Vino::isEnable() const
{
    return Utils::isWayland();
}
