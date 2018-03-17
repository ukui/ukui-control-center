/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 1999 Free Software Foundation, Inc.
 * Copyright (C) 2007 Vincent Untz.
 * Copyright (C) 2008 Lucas Rocha.
 * Copyright (C) 2008 William Jon McCann <jmccann@redhat.com>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gsm-properties-dialog.h"
#include "gsm-app-dialog.h"
#include "gsm-util.h"
#include "gsp-app.h"
#include "gsp-app-manager.h"
#include <stdlib.h>
//#define GSM_PROPERTIES_DIALOG_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GSM_TYPE_PROPERTIES_DIALOG, GsmPropertiesDialogPrivate))

#define GTKBUILDER_FILE "session-properties.ui"

#define CAPPLET_TREEVIEW_WIDGET_NAME      "session_properties_treeview"
#define CAPPLET_ADD_WIDGET_NAME           "session_properties_add_button"
#define CAPPLET_DELETE_WIDGET_NAME        "session_properties_delete_button"
#define CAPPLET_EDIT_WIDGET_NAME          "session_properties_edit_button"
#define CAPPLET_SAVE_WIDGET_NAME          "session_properties_save_button"
#define CAPPLET_REMEMBER_WIDGET_NAME      "session_properties_remember_toggle"

#define STARTUP_APP_ICON     "system-run"

#define SPC_SETTINGS_SCHEMA          "org.ukui.session"
#define SPC_SETTINGS_AUTOSAVE_KEY    "auto-save-session"
GtkBuilder * topbuilder = NULL;
struct GsmPropertiesDialogPrivate
{
        GtkBuilder        *xml;
        GtkListStore      *list_store;
        GtkTreeModel      *tree_filter;

        GtkTreeView       *treeview;
        GtkWidget         *add_button;
        GtkWidget         *delete_button;
        GtkWidget         *edit_button;

        GSettings         *settings;

        GspAppManager     *manager;
};

enum {
        STORE_COL_VISIBLE = 0,
        STORE_COL_ENABLED,
        STORE_COL_GICON,
        STORE_COL_DESCRIPTION,
        STORE_COL_APP,
        STORE_COL_SEARCH,
        STORE_COL_LABEL,
        STORE_COL_SPACE,
        NUMBER_OF_COLUMNS
};

/*static void     gsm_properties_dialog_class_init  (GsmPropertiesDialogClass *klass);
static void     gsm_properties_dialog_init        (GsmPropertiesDialog      *properties_dialog);
static void     gsm_properties_dialog_finalize    (GObject                  *object);

G_DEFINE_TYPE (GsmPropertiesDialog, gsm_properties_dialog, GTK_TYPE_DIALOG)
*/
static GsmPropertiesDialog Dialog1;
static GsmPropertiesDialogPrivate Dialog2;
GsmPropertiesDialog *dialog1;
GsmPropertiesDialogPrivate *dialog2;

static gboolean
find_by_app (GtkTreeModel *model,
             GtkTreeIter  *iter,
             GspApp       *app)
{
        GspApp *iter_app = NULL;

        if (!gtk_tree_model_get_iter_first (model, iter)) {
                return FALSE;
        }

        do {
                gtk_tree_model_get (model, iter,
                                    STORE_COL_APP, &iter_app,
                                    -1);

                if (iter_app == app) {
                        g_object_unref (iter_app);
                        return TRUE;
                }
        } while (gtk_tree_model_iter_next (model, iter));

        return FALSE;
}

const char * kylin_set_the_label(GspApp *app){
        const char *label;
        gboolean enabled = gsp_app_get_enabled(app);
        if(enabled)
                label = _("enabled");
        else
                label = _("disabled");
        return label;
}


