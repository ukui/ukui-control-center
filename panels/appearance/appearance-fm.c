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
static GSettings * psettings = NULL;

GtkWidget * doublecombo;
GtkWidget * middlecombo;
GtkWidget * rightcombo;

GtkWidget * delwarningSwitch;
GtkWidget * homedesktopSwitch;
GtkWidget * notificationSwitch;
GtkWidget * viewerCombo;
GtkWidget * defaultviewBox;

#define DOUBLE "action-double-click-titlebar"
#define MIDDLE "action-middle-click-titlebar"
#define RIGHT "action-right-click-titlebar"

#define DEL_WARNING "confirm-trash"
#define H_D "desktop-is-home-dir"
#define NOTIFICATION "show-notifications"
#define DEFAULT_VIEWER "default-folder-viewer"

enum
{
    PEONY_DEFAULT_FOLDER_VIEWER_ICON_VIEW,
    PEONY_DEFAULT_FOLDER_VIEWER_COMPACT_VIEW,
    PEONY_DEFAULT_FOLDER_VIEWER_LIST_VIEW,
    PEONY_DEFAULT_FOLDER_VIEWER_COMPUTER_VIEW,
    PEONY_DEFAULT_FOLDER_VIEWER_OTHER
};

static void on_combo_changed_cb(GtkComboBox * combobox, gpointer userdata){
    gint current;
    gchar * key = (gchar * )userdata;

    current = gtk_combo_box_get_active(GTK_COMBO_BOX(combobox));
    if(current == -1)
    return;

    if(current == 0)
    g_settings_set_string(settings, key, "none");
    else if(current == 1){
        gchar * filename = "/usr/share/glib-2.0/schemas/org.mate.marco.gschema.xml";
        if (g_file_test(filename, G_FILE_TEST_EXISTS))
            g_settings_set_string(settings, key, "toggle_maximize");
        else
            g_settings_set_string(settings, key, "toggle-maximize");
    }
    else if(current == 2)
    g_settings_set_string(settings, key, "minimize");
}

gboolean preferences_switch_set_cb(GtkSwitch * widget, gboolean state, gpointer userdata){
    const gchar * key = (const gchar *) userdata;

    g_settings_set_boolean(psettings, userdata, state);
}

static void preferences_combo_changed_cb(GtkComboBox * combobox, gpointer userdata){
    gint current;
    current = gtk_combo_box_get_active(GTK_COMBO_BOX(viewerCombo));
    if (current == -1)
        return;

    if (current == 0)
        g_settings_set_enum(psettings, DEFAULT_VIEWER, PEONY_DEFAULT_FOLDER_VIEWER_ICON_VIEW);
    else if (current == 1)
        g_settings_set_enum(psettings, DEFAULT_VIEWER, PEONY_DEFAULT_FOLDER_VIEWER_LIST_VIEW);
    //else if (current == 2)
    //    g_settings_set_string(psettings, DEFAULT_VIEWER, "list_view");
    //else if (current == 3)
    //    g_settings_set_string(psettings, DEFAULT_VIEWER, "computer_view");
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
    else if (strcmp(doubleCur, "toggle-maximize") == 0 || strcmp(doubleCur, "toggle_maximize") == 0)
    doubleIndex = 1;
    else if (strcmp(doubleCur, "minimize") == 0)
    doubleIndex = 2;
    //middle combo box
    if(strcmp(middleCur, "none") == 0)
    middleIndex = 0;
    else if (strcmp(middleCur, "toggle-maximize") == 0 || strcmp(middleCur, "toggle_maximize") == 0)
    middleIndex = 1;
    else if (strcmp(middleCur, "minimize") == 0)
    middleIndex = 2;
    //right combo box
    if(strcmp(rightCur, "none") == 0)
    rightIndex = 0;
    else if (strcmp(rightCur, "toggle-maximize") == 0 || strcmp(rightCur, "toggle_maximize") == 0)
    rightIndex = 1;
    else if (strcmp(rightCur, "minimize") == 0)
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
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(doublecombo), _("minimize"));

    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(middlecombo), _("none"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(middlecombo), _("toggle-maximize"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(middlecombo), _("minimize"));

    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rightcombo), _("none"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rightcombo), _("toggle-maximize"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rightcombo), _("minimize"));
}

