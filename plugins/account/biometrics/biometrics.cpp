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
#include "biometrics.h"

Biometrics::Biometrics() : mFirstLoad(true)
{
    pluginName = tr("Biometrics");
    pluginType = ACCOUNT;
}

Biometrics::~Biometrics()
{

}

QString Biometrics::get_plugin_name()
{
    return pluginName;
}

int Biometrics::get_plugin_type()
{
    return pluginType;
}

QWidget *Biometrics::get_plugin_ui()
{
    if (mFirstLoad) {
        mFirstLoad = false;
        // will delete by takewidget
        pluginWidget = new BiometricsWidget;
    }

    return pluginWidget;
}

void Biometrics::plugin_delay_control()
{

}

const QString Biometrics::name() const
{
    return QStringLiteral("biometrics");
}
