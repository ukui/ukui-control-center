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
#ifndef THEME_H
#define THEME_H

#include <QWidget>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"
#include <QToolButton>
#include <QSignalMapper>
#include <QMap>
#include <QDir>

#include <QGSettings/QGSettings>

#include "iconthemewidget.h"
#include "../../pluginsComponent/switchbutton.h"
#include "../../pluginsComponent/customwidget.h"

#define DESKTOP_SCHEMA "org.ukui.peony.desktop"

#define COMPUTER_ICON "computer-icon-visible"
#define HOME_ICON "home-icon-visible"
#define NETWORK_ICON "network-icon-visible"
#define TRASH_ICON "trash-icon-visible"
#define VOLUMES_ICON "volumes-visible"

#define INTERFACE_SCHEMA "org.mate.interface"
#define MARCO_SCHEMA "org.gnome.desktop.wm.preferences"

#define GTK_THEME_KEY "gtk-theme"
#define ICON_THEME_KEY "icon-theme"
#define MARCO_THEME_KEY "theme"

namespace Ui {
class Theme;
}

class Theme : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Theme();
    ~Theme();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    void status_init();
    void component_init();
    void refresh_btn_select_status();
    void refresh_icon_theme();

private:
    Ui::Theme *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

    QGSettings * ifsettings;
    QGSettings * marcosettings;
    QGSettings * desktopsettings;

    QMap<QString, QToolButton *> delbtnMap;
    QMap<QString, SwitchButton *> delsbMap;

    QMap<QString, IconThemeWidget *> delframeMap;

public slots:
    void set_theme_slots(QString value);
    void desktop_icon_settings_slots(QString key);
    void icon_theme_changed_slot(QString name);
};

#endif // THEME_H
