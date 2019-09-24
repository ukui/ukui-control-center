/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2004-2006 William Jon McCann <mccann@jhu.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * Authors: William Jon McCann <mccann@jhu.edu>
 *          Rodrigo Moya <rodrigo@novell.com>
 *
 */

#include "config.h"

#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          /* For uid_t, gid_t */

#include <glib/gi18n.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include <gio/gio.h>

#if GTK_CHECK_VERSION (3, 0, 0)
#define MATE_DESKTOP_USE_UNSTABLE_API
#include <mate-desktop-2.0/libmate-desktop/mate-desktop-utils.h>
#define gdk_spawn_command_line_on_screen mate_gdk_spawn_command_line_on_screen
#endif

#include "copy-theme-dialog.h"

#include "gs-theme-manager.h"
#include "gs-job.h"
#include "gs-prefs.h" /* for GS_MODE enum */
#include "kpm-brightness.h"

#define GTK_BUILDER_FILE "ukui-fullscreen-preview.ui"

#define LOCKDOWN_SETTINGS_SCHEMA "org.mate.lockdown"
#define KEY_LOCK_DISABLE "disable-lock-screen"

#define SESSION_SETTINGS_SCHEMA "org.ukui.session"
#define KEY_IDLE_DELAY "idle-delay"

#define GSETTINGS_SCHEMA "org.ukui.screensaver"
#define KEY_LOCK "lock-enabled"
#define KEY_IDLE_ACTIVATION_ENABLED "idle-activation-enabled"
#define KEY_MODE "mode"
#define KEY_LOCK_DELAY "lock-delay"
#define KEY_CYCLE_DELAY "cycle-delay"
#define KEY_THEMES "themes"

#define GPM_COMMAND "ukui-power-preferences"

#define KPM_SETTINGS_BRIGHTNESS_AC			"brightness-ac"
#define KPM_SETTINGS_SCHEMA				"org.ukui.power-manager"

enum
{
    NAME_COLUMN = 0,
    ID_COLUMN,
    N_COLUMNS
};

/* Drag and drop info */
enum
{
    TARGET_URI_LIST,
    TARGET_NS_URL
};

static GtkTargetEntry drop_types [] =
{
    { "text/uri-list", 0, TARGET_URI_LIST },
    { "_NETSCAPE_URL", 0, TARGET_NS_URL }
};

static GtkBuilder     *builder = NULL;
static GtkBuilder     *builder_preview = NULL;   //预览时显示的界面
static GSThemeManager *theme_manager = NULL;
static GSJob          *job = NULL;
static GSettings      *screensaver_settings = NULL;
static GSettings      *session_settings = NULL;
static GSettings      *lockdown_settings = NULL;
static GSettings      *brightness_settings = NULL;

static gint32
config_get_activate_delay (gboolean *is_writable)
{
    gint32 delay;

    if (is_writable)
    {
        *is_writable = g_settings_is_writable (session_settings,
                                               KEY_IDLE_DELAY);
    }

    delay = g_settings_get_int (session_settings, KEY_IDLE_DELAY);

    if (delay < 1)
    {
        delay = 1;
    }

    return delay;
}

static void
config_set_activate_delay (gint32 timeout)
{
    g_settings_set_int (session_settings, KEY_IDLE_DELAY, timeout);
}

static int
config_get_mode (gboolean *is_writable)
{
    int mode;

    if (is_writable)
    {
        *is_writable = g_settings_is_writable (screensaver_settings,
                                               KEY_MODE);
    }

    mode = g_settings_get_enum (screensaver_settings, KEY_MODE);

    return mode;
}

static void
config_set_mode (int mode)
{
    g_settings_set_enum (screensaver_settings, KEY_MODE, mode);
}

static char *
config_get_theme (gboolean *is_writable)
{
    char *name;
    int   mode;
    screensaver_settings = g_settings_new (GSETTINGS_SCHEMA);

    if (is_writable)
    {
        gboolean can_write_theme;
        gboolean can_write_mode;

        can_write_theme = g_settings_is_writable (screensaver_settings,
                                                  KEY_THEMES);
        can_write_mode = g_settings_is_writable (screensaver_settings,
                                                 KEY_MODE);
        *is_writable = can_write_theme && can_write_mode;
    }

    mode = config_get_mode (NULL);

    name = NULL;
    if (mode == GS_MODE_BLANK_ONLY)
    {
        name = g_strdup ("__blank-only");
    }
    else if (mode == GS_MODE_RANDOM)
    {
        name = g_strdup ("__random");
    }
    else
    {
        gchar **strv;
        strv = g_settings_get_strv (screensaver_settings,
                                    KEY_THEMES);
        if (strv != NULL) {
            name = g_strdup (strv[0]);
        }
        else
        {
            /* TODO: handle error */
            /* default to blank */
            name = g_strdup ("__blank-only");
        }

        g_strfreev (strv);
    }

    return name;
}

static gchar **
get_all_theme_ids (GSThemeManager *theme_manager)
{
    gchar **ids = NULL;
    GSList *entries;
    GSList *l;
    guint idx = 0;
    char *info_id;

    entries = gs_theme_manager_get_info_list (theme_manager);
    ids = g_new0 (gchar *, g_slist_length (entries) + 1);
    for (l = entries; l; l = l->next)
    {
        GSThemeInfo *info = l->data;

        info_id = gs_theme_info_get_id (info);
        //屏蔽掉随机中ukui-screensaver中的屏保
        if(!strcmp(info_id, "screensavers-gnomelogo-floaters")
                || !strcmp(info_id, "screensavers-footlogo-floaters")
                || !strcmp(info_id, "screensavers-popsquares")
                || !strcmp(info_id, "screensavers-personal-slideshow")
                || !strcmp(info_id, "screensavers-cosmos-slideshow")
                || !strcmp(info_id, "ukui-screensavers-cosmos-slideshow")
                || !strcmp(info_id, "ukui-screensavers-footlogo-floaters")
                || !strcmp(info_id, "ukui-screensavers-popsquares")
                || !strcmp(info_id, "ukui-screensavers-personal-slideshow")
                || !strcmp(info_id, "ukui-screensavers-gnomelogo-floaters"))
            continue;
        ids[idx++] = g_strdup (info_id);
        gs_theme_info_unref (info);
    }
    g_slist_free (entries);

    return ids;
}

