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
#include "appearance-theme.h"
#include <glib/gi18n.h>
#define theme_gtk(x,y) GTK_WIDGET(gtk_builder_get_object(x.builder, y))
#define COLOR_DIR "/usr/share/ukui-control-center/color"

typedef struct _ThemeData ThemeData;
struct _ThemeData {
    GtkBuilder * builder;
    GtkWidget * color_button_gray;
    GtkWidget * color_button_blue;
    GtkWidget * color_button_yellow;
    GtkWidget * color_button_red;
    GtkWidget * color_button_green;
    GtkWidget * color_button_green_jun;
    GtkWidget * color_button_purple;
    GtkWidget * color_button_lightblue;
    GtkWidget * color_button_orange;
    GtkWidget * color_button_black;
    GSettings * theme_setting;
	GSettings * theme_gtk_setting;
	GtkWidget * appearance_show_label;
};
ThemeData themedata;
static gboolean mute_lock;

gchar * iconblack = "ukui-icon-theme"; 
gchar * iconblue = "ukui-icon-theme-one";

static gboolean reset_lock(gpointer user_data){
	mute_lock = TRUE;
    gtk_label_set_text(GTK_LABEL(themedata.appearance_show_label), _("Please choose a color that you like as the theme colors."));
	return FALSE;
}

static void set_theme_color(GtkWidget * widget, GdkEvent *event, gpointer user_data){
	int systemback;
	if(mute_lock){
		mute_lock = FALSE;
    	g_settings_set_string(themedata.theme_setting,MARCO_THEME_KEY, user_data);
	g_settings_set_string(themedata.theme_gtk_setting,GTK_THEME_KEY, user_data);
	if (g_strrstr(user_data, "blue"))
		g_settings_set_string(themedata.theme_gtk_setting, ICON_THEME_KEY, iconblue);
	else
		g_settings_set_string(themedata.theme_gtk_setting, ICON_THEME_KEY, iconblack);

    //	systemback = system("/usr/bin/killall mate-panel");
//        systemback = system("/usr/bin/nohup /usr/bin/mate-panel --replace &");

        g_timeout_add(100, (GSourceFunc)reset_lock, NULL);
		return;
	}
//    gtk_label_set_text(GTK_LABEL(themedata.appearance_show_label), _("Set up disable. Please wait at least 5 seconds that system subject will restart"));
}

static void component_init(){
    themedata.color_button_blue = theme_gtk(themedata, "color_button_blue");
    gtk_button_set_image(themedata.color_button_blue, gtk_image_new_from_file(COLOR_DIR"/ukui-blue.png"));
    g_signal_connect(themedata.color_button_blue, "button-release-event", G_CALLBACK(set_theme_color), "ukui-blue");

    themedata.color_button_black = theme_gtk(themedata, "color_button_black");
    gtk_button_set_image(themedata.color_button_black, gtk_image_new_from_file(COLOR_DIR"/ukui-black.png"));
    g_signal_connect(themedata.color_button_black, "button-release-event", G_CALLBACK(set_theme_color), "ukui-black");

    //themedata.color_button_gray = theme_gtk(themedata, "color_button_gray");
    //gtk_button_set_image(themedata.color_button_gray, gtk_image_new_from_file(COLOR_DIR"/ukui-gray.png"));
    //g_signal_connect(themedata.color_button_gray, "button-release-event", G_CALLBACK(set_theme_color), "ukui-theme-gray");

    /*themedata.color_button_yellow = theme_gtk(themedata, "color_button_yellow");
    gtk_button_set_image(themedata.color_button_yellow, gtk_image_new_from_file(COLOR_DIR"/ukui-yellow.png"));
    g_signal_connect(themedata.color_button_yellow, "button-release-event", G_CALLBACK(set_theme_color), "ukui-theme-yellow");

    themedata.color_button_red = theme_gtk(themedata, "color_button_red");
    gtk_button_set_image(themedata.color_button_red, gtk_image_new_from_file(COLOR_DIR"/ukui-red.png"));
    g_signal_connect(themedata.color_button_red, "button-release-event", G_CALLBACK(set_theme_color), "ukui-theme-red");

    themedata.color_button_green = theme_gtk(themedata, "color_button_green");
    gtk_button_set_image(themedata.color_button_green, gtk_image_new_from_file(COLOR_DIR"/ukui-green.png"));
    g_signal_connect(themedata.color_button_green, "button-release-event", G_CALLBACK(set_theme_color), "ukui-theme-green");

    themedata.color_button_green_jun = theme_gtk(themedata, "color_button_green_jun");
    gtk_button_set_image(themedata.color_button_green_jun, gtk_image_new_from_file(COLOR_DIR"/ukui-green-jun.png"));
    g_signal_connect(themedata.color_button_green_jun, "button-release-event", G_CALLBACK(set_theme_color), "ukui-theme-jun");


    themedata.color_button_purple = theme_gtk(themedata, "color_button_purple");
    gtk_button_set_image(themedata.color_button_purple, gtk_image_new_from_file(COLOR_DIR"/ukui-purple.png"));
    g_signal_connect(themedata.color_button_purple, "button-release-event", G_CALLBACK(set_theme_color), "ukui-theme-purple");*/

    /*themedata.color_button_lightblue = theme_gtk(themedata, "color_button_lightblue");
    gtk_button_set_image(themedata.color_button_lightblue, gtk_image_new_from_file(COLOR_DIR"/ukui-light-blue.png"));
    g_signal_connect(themedata.color_button_lightblue, "button-release-event", G_CALLBACK(set_theme_color), "ukui-light-blue");
	*/
    /*themedata.color_button_orange = theme_gtk(themedata, "color_button_orange");
    gtk_button_set_image(themedata.color_button_orange, gtk_image_new_from_file(COLOR_DIR"/ukui-orange.png"));
    g_signal_connect(themedata.color_button_orange, "button-release-event", G_CALLBACK(set_theme_color), "ukui-theme-orange");*/

	themedata.appearance_show_label = theme_gtk(themedata, "appearance_show_label");
}

static void load_current_theme(){
    gchar * current_theme;
    current_theme = g_settings_get_string(themedata.theme_setting, MARCO_THEME_KEY);
}

void theme_init(AppearanceData * data)
{
    themedata.builder = data->ui;
    themedata.theme_setting = data->marco_settings;
	themedata.theme_gtk_setting = data->interface_settings;
	mute_lock = TRUE;
    component_init();
    load_current_theme();
}


