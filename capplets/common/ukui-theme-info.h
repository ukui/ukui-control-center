/* ukui-theme-info.h - UKUI Theme information

   Copyright (C) 2002 Jonathan Blandford <jrb@gnome.org>
   Copyright (C) 2016,Tianjin KYLIN Information Technology Co., Ltd.
   All rights reserved.

   This file is part of the Ukui Library.

   The Ukui Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Ukui Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Ukui Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

#ifndef UKUI_THEME_INFO_H
#define UKUI_THEME_INFO_H

#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>

typedef enum {
	UKUI_THEME_TYPE_METATHEME,
	UKUI_THEME_TYPE_ICON,
	UKUI_THEME_TYPE_CURSOR,
	UKUI_THEME_TYPE_REGULAR
} UkuiThemeType;

typedef enum {
	UKUI_THEME_CHANGE_CREATED,
	UKUI_THEME_CHANGE_DELETED,
	UKUI_THEME_CHANGE_CHANGED
} UkuiThemeChangeType;

typedef enum {
	UKUI_THEME_UKWM = 1 << 0,
	UKUI_THEME_GTK_2 = 1 << 1,
	UKUI_THEME_GTK_2_KEYBINDING = 1 << 2
} UkuiThemeElement;

typedef struct _UkuiThemeCommonInfo UkuiThemeCommonInfo;
typedef struct _UkuiThemeCommonInfo UkuiThemeIconInfo;
struct _UkuiThemeCommonInfo
{
	UkuiThemeType type;
	gchar* path;
	gchar* name;
	gchar* readable_name;
	gint priority;
	gboolean hidden;
};

typedef struct _UkuiThemeInfo UkuiThemeInfo;
struct _UkuiThemeInfo
{
	UkuiThemeType type;
	gchar* path;
	gchar* name;
	gchar* readable_name;
	gint priority;
	gboolean hidden;

	guint has_gtk : 1;
	guint has_keybinding : 1;
	guint has_ukwm : 1;
};

typedef struct _UkuiThemeCursorInfo UkuiThemeCursorInfo;
struct _UkuiThemeCursorInfo {
	UkuiThemeType type;
	gchar* path;
	gchar* name;
	gchar* readable_name;
	gint priority;
	gboolean hidden;

	GArray* sizes;
	GdkPixbuf* thumbnail;
};

typedef struct _UkuiThemeMetaInfo UkuiThemeMetaInfo;
struct _UkuiThemeMetaInfo {
	UkuiThemeType type;
	gchar* path;
	gchar* name;
	gchar* readable_name;
	gint priority;
	gboolean hidden;

	gchar* comment;
	gchar* icon_file;

	gchar* gtk_theme_name;
	gchar* gtk_color_scheme;
	gchar* ukwm_theme_name;
	gchar* icon_theme_name;
	gchar* notification_theme_name;
	gchar* sound_theme_name;
	gchar* cursor_theme_name;
	guint cursor_size;

	gchar* application_font;
	gchar* documents_font;
	gchar* desktop_font;
	gchar* windowtitle_font;
	gchar* monospace_font;
	gchar* background_image;
};

enum {
	COLOR_FG,
	COLOR_BG,
	COLOR_TEXT,
	COLOR_BASE,
	COLOR_SELECTED_FG,
	COLOR_SELECTED_BG,
	COLOR_TOOLTIP_FG,
	COLOR_TOOLTIP_BG,
	NUM_SYMBOLIC_COLORS
};

typedef void (*ThemeChangedCallback) (UkuiThemeCommonInfo* theme, UkuiThemeChangeType change_type, UkuiThemeElement element_type, gpointer user_data);

#define UKUI_THEME_ERROR ukui_theme_info_error_quark()

enum {
	UKUI_THEME_ERROR_GTK_THEME_NOT_AVAILABLE = 1,
	UKUI_THEME_ERROR_WM_THEME_NOT_AVAILABLE,
	UKUI_THEME_ERROR_ICON_THEME_NOT_AVAILABLE,
	UKUI_THEME_ERROR_GTK_ENGINE_NOT_AVAILABLE,
	UKUI_THEME_ERROR_UNKNOWN
};


/* GTK/Ukwm/keybinding Themes */
UkuiThemeInfo     *ukui_theme_info_new                   (void);
void                ukui_theme_info_free                  (UkuiThemeInfo     *theme_info);
UkuiThemeInfo     *ukui_theme_info_find                  (const gchar        *theme_name);
GList              *ukui_theme_info_find_by_type          (guint               elements);
GQuark              ukui_theme_info_error_quark           (void);
gchar              *gtk_theme_info_missing_engine          (const gchar *gtk_theme,
                                                            gboolean nameOnly);

/* Icon Themes */
UkuiThemeIconInfo *ukui_theme_icon_info_new              (void);
void                ukui_theme_icon_info_free             (UkuiThemeIconInfo *icon_theme_info);
UkuiThemeIconInfo *ukui_theme_icon_info_find             (const gchar        *icon_theme_name);
GList              *ukui_theme_icon_info_find_all         (void);
gint                ukui_theme_icon_info_compare          (UkuiThemeIconInfo *a,
							    UkuiThemeIconInfo *b);

/* Cursor Themes */
UkuiThemeCursorInfo *ukui_theme_cursor_info_new	   (void);
void                  ukui_theme_cursor_info_free	   (UkuiThemeCursorInfo *info);
UkuiThemeCursorInfo *ukui_theme_cursor_info_find	   (const gchar          *name);
GList                *ukui_theme_cursor_info_find_all	   (void);
gint                  ukui_theme_cursor_info_compare      (UkuiThemeCursorInfo *a,
							    UkuiThemeCursorInfo *b);

/* Meta themes*/
UkuiThemeMetaInfo *ukui_theme_meta_info_new              (void);
void                ukui_theme_meta_info_free             (UkuiThemeMetaInfo *meta_theme_info);
UkuiThemeMetaInfo *ukui_theme_meta_info_find             (const gchar        *meta_theme_name);
GList              *ukui_theme_meta_info_find_all         (void);
gint                ukui_theme_meta_info_compare          (UkuiThemeMetaInfo *a,
							    UkuiThemeMetaInfo *b);
gboolean            ukui_theme_meta_info_validate         (const UkuiThemeMetaInfo *info,
                                                            GError            **error);
UkuiThemeMetaInfo *ukui_theme_read_meta_theme            (GFile              *meta_theme_uri);

/* Other */
void                ukui_theme_init                       (void);
void                ukui_theme_info_register_theme_change (ThemeChangedCallback func,
							    gpointer             data);

#if GTK_CHECK_VERSION (3, 0, 0)
gboolean            ukui_theme_color_scheme_parse         (const gchar         *scheme,
							    GdkRGBA             *colors);
#else
gboolean            ukui_theme_color_scheme_parse         (const gchar         *scheme,
							    GdkColor            *colors);
#endif
gboolean            ukui_theme_color_scheme_equal         (const gchar         *s1,
							    const gchar         *s2);

#endif /* UKUI_THEME_INFO_H */
