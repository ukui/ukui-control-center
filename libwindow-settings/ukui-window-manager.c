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

#include "ukui-window-manager.h"

#include <gmodule.h>

static GObjectClass *parent_class;

struct _UkuiWindowManagerPrivate {
        char *window_manager_name;
        MateDesktopItem *ditem;
};

GObject *
ukui_window_manager_new (MateDesktopItem *it)
{
        const char *settings_lib;
        char *module_name;
        UkuiWindowManagerNewFunc wm_new_func = NULL;
        GObject *wm;
        GModule *module;
        gboolean success;

        settings_lib = ukui_desktop_item_get_string (it, "X-UKUI-WMSettingsModule");

        module_name = g_module_build_path (UKUI_WINDOW_MANAGER_MODULE_PATH,
                                           settings_lib);

        module = g_module_open (module_name, G_MODULE_BIND_LAZY);
        if (module == NULL) {
                g_warning ("Couldn't load window manager settings module `%s' (%s)", module_name, g_module_error ());
		g_free (module_name);
                return NULL;
        }

        success = g_module_symbol (module, "window_manager_new",
                                   (gpointer *) &wm_new_func);  
  
        if ((!success) || wm_new_func == NULL) {
                g_warning ("Couldn't load window manager settings module `%s`, couldn't find symbol \'window_manager_new\'", module_name);
		g_free (module_name);
                return NULL;
        }

	g_free (module_name);

        wm = (* wm_new_func) (UKUI_WINDOW_MANAGER_INTERFACE_VERSION);

        if (wm == NULL)
                return NULL;
        
        (UKUI_WINDOW_MANAGER (wm))->p->window_manager_name = g_strdup (ukui_desktop_item_get_string (it, MATE_DESKTOP_ITEM_NAME));
        (UKUI_WINDOW_MANAGER (wm))->p->ditem = ukui_desktop_item_ref (it);
  
        return wm;
}

const char * 
ukui_window_manager_get_name (UkuiWindowManager *wm)
{
        return wm->p->window_manager_name;
}

MateDesktopItem *
ukui_window_manager_get_ditem (UkuiWindowManager *wm)
{
        return ukui_desktop_item_ref (wm->p->ditem);
}

GList *
ukui_window_manager_get_theme_list (UkuiWindowManager *wm)
{
        UkuiWindowManagerClass *klass = UKUI_WINDOW_MANAGER_GET_CLASS (wm);
        if (klass->get_theme_list)
                return klass->get_theme_list (wm);
        else
                return NULL;
}

char *
ukui_window_manager_get_user_theme_folder (UkuiWindowManager *wm)
{
        UkuiWindowManagerClass *klass = UKUI_WINDOW_MANAGER_GET_CLASS (wm);
        if (klass->get_user_theme_folder)
                return klass->get_user_theme_folder (wm);
        else
                return NULL;
}

void
ukui_window_manager_get_double_click_actions (UkuiWindowManager              *wm,
                                               const UkuiWMDoubleClickAction **actions,
                                               int                             *n_actions)
{
        UkuiWindowManagerClass *klass = UKUI_WINDOW_MANAGER_GET_CLASS (wm);

        *actions = NULL;
        *n_actions = 0;
        
        if (klass->get_double_click_actions)
                klass->get_double_click_actions (wm, actions, n_actions);
}

void
ukui_window_manager_change_settings  (UkuiWindowManager    *wm,
                                       const UkuiWMSettings *settings)
{
        UkuiWindowManagerClass *klass = UKUI_WINDOW_MANAGER_GET_CLASS (wm);
        
        (* klass->change_settings) (wm, settings);
}

void
ukui_window_manager_get_settings (UkuiWindowManager *wm,
                                   UkuiWMSettings    *settings)
{
        UkuiWindowManagerClass *klass = UKUI_WINDOW_MANAGER_GET_CLASS (wm);
        int mask;

        mask = (* klass->get_settings_mask) (wm);
        settings->flags &= mask; /* avoid back compat issues by not returning
                                  * fields to the caller that the WM module
                                  * doesn't know about
                                  */
        
        (* klass->get_settings) (wm, settings);
}

