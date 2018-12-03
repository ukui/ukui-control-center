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

GtkWidget * computerSwitch;
GtkWidget * homeSwitch;
GtkWidget * networkSwitch;
GtkWidget * trashSwitch;
GtkWidget * volumesSwitch;

#define COMPUTER_ICON "computer-icon-visible"
#define HOME_ICON "home-icon-visible"
#define NETWORK_ICON "network-icon-visible"
#define TRASH_ICON "trash-icon-visible"
#define VOLUMES "volumes-visible"



gboolean switch_state_set_cb(GtkSwitch * widget, gboolean state, gpointer userdata){

    const gchar * key = (const gchar *) userdata;

    g_settings_set_boolean(settings, userdata, state);

}

static void init_switch(){
    gtk_switch_set_active(computerSwitch, g_settings_get_boolean(settings, COMPUTER_ICON));
    gtk_switch_set_active(homeSwitch, g_settings_get_boolean(settings, HOME_ICON));
    gtk_switch_set_active(networkSwitch, g_settings_get_boolean(settings, NETWORK_ICON));
    gtk_switch_set_active(trashSwitch, g_settings_get_boolean(settings, TRASH_ICON));
    gtk_switch_set_active(volumesSwitch, g_settings_get_boolean(settings, VOLUMES));

}

void desktopsettings_init(AppearanceData * data){
    builder = data->ui;

    settings = g_settings_new("org.ukui.peony.desktop");

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

    init_switch();
}

void desktopsettings_shutdown(AppearanceData *data){

}