static void
config_set_theme (const char *theme_id)
{
    gchar **strv = NULL;
    int     mode;
    screensaver_settings = g_settings_new (GSETTINGS_SCHEMA);

    if (theme_id && strcmp (theme_id, "__blank-only") == 0)
    {
        mode = GS_MODE_BLANK_ONLY;    //0
    }
    else if (theme_id && strcmp (theme_id, "__random") == 0)
    {
        mode = GS_MODE_RANDOM;        //1

        /* set the themes key to contain all available screensavers */
        strv = get_all_theme_ids (theme_manager);
    }
    else
    {
        mode = GS_MODE_SINGLE;        //2
        strv = g_strsplit (theme_id, "%%%", 1);
    }

    config_set_mode (mode);

    g_settings_set_strv (screensaver_settings,
                         KEY_THEMES,
                         (const gchar * const*) strv);

    g_strfreev (strv);

}

static gboolean
config_get_enabled (gboolean *is_writable)
{
    int enabled;

    if (is_writable)
    {
        *is_writable = g_settings_is_writable (screensaver_settings,
                                               KEY_LOCK);
    }

    enabled = g_settings_get_boolean (screensaver_settings, KEY_IDLE_ACTIVATION_ENABLED);

    return enabled;
}

static void
config_set_enabled (gboolean enabled)
{
    g_settings_set_boolean (screensaver_settings, KEY_IDLE_ACTIVATION_ENABLED, enabled);
    if(enabled)
        system("ukui-screensaver-command --exit; nohup ukui-screensaver > /dev/null 2>&1 &");
}

static gboolean
config_get_lock (gboolean *is_writable)
{
    gboolean lock;

    if (is_writable)
    {
        *is_writable = g_settings_is_writable (screensaver_settings,
                                               KEY_LOCK);
    }

    lock = g_settings_get_boolean (screensaver_settings, KEY_LOCK);

    return lock;
}

static gboolean
config_get_lock_disabled ()
{
    return g_settings_get_boolean (lockdown_settings, KEY_LOCK_DISABLE);
}

static void
config_set_lock (gboolean lock)
{
    g_settings_set_boolean (screensaver_settings, KEY_LOCK, lock);
}

static void
preview_clear (GtkWidget *widget)
{
    //realize用于实例化该widget
    gtk_widget_realize(widget);
    if(gtk_widget_get_window(widget) == NULL){
        g_warning("----The GdkWindow is NULL.----");
        return;
    }
    cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_fill(cr);
    cairo_destroy(cr);
    gtk_widget_queue_draw (widget);
}

static void
job_set_theme (GSJob      *job,
               const char *theme)
{
    GSThemeInfo *info;
    const char  *command;

    command = NULL;

    info = gs_theme_manager_lookup_theme_info (theme_manager, theme);
    if (info != NULL)
    {
        command = gs_theme_info_get_exec (info);
    }

    gs_job_set_command (job, command);

    if (info != NULL)
    {
        gs_theme_info_unref (info);
    }
}

static void
preview_set_theme (GtkWidget  *widget,
                   const char *theme,
                   const char *name)
{
    GtkWidget *label;
    char      *markup;

    if (job != NULL)
    {
        gs_job_stop (job);
    }


    preview_clear (widget);

    //label是预览时的间隔空白，传进来name只是为了给预览时的窗口加上标题
    label = GTK_WIDGET (gtk_builder_get_object (builder_preview, "fullscreen_preview_theme_label"));
    markup = g_markup_printf_escaped ("<i>%s</i>", name);
    gtk_label_set_markup (GTK_LABEL (label), markup);
    g_free (markup);

    if ((theme && strcmp (theme, "__blank-only") == 0))
    {

    }
    else if (theme && strcmp (theme, "__random") == 0)
    {
        gchar **themes;

        themes = get_all_theme_ids (theme_manager);
        if (themes != NULL)
        {
            gint32  i;

            i = g_random_int_range (0, g_strv_length (themes));
            job_set_theme (job, themes[i]);
            g_strfreev (themes);

            gs_job_start (job);
        }
    }
    else
    {
        job_set_theme (job, theme);
        gs_job_start (job);
    }
}

//帮助界面
static void
help_display (void)
{
    GError *error;

    error = NULL;
    //gtk_show_uri (NULL, "help:ukui-user-guide/prefs-screensaver", GDK_CURRENT_TIME, &error);
    gtk_show_uri (NULL, "help:ubuntu-kylin-help/index", GDK_CURRENT_TIME, &error);

    if (error != NULL)
    {
        GtkWidget *d;

        d = gtk_message_dialog_new (NULL,
                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
                                    "%s", error->message);
        gtk_dialog_run (GTK_DIALOG (d));
        gtk_widget_destroy (d);
        g_error_free (error);
    }

}

static GSList *
get_theme_info_list (void)
{
    return gs_theme_manager_get_info_list (theme_manager);
}

