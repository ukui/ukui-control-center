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

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>

#include "ukccsessionserver.h"
#include "session_adaptor.h"
#include "screenStruct.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("Kylin Team");
    app.setApplicationName("ukcc-session-service");
    qRegisterMetaType<ScreenConfig>("ScreenConfig");
    qDBusRegisterMetaType<ScreenConfig>();

    ukccSessionServer service;
    new InterfaceAdaptor(&service);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (!sessionBus.registerService("org.ukui.ukcc.session")) {
        qCritical() << "QDbus register service failed reason:" << sessionBus.lastError();
        exit(1);
    }

    if (!sessionBus.registerObject("/", &service)) {
        qCritical() << "QDbus register object failed reason:" << sessionBus.lastError();
        exit(2);
    }
    return app.exec();
}
