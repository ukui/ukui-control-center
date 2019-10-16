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
#ifndef KEYBOARDCONTROL_H
#define KEYBOARDCONTROL_H

#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QWidget>

#include "kylin-keyboard-interface.h"
#include "kylin-interface-interface.h"
#include <QGSettings/QGSettings>

#include "../../pluginsComponent/switchbutton.h"
#include "../../pluginsComponent/customwidget.h"

//#include "keyboardlayout.h"
#include "kbdlayoutmanager.h"
#include "customlineedit.h"
#include "keymap.h"
#include "addshortcut.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
//#include <gio/gdesktopappinfo.h>
#include <dconf/dconf.h>
}

namespace Ui {
class KeyboardControl;
}

enum KeyMapping{

};

class KeyboardControl : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    KeyboardControl();
    ~KeyboardControl() Q_DECL_OVERRIDE;

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget *get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    void component_init();
    void status_init();

    void rebuild_layouts_component();

    void append_keys_from_desktop();
    void append_keys_from_system();
    void append_keys_from_custom();

    void rebuild_item();
    void fill_item_from_desktop();
    void fill_item_from_custom();

    QList<char*> _list_exists_custom_gsettings_dir();
    QString find_free_custom_gsettings_path();

    bool key_is_forbidden(QString key);

    QString binding_from_string(QString keyString);
    QString binding_name(QList<int> shortcutList);

    void remove_custom_shortcut();
    void add_custom_shortcut(QString path, QString name, QString exec);
    void update_custom_shortcut(int row, int column);

private:
    Ui::KeyboardControl *ui;

    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

    QGSettings * kbdsettings;
    QGSettings * desktopsettings;
    QGSettings * syskeysettings;

    SwitchButton * keySwitchBtn;
    SwitchButton * cursorSwitchBtn;

//    KeyboardLayout * kbl;
    KbdLayoutManager * layoutmanagerObj;

    QStringList desktopshortcut;

    AddShortcut * adddialog;

    KeyMap * keymapobj;

private slots:
//    void activate_key_repeat_changed_slot(bool status);
    void layout_combobox_changed_slot(int index);
    void receive_shortcut_slot(QList<int> shortcutList);

    void cursor_blink_active_slot(bool status);
    void key_repeat_active_slot(bool status);
};

#endif // KEYBOARDCONTROL_H