//加载屏保
static void
//populate_model (GtkTreeStore *store)
populate_model (GtkTreeModel *model)
{
    GtkTreeIter iter;
    GSList     *themes        = NULL;
    GSList     *l;
    GtkWidget *theme_combo_box = GTK_WIDGET(gtk_builder_get_object(builder, "savers_combox"));

    /*
        //黑屏
    gtk_tree_store_append (store, &iter, NULL);
    gtk_tree_store_set (store, &iter,
                        NAME_COLUMN, _("Blank screen"),
                        ID_COLUMN, "__blank-only",
                        -1);
        //随机
    gtk_tree_store_append (store, &iter, NULL);
    gtk_tree_store_set (store, &iter,
                        NAME_COLUMN, _("Random"),
                        ID_COLUMN, "__random",
                        -1);
        //分隔符
    gtk_tree_store_append (store, &iter, NULL);
    gtk_tree_store_set (store, &iter,
                        NAME_COLUMN, NULL,
                        ID_COLUMN, "__separator",
                        -1);
        */

    //只是添加的name，并没有添加id
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(theme_combo_box), _("Blank screen"));
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(theme_combo_box), _("Random"));

    //其他主题屏保
    themes = get_theme_info_list ();

    if (themes == NULL)
    {
        return;
    }

    for (l = themes; l; l = l->next)
    {
        const char  *name;
        const char  *id;
        GSThemeInfo *info = l->data;

        if (info == NULL)
        {
            continue;
        }

        name = gs_theme_info_get_name (info);
        id = gs_theme_info_get_id (info);
        //屏蔽掉下拉框中的ukui-screensaver中的屏保
        if (!strcmp(id, "screensavers-popsquares")
                || !strcmp(id, "screensavers-gnomelogo-floaters")
                || !strcmp(id, "screensavers-footlogo-floaters")
                || !strcmp(id, "screensavers-personal-slideshow")
                || !strcmp(id, "screensavers-cosmos-slideshow")
                || !strcmp(id, "ukui-screensavers-popsquares")
                || !strcmp(id, "ukui-screensavers-gnomelogo-floaters")
                || !strcmp(id, "ukui-screensavers-footlogo-floaters")
                || !strcmp(id, "ukui-screensavers-personal-slideshow")
                || !strcmp(id, "ukui-screensavers-cosmos-slideshow"))
            continue;

        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(theme_combo_box), name);

        gs_theme_info_unref (info);
    }

    g_slist_free (themes);
}

static void
tree_selection_previous (GtkTreeSelection *selection)
{
    GtkTreeIter   iter;
    GtkTreeModel *model;
    GtkTreePath  *path;

    if (! gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        return;
    }

    path = gtk_tree_model_get_path (model, &iter);
    if (gtk_tree_path_prev (path))
    {
        gtk_tree_selection_select_path (selection, path);
    }
}

static void
tree_selection_next (GtkTreeSelection *selection)
{
    GtkTreeIter   iter;
    GtkTreeModel *model;
    GtkTreePath  *path;

    if (! gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        return;
    }

    path = gtk_tree_model_get_path (model, &iter);
    gtk_tree_path_next (path);
    gtk_tree_selection_select_path (selection, path);
}

static void
combo_box_changed_cb (GtkWidget *theme_combo_box)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    char *theme;
    GtkWidget *preview  = GTK_WIDGET (gtk_builder_get_object (builder, "preview_area"));
    gtk_widget_show_all(preview);
    if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(theme_combo_box), &iter))
    {
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(theme_combo_box));
        gtk_tree_model_get(model, &iter, 0, &theme, -1);
        GList *themes = NULL;
        GList *l;
        themes = get_theme_info_list ();
        if (themes == NULL)
        {
            return;
        }
        //处理黑屏和随机的情况，因为只有theme(对应name)
        if(!strcmp(theme, _("Random"))){
            char *random_id = "__random";
            preview_set_theme (preview, random_id, theme);
            config_set_theme (random_id);
        }
        if(!strcmp(theme, _("Blank screen"))){
            char *blank_id = "__blank-only";
            preview_set_theme (preview, blank_id, theme);
            config_set_theme (blank_id);
        }

        //其他的屏保，通过theme(name)可以获得其id值
        for (l = themes; l; l = l->next)
        {
            char  *name;
            char  *id;
            GSThemeInfo *info = l->data;

            if (info == NULL)
            {
                continue;
            }

            name = gs_theme_info_get_name (info);
            id = gs_theme_info_get_id (info);

            if(!strcmp(theme, name))
            {
                //设置屏保
                preview_set_theme (preview, id, name);
                config_set_theme (id);
            }

            gs_theme_info_unref (info);
        }
        //g_free(theme);
        g_slist_free (themes);
    }
}

//为了保持和锁屏的时间布局一致，这里用label来显示值的变化
static void
brightness_value_changed(GtkRange *range,
                         gpointer user_data)
{
    gchar label_text[10];
    int value = gtk_range_get_value(range);
    sprintf(label_text, "%d%%", value);
    GtkWidget *label_value = GTK_WIDGET (gtk_builder_get_object (builder, "label_value"));
    gtk_label_set_text(GTK_LABEL(label_value),label_text);
}

static void
activate_delay_value_changed_cb (GtkRange *range,
                                 gpointer  user_data)
{
    gchar *label_text;
    int value = gtk_range_get_value (range);
    int time = value * 60;
    int min = (time % (60 * 60)) / 60;
    int hour = time / (60 * 60);
    if (!hour)
        label_text = g_strdup_printf (_("%d min"), min);
    else
        label_text = g_strdup_printf (_("%d hour %d min"), hour,min);
    GtkWidget *label = GTK_WIDGET (gtk_builder_get_object (builder, "scale_label"));
    gtk_label_set_text(GTK_LABEL(label),"");
    gtk_label_set_text(GTK_LABEL(label),label_text);
    config_set_activate_delay ((gint32)value);
}

static int
compare_theme_names (char *name_a,
                     char *name_b,
                     char *id_a,
                     char *id_b)
{

    if (id_a == NULL)
    {
        return 1;
    }
    else if (id_b == NULL)
    {
        return -1;
    }

    if (strcmp (id_a, "__blank-only") == 0)
    {
        return -1;
    }
    else if (strcmp (id_b, "__blank-only") == 0)
    {
        return 1;
    }
    else if (strcmp (id_a, "__random") == 0)
    {
        return -1;
    }
    else if (strcmp (id_b, "__random") == 0)
    {
        return 1;
    }
    else if (strcmp (id_a, "__separator") == 0)
    {
        return -1;
    }
    else if (strcmp (id_b, "__separator") == 0)
    {
        return 1;
    }

    if (name_a == NULL)
    {
        return 1;
    }
    else if (name_b == NULL)
    {
        return -1;
    }

    return g_utf8_collate (name_a, name_b);
}