static void
_fill_iter_from_app (GtkListStore *list_store,
                     GtkTreeIter  *iter,
                     GspApp       *app)
{
        gboolean    hidden;
        gboolean    display;
        gboolean    enabled;
        gboolean    shown;
        GIcon      *icon;
        const char *description;
        const char *app_name;
        const char *label;
        const char *space = "                                                             ";

        hidden      = gsp_app_get_hidden (app);
        display     = gsp_app_get_display (app);
        enabled     = gsp_app_get_enabled (app);
        shown       = gsp_app_get_shown (app);
        icon        = gsp_app_get_icon (app);
        description = gsp_app_get_description (app);
        app_name    = gsp_app_get_name (app);
        label       = kylin_set_the_label(app);

        if (G_IS_THEMED_ICON (icon)) {
                GtkIconTheme       *theme;
                const char * const *icon_names;

                theme = gtk_icon_theme_get_default ();
                icon_names = g_themed_icon_get_names (G_THEMED_ICON (icon));
                if (icon_names[0] == NULL ||
                    !gtk_icon_theme_has_icon (theme, icon_names[0])) {
                        g_object_unref (icon);
                        icon = NULL;
                }
        } else if (G_IS_FILE_ICON (icon)) {
                GFile *iconfile;

                iconfile = g_file_icon_get_file (G_FILE_ICON (icon));
                if (!g_file_query_exists (iconfile, NULL)) {
                        g_object_unref (icon);
                        icon = NULL;
                }
        }

        if (icon == NULL) {
                icon = g_themed_icon_new (STARTUP_APP_ICON);
        }
        gtk_list_store_set (list_store, iter,
                            STORE_COL_VISIBLE, !hidden && shown && display,
                            STORE_COL_ENABLED, enabled,
                            STORE_COL_GICON, icon,
                            STORE_COL_DESCRIPTION, description,
                            STORE_COL_APP, app,
                            STORE_COL_SEARCH, app_name,
                            STORE_COL_LABEL, label,
                            STORE_COL_SPACE, space,
                            -1);
        g_object_unref (icon);
}

static void
_app_changed (GsmPropertiesDialog *dialog,
              GspApp              *app)
{
        GtkTreeIter iter;

        if (!find_by_app (GTK_TREE_MODEL (dialog->priv->list_store),
                          &iter, app)) {
                return;
        }

        _fill_iter_from_app (dialog->priv->list_store, &iter, app);
}

static void
append_app (GsmPropertiesDialog *dialog,
            GspApp              *app)
{
    GtkTreeIter   iter;
    gtk_list_store_append (dialog->priv->list_store, &iter);
    _fill_iter_from_app (dialog->priv->list_store, &iter, app);
    g_signal_connect_swapped (app, "changed",
                              G_CALLBACK (_app_changed), dialog);
}

static void
_app_added (GsmPropertiesDialog *dialog,
            GspApp              *app,
            GspAppManager       *manager)
{
    append_app (dialog, app);
}

static void
_app_removed (GsmPropertiesDialog *dialog,
              GspApp              *app,
              GspAppManager       *manager)
{
        GtkTreeIter iter;

        if (!find_by_app (GTK_TREE_MODEL (dialog->priv->list_store),
                          &iter, app)) {
                return;
        }

        g_signal_handlers_disconnect_by_func (app,
                                              _app_changed,
                                              dialog);
        gtk_list_store_remove (dialog->priv->list_store, &iter);
}


static void
populate_model (GsmPropertiesDialog *dialog)
{
        GSList *apps;
        GSList *l;

        apps = gsp_app_manager_get_apps (dialog->priv->manager);
        for (l = apps; l != NULL; l = l->next) {
                append_app (dialog, GSP_APP (l->data));
        }
        g_slist_free (apps);
}

static void
on_selection_changed (GtkTreeSelection    *selection,
                      GsmPropertiesDialog *dialog)
{
        gboolean sel;

        sel = gtk_tree_selection_get_selected (selection, NULL, NULL);

        gtk_widget_set_sensitive (dialog->priv->edit_button, sel);
        gtk_widget_set_sensitive (dialog->priv->delete_button, sel);
}

