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
#include <stdlib.h>
#include <stdio.h>
#include "appearance-main.h"
#include "appearance-desktop.h"
#include "appearance-font.h"
#include "appearance-windows.h"
#include "appearance-fm.h"
#include "appearance-media.h"
#include "appearance-theme.h"

AppearanceData *data;

void init_appearance_data(GtkBuilder * builder)
{
	GtkWidget * appearance_notebook;
	data=NULL;
	data = g_new(AppearanceData, 1);
	data->wp_settings = g_settings_new(WP_SCHEMA);
	data->peony_settings = g_settings_new(PEONY_SCHEMA);
	data->interface_settings = g_settings_new (INTERFACE_SCHEMA);
	data->marco_settings = g_settings_new (MARCO_SCHEMA);
	data->font_settings=g_settings_new(FONT_RENDER_SCHEMA);
	data->ui = builder;
	appearance_notebook = GTK_WIDGET(gtk_builder_get_object(builder, "appearance_notebook"));
	//Fixme: I remove page2 and page3 to wait designer give me pointe,remove theme page

       // gtk_notebook_remove_page(GTK_NOTEBOOK(appearance_notebook), 2);

	//notice that when you remove page1, page2 become page1,so remove screensaver page
        //gtk_notebook_remove_page(GTK_NOTEBOOK(appearance_notebook), 1);
	//remove page4
    //gtk_notebook_remove_page(GTK_NOTEBOOK(appearance_notebook), 1);
	//设置gsetting，保证scolor和pcolor为黑色
	//由于gsetting比较耗时，暂时注释
	//g_settings_set_string(data->wp_settings, WP_PCOLOR_KEY, "#000000000000");
	//g_settings_set_string(data->wp_settings, WP_SCOLOR_KEY, "#000000000000");
	data->thumb_factory = mate_desktop_thumbnail_factory_new (MATE_DESKTOP_THUMBNAIL_SIZE_NORMAL);
}

void add_appearance_app(GtkBuilder *builder)
{
	g_debug("appearance");
	gchar ** wallpaper_files = NULL;
	init_appearance_data(builder);
	if (!data)
		return;
	desktop_init(data, (const gchar **)wallpaper_files);
    theme_init(data);
    font_init(data);
    windows_init(data);
    media_init(data);
    filemanager_init(data);
	g_strfreev(wallpaper_files);
}

void destory_appearance_app()
{
	font_shutdown(data);
	desktop_shutdown(data);
	g_object_unref (data->thumb_factory);
	g_object_unref (data->wp_settings);
	g_object_unref (data->peony_settings);
	g_object_unref (data->interface_settings);
	g_object_unref (data->marco_settings);
	g_object_unref (data->font_settings);
	g_free(data);
}