static int
compare_theme  (GtkTreeModel *model,
                GtkTreeIter  *a,
                GtkTreeIter  *b,
                gpointer      user_data)
{
    char *name_a;
    char *name_b;
    char *id_a;
    char *id_b;
    int   result;

    gtk_tree_model_get (model, a, NAME_COLUMN, &name_a, -1);
    gtk_tree_model_get (model, b, NAME_COLUMN, &name_b, -1);
    gtk_tree_model_get (model, a, ID_COLUMN, &id_a, -1);
    gtk_tree_model_get (model, b, ID_COLUMN, &id_b, -1);

    result = compare_theme_names (name_a, name_b, id_a, id_b);

    g_free (name_a);
    g_free (name_b);
    g_free (id_a);
    g_free (id_b);

    return result;
}

static void
setup_combo_box (GtkWidget *theme_combo_box,
                 GtkWidget *preview)
{
    GtkTreeModel *model;
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(theme_combo_box));
    populate_model (GTK_TREE_MODEL(model));
    //GtkWidget *theme_combo_box = GTK_WIDGET(gtk_builder_get_object(builder, "comboboxtext1"));
    g_signal_connect (G_OBJECT (theme_combo_box), "changed",
                      G_CALLBACK (combo_box_changed_cb),
                      NULL);
}


//初始化combo_box的值
static void
setup_combo_box_selection (GtkWidget *combo_box)
{
    char            *theme;
    char            *name;
    char            *combo_box_text;
    GtkTreeModel *model;
    GtkTreeIter   iter;
    gboolean      is_writable;
    gboolean      valid;

    //获取到当前所设置的屏保的id
    theme = config_get_theme (&is_writable);

    if (! is_writable)
    {
        gtk_widget_set_sensitive (combo_box, FALSE);
    }
    //先通过id获取到name
    if (!strcmp(theme, "__random"))
        name = _("Random");
    if (!strcmp(theme, "__blank-only"))
        name = _("Blank screen");

    GList *themes = NULL;
    GList *l;
    themes = get_theme_info_list ();
    if (themes == NULL)
    {
        return;
    }
    for (l = themes; l; l = l->next)
    {
        //const char  *name;
        const char  *id;
        GSThemeInfo *info = l->data;

        if (info == NULL)
        {
            continue;
        }

        //name = gs_theme_info_get_name (info);
        id = gs_theme_info_get_id (info);

        if(!strcmp(theme, id))
        {
            name = gs_theme_info_get_name (info);
            break;
        }
        gs_theme_info_unref (info);
    }

    //再通过当前name和从combo_box中获得到的name进行比较
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box));
    valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);

    while (valid && name)
    {
        gtk_tree_model_get(model, &iter, 0, &combo_box_text, -1);
        if(!strcmp(name, combo_box_text)){
            gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo_box), &iter);
            //preview_set_theme (preview, theme, name);
            valid = FALSE;
        }
        else
            valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(model), &iter);
    }
    g_free(theme);
    g_slist_free (themes);

}

//加载combo_box中的各种屏保
static void
reload_themes (void)
{
    GtkWidget *theme_combo_box;
    GtkTreeModel *model;
    theme_combo_box = GTK_WIDGET(gtk_builder_get_object(builder, "savers_combox"));
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(theme_combo_box));
    populate_model (GTK_TREE_MODEL(model));
    gtk_combo_box_set_model(GTK_COMBO_BOX (theme_combo_box),
                            GTK_TREE_MODEL (model));
}

static void
theme_copy_complete_cb (GtkWidget *dialog, gpointer user_data)
{
    reload_themes ();
    gtk_widget_destroy (dialog);
}

static void
theme_installer_run (GtkWidget *prefs_dialog, GList *files)
{
    GtkWidget *copy_dialog;

    copy_dialog = copy_theme_dialog_new (files);
    g_list_foreach (files, (GFunc) (g_object_unref), NULL);
    g_list_free (files);

    gtk_window_set_transient_for (GTK_WINDOW (copy_dialog),
                                  GTK_WINDOW (prefs_dialog));
    gtk_window_set_icon_name (GTK_WINDOW (copy_dialog),
                              "preferences-desktop-screensaver");

    g_signal_connect (copy_dialog, "complete",
                      G_CALLBACK (theme_copy_complete_cb), NULL);

    copy_theme_dialog_begin (COPY_THEME_DIALOG (copy_dialog));
}

/* Callback issued during drag movements */
static gboolean
drag_motion_cb (GtkWidget      *widget,
                GdkDragContext *context,
                int             x,
                int             y,
                guint           time,
                gpointer        data)
{
    return FALSE;
}

/* Callback issued during drag leaves */
static void
drag_leave_cb (GtkWidget      *widget,
               GdkDragContext *context,
               guint           time,
               gpointer        data)
{
    gtk_widget_queue_draw (widget);
}

/* GIO has no version of mate_vfs_uri_list_parse(), so copy from MateVFS
 * and re-work to create GFiles.
**/
static GList *
uri_list_parse (const gchar *uri_list)
{
    const gchar *p, *q;
    gchar *retval;
    GFile *file;
    GList *result = NULL;

    g_return_val_if_fail (uri_list != NULL, NULL);

    p = uri_list;

    /* We don't actually try to validate the URI according to RFC
     * 2396, or even check for allowed characters - we just ignore
     * comments and trim whitespace off the ends.  We also
     * allow LF delimination as well as the specified CRLF.
     */
    while (p != NULL)
    {
        if (*p != '#')
        {
            while (g_ascii_isspace (*p))
                p++;

            q = p;
            while ((*q != '\0')
                   && (*q != '\n')
                   && (*q != '\r'))
                q++;

            if (q > p)
            {
                q--;
                while (q > p
                       && g_ascii_isspace (*q))
                    q--;

                retval = g_malloc (q - p + 2);
                strncpy (retval, p, q - p + 1);
                retval[q - p + 1] = '\0';

                file = g_file_new_for_uri (retval);

                g_free (retval);

                if (file != NULL)
                    result = g_list_prepend (result, file);
            }
        }
        p = strchr (p, '\n');
        if (p != NULL)
            p++;
    }

    return g_list_reverse (result);
}