static void
on_startup_enabled_toggled (GtkCellRendererToggle *cell_renderer,
                            char                  *path,
                            GsmPropertiesDialog   *dialog)
{
        GtkTreeIter iter;
        GspApp     *app;
        gboolean    active;

        if (!gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (dialog->priv->tree_filter),
                                                  &iter, path)) {
                return;
        }

        app = NULL;
        gtk_tree_model_get (GTK_TREE_MODEL (dialog->priv->tree_filter),
                            &iter,
                            STORE_COL_APP, &app,
                            -1);

        active = gtk_cell_renderer_toggle_get_active (cell_renderer);
        active = !active;

        if (app) {
                gsp_app_set_enabled (app, active);
                g_object_unref (app);
        }
}

static void
on_drag_data_received (GtkWidget           *widget,
                       GdkDragContext      *drag_context,
                       gint                 x,
                       gint                 y,
                       GtkSelectionData    *data,
                       guint                info,
                       guint                time,
                       GsmPropertiesDialog *dialog)
{
        gboolean dnd_success;

        dnd_success = FALSE;

        if (data != NULL) {
                char **filenames;
                int    i;

                filenames = gtk_selection_data_get_uris (data);

                for (i = 0; filenames[i] && filenames[i][0]; i++) {
                        /* Return success if at least one file succeeded */
                        gboolean file_success;
                        file_success = gsp_app_copy_desktop_file (filenames[i]);
                        dnd_success = dnd_success || file_success;
                }

                g_strfreev (filenames);
        }

        gtk_drag_finish (drag_context, dnd_success, FALSE, time);
        g_signal_stop_emission_by_name (widget, "drag_data_received");
}

static void
on_drag_begin (GtkWidget           *widget,
               GdkDragContext      *context,
               GsmPropertiesDialog *dialog)
{
        GtkTreePath *path;
        GtkTreeIter  iter;
        GspApp      *app;

        gtk_tree_view_get_cursor (GTK_TREE_VIEW (widget), &path, NULL);
        gtk_tree_model_get_iter (GTK_TREE_MODEL (dialog->priv->tree_filter),
                                 &iter, path);
        gtk_tree_path_free (path);

        gtk_tree_model_get (GTK_TREE_MODEL (dialog->priv->tree_filter),
                            &iter,
                            STORE_COL_APP, &app,
                            -1);

        if (app) {
                g_object_set_data_full (G_OBJECT (context), "gsp-app",
                                        g_object_ref (app), g_object_unref);
                g_object_unref (app);
        }

}

static void
on_drag_data_get (GtkWidget           *widget,
                  GdkDragContext      *context,
                  GtkSelectionData    *selection_data,
                  guint                info,
                  guint                time,
                  GsmPropertiesDialog *dialog)
{
        GspApp *app;

        app = g_object_get_data (G_OBJECT (context), "gsp-app");
        if (app) {
                const char *uris[2];
                char       *uri;

                uri = g_filename_to_uri (gsp_app_get_path (app), NULL, NULL);

                uris[0] = uri;
                uris[1] = NULL;
                gtk_selection_data_set_uris (selection_data, (char **) uris);

                g_free (uri);
        }
}

static void
on_add_app_clicked (GtkWidget           *widget,
                    GsmPropertiesDialog *dialog)
{
        GtkWidget  *add_dialog;
        char       *name;
        char       *exec;
        char       *comment;
        char       *icon;
        add_dialog = gsm_app_dialog_new (NULL, NULL, NULL);
        //gtk_window_set_transient_for (GTK_WINDOW (add_dialog),
        //                              GTK_WINDOW (dialog));

        if (gsm_app_dialog_run (GSM_APP_DIALOG (add_dialog),
                                &name, &exec, &comment, &icon)) {
                if(8 == gsp_app_create (name, comment, exec ,icon))
                {
                    GtkWidget *msgbox;
                    msgbox = gtk_message_dialog_new (GTK_WINDOW (add_dialog),
                                                     GTK_DIALOG_MODAL,
                                                     GTK_MESSAGE_ERROR,
                                                     GTK_BUTTONS_CLOSE,
                                                     "%s开机启动已经存在！", name);


                    gtk_widget_set_name(GTK_WIDGET(msgbox), "ukuicc");
                    gtk_dialog_run (GTK_DIALOG (msgbox));
                    gtk_widget_destroy (msgbox);
                }
                g_free (name);
                g_free (exec);
                g_free (comment);
                g_free (icon);
        }
}

