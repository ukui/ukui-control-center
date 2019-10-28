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
#ifndef FONTS_H
#define FONTS_H

#include <QWidget>
#include <QPushButton>
#include <QAbstractButton>
#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QFontDatabase>

#include "../../pluginsComponent/customwidget.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
#include <stdlib.h>
}

#include <QGSettings/QGSettings>

namespace Ui {
class Fonts;
}

class Fonts : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Fonts();
    ~Fonts();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    void component_init();
    void status_init();
    void status_setup();
    void get_default_fontinfo();
    void get_current_fonts();
    QStringList split_fontname_size(QString value);

private:
    Ui::Fonts *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

    QGSettings * ifsettings;
    QGSettings * marcosettings;
    QGSettings * peonysettings;
    QGSettings * rendersettings;

    QStringList gtkfontStrList;
    QStringList docfontStrList;
    QStringList monospacefontStrList;
    QStringList peonyfontStrList;
    QStringList titlebarfontStrList;

    QFontDatabase fontdb;

private slots:
    void reset_default_slot();
    void combobox_changed_slot(QString text);
    void radiobtn_clicked_slot(int indexnum);
    void pushbtn_clicked_slot(QAbstractButton * button);

};

#endif // FONTS_H