/* Callback issued on actual drops. Attempts to load the file dropped. */
static void
drag_data_received_cb (GtkWidget        *widget,
                       GdkDragContext   *context,
                       int               x,
                       int               y,
                       GtkSelectionData *selection_data,
                       guint             info,
                       guint             time,
                       gpointer          data)
{
    GList     *files;

    if (!(info == TARGET_URI_LIST || info == TARGET_NS_URL))
        return;

    files = uri_list_parse ((char *) gtk_selection_data_get_data (selection_data));
    if (files != NULL)
    {
        GtkWidget *prefs_dialog;

        prefs_dialog = GTK_WIDGET (gtk_builder_get_object (builder, "prefs_dialog"));
        theme_installer_run (prefs_dialog, files);
    }
}

static void
lock_checkbox_toggled (GtkToggleButton *button, gpointer user_data)
{
    config_set_lock (gtk_toggle_button_get_active (button));
}

static void
enabled_checkbox_toggled (GtkToggleButton *button, gpointer user_data)
{
    config_set_enabled (gtk_toggle_button_get_active (button));
}

static void
ui_disable_lock (gboolean disable)
{
    GtkWidget *widget;

    widget = GTK_WIDGET (gtk_builder_get_object (builder, "screensaver_lock_checkbox"));
    gtk_widget_set_sensitive (widget, !disable);
    if (disable)
    {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), FALSE);
    }
}

static void
ui_set_lock (gboolean enabled)
{
    GtkWidget *widget;
    gboolean   active;
    gboolean   lock_disabled;

    widget = GTK_WIDGET (gtk_builder_get_object (builder, "screensaver_lock_checkbox"));

    active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    if (active != enabled)
    {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), enabled);
    }
    lock_disabled = config_get_lock_disabled ();
    ui_disable_lock (lock_disabled);
}

static void
ui_set_enabled (gboolean enabled)
{
    GtkWidget *widget;
    gboolean   active;
    gboolean   is_writable;
    gboolean   lock_disabled;

    widget = GTK_WIDGET (gtk_builder_get_object (builder, "screensaver_enable_checkbox"));
    active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    if (active != enabled)
    {
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), enabled);
    }

    widget = GTK_WIDGET (gtk_builder_get_object (builder, "screensaver_lock_checkbox"));
    config_get_lock (&is_writable);
    if (is_writable)
    {
        gtk_widget_set_sensitive (widget, enabled);
    }
    lock_disabled = config_get_lock_disabled ();
    ui_disable_lock(lock_disabled);
}

static void
ui_set_delay (int delay)
{
    GtkWidget *widget;

    widget = GTK_WIDGET (gtk_builder_get_object (builder, "activate_delay_hscale"));
    gtk_range_set_value (GTK_RANGE (widget), delay);
}

static void
key_changed_cb (GSettings *settings, const gchar *key, gpointer data)
{
    if (strcmp (key, KEY_IDLE_ACTIVATION_ENABLED) == 0)
    {
        gboolean enabled;

        enabled = g_settings_get_boolean (settings, key);

        ui_set_enabled (enabled);
    }
    else if (strcmp (key, KEY_LOCK) == 0)
    {
        gboolean enabled;

        enabled = g_settings_get_boolean (settings, key);

        ui_set_lock (enabled);
    }
    else if (strcmp (key, KEY_LOCK_DISABLE) == 0)
    {
        gboolean disabled;

        disabled = g_settings_get_boolean (settings, key);

        ui_disable_lock (disabled);
    }
    else if (strcmp (key, KEY_THEMES) == 0)
    {
        GtkWidget *combo_box;
        combo_box = GTK_WIDGET (gtk_builder_get_object (builder, "savers_combox"));
        setup_combo_box_selection (combo_box);
    }
    else if (strcmp (key, KEY_IDLE_DELAY) == 0)
    {
        int delay;

        delay = g_settings_get_int (settings, key);
        ui_set_delay (delay);

    }
    else
    {
        /*g_warning ("Config key not handled: %s", key);*/
    }
}

//预览下的向前操作的callback
static void
fullscreen_preview_previous_cb (GtkWidget *fullscreen_preview_window,
                                gpointer   user_data)
{
    GtkWidget        *combo_box;
    GtkTreeIter      iter;
    GtkTreeModel     *model;

    combo_box = GTK_WIDGET (gtk_builder_get_object (builder, "savers_combox"));
    gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combo_box), &iter);
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box));
    //这里因为gtk_tree_model_iter_previous这个api是3.0的，所以我也不知道怎么获取它的上一个迭代器。

    if(gtk_tree_model_iter_previous(model, &iter))
        gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo_box), &iter);
    else
        return;
}

//预览模式下向后操作的callback
static void
fullscreen_preview_next_cb (GtkWidget *fullscreen_preview_window,
                            gpointer   user_data)
{
    GtkWidget        *combo_box;
    GtkTreeIter      iter;
    GtkTreeModel     *model;

    combo_box = GTK_WIDGET (gtk_builder_get_object (builder, "savers_combox"));
    gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combo_box), &iter);
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(combo_box));
    if(gtk_tree_model_iter_next(model, &iter))
        gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo_box), &iter);
    else
        return;
}

//预览模式下取消的callback
static void
fullscreen_preview_cancelled_cb (GtkWidget *button,
                                 gpointer   user_data)
{

    GtkWidget *fullscreen_preview_area;
    GtkWidget *fullscreen_preview_window;
    GtkWidget *preview_area;
    GtkWidget *dialog;

    preview_area = GTK_WIDGET (gtk_builder_get_object (builder, "preview_area"));
    gs_job_set_widget (job, preview_area);

    fullscreen_preview_area = GTK_WIDGET (gtk_builder_get_object (builder_preview, "fullscreen_preview_area"));
    preview_clear (fullscreen_preview_area);

    fullscreen_preview_window = GTK_WIDGET (gtk_builder_get_object (builder_preview, "fullscreen_preview_window"));
    gtk_widget_hide (fullscreen_preview_window);
}

//预览显示的回调
static void
fullscreen_preview_start_cb (GtkWidget *widget,
                             gpointer   user_data)
{
    GtkWidget *fullscreen_preview_area;
    GtkWidget *fullscreen_preview_window;

    fullscreen_preview_window = GTK_WIDGET (gtk_builder_get_object (builder_preview, "fullscreen_preview_window"));

    gtk_window_fullscreen (GTK_WINDOW (fullscreen_preview_window));
    gtk_window_set_keep_above (GTK_WINDOW (fullscreen_preview_window), TRUE);

    gtk_widget_show (fullscreen_preview_window);
    gtk_widget_grab_focus (fullscreen_preview_window);

    fullscreen_preview_area = GTK_WIDGET (gtk_builder_get_object (builder_preview, "fullscreen_preview_area"));


    preview_clear (fullscreen_preview_area);
    gs_job_set_widget (job, fullscreen_preview_area);
}