static void
on_delete_app_clicked (GtkWidget           *widget,
                       GsmPropertiesDialog *dialog)
{
        GtkTreeSelection *selection;
        GtkTreeIter       iter;
        GspApp           *app;

        selection = gtk_tree_view_get_selection (dialog->priv->treeview);

        if (!gtk_tree_selection_get_selected (selection, NULL, &iter)) {
                return;
        }

        app = NULL;
        gtk_tree_model_get (GTK_TREE_MODEL (dialog->priv->tree_filter),
                            &iter,
                            STORE_COL_APP, &app,
                            -1);

        if (app) {
                gsp_app_delete (app);
//                g_object_unref (app);arm64下会崩溃
        }
}

static char * get_string_from_guint64(guint64 gu){
    GDateTime * time;
    static gchar str[50];
    gint year, month, day;
    time = g_date_time_new_from_unix_local(gu);
    g_date_time_get_ymd(time, &year, &month, &day);
    g_sprintf(str, _("%d/%d/%d/  %d:%d:%d"),year,month,day,g_date_time_get_hour(time),g_date_time_get_minute(time),g_date_time_get_second(time));
    return str;
}

//example 1: Exec =/usr/bin/youker-assistant
//example 2: Exec =ukui-search-tool
//example 3: Exec =libreoffice --math
//example 4: Exec =smplayer --add-to-playlist %U
//and others example, like 1&3 1&4 ...
//we need to consider $PATH, and return string like "Exec = /usr/bin/smplayer --add-to-playlist %U"
//there must be 3 parts at least, name(/usr/bin/smplayer),argument(--add-to-playlist) and format(%U)
static char * get_absolute_app_path(char * exec){
    char * absolute_exec;
    char ** tmp_str;
    char * env;
    char ** tmp_env;
    int size;
    char file_path[50];
    if (exec) {
        tmp_str = g_strsplit(exec, " ", -1);
        env = g_getenv("PATH");
        tmp_env = g_strsplit(env, ":", -1);
        if (g_file_test(tmp_str[0],G_FILE_TEST_EXISTS)){
            absolute_exec = tmp_str[0];
            return absolute_exec;
        }
        for(;*tmp_env;tmp_env++){
            g_sprintf(file_path, "%s/%s",*tmp_env,tmp_str[0]);
            if (g_file_test(file_path, G_FILE_TEST_EXISTS)){
                absolute_exec = file_path;
                return absolute_exec;
            }
        }
    }
}

