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

#ifndef SECURITYCENTER_H
#define SECURITYCENTER_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include <QDBusInterface>
#include <QDBusArgument>
#include <QList>

#include <QMouseEvent>
#include <QLabel>
#include <QTimer>

#include "shell/interface.h"
#include "FlowLayout/flowlayout.h"
#include "ksc_main_page_widget.h"

class SecurityCenter : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    SecurityCenter();
    ~SecurityCenter();

public:
    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;

private:
    QString pluginName;
    int pluginType;
    ksc_main_page_widget * pluginWidget;

    bool mFirstLoad;

};
#endif // SECURITYCENTER_H
