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

GtkWidget * doublecombo;
GtkWidget * middlecombo;
GtkWidget * rightcombo;

static GSettings * settings = NULL;
#define DOUBLE "action-double-click-titlebar"
#define MIDDLE "action-middle-click-titlebar"
#define RIGHT "action-right-click-titlebar"


static void on_combo_changed_cb(GtkComboBox * combobox, gpointer userdata){
    gint current;
    gchar * key = (gchar * )userdata;

    current = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox));
    if(current == -1)
	return;

    if(current == 0)
	g_settings_set_string(settings, key, "none");
    else if(current == 1)
	g_settings_set_string(settings, key, "toggle_maximize");
    else if(current == 2)
	g_settings_set_string(settings, key, "lower");
}

static void setup_combo(){
    gchar *doubleCur, *middleCur, *rightCur ;
    gint doubleIndex, middleIndex, rightIndex;

    doubleCur = g_settings_get_string(settings, DOUBLE);
    middleCur = g_settings_get_string(settings, MIDDLE);
    rightCur = g_settings_get_string(settings, RIGHT);

    //double combo box
    if(strcmp(doubleCur, "none") == 0)
	doubleIndex = 0;
    else if (strcmp(doubleCur, "toggle-maximize") == 0)
	doubleIndex = 1;
    else if (strcmp(doubleCur, "lower") == 0)
	doubleIndex = 2;
    //middle combo box
    if(strcmp(middleCur, "none") == 0)
	middleIndex = 0;
    else if (strcmp(middleCur, "toggle-maximize") == 0)
	middleIndex = 1;
    else if (strcmp(middleCur, "lower") == 0)
	middleIndex = 2;
    //right combo box
    if(strcmp(rightCur, "none") == 0)
	rightIndex = 0;
    else if (strcmp(rightCur, "toggle-maximize") == 0)
	rightIndex = 1;
    else if (strcmp(rightCur, "lower") == 0)
	rightIndex = 2;

    g_signal_handlers_block_by_func(doublecombo, on_combo_changed_cb, DOUBLE);
    g_signal_handlers_block_by_func(middlecombo, on_combo_changed_cb, MIDDLE);
    g_signal_handlers_block_by_func(rightcombo, on_combo_changed_cb, RIGHT);
    gtk_combo_box_set_active(GTK_COMBO_BOX(doublecombo), doubleIndex);
    gtk_combo_box_set_active(GTK_COMBO_BOX(middlecombo), middleIndex);
    gtk_combo_box_set_active(GTK_COMBO_BOX(rightcombo), rightIndex);
    g_signal_handlers_unblock_by_func(doublecombo, on_combo_changed_cb, DOUBLE);
    g_signal_handlers_unblock_by_func(middlecombo, on_combo_changed_cb, MIDDLE);
    g_signal_handlers_unblock_by_func(rightcombo, on_combo_changed_cb, RIGHT);
}

static void populate_combo_data(){

    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(doublecombo), _("none"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(doublecombo), _("toggle-maximize"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(doublecombo), _("lower"));

    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(middlecombo), _("none"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(middlecombo), _("toggle-maximize"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(middlecombo), _("lower"));

    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rightcombo), _("none"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rightcombo), _("toggle-maximize"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rightcombo), _("lower"));
}

void windows_init(AppearanceData * data){
    GtkWidget * default_button;
    builder = data->ui;
    settings = g_settings_new("org.mate.Marco.general");
    
    doublecombo = GTK_WIDGET(gtk_builder_get_object(builder, "dCombox"));
    g_signal_connect(G_OBJECT(doublecombo), "changed", G_CALLBACK(on_combo_changed_cb), DOUBLE);

    middlecombo = GTK_WIDGET(gtk_builder_get_object(builder, "mCombox"));
    g_signal_connect(G_OBJECT(middlecombo), "changed", G_CALLBACK(on_combo_changed_cb), MIDDLE);

    rightcombo = GTK_WIDGET(gtk_builder_get_object(builder, "rCombox"));
    g_signal_connect(G_OBJECT(rightcombo), "changed", G_CALLBACK(on_combo_changed_cb), RIGHT);

    populate_combo_data();
    setup_combo();
}

void windows_shutdown(AppearanceData *data){

}
