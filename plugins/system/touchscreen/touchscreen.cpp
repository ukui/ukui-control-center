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

#include "touchscreen.h"
#include "ui_touchscreen.h"


#include <KF5/KScreen/kscreen/getconfigoperation.h>
#include <KF5/KScreen/kscreen/output.h>
#include <QDebug>
#include <QThread>

TouchScreen::TouchScreen() : mFirstLoad(true)
{
    pluginName = tr("TouchScreen");
    pluginType = SYSTEM;
}

TouchScreen::~TouchScreen() {
}

QWidget *TouchScreen::get_plugin_ui() {
    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new Widget;
        QObject::connect(new KScreen::GetConfigOperation(), &KScreen::GetConfigOperation::finished,
                         [&](KScreen::ConfigOperation *op) {
            QThread::usleep(20000);
            pluginWidget->setConfig(qobject_cast<KScreen::GetConfigOperation*>(op)->config());
        });
    }
    return pluginWidget;
}

QString TouchScreen::get_plugin_name() {
    return pluginName;
}

int TouchScreen::get_plugin_type() {
    return pluginType;
}

void TouchScreen::plugin_delay_control() {

}

const QString TouchScreen::name() const {

    return QStringLiteral("touchscreen");
}