static void
constrain_list_size (GtkWidget      *widget,
                     GtkRequisition *requisition,
                     GtkWidget      *to_size)
{
    GtkRequisition req;
    int            max_height;

    /* constrain height to be the tree height up to a max */
    max_height = (gdk_screen_get_height (gtk_widget_get_screen (widget))) / 4;

    gtk_widget_size_request (to_size, &req);

    requisition->height = MIN (req.height, max_height);
}

static gboolean
check_is_root_user (void)
{
#ifndef G_OS_WIN32
    uid_t ruid, euid, suid; /* Real, effective and saved user ID's */
    gid_t rgid, egid, sgid; /* Real, effective and saved group ID's */

#ifdef HAVE_GETRESUID
    if (getresuid (&ruid, &euid, &suid) != 0 ||
            getresgid (&rgid, &egid, &sgid) != 0)
#endif /* HAVE_GETRESUID */
    {
        suid = ruid = getuid ();
        sgid = rgid = getgid ();
        euid = geteuid ();
        egid = getegid ();
    }

    if (ruid == 0)
    {
        return TRUE;
    }

#endif
    return FALSE;
}

static void
setup_for_root_user (void)
{
    GtkWidget *lock_checkbox;
    GtkWidget *label;

    lock_checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "screensaver_lock_checkbox"));
    label = GTK_WIDGET (gtk_builder_get_object (builder, "root_warning_label"));

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (lock_checkbox), FALSE);
    gtk_widget_set_sensitive (lock_checkbox, FALSE);

    gtk_widget_show (label);
}

static GdkVisual *
get_best_visual (void)
{
    char         *command;
    char         *std_output;
    int           exit_status;
    GError       *error;
    unsigned long v;
    char          c;
    GdkVisual    *visual;
    gboolean      res;

    visual = NULL;

    command = g_build_filename (LIBEXECDIR, "ukui-screensaver-gl-helper", NULL);

    error = NULL;
    res = g_spawn_command_line_sync (command,
                                     &std_output,
                                     NULL,
                                     &exit_status,
                                     &error);

    if (! res)
    {
        g_debug ("Could not run command '%s': %s", command, error->message);
        g_error_free (error);
        goto out;
    }

    if (1 == sscanf (std_output, "0x%lx %c", &v, &c))
    {
        if (v != 0)
        {
            VisualID      visual_id;

            visual_id = (VisualID) v;
            visual = gdk_x11_screen_lookup_visual (gdk_screen_get_default (), visual_id);

            g_debug ("Found best visual for GL: 0x%x",
                     (unsigned int) visual_id);
        }
    }

out:
    g_free (std_output);
    g_free (command);

    return visual;
}

//#if GTK_CHECK_VERSION (3, 0, 0)
/* copied from gs-window-x11.c */
extern char **environ;

static gchar **
spawn_make_environment_for_screen (GdkScreen  *screen,
                                   gchar     **envp)
{
    gchar **retval = NULL;
    gchar  *display_name;
    gint    display_index = -1;
    gint    i, env_len;

    g_return_val_if_fail (GDK_IS_SCREEN (screen), NULL);

    if (envp == NULL)
        envp = environ;

    for (env_len = 0; envp[env_len]; env_len++)
        if (strncmp (envp[env_len], "DISPLAY", strlen ("DISPLAY")) == 0)
            display_index = env_len;

    retval = g_new (char *, env_len + 1);
    retval[env_len] = NULL;

    display_name = gdk_screen_make_display_name (screen);

    for (i = 0; i < env_len; i++)
        if (i == display_index)
            retval[i] = g_strconcat ("DISPLAY=", display_name, NULL);
        else
            retval[i] = g_strdup (envp[i]);

    g_assert (i == env_len);

    g_free (display_name);

    return retval;
}

static gboolean
spawn_command_line_on_screen_sync (GdkScreen    *screen,
                                   const gchar  *command_line,
                                   char        **standard_output,
                                   char        **standard_error,
                                   int          *exit_status,
                                   GError      **error)
{
    char     **argv = NULL;
    char     **envp = NULL;
    gboolean   retval;

    g_return_val_if_fail (command_line != NULL, FALSE);

    if (! g_shell_parse_argv (command_line, NULL, &argv, error))
    {
        return FALSE;
    }

    envp = spawn_make_environment_for_screen (screen, NULL);

    retval = g_spawn_sync (NULL,
                           argv,
                           envp,
                           G_SPAWN_SEARCH_PATH,
                           NULL,
                           NULL,
                           standard_output,
                           standard_error,
                           exit_status,
                           error);

    g_strfreev (argv);
    g_strfreev (envp);

    return retval;
}


static GdkVisual *
get_best_visual_for_screen (GdkScreen *screen)
{
    char         *command;
    char         *std_output;
    int           exit_status;
    GError       *error;
    unsigned long v;
    char          c;
    GdkVisual    *visual;
    gboolean      res;

    visual = NULL;

    command = g_build_filename (LIBEXECDIR, "ukui-screensaver-gl-helper", NULL);

    error = NULL;
    std_output = NULL;
    res = spawn_command_line_on_screen_sync (screen,
                                             command,
                                             &std_output,
                                             NULL,
                                             &exit_status,
                                             &error);
    if (! res)
    {
        g_debug ("Could not run command '%s': %s", command, error->message);
        g_error_free (error);
        goto out;
    }

    if (1 == sscanf (std_output, "0x%lx %c", &v, &c))
    {
        if (v != 0)
        {
            VisualID      visual_id;

            visual_id = (VisualID) v;
            visual = gdk_x11_screen_lookup_visual (screen, visual_id);

            g_debug ("Found best GL visual for screen %d: 0x%x",
                     gdk_screen_get_number (screen),
                     (unsigned int) visual_id);
        }
    }
out:
    g_free (std_output);
    g_free (command);

    return visual;
}


