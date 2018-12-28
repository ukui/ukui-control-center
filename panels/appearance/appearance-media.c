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

GtkWidget * automountSwitch;
GtkWidget * autoopenSwitch;
GtkWidget * runSwitch;

#define AUTO_MOUNT "automount"
#define AUTO_OPEN "automount-open"
#define RUN "autorun-never"


gboolean switch_set_cb(GtkSwitch * widget, gboolean state, gpointer userdata){

    const gchar * key = (const gchar *) userdata;

    g_settings_set_boolean(settings, userdata, state);

}

static void init_switch(){
    gtk_switch_set_active(automountSwitch, g_settings_get_boolean(settings, AUTO_MOUNT));
    gtk_switch_set_active(autoopenSwitch, g_settings_get_boolean(settings, AUTO_OPEN));
    gtk_switch_set_active(runSwitch, g_settings_get_boolean(settings, RUN));
}

void media_init(AppearanceData * data){
    builder = data->ui;

    settings = g_settings_new("org.ukui.media-handling");

    automountSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "automountSwitch"));
    g_signal_connect(G_OBJECT(automountSwitch), "state-set", G_CALLBACK(switch_set_cb), AUTO_MOUNT);

    autoopenSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "autoopenSwitch"));
    g_signal_connect(G_OBJECT(autoopenSwitch), "state-set", G_CALLBACK(switch_set_cb), AUTO_OPEN);

    runSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "runSwitch"));
    g_signal_connect(G_OBJECT(runSwitch), "state-set", G_CALLBACK(switch_set_cb), RUN);

    init_switch();
}

void media_shutdown(AppearanceData *data){

}