static void init_combo(){
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(viewerCombo), _("icon_view"));
    //gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(viewerCombo), _("compact_view"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(viewerCombo), _("list_view"));
    //gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(viewerCombo), _("computer_view"));

    gchar * current;
    gint index;
    current = g_settings_get_string(psettings, DEFAULT_VIEWER);

    if (strcmp(current, "icon-view") == 0)
        index = 0;
    //else if (strcmp(current, "compact-view") == 0)
    //    index = 1;
    else if (strcmp(current, "list-view") == 0)
        index = 1;
    //else if (strcmp(current, "computer-view") == 0)
    //    index = 3;
    else
        index = -1;

    g_signal_handlers_block_by_func(viewerCombo, preferences_combo_changed_cb, NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(viewerCombo), index);
    g_signal_handlers_unblock_by_func(viewerCombo, preferences_combo_changed_cb, NULL);
}

static void init_switch(){
    gtk_switch_set_active(delwarningSwitch, g_settings_get_boolean(psettings, DEL_WARNING));
    gtk_switch_set_active(homedesktopSwitch, g_settings_get_boolean(psettings, H_D));
    gtk_switch_set_active(notificationSwitch, g_settings_get_boolean(psettings, NOTIFICATION));

}

void filemanager_init(AppearanceData * data){
    builder = data->ui;

    gchar * filename = "/usr/share/glib-2.0/schemas/org.mate.marco.gschema.xml";
    if (g_file_test(filename, G_FILE_TEST_EXISTS))
        settings = g_settings_new("org.mate.Marco.general");
    else
        settings = g_settings_new("org.gnome.desktop.wm.preferences");
    psettings = g_settings_new("org.ukui.peony.preferences");

    doublecombo = GTK_WIDGET(gtk_builder_get_object(builder, "dCombox"));
    g_signal_connect(G_OBJECT(doublecombo), "changed", G_CALLBACK(on_combo_changed_cb), DOUBLE);

    middlecombo = GTK_WIDGET(gtk_builder_get_object(builder, "mCombox"));
    g_signal_connect(G_OBJECT(middlecombo), "changed", G_CALLBACK(on_combo_changed_cb), MIDDLE);

    rightcombo = GTK_WIDGET(gtk_builder_get_object(builder, "rCombox"));
    g_signal_connect(G_OBJECT(rightcombo), "changed", G_CALLBACK(on_combo_changed_cb), RIGHT);

    populate_combo_data();
    setup_combo();



    delwarningSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "delwarningSwitch"));
    g_signal_connect(G_OBJECT(delwarningSwitch), "state-set", G_CALLBACK(preferences_switch_set_cb), DEL_WARNING);

    homedesktopSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "homedesktopSwitch"));
    g_signal_connect(G_OBJECT(homedesktopSwitch), "state-set", G_CALLBACK(preferences_switch_set_cb), H_D);

    notificationSwitch = GTK_WIDGET(gtk_builder_get_object(builder, "notificationSwitch"));
    g_signal_connect(G_OBJECT(notificationSwitch), "state-set", G_CALLBACK(preferences_switch_set_cb), NOTIFICATION);

    viewerCombo = GTK_WIDGET(gtk_builder_get_object(builder, "viewerCombo"));
    g_signal_connect(viewerCombo, "changed", G_CALLBACK(preferences_combo_changed_cb), NULL);


    defaultviewBox = GTK_WIDGET(gtk_builder_get_object(builder, "defaultviewBox"));
    gtk_widget_set_no_show_all(defaultviewBox, TRUE);
    gtk_widget_hide(defaultviewBox);

    init_switch();
    init_combo();
}

void filemanager_shutdown(AppearanceData *data){

}
