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

extern gboolean dispalyIsOk;
extern GtkBuilder *builder;
extern GtkWindow *window;
extern GtkApplication * app;

typedef struct _ButtonData ButtonData;

struct _ButtonData
{
	GtkViewport *vp;
	guint page;
	gchar *title;
};

extern ButtonData data_time;
extern ButtonData data_count;
extern ButtonData data_theme;
extern ButtonData data_app;
extern ButtonData data_network;
extern ButtonData data_key;
extern ButtonData data_mouse;
extern ButtonData data_printer;
extern ButtonData data_sound;
extern ButtonData data_net;
extern ButtonData data_start;
extern ButtonData data_display;
extern ButtonData data_power;
extern ButtonData data_system;

extern GtkNotebook *notebook1;
extern GtkNotebook *notebook2;

extern GtkButton *bt_startpage;

extern GtkButton *button1;
extern GtkButton *button2;
extern GtkButton *button3;
extern GtkButton *button4;
extern GtkButton *button5;
extern GtkButton *button6;
extern GtkButton *button7;
extern GtkButton *button8;
extern GtkButton *button9;
extern GtkButton *button10;
extern GtkButton *button11;
extern GtkButton *button12;
extern GtkButton *button13;
extern GtkButton *button14;

extern GtkViewport *vp_time;
extern GtkButton *bt_time;
extern GtkViewport *vp_count;
extern GtkButton *bt_count;
extern GtkViewport *vp_theme;
extern GtkButton *bt_theme;
extern GtkViewport *vp_app;
extern GtkButton *bt_app;
extern GtkViewport *vp_network;
extern GtkButton *bt_network;
extern GtkViewport *vp_key;
extern GtkButton *bt_key;
extern GtkViewport *vp_mouse;
extern GtkButton *bt_mouse;
extern GtkViewport *vp_printer;
extern GtkButton *bt_printer;
extern GtkViewport *vp_sound;
extern GtkButton *bt_sound;
extern GtkViewport *vp_net;
extern GtkButton *bt_net;
extern GtkViewport *vp_start;
extern GtkButton *bt_start;
extern GtkViewport *vp_display;
extern GtkButton *bt_display;
extern GtkViewport *vp_power;
extern GtkButton *bt_power;
extern GtkViewport *vp_system;
extern GtkButton *bt_system;

gboolean on_all_quit();
void ukui_init(int *argc, char **argv[]);
void hide_viewport();
void init_signals();
void set_sidebar_sensitive();
void app_set_theme(const gchar *theme_path);
#endif  /* __MAINWINDOW_H */
