/* -*- mode: c; style: linux -*- */

/* keyboard-properties.c
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2001 Jonathan Blandford
 *
 * Written by: Bradford Hovinen <hovinen@ximian.com>
 *             Rachel Hestilow <hestilow@ximian.com>
 *	       Jonathan Blandford <jrb@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include "mate-keyboard-properties.h"
#include <gio/gio.h>
#include <glib.h>
#include <stdlib.h>
#include <mate-settings-daemon/mate-settings-client.h>
#include "mate-keyboard-properties-a11y.h"
#include "mate-keyboard-properties-xkb.h"
#include "mate-keybinding-properties.h"
#define KEYBOARD_SCHEMA "org.mate.peripherals-keyboard"
#define INTERFACE_SCHEMA "org.mate.interface"
#define TYPING_BREAK_SCHEMA "org.mate.typing-break"
#define MWID(s) GTK_WIDGET(gtk_builder_get_object(builder, s))
enum {
	RESPONSE_APPLY = 1,
	RESPONSE_CLOSE
};

static GSettings * keyboard_settings = NULL;
static GSettings * interface_settings = NULL;
static GSettings * typing_break_settings = NULL;

gboolean activate_settings_daemon();

gboolean activate_settings_daemon()
{
    DBusGConnection * connection = NULL;
    DBusGProxy * proxy =NULL;
    GError * error = NULL;

    connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if (connection == NULL) 
    {
        g_warning("keyboard connect error\n");
        g_error_free(error);
        return FALSE;
    }
    proxy = dbus_g_proxy_new_for_name(connection,
                                      "org.mate.SettingsDaemon",
                                      "/org/mate/SettingsDaemon",
                                      "org.mate.SettingsDaemon");
    dbus_g_connection_unref(connection);
    if (proxy ==NULL) 
    {
        g_warning("Keyboard's SettingsDaemon proxy get error");
        return FALSE;
    }
    if (!org_mate_SettingsDaemon_awake(proxy, &error))
    {
        g_warning("awake SettingsDaemon error%s\n",error->message);
        return FALSE;
    }
    return TRUE;
}

void
create_dialog (GtkBuilder * builder)
{
	GtkSizeGroup *size_group;
	GtkWidget *image;

	size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
    	gtk_size_group_add_widget (size_group, MWID ("repeat_slow_label"));
    	gtk_size_group_add_widget (size_group, MWID ("delay_short_label"));
    	gtk_size_group_add_widget (size_group, MWID ("blink_slow_label"));
	g_object_unref (G_OBJECT (size_group));

	size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
    	gtk_size_group_add_widget (size_group, MWID ("repeat_fast_label"));
    	gtk_size_group_add_widget (size_group, MWID ("delay_long_label"));
    	gtk_size_group_add_widget (size_group, MWID ("blink_fast_label"));
	g_object_unref (G_OBJECT (size_group));

	size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
    	gtk_size_group_add_widget (size_group, MWID ("repeat_delay_scale"));
    	gtk_size_group_add_widget (size_group, MWID ("repeat_speed_scale"));
    	gtk_size_group_add_widget (size_group, MWID ("cursor_blink_time_scale"));
	g_object_unref (G_OBJECT (size_group));

	image = gtk_image_new_from_stock (GTK_STOCK_ADD, GTK_ICON_SIZE_BUTTON);
    	gtk_button_set_image (GTK_BUTTON (MWID ("xkb_layouts_add")), image);

	image = gtk_image_new_from_stock (GTK_STOCK_REFRESH, GTK_ICON_SIZE_BUTTON);
    	gtk_button_set_image (GTK_BUTTON (MWID ("xkb_reset_to_defaults")), image);
}

void
setup_dialog (GtkBuilder * builder)
{
	gchar *monitor;

    GtkWidget *label_entry =  GTK_WIDGET(gtk_builder_get_object(builder, "label20"));
    gtk_label_set_xalign(GTK_LABEL(label_entry), 0.0);
	g_settings_bind (keyboard_settings,
			 "repeat",
                     	 MWID ("repeat_toggle"),
			 "active",
			 G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (keyboard_settings,
			 "repeat",
                     	 MWID ("repeat_table"),
			 "sensitive",
			 G_SETTINGS_BIND_DEFAULT);

	g_settings_bind (keyboard_settings,
			 "delay",
                     	 gtk_range_get_adjustment (GTK_RANGE (MWID ("repeat_delay_scale"))),
			 "value",
			 G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (keyboard_settings,
			 "rate",
                     	 gtk_range_get_adjustment (GTK_RANGE (MWID ("repeat_speed_scale"))),
			 "value",
			 G_SETTINGS_BIND_DEFAULT);

	g_settings_bind (interface_settings,
			 "cursor-blink",
                     	 MWID ("cursor_toggle"),
			 "active",
			 G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (interface_settings,
			 "cursor-blink",
                     	 MWID ("cursor_hbox"),
			 "sensitive",
			 G_SETTINGS_BIND_DEFAULT);
	g_settings_bind (interface_settings,
			 "cursor-blink-time",
                     	 gtk_range_get_adjustment (GTK_RANGE (MWID ("cursor_blink_time_scale"))),
			 "value",
			 G_SETTINGS_BIND_DEFAULT);
    setup_xkb_tabs (builder);
    //hide two parts by Spy
    //gtk_widget_hide(MWID("xkb_models_box"));
    //gtk_widget_hide(MWID("xkb_layout_options"));
   // setup_a11y_tabs (builder);
}

void add_keyboard_app(GtkBuilder *builder)
{
    g_debug("add_keyboard_app");

	GtkWidget *keyboard_notebook;
	keyboard_notebook = GTK_WIDGET(gtk_builder_get_object(builder, "keyboard_notebook"));
	gtk_notebook_remove_page(GTK_NOTEBOOK(keyboard_notebook), 2);

	activate_settings_daemon();

    keyboard_settings = g_settings_new (KEYBOARD_SCHEMA);
    interface_settings = g_settings_new (INTERFACE_SCHEMA);
    typing_break_settings = g_settings_new (TYPING_BREAK_SCHEMA);
    init_keybinding_tabs(builder);
    create_dialog (builder);
    setup_dialog (builder);
}

void destory_keyboard_app(GtkBuilder * builder)
{
    	//finalize_a11y_tabs ();
    	destroy_keybinding_tabs(builder);
	g_object_unref (keyboard_settings);
	g_object_unref (interface_settings);
	g_object_unref (typing_break_settings);
}
