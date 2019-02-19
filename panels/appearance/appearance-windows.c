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

GtkWidget * computerCheck;
GtkWidget * homeCheck;
GtkWidget * networkCheck;
GtkWidget * trashCheck;
GtkWidget * volumesCheck;

#define COMPUTER_ICON "computer-icon-visible"
#define HOME_ICON "home-icon-visible"
#define NETWORK_ICON "network-icon-visible"
#define TRASH_ICON "trash-icon-visible"
#define VOLUMES "volumes-visible"

gboolean check_state_set_cb(GtkWidget * widget, gpointer userdata){

    const gchar * key = (const gchar *) userdata;
    g_settings_set_boolean(settings, key, gtk_toggle_button_get_active(widget));
}

static void check_init(){
    gtk_toggle_button_set_active(computerCheck, g_settings_get_boolean(settings, COMPUTER_ICON));
    gtk_toggle_button_set_active(homeCheck, g_settings_get_boolean(settings, HOME_ICON));
    gtk_toggle_button_set_active(networkCheck, g_settings_get_boolean(settings, NETWORK_ICON));
    gtk_toggle_button_set_active(trashCheck, g_settings_get_boolean(settings, TRASH_ICON));
    gtk_toggle_button_set_active(volumesCheck, g_settings_get_boolean(settings, VOLUMES));
}

void windows_init(AppearanceData * data){
    builder = data->ui;

    settings = g_settings_new("org.ukui.peony.desktop");

    computerCheck = GTK_WIDGET(gtk_builder_get_object(builder, "computerCheck"));
    g_signal_connect(G_OBJECT(computerCheck), "toggled", G_CALLBACK(check_state_set_cb), COMPUTER_ICON);

    homeCheck = GTK_WIDGET(gtk_builder_get_object(builder, "homeCheck"));
    g_signal_connect(G_OBJECT(homeCheck), "toggled", G_CALLBACK(check_state_set_cb), HOME_ICON);

    networkCheck = GTK_WIDGET(gtk_builder_get_object(builder, "networkCheck"));
    g_signal_connect(G_OBJECT(networkCheck), "toggled", G_CALLBACK(check_state_set_cb), NETWORK_ICON);

    trashCheck = GTK_WIDGET(gtk_builder_get_object(builder, "trashCheck"));
    g_signal_connect(G_OBJECT(trashCheck), "toggled", G_CALLBACK(check_state_set_cb), TRASH_ICON);

    volumesCheck = GTK_WIDGET(gtk_builder_get_object(builder, "volumesCheck"));
    g_signal_connect(G_OBJECT(volumesCheck), "toggled", G_CALLBACK(check_state_set_cb), VOLUMES);

    check_init();
}

void windows_shutdown(AppearanceData *data){

}