static void
on_edit_app_clicked (GtkWidget           *widget,
                     GsmPropertiesDialog *dialog)
{
        GtkTreeSelection *selection;
        GtkTreeIter       iter;
        GspApp           *app;
        GtkWidget * builder;
        GError * error= NULL;
        GtkWidget  *edit_dialog;
        GtkDialog * pro_dialog;
        builder = gtk_builder_new();
        if (gtk_builder_add_from_file(builder, UIDIR"/program_properties.ui", &error ) == 0){
            g_warning("Could not load ui file occurred error:%s", error->message);
            return;
        }
        edit_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "dialog"));
        selection = gtk_tree_view_get_selection (dialog->priv->treeview);

        if (!gtk_tree_selection_get_selected (selection, NULL, &iter)) {
                return;
        }

        app = NULL;
        gtk_tree_model_get (GTK_TREE_MODEL (dialog->priv->tree_filter),
                            &iter,
                            STORE_COL_APP, &app,
                            -1);

        if (app) {
                char       *name;
                char       *exec;
                char       *comment;
                char       *absolute_exec=NULL;
                GIcon    *icon;
                char       *title;
                gint res;
                GFile * exec_file;
                GError * err= NULL;
                GFileInfo * exec_info;
                name = gsp_app_get_name(app);
                exec = gsp_app_get_exec(app);
                absolute_exec = get_absolute_app_path(exec);
                comment = gsp_app_get_comment(app);
                icon = gsp_app_get_icon(app);
                title = g_strdup_printf("%s %s",name,_("property"));
                gtk_window_set_title(edit_dialog,title);
                //set label
                if (comment)
                    gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "pro_des")), comment);
                else
                    gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "pro_des")), gsp_app_get_description(app));
                gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "pro_name")), name);
                gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "pro_exec")), exec);
                if (icon == NULL){
                    gtk_image_set_from_icon_name(GTK_IMAGE(gtk_builder_get_object(builder, "pro_icon")), STARTUP_APP_ICON, GTK_ICON_SIZE_BUTTON);
                }
                else
                    gtk_image_set_from_gicon(GTK_IMAGE(gtk_builder_get_object(builder, "pro_icon")), icon,GTK_ICON_SIZE_BUTTON);
                exec_file = g_file_new_for_path(absolute_exec);
                exec_info = g_file_query_info(exec_file, G_FILE_ATTRIBUTE_STANDARD_SIZE","
                                              G_FILE_ATTRIBUTE_TIME_MODIFIED","
                                              G_FILE_ATTRIBUTE_TIME_ACCESS,
                                              G_FILE_QUERY_INFO_NONE,
                                              NULL,
                                              &err );
                if (err != NULL){
                    g_warning("get exec info error:%s", err->message);
                }
                g_object_unref(exec_file);
                goffset off = g_file_info_get_size(exec_info);
                gchar str[20];
                g_sprintf(str, _("%dByte"),(gint )off);
                gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "pro_size")), str);
                guint64 modified_time;
                guint64 accessed_time;
                char * modified_str = NULL;
                char * accessed_str = NULL;
                modified_time = g_file_info_get_attribute_uint64(exec_info, G_FILE_ATTRIBUTE_TIME_MODIFIED);
                modified_str = get_string_from_guint64(modified_time);
                gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "pro_cha_time")), modified_str);
                accessed_time = g_file_info_get_attribute_uint64(exec_info, G_FILE_ATTRIBUTE_TIME_ACCESS);
                accessed_str = get_string_from_guint64(accessed_time);
                gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "pro_cre_time")), accessed_str);
                gtk_dialog_add_button(GTK_DIALOG(edit_dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
                res = gtk_dialog_run(edit_dialog);
                g_free(title);
                gtk_widget_destroy(edit_dialog);
                g_object_unref (app);
                g_object_unref(exec_info);
        }
}

static void
on_row_activated (GtkTreeView         *tree_view,
                  GtkTreePath         *path,
                  GtkTreeViewColumn   *column,
                  GsmPropertiesDialog *dialog)
{
        on_edit_app_clicked (NULL, dialog);
}

static void
on_save_session_clicked (GtkWidget           *widget,
                         GsmPropertiesDialog *dialog)
{
        g_debug ("Session saving is not implemented yet!");
}

