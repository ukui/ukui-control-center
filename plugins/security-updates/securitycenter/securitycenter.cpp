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

#include "securitycenter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QProcess>
#include <QDebug>
#include <QFontMetrics>
#include <locale.h>
#include <libintl.h>

SecurityCenter::SecurityCenter() : mFirstLoad(true)
{
    pluginName = tr("Security Center");
    pluginType = UPDATE;

    setlocale(LC_ALL, "");
    bindtextdomain("ksc-defender", "/usr/share/locale");
    bind_textdomain_codeset("ksc-defender", "UTF-8");
    textdomain("ksc-defender");
}

SecurityCenter::~SecurityCenter()
{
    if (!mFirstLoad) {

    }
}

QString SecurityCenter::get_plugin_name(){
    return pluginName;
}

int SecurityCenter::get_plugin_type(){
    return pluginType;
}

QWidget * SecurityCenter::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new ksc_main_page_widget();
//        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
//        ui->setupUi(pluginWidget);

//        initSearchText();
//        initComponent();

//        connect(ui->pushButton, &QPushButton::clicked, [=]{
//            QString cmd = "/usr/sbin/ksc-defender";
//            runExternalApp(cmd);
//        });
    }else{
        pluginWidget->refresh_data();
    }

    return pluginWidget;
}

void SecurityCenter::plugin_delay_control(){

}

const QString SecurityCenter::name() const {

    return QStringLiteral("securitycenter");
}
