/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "upgrade.h"

#include <QWidget>
#include <QMainWindow>

Upgrade::Upgrade() :mFirstLoad(true) {
    pluginName = tr("Upgrade");
    pluginType = UPDATE;
}

Upgrade::~Upgrade() {

}

QString Upgrade::get_plugin_name() {
    return pluginName;
}

int Upgrade::get_plugin_type() {
    return pluginType;
}

QWidget *Upgrade::get_plugin_ui() {
    if (mFirstLoad) {
        mFirstLoad = false;
        // will delete by takewidget
        pluginWidget = new UpgradeMain("");
    }

    return pluginWidget;
}

void Upgrade::plugin_delay_control() {

}

const QString Upgrade::name() const {
    return QStringLiteral("upgrade");
}
