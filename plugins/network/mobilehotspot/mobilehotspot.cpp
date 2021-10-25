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

QString MobileHotspot::plugini18nName() {
    return pluginName;
}

int MobileHotspot::pluginTypes() {
    return pluginType;
}

QWidget *MobileHotspot::pluginUi() {
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

const QString MobileHotspot::name() const {

    return QStringLiteral("MobileHotspot");
}

bool MobileHotspot::isShowOnHomePage() const
{
    return false;
}

QIcon MobileHotspot::icon() const
{
    return QIcon();
}

bool MobileHotspot::isEnable() const
{
    return true;
}

