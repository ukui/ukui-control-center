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
#ifndef DISPLAYSET_H
#define DISPLAYSET_H

#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include "../../pluginsComponent/customwidget.h"
#include "../../pluginsComponent/switchbutton.h"

#include <QGraphicsScene>
#include <QProcess>

#include "graphicsitem.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#define MATE_DESKTOP_USE_UNSTABLE_API
#include <mate-rr.h>
#include <mate-rr-config.h>
#include <mate-rr-labeler.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <time.h>
}

#define POWER_MANAGER_SCHEMA "org.ukui.power-manager"
#define BRIGHTNESS_AC_KEY "brightness-ac"

namespace Ui {
class DisplayWindow;
}

typedef struct _Monitor{
    MateRRScreen *screen;
    MateRRConfig  * current_configuration;
    MateRRLabeler * labeler;
    MateRROutputInfo * current_output;

    guint32 ApplyBtnClickTimeStamp;

    DBusGConnection * connection;
    DBusGProxy * proxy;
    DBusGProxyCall * proxy_call;

    GtkWidget * window;
}Monitor;

typedef struct _Edge{
    MateRROutputInfo * output;
    int x1, x2;
    int y1, y2;
}Edge;

typedef struct _Snap{
    Edge * snapper;
    Edge * snappee;
    int dy, dx;
}Snap;

class DisplaySet : public QObject, CommonInterface{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    DisplaySet();
    ~DisplaySet();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    CustomWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

    void monitor_init();
    void component_init();
    void status_init();

    void rebuild_ui();
    void rebuild_view();
    void rebuild_mirror_monitor();
    void rebuild_monitor_switchbtn();
    void rebuild_image_switchbtn();
    void rebuild_monitor_combo();
    void rebuild_resolution_combo();
    void rebuild_rotation_combo();
    void rebuild_refresh_combo();

    double monitor_item_scale();
    void _get_geometry(MateRROutputInfo * output, int * w, int * h);
    gboolean _output_overlaps(MateRROutputInfo * output);
    void layout_outputs_horizontally();
    gboolean _get_clone_size(int *width, int *height);
    gboolean _output_info_supports_mode(MateRROutputInfo * info, int width, int height);
    gboolean _mirror_screen_is_supported();
    int _active_output_count();
    int _connect_output_count();
    void _connect_output_total_resolution(int * total_w, int * total_h);
    void _select_resolution_for_current_output();
    MateRROutputInfo * _get_output_for_window (MateRRConfig *configuration, GdkWindow *window);
    MateRRMode ** _get_current_modes();
    void _realign_output_after_resolution_changed(MateRROutputInfo * output_changed, int oldwidth, int oldheight);
    gboolean _sanitize_save_configuration();
    void _ensure_current_configuration_is_saved();
    void _begin_version2_apply_configuration();
    static void _apply_configuration_callback();

    void _list_edges(bool diff_edges);
    void _list_snaps();

    void _add_edge(MateRROutputInfo * output, int x1, int y1, int x2, int y2, bool diff_edges);
    void _add_edge_snaps(Edge * snapper, Edge * snappee);
    void _add_snap(Snap snap);
    bool _horizontal_overlap(Edge * snapper, Edge * snappee);
    bool _vertical_overlap(Edge * snapper, Edge * snappee);
    bool _overlap(int s1, int e1, int s2, int e2);
    bool _config_is_aligned();
    bool _output_is_aligned(MateRROutputInfo * output);
    bool _edges_align(Edge * e1, Edge * e2);
    bool _corner_on_edge(int x, int y, Edge * e);

    bool support_brightness();
    bool brightness_setup_display();
    bool isNumber(QString str);

private:
    Ui::DisplayWindow * ui;
    QString pluginName;
    int pluginType;
    CustomWidget * pluginWidget;

    QGraphicsScene * scene;

    GSettings  * brightnessgsettings;

    Monitor monitor;

    SwitchButton * activemonitorBtn;
    SwitchButton * mirrormonitorBtn;

private slots:
    void selected_item_changed_slot();
    void output_drabed_slot();

    void monitor_active_changed_slot();
    void mirror_monitor_changed_slot();
    void set_primary_clicked_slot();

    void brightness_value_changed_slot(int value);
    void refresh_changed_slot(int index);
    void rotation_changed_slot(int index);
    void resolution_changed_slot(int index);
    void monitor_changed_slot(int index);

    void apply_btn_clicked_slot();
};

#endif // DISPLAYSET_H