static void
setup_dialog (GsmPropertiesDialog *dialog)
{
        GtkTreeView       *treeview;
        GtkWidget         *button;
        GtkTreeModel      *tree_filter;
        GtkTreeViewColumn *column;
        GtkCellRenderer   *renderer;
        GtkTreeSelection  *selection;
        GtkTargetList     *targetlist;

        dialog->priv->list_store = gtk_list_store_new (NUMBER_OF_COLUMNS,
                                                       G_TYPE_BOOLEAN,
                                                       G_TYPE_BOOLEAN,
                                                       G_TYPE_ICON,
                                                       G_TYPE_STRING,
                                                       G_TYPE_OBJECT,
                                                       G_TYPE_STRING,
                                                       G_TYPE_STRING,
                                                       G_TYPE_STRING);
        tree_filter = gtk_tree_model_filter_new (GTK_TREE_MODEL (dialog->priv->list_store),NULL);
        g_object_unref (dialog->priv->list_store);
        dialog->priv->tree_filter = tree_filter;
        //根据这个属性的bool值判断是否过滤此行
        gtk_tree_model_filter_set_visible_column (GTK_TREE_MODEL_FILTER (tree_filter),STORE_COL_VISIBLE);

        treeview = GTK_TREE_VIEW (gtk_builder_get_object (dialog->priv->xml,CAPPLET_TREEVIEW_WIDGET_NAME));
        dialog->priv->treeview = treeview;

        gtk_tree_view_set_model (treeview, tree_filter);
        g_object_unref (tree_filter);

        gtk_tree_view_set_headers_visible (treeview, FALSE);
        g_signal_connect (treeview,
                          "row-activated",
                          G_CALLBACK (on_row_activated),
                          dialog);

        selection = gtk_tree_view_get_selection (treeview);
        gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
        g_signal_connect (selection,
                          "changed",
                          G_CALLBACK (on_selection_changed),
                          dialog);

        /* ICON COLUMN */
        renderer = gtk_cell_renderer_pixbuf_new ();
        gtk_cell_renderer_set_padding(renderer, 0, 5);
        column = gtk_tree_view_column_new_with_attributes ("Icon",
                                                           renderer,
                                                           "gicon", STORE_COL_GICON,
                                                           "sensitive", STORE_COL_ENABLED,
                                                           NULL);
        g_object_set (renderer,
                      "stock-size", GSM_PROPERTIES_ICON_SIZE,
                      NULL);
        gtk_tree_view_append_column (treeview, column);

        /* NAME COLUMN */
        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("Program",
                                                           renderer,
                                                           "markup", STORE_COL_DESCRIPTION,
                                                           "sensitive", STORE_COL_ENABLED,
                                                           NULL);
//        g_object_set (renderer,
//                      "ellipsize", PANGO_ELLIPSIZE_END,
//                      NULL);
        gtk_tree_view_append_column (treeview, column);

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes ("Space",
                                                           renderer,
                                                           "markup", STORE_COL_SPACE,
                                                           "sensitive", STORE_COL_ENABLED,
                                                           NULL);
        gtk_tree_view_append_column (treeview, column);

         /* CHECKBOX COLUMN */
        renderer = gtk_cell_renderer_toggle_new ();
        column = gtk_tree_view_column_new_with_attributes ("Enabled",
                                                           renderer,
                                                           "active", STORE_COL_ENABLED,
                                                           NULL);
        gtk_tree_view_append_column (treeview, column);
        g_signal_connect (renderer,
                          "toggled",
                          G_CALLBACK (on_startup_enabled_toggled),
                          dialog);
        /* LABEL COLUMN */
        renderer = gtk_cell_renderer_text_new ();

        column = gtk_tree_view_column_new_with_attributes("Lable",
                                                          renderer,
                                                          "markup",STORE_COL_LABEL,
                                                          "sensitive", STORE_COL_ENABLED,
                                                          NULL);
        gtk_tree_view_append_column (treeview, column);


        gtk_tree_view_column_set_sort_column_id (column, STORE_COL_DESCRIPTION);
        gtk_tree_view_set_search_column (treeview, STORE_COL_SEARCH);
        gtk_tree_view_set_rules_hint (treeview, TRUE);

        gtk_tree_view_enable_model_drag_source (treeview,
                                                GDK_BUTTON1_MASK|GDK_BUTTON2_MASK,
                                                NULL, 0,
                                                GDK_ACTION_COPY);
        gtk_drag_source_add_uri_targets (GTK_WIDGET (treeview));

        gtk_drag_dest_set (GTK_WIDGET (treeview),
                           GTK_DEST_DEFAULT_ALL,
                           NULL, 0,
                           GDK_ACTION_COPY);

        gtk_drag_dest_add_uri_targets (GTK_WIDGET (treeview));
        /* we don't want to accept drags coming from this widget */
        targetlist = gtk_drag_dest_get_target_list (GTK_WIDGET (treeview));
        if (targetlist != NULL) {
                GtkTargetEntry *targets;
                gint n_targets;
                gint i;

                targets = gtk_target_table_new_from_list (targetlist, &n_targets);
                for (i = 0; i < n_targets; i++)
                        targets[i].flags = GTK_TARGET_OTHER_WIDGET;

                targetlist = gtk_target_list_new (targets, n_targets);
                gtk_drag_dest_set_target_list (GTK_WIDGET (treeview), targetlist);
                gtk_target_list_unref (targetlist);

                gtk_target_table_free (targets, n_targets);
        }

        g_signal_connect (treeview, "drag_begin",
                          G_CALLBACK (on_drag_begin),
                          dialog);
        g_signal_connect (treeview, "drag_data_get",
                          G_CALLBACK (on_drag_data_get),
                          dialog);
        g_signal_connect (treeview, "drag_data_received",
                          G_CALLBACK (on_drag_data_received),
                          dialog);

        gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (dialog->priv->list_store),
                                              STORE_COL_DESCRIPTION,
                                              GTK_SORT_ASCENDING);

        button = GTK_WIDGET (gtk_builder_get_object (dialog->priv->xml,
                                                     CAPPLET_ADD_WIDGET_NAME));
        dialog->priv->add_button = button;
        g_signal_connect (button,
                          "clicked",
                          G_CALLBACK (on_add_app_clicked),
                          dialog);

        button = GTK_WIDGET (gtk_builder_get_object (dialog->priv->xml,
                                                     CAPPLET_DELETE_WIDGET_NAME));
        dialog->priv->delete_button = button;
        g_signal_connect (button,
                          "clicked",
                          G_CALLBACK (on_delete_app_clicked),
                          dialog);

        button = GTK_WIDGET (gtk_builder_get_object (dialog->priv->xml,
                                                     CAPPLET_EDIT_WIDGET_NAME));
        dialog->priv->edit_button = button;
        g_signal_connect (button,
                          "clicked",
                          G_CALLBACK (on_edit_app_clicked),
                          dialog);

        //dialog->priv->settings = g_settings_new(SPC_SETTINGS_SCHEMA);
        //button = GTK_WIDGET (gtk_builder_get_object (dialog->priv->xml,
        //                                             CAPPLET_REMEMBER_WIDGET_NAME));
        //g_settings_bind (dialog->priv->settings, SPC_SETTINGS_AUTOSAVE_KEY,
        //                 button, "active", G_SETTINGS_BIND_DEFAULT);

        //button = GTK_WIDGET (gtk_builder_get_object (dialog->priv->xml,
        //                                             CAPPLET_SAVE_WIDGET_NAME));
        //g_signal_connect (button,
        //                  "clicked",
        //                  G_CALLBACK (on_save_session_clicked),
        //                  dialog);

        dialog->priv->manager = gsp_app_manager_get ();
        gsp_app_manager_fill (dialog->priv->manager);
        g_signal_connect_swapped (dialog->priv->manager, "added",
                                  G_CALLBACK (_app_added), dialog);
        g_signal_connect_swapped (dialog->priv->manager, "removed",
                                  G_CALLBACK (_app_removed), dialog);
        populate_model (dialog);
}

void gsm_properties_dialog_new(GtkBuilder *builder)
{
        topbuilder = builder;
        dialog1 = &Dialog1;
        dialog2 = &Dialog2;
        dialog1->priv = dialog2;
        dialog1->priv->xml = builder;
        setup_dialog(dialog1);
}
void gsm_properties_dialog_destroy(){
    if (dialog1->priv->manager !=NULL){
        g_object_unref(dialog1->priv->manager);
        dialog1->priv->manager = NULL;
    }
    g_object_unref(dialog1->priv->settings);
}

