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
#ifndef __MAINWINDOW_H
#define __MAINWINDOW_H

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib/gstdio.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
gboolean dispalyIsOk;
GtkBuilder *builder;
GtkWindow *window;
GtkApplication * app;

typedef struct _ButtonData ButtonData;

struct _ButtonData
{
	GtkViewport *vp;
	guint page;
	gchar *title;
};

ButtonData data_time;
ButtonData data_count;
ButtonData data_theme;
ButtonData data_app;
ButtonData data_network;
ButtonData data_key;
ButtonData data_mouse;
ButtonData data_printer;
ButtonData data_sound;
ButtonData data_net;
ButtonData data_start;
ButtonData data_display;
ButtonData data_power;
ButtonData data_system;

GtkNotebook *notebook1;
GtkNotebook *notebook2;

GtkButton *bt_startpage;

GtkButton *button1;
GtkButton *button2;
GtkButton *button3;
GtkButton *button4;
GtkButton *button5;
GtkButton *button6;
GtkButton *button7;
GtkButton *button8;
GtkButton *button9;
GtkButton *button10;
GtkButton *button11;
GtkButton *button12;
GtkButton *button13;
GtkButton *button14;

GtkViewport *vp_time;
GtkButton *bt_time;
GtkViewport *vp_count;
GtkButton *bt_count;
GtkViewport *vp_theme;
GtkButton *bt_theme;
GtkViewport *vp_app;
GtkButton *bt_app;
GtkViewport *vp_network;
GtkButton *bt_network;
GtkViewport *vp_key;
GtkButton *bt_key;
GtkViewport *vp_mouse;
GtkButton *bt_mouse;
GtkViewport *vp_printer;
GtkButton *bt_printer;
GtkViewport *vp_sound;
GtkButton *bt_sound;
GtkViewport *vp_net;
GtkButton *bt_net;
GtkViewport *vp_start;
GtkButton *bt_start;
GtkViewport *vp_display;
GtkButton *bt_display;
GtkViewport *vp_power;
GtkButton *bt_power;
GtkViewport *vp_system;
GtkButton *bt_system;

gboolean on_all_quit();
void ukui_init(int *argc, char **argv[]);
void hide_viewport();
void init_signals();
void set_sidebar_sensitive();
#endif  /* __MAINWINDOW_H */