static void
ukui_window_manager_init (UkuiWindowManager *ukui_window_manager, UkuiWindowManagerClass *class)
{
	ukui_window_manager->p = g_new0 (UkuiWindowManagerPrivate, 1);
}

static void
ukui_window_manager_finalize (GObject *object) 
{
	UkuiWindowManager *ukui_window_manager;

	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_UKUI_WINDOW_MANAGER (object));

	ukui_window_manager = UKUI_WINDOW_MANAGER (object);

	g_free (ukui_window_manager->p);

	parent_class->finalize (object);
}

enum {
  SETTINGS_CHANGED,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static void
ukui_window_manager_class_init (UkuiWindowManagerClass *class) 
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (class);

	object_class->finalize = ukui_window_manager_finalize;
        
	parent_class = g_type_class_peek_parent (class);

        
        signals[SETTINGS_CHANGED] =
                g_signal_new ("settings_changed",
                              G_OBJECT_CLASS_TYPE (class),
                              G_SIGNAL_RUN_FIRST | G_SIGNAL_NO_RECURSE,
                              G_STRUCT_OFFSET (UkuiWindowManagerClass, settings_changed),
                              NULL, NULL,
                              g_cclosure_marshal_VOID__VOID,
                              G_TYPE_NONE, 0);
}


GType
ukui_window_manager_get_type (void)
{
	static GType ukui_window_manager_type = 0;

	if (!ukui_window_manager_type) {
		static GTypeInfo ukui_window_manager_info = {
			sizeof (UkuiWindowManagerClass),
			NULL, /* GBaseInitFunc */
			NULL, /* GBaseFinalizeFunc */
			(GClassInitFunc) ukui_window_manager_class_init,
			NULL, /* GClassFinalizeFunc */
			NULL, /* user-supplied data */
			sizeof (UkuiWindowManager),
			0, /* n_preallocs */
			(GInstanceInitFunc) ukui_window_manager_init,
			NULL
		};

		ukui_window_manager_type = 
			g_type_register_static (G_TYPE_OBJECT, 
						"UkuiWindowManager",
						&ukui_window_manager_info, 0);                
	}

	return ukui_window_manager_type;
}


void
ukui_window_manager_settings_changed (UkuiWindowManager *wm)
{
        g_signal_emit (wm, signals[SETTINGS_CHANGED], 0);
}

/* Helper functions for UkuiWMSettings */
UkuiWMSettings *
ukui_wm_settings_copy (UkuiWMSettings *settings)
{
        UkuiWMSettings *retval;

        g_return_val_if_fail (settings != NULL, NULL);

        retval = g_new (UkuiWMSettings, 1);
        *retval = *settings;

        if (retval->flags & UKUI_WM_SETTING_FONT)
                retval->font = g_strdup (retval->font);
        if (retval->flags & UKUI_WM_SETTING_MOUSE_MOVE_MODIFIER)
                retval->mouse_move_modifier = g_strdup (retval->mouse_move_modifier);
        if (retval->flags & UKUI_WM_SETTING_THEME)
                retval->theme = g_strdup (retval->theme);

        return retval;
}

void
ukui_wm_settings_free (UkuiWMSettings *settings)
{
        g_return_if_fail (settings != NULL);

        if (settings->flags & UKUI_WM_SETTING_FONT)
                g_free ((void *) settings->font);
        if (settings->flags & UKUI_WM_SETTING_MOUSE_MOVE_MODIFIER)
                g_free ((void *) settings->mouse_move_modifier);
        if (settings->flags & UKUI_WM_SETTING_THEME)
                g_free ((void *)settings->theme);

        g_free (settings);
}

