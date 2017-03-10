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
#include <libintl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "dialog-main.h"
#include <glib.h>
#include <gtk/gtk.h>

#include "gvc-mixer-dialog.h"
#include "gvc-log.h"

#define GVCA_DBUS_NAME "org.mate.VolumeControl"
#define DIALOG_POPUP_TIMEOUT 3
GtkBuilder * top_builder;
static gboolean show_version = FALSE;
static gboolean debug = FALSE;
static gchar* page = NULL;

static guint popup_id = 0;
static GtkWidget *dialog = NULL;
static GtkWidget *warning_dialog = NULL;

static void
on_control_ready (GvcMixerControl *control,
                  gpointer       app)
{
	if (popup_id != 0) {
		g_source_remove (popup_id);
		popup_id = 0;
	}
	if (warning_dialog != NULL) {
		gtk_widget_destroy (warning_dialog);
		warning_dialog = NULL;
	}

        if (dialog)
                return;

        dialog = GTK_WIDGET (gvc_mixer_dialog_new (control,top_builder));
        gvc_mixer_dialog_set_page(GVC_MIXER_DIALOG (dialog), page);

        //g_signal_connect (app, "message-received",
        //                  G_CALLBACK (message_received_cb), dialog);
}

static void
warning_dialog_answered (GtkDialog *d,
			 gpointer data)
{
	gtk_widget_destroy (warning_dialog);
	gtk_main_quit ();
}

static gboolean
dialog_popup_timeout (gpointer data)
{
	warning_dialog = gtk_message_dialog_new (GTK_WINDOW(dialog),
						 0,
						 GTK_MESSAGE_INFO,
						 GTK_BUTTONS_CANCEL,
                         "Waiting for sound system to respond");
	g_signal_connect (warning_dialog, "response",
			  G_CALLBACK (warning_dialog_answered), NULL);
	g_signal_connect (warning_dialog, "close",
			  G_CALLBACK (warning_dialog_answered), NULL);

	gtk_widget_show (warning_dialog);

	return FALSE;
}

static void
on_control_connecting (GvcMixerControl *control,
                       gpointer       app)
{
        if (popup_id != 0)
                return;

        popup_id = g_timeout_add_seconds (DIALOG_POPUP_TIMEOUT,
                                          dialog_popup_timeout,
                                          NULL);
}

void add_volume_control_app (GtkBuilder * builder)
{
		g_warning("volume-control");
        GError             *error;
        GvcMixerControl    *control;
        top_builder = builder;
        gvc_log_init ();
        error = NULL;
        gvc_log_set_debug (debug);

        control = gvc_mixer_control_new ("MATE Volume Control Dialog");
        g_signal_connect (control,
                          "connecting",
                          G_CALLBACK (on_control_connecting),
                          NULL);
        g_signal_connect (control,
                          "ready",
                          G_CALLBACK (on_control_ready),
                          NULL);
        gvc_mixer_control_open (control);
        /*if (control != NULL) {
                g_object_unref (control);
        }

        return 0;*/
}

void destroy_volume_control(){
    //if (control)
}
