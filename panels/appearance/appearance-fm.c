/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2016 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "appearance-font.h"
#include <glib/gi18n.h>
#include <pango/pangocairo.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <gdk/gdkx.h>
#include <gdk/gdk.h>

static GtkBuilder * builder = NULL;

static GSettings * settings = NULL;
static GSettings * psettings = NULL;

GtkWidget * computerSwitch;
GtkWidget * homeSwitch;
GtkWidget * networkSwitch;
GtkWidget * trashSwitch;
GtkWidget * volumesSwitch;

GtkWidget * delwarningSwitch;
GtkWidget * homedesktopSwitch;
GtkWidget * notificationSwitch;
GtkWidget * viewerCombo;

#define COMPUTER_ICON "computer-icon-visible"
#define HOME_ICON "home-icon-visible"
#define NETWORK_ICON "network-icon-visible"
#define TRASH_ICON "trash-icon-visible"
#define VOLUMES "volumes-visible"

#define DEL_WARNING "confirm-trash"
#define H_D "desktop-is-home-dir"
#define NOTIFICATION "show-notifications"
#define DEFAULT_VIEWER "default-folder-viewer"


gboolean switch_state_set_cb(GtkSwitch * widget, gboolean state, gpointer userdata){

    const gchar * key = (const gchar *) userdata;

    g_settings_set_boolean(settings, userdata, state);

}

gboolean preferences_switch_set_cb(GtkSwitch * widget, gboolean state, gpointer userdata){
    const gchar * key = (const gchar *) userdata;

    g_settings_set_boolean(psettings, userdata, state);
}

static void preferences_combo_changed_cb(GtkComboBox * combobox, gpointer userdata){
    gint current;
    current = gtk_combo_box_get_active(GTK_COMBO_BOX(viewerCombo));
    if (current == -1)
        return;

    if (current == 0)
        g_settings_set_string(psettings, DEFAULT_VIEWER, "icon_view");
    else if (current == 1)
        g_settings_set_string(psettings, DEFAULT_VIEWER, "compact_view");
    else if (current == 2)
        g_settings_set_string(psettings, DEFAULT_VIEWER, "list_view");
    else if (current == 3)
        g_settings_set_string(psettings, DEFAULT_VIEWER, "computer_view");
}

static void init_combo(){
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(viewerCombo), _("icon_view"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(viewerCombo), _("compact_view"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(viewerCombo), _("list_view"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(viewerCombo), _("computer_view"));

    gchar * current;
    gint index;
    current = g_settings_get_string(psettings, DEFAULT_VIEWER);

    if (strcmp(current, "icon-view") == 0)
        index = 0;
    else if (strcmp(current, "compact-view") == 0)
        index = 1;
    else if (strcmp(current, "list-view") == 0)
        index = 2;
    else if (strcmp(current, "computer-view") == 0)
        index = 3;
    else
        index = -1;

    g_signal_handlers_block_by_func(viewerCombo, preferences_combo_changed_cb, NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(viewerCombo), index);
    g_signal_handlers_unblock_by_func(viewerCombo, preferences_combo_changed_cb, NULL);
}

static void init_switch(){
    gtk_switch_set_active(computerSwitch, g_settings_get_boolean(settings, COMPUTER_ICON));
    gtk_switch_set_active(homeSwitch, g_settings_get_boolean(settings, HOME_ICON));
    gtk_switch_set_active(networkSwitch, g_settings_get_boolean(settings, NETWORK_ICON));
    gtk_switch_set_active(trashSwitch, g_settings_get_boolean(settings, TRASH_ICON));
    gtk_switch_set_active(volumesSwitch, g_settings_get_boolean(settings, VOLUMES));

    gtk_switch_set_active(delwarningSwitch, g_settings_get_boolean(psettings, DEL_WARNING));
    gtk_switch_set_active(homedesktopSwitch, g_settings_get_boolean(psettings, H_D));
    gtk_switch_set_active(notificationSwitch, g_settings_get_boolean(psettings, NOTIFICATION));

}

void filemanager_init(AppearanceData * data){
    builder = data->ui;

    settings = g_settings_new("org.ukui.peony.desktop");
    psettings = g_settings_new("org.ukui.peony.preferences");

    computerSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "computerSwitch"));
    g_signal_connect(G_OBJECT(computerSwitch), "state-set", G_CALLBACK(switch_state_set_cb), COMPUTER_ICON);

    homeSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "homeSwitch"));
    g_signal_connect(G_OBJECT(homeSwitch), "state-set", G_CALLBACK(switch_state_set_cb), HOME_ICON);

    networkSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "networkSwitch"));
    g_signal_connect(G_OBJECT(networkSwitch), "state-set", G_CALLBACK(switch_state_set_cb), NETWORK_ICON);

    trashSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "trashSwitch"));
    g_signal_connect(G_OBJECT(trashSwitch), "state-set", G_CALLBACK(switch_state_set_cb), TRASH_ICON);

    volumesSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "volumesSwitch"));
    g_signal_connect(G_OBJECT(volumesSwitch), "state-set", G_CALLBACK(switch_state_set_cb), VOLUMES);



    delwarningSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "delwarningSwitch"));
    g_signal_connect(G_OBJECT(delwarningSwitch), "state-set", G_CALLBACK(preferences_switch_set_cb), DEL_WARNING);

    homedesktopSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "homedesktopSwitch"));
    g_signal_connect(G_OBJECT(homedesktopSwitch), "state-set", G_CALLBACK(preferences_switch_set_cb), H_D);

    notificationSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "notificationSwitch"));
    g_signal_connect(G_OBJECT(notificationSwitch), "state-set", G_CALLBACK(preferences_switch_set_cb), NOTIFICATION);

    viewerCombo = GTK_WIDGET(gtk_builder_get_object(builder, "viewerCombo"));
    g_signal_connect(viewerCombo, "changed", G_CALLBACK(preferences_combo_changed_cb), NULL);

    init_switch();
    init_combo();
}

void filemanager_shutdown(AppearanceData *data){

}
