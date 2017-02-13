/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* ukui-window-manager.h
 * Copyright (C) 2002 Seth Nickell
 * Copyright (C) 2002 Red Hat, Inc.
 * Copyright (C) 2016,Tianjin KYLIN Information Technology Co., Ltd.
 * Written by: Seth Nickell <snickell@stanford.edu>,
 *             Havoc Pennington <hp@redhat.com>
 * Modified by: zhangshuhao <zhangshuhao@kylinos.cn>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef UKUI_WINDOW_MANAGER_H
#define UKUI_WINDOW_MANAGER_H

#include <glib-object.h>

#include <libukui-desktop/ukui-desktop-item.h>

/* Increment if backward-incompatible changes are made, so we get a clean
 * error. In principle the libtool versioning handles this, but
 * in combination with dlopen I don't quite trust that.
 */
#define UKUI_WINDOW_MANAGER_INTERFACE_VERSION 1

typedef GObject * (* UkuiWindowManagerNewFunc) (int expected_interface_version);

typedef enum
{
        UKUI_WM_SETTING_FONT                = 1 << 0,
        UKUI_WM_SETTING_MOUSE_FOCUS         = 1 << 1,
        UKUI_WM_SETTING_AUTORAISE           = 1 << 2,
        UKUI_WM_SETTING_AUTORAISE_DELAY     = 1 << 3,
        UKUI_WM_SETTING_MOUSE_MOVE_MODIFIER = 1 << 4,
        UKUI_WM_SETTING_THEME               = 1 << 5,
        UKUI_WM_SETTING_DOUBLE_CLICK_ACTION = 1 << 6,
        UKUI_WM_SETTING_COMPOSITING_MANAGER = 1 << 7,
        UKUI_WM_SETTING_COMPOSITING_ALTTAB  = 1 << 8,
        UKUI_WM_SETTING_MASK                =
        UKUI_WM_SETTING_FONT                |
        UKUI_WM_SETTING_MOUSE_FOCUS         |
        UKUI_WM_SETTING_AUTORAISE           |
        UKUI_WM_SETTING_AUTORAISE_DELAY     |
        UKUI_WM_SETTING_MOUSE_MOVE_MODIFIER |
        UKUI_WM_SETTING_THEME               |
        UKUI_WM_SETTING_DOUBLE_CLICK_ACTION |
        UKUI_WM_SETTING_COMPOSITING_MANAGER |
        UKUI_WM_SETTING_COMPOSITING_ALTTAB
} UkuiWMSettingsFlags;

typedef struct
{
        int number;
        const char *human_readable_name;
} UkuiWMDoubleClickAction;

typedef struct
{
        UkuiWMSettingsFlags flags; /* this allows us to expand the struct
                                     * while remaining binary compatible
                                     */
        const char *font;
        int autoraise_delay;
        /* One of the strings "Alt", "Control", "Super", "Hyper", "Meta" */
        const char *mouse_move_modifier;
        const char *theme;
        int double_click_action;

        guint focus_follows_mouse : 1;
        guint autoraise : 1;

        gboolean compositing_manager;
        gboolean compositing_fast_alt_tab;

} UkuiWMSettings;

#ifdef __cplusplus
extern "C" {
#endif

#define UKUI_WINDOW_MANAGER(obj)          G_TYPE_CHECK_INSTANCE_CAST (obj, ukui_window_manager_get_type (), UkuiWindowManager)
#define UKUI_WINDOW_MANAGER_CLASS(klass)  G_TYPE_CHECK_CLASS_CAST (klass, ukui_window_manager_get_type (), UkuiWindowManagerClass)
#define IS_UKUI_WINDOW_MANAGER(obj)       G_TYPE_CHECK_INSTANCE_TYPE (obj, ukui_window_manager_get_type ())
#define UKUI_WINDOW_MANAGER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), ukui_window_manager_get_type, UkuiWindowManagerClass))

typedef struct _UkuiWindowManager UkuiWindowManager;
typedef struct _UkuiWindowManagerClass UkuiWindowManagerClass;

typedef struct _UkuiWindowManagerPrivate UkuiWindowManagerPrivate;

struct _UkuiWindowManager
{
        GObject parent;

        UkuiWindowManagerPrivate *p;
};

struct _UkuiWindowManagerClass
{
        GObjectClass klass;

        void         (* settings_changed)       (UkuiWindowManager    *wm);

        void         (* change_settings)        (UkuiWindowManager    *wm,
                                                 const UkuiWMSettings *settings);
        void         (* get_settings)           (UkuiWindowManager    *wm,
                                                 UkuiWMSettings       *settings);

        GList *      (* get_theme_list)         (UkuiWindowManager *wm);
        char *       (* get_user_theme_folder)  (UkuiWindowManager *wm);

        int          (* get_settings_mask)      (UkuiWindowManager *wm);

        void         (* get_double_click_actions) (UkuiWindowManager              *wm,
                                                   const UkuiWMDoubleClickAction **actions,
                                                   int                             *n_actions);

        void         (* padding_func_1)         (UkuiWindowManager *wm);
        void         (* padding_func_2)         (UkuiWindowManager *wm);
        void         (* padding_func_3)         (UkuiWindowManager *wm);
        void         (* padding_func_4)         (UkuiWindowManager *wm);
        void         (* padding_func_5)         (UkuiWindowManager *wm);
        void         (* padding_func_6)         (UkuiWindowManager *wm);
        void         (* padding_func_7)         (UkuiWindowManager *wm);
        void         (* padding_func_8)         (UkuiWindowManager *wm);
        void         (* padding_func_9)         (UkuiWindowManager *wm);
        void         (* padding_func_10)        (UkuiWindowManager *wm);
};

GObject *         ukui_window_manager_new                     (UkuiDesktopItem   *item);
GType             ukui_window_manager_get_type                (void);
const char *      ukui_window_manager_get_name                (UkuiWindowManager *wm);
UkuiDesktopItem *ukui_window_manager_get_ditem               (UkuiWindowManager *wm);

/* GList of char *'s */
GList *           ukui_window_manager_get_theme_list          (UkuiWindowManager *wm);
char *            ukui_window_manager_get_user_theme_folder   (UkuiWindowManager *wm);

/* only uses fields with their flags set */
void              ukui_window_manager_change_settings  (UkuiWindowManager    *wm,
                                                         const UkuiWMSettings *settings);
/* only gets fields with their flags set (and if it fails to get a field,
 * it unsets that flag, so flags should be checked on return)
 */
void              ukui_window_manager_get_settings     (UkuiWindowManager *wm,
                                                         UkuiWMSettings    *settings);

void              ukui_window_manager_settings_changed (UkuiWindowManager *wm);

void ukui_window_manager_get_double_click_actions (UkuiWindowManager              *wm,
                                                    const UkuiWMDoubleClickAction **actions,
                                                    int                             *n_actions);

/* Helper functions for UkuiWMSettings */
UkuiWMSettings *ukui_wm_settings_copy (UkuiWMSettings *settings);
void             ukui_wm_settings_free (UkuiWMSettings *settings);

#ifdef __cplusplus
}
#endif

#endif /* UKUI_WINDOW_MANAGER_H */