static void
widget_set_best_visual (GtkWidget *widget)
{
    GdkVisual *visual;

    g_return_if_fail (widget != NULL);

    visual = get_best_visual_for_screen (gtk_widget_get_screen (widget));
    if (visual != NULL)
    {
        gtk_widget_set_visual (widget, visual);
        g_object_unref (visual);
    }
}

static gboolean
setup_treeview_idle (gpointer data)
{
    GtkWidget *preview;
    GtkWidget *theme_combo_box;

    preview  = GTK_WIDGET (gtk_builder_get_object (builder, "preview_area"));
    //treeview = GTK_WIDGET (gtk_builder_get_object (builder, "savers_treeview"));
    theme_combo_box = GTK_WIDGET (gtk_builder_get_object (builder, "savers_combox"));

    setup_combo_box (theme_combo_box, preview);
    setup_combo_box_selection (theme_combo_box);
    //setup_treeview_selection (treeview);

    return FALSE;
}

static gboolean
is_program_in_path (const char *program)
{
    char *tmp = g_find_program_in_path (program);
    if (tmp != NULL)
    {
        g_free (tmp);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void init_scale(GtkWidget *scale_label,gdouble time)
{
    gchar *label_text;
    int timeout = time;
    int hour = timeout/60;
    int min = timeout%60;
    if (!hour)
        label_text = g_strdup_printf (_("%d min"), min);
    else
        label_text = g_strdup_printf (_("%d hour %d min"), hour,min);
    gtk_label_set_text(GTK_LABEL(scale_label),label_text);
}

//void show_preview(GtkWidget *widget, gpointer user_data)
//{
//    g_warning("------------111111111-----------");
//    GtkWidget *combo_box  = GTK_WIDGET (gtk_builder_get_object (builder, "savers_combox"));
//    combo_box_changed_cb (combo_box);
//}

//magical function
gboolean show_preview(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    GdkRGBA black = { 0.0, 0.0, 0.0, 1.0 };
    gdk_cairo_set_source_rgba(cr, &black);
    cairo_paint(cr);
    return FALSE;
}

static void
init_capplet (void)
{
    GtkWidget *dialog;
    GtkWidget *preview;
    GtkWidget *combo_box;
    GtkWidget *list_scroller;
    GtkWidget *activate_delay_hscale;
    GtkWidget *activate_delay_hbox;
    GtkWidget *label;
    GtkWidget *enabled_checkbox;
    GtkWidget *lock_checkbox;
    GtkWidget *root_warning_label;
    GtkWidget *preview_button;
    GtkWidget *gpm_button;
    GtkWidget *fullscreen_preview_window;
    GtkWidget *fullscreen_preview_previous;
    GtkWidget *fullscreen_preview_next;
    GtkWidget *fullscreen_preview_area;
    GtkWidget *fullscreen_preview_close;
    char      *gtk_builder_file;
    gdouble    activate_delay;
    gboolean   enabled;
    gboolean   is_writable;
    GError    *error=NULL;
    gint       mode;
    GtkWidget *vp_screen;

    //gtk_builder_file = g_build_filename (GTKBUILDERDIR, GTK_BUILDER_FILE, NULL);
    builder_preview = gtk_builder_new();
    if (!gtk_builder_add_from_file(builder_preview, UIDIR "/ukui-fullscreen-preview.ui", &error))
    {
        g_warning("Couldn't load builder file: %s", error->message);
        g_error_free(error);
    }
    g_free (gtk_builder_file);


    if (builder == NULL)
    {

        dialog = gtk_message_dialog_new (NULL,
                                         0, GTK_MESSAGE_ERROR,
                                         GTK_BUTTONS_OK,
                                         _("Could not load the main interface"));
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                                  _("Please make sure that the screensaver is properly installed"));

        gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                         GTK_RESPONSE_OK);
        gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);
        exit (1);
    }

    preview            = GTK_WIDGET (gtk_builder_get_object (builder, "preview_area"));
    activate_delay_hscale = GTK_WIDGET (gtk_builder_get_object (builder, "activate_delay_hscale"));
    enabled_checkbox   = GTK_WIDGET (gtk_builder_get_object (builder, "screensaver_enable_checkbox"));
    lock_checkbox      = GTK_WIDGET (gtk_builder_get_object (builder, "screensaver_lock_checkbox"));
    preview_button     = GTK_WIDGET (gtk_builder_get_object (builder, "preview_button"));
    combo_box          = GTK_WIDGET (gtk_builder_get_object (builder, "savers_combox"));
    GtkWidget *scale_label = GTK_WIDGET (gtk_builder_get_object (builder, "scale_label"));
    gtk_label_set_xalign(GTK_LABEL(scale_label), 0.0);
    //调节显示器亮度的相关设置
    GtkWidget *label_ac_brightness = GTK_WIDGET (gtk_builder_get_object (builder, "label_ac_brightness"));
    gtk_label_set_xalign(GTK_LABEL(label_ac_brightness), 0.0);
    GtkWidget *label_value = GTK_WIDGET (gtk_builder_get_object (builder, "label_value"));
    gtk_label_set_xalign(GTK_LABEL(label_value), 0.17);
    GtkWidget *hscale_ac_brightness = GTK_WIDGET (gtk_builder_get_object (builder, "hscale_ac_brightness"));
    brightness_settings = g_settings_new (KPM_SETTINGS_SCHEMA);
    g_settings_bind (brightness_settings, KPM_SETTINGS_BRIGHTNESS_AC,
                     gtk_range_get_adjustment (GTK_RANGE (hscale_ac_brightness)), "value",
                     G_SETTINGS_BIND_DEFAULT);
    //初始化显示器亮度的设置,直接调用回调
    brightness_value_changed(hscale_ac_brightness, NULL);
    g_signal_connect(hscale_ac_brightness, "value-changed",
                     G_CALLBACK (brightness_value_changed), NULL);
    //检查硬件是否支持亮度设置
    KpmBrightness *brightness = kpm_brightness_new ();
    gboolean has_lcd = kpm_brightness_has_hw (brightness);
    g_object_unref (brightness);
    //不支持时隐藏设置选项并调整布局
    if(has_lcd == FALSE)
    {
        GtkWidget *widget = GTK_WIDGET (gtk_builder_get_object (builder, "brightness_hbox"));
        gtk_widget_hide(widget);
        widget = GTK_WIDGET (gtk_builder_get_object (builder, "layout10_screensaver"));
        gtk_layout_move(GTK_LAYOUT(widget), enabled_checkbox, 25, 415);
        gtk_layout_move(GTK_LAYOUT(widget), lock_checkbox, 25, 450);
    }
    fullscreen_preview_window = GTK_WIDGET (gtk_builder_get_object (builder_preview, "fullscreen_preview_window"));
    fullscreen_preview_area = GTK_WIDGET (gtk_builder_get_object (builder_preview, "fullscreen_preview_area"));
    fullscreen_preview_close = GTK_WIDGET (gtk_builder_get_object (builder_preview, "fullscreen_preview_close"));
    fullscreen_preview_previous = GTK_WIDGET (gtk_builder_get_object (builder_preview, "fullscreen_preview_previous_button"));
    fullscreen_preview_next = GTK_WIDGET (gtk_builder_get_object (builder_preview, "fullscreen_preview_next_button"));

    preview_clear (fullscreen_preview_area);
    GdkColor black;
    gdk_color_parse("BLACK", &black);
    gtk_widget_modify_bg(fullscreen_preview_area, GTK_STATE_NORMAL, &black);

    //因为没有向前的api，所以暂时先屏蔽掉向前和向后预览
    gtk_widget_set_no_show_all (fullscreen_preview_previous, FALSE);
    gtk_widget_set_no_show_all (fullscreen_preview_next, FALSE);
    gtk_widget_hide(fullscreen_preview_previous);
    gtk_widget_hide(fullscreen_preview_next);

    g_signal_connect(preview, "draw", G_CALLBACK(show_preview), NULL);
    g_signal_connect(fullscreen_preview_area, "draw", G_CALLBACK(show_preview), NULL);

    label              = GTK_WIDGET (gtk_builder_get_object (builder, "activate_delay_label"));
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), activate_delay_hscale);

    label              = GTK_WIDGET (gtk_builder_get_object (builder, "label63_screen"));
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);

    widget_set_best_visual (preview);

    if (! is_program_in_path (GPM_COMMAND))
    {
        gtk_widget_set_no_show_all (gpm_button, TRUE);
        gtk_widget_hide (gpm_button);
    }

    screensaver_settings = g_settings_new (GSETTINGS_SCHEMA);
    g_signal_connect (screensaver_settings,
                      "changed",
                      G_CALLBACK (key_changed_cb),
                      NULL);

    session_settings = g_settings_new (SESSION_SETTINGS_SCHEMA);
    g_signal_connect (session_settings,
                      "changed::" KEY_IDLE_DELAY,
                      G_CALLBACK (key_changed_cb),
                      NULL);


    lockdown_settings = g_settings_new (LOCKDOWN_SETTINGS_SCHEMA);
    g_signal_connect (lockdown_settings,
                      "changed::" KEY_LOCK_DISABLE,
                      G_CALLBACK (key_changed_cb),
                      NULL);
    activate_delay = config_get_activate_delay (&is_writable);
    ui_set_delay (activate_delay);
    if (! is_writable)
    {
        gtk_widget_set_sensitive (activate_delay_hbox, FALSE);
    }

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (lock_checkbox), config_get_lock (&is_writable));
    if (! is_writable)
    {
        gtk_widget_set_sensitive (lock_checkbox, FALSE);
    }
    g_signal_connect (lock_checkbox, "toggled",
                      G_CALLBACK (lock_checkbox_toggled), NULL);

    enabled = config_get_enabled (&is_writable);
    init_scale(scale_label, activate_delay);
    ui_set_enabled (enabled);
    if (! is_writable)
    {
        gtk_widget_set_sensitive (enabled_checkbox, FALSE);
    }
    g_signal_connect (enabled_checkbox, "toggled",
                      G_CALLBACK (enabled_checkbox_toggled), NULL);

    /* Update list of themes if using random screensaver */
    mode = g_settings_get_enum (screensaver_settings, KEY_MODE);
    if (mode == GS_MODE_RANDOM) {
        gchar **list;
        list = get_all_theme_ids (theme_manager);
        g_settings_set_strv (screensaver_settings, KEY_THEMES, (const gchar * const*) list);
        g_strfreev (list);
    }
    preview_clear (preview);
    //设置job中的widget，用于显示屏保
    gs_job_set_widget (job, preview);

    if (check_is_root_user ())
    {
        setup_for_root_user ();
    }

    g_signal_connect (activate_delay_hscale, "value-changed",
                      G_CALLBACK (activate_delay_value_changed_cb), NULL);

    g_signal_connect (preview_button, "clicked",
                      G_CALLBACK (fullscreen_preview_start_cb),
                      combo_box);

    g_signal_connect (fullscreen_preview_close, "clicked",
                      G_CALLBACK (fullscreen_preview_cancelled_cb), NULL);
    g_signal_connect (fullscreen_preview_previous, "clicked",
                      G_CALLBACK (fullscreen_preview_previous_cb), NULL);
    g_signal_connect (fullscreen_preview_next, "clicked",
                      G_CALLBACK (fullscreen_preview_next_cb), NULL);

    g_idle_add ((GSourceFunc)setup_treeview_idle, NULL);
}

static void
finalize_capplet (void)
{
    //正确的做法是在回调里直接new一个，修改完之后再减引用计数，但这里设置的是一个
    //全局的gsettings所以不能这样。
    //g_object_unref (screensaver_settings);
    //g_object_unref (session_settings);
    //g_object_unref (lockdown_settings);
}

void screensaver_init(GtkBuilder *screensaver_builder)
{
    g_warning("add screensaver");
    builder = screensaver_builder;
    job = gs_job_new();
    theme_manager = gs_theme_manager_new ();
    init_capplet();
    GSPrefs *prefs = gs_prefs_new ();
    // finalize_capplet();
}
