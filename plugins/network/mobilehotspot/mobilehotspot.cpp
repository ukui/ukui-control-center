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
#include "mobilehotspot.h"

#include "../shell/utils/utils.h"

#include <QDebug>


MobileHotspot::MobileHotspot() :  mFirstLoad(true) {
    pluginName = tr("MobileHotspot");
    pluginType = NETWORK;
}

MobileHotspot::~MobileHotspot()
{
    if (!mFirstLoad) {
//        delete pluginWidget;
//        pluginWidget = nullptr;

    }
}

QString MobileHotspot::get_plugin_name() {
    return pluginName;
}

int MobileHotspot::get_plugin_type() {
    return pluginType;
}

QWidget *MobileHotspot::get_plugin_ui() {
    if (mFirstLoad) {
        mFirstLoad = false;

        pluginWidget = new MobileHotspotWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        qDBusRegisterMetaType<QMap<QString, bool> >();
        qDBusRegisterMetaType<QVector<QStringList> >();
        qDBusRegisterMetaType<QMap<QString, QVector<QStringList> >>();
        m_interface = new QDBusInterface("com.kylin.network", "/com/kylin/network",
                                         "com.kylin.network",
                                         QDBusConnection::sessionBus());
        if(!m_interface->isValid()) {
            qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        }

    }
    return pluginWidget;
}

void MobileHotspot::plugin_delay_control() {

}

const QString MobileHotspot::name() const {

    return QStringLiteral("mobilehotspot");
}

