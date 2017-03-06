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
#ifndef __KPMPREFSCORE_H
#define __KPMPREFSCORE_H

#include <glib-object.h>

G_BEGIN_DECLS

#define KPM_TYPE_PREFS          (kpm_prefs_get_type ())
#define KPM_PREFS(o)            (G_TYPE_CHECK_INSTANCE_CAST ((o), KPM_TYPE_PREFS, KpmPrefs))
#define KPM_PREFS_CLASS(k)      (G_TYPE_CHECK_CLASS_CAST((k), KPM_TYPE_PREFS, KpmPrefsClass))
#define KPM_IS_PREFS(o)         (G_TYPE_CHECK_INSTANCE_TYPE ((o), KPM_TYPE_PREFS))
#define KPM_IS_PREFS_CLASS(k)   (G_TYPE_CHECK_CLASS_TYPE ((k), KPM_TYPE_PREFS))
#define KPM_PREFS_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS ((o), KPM_TYPE_PREFS, KpmPrefsClass))

typedef struct KpmPrefsPrivate KpmPrefsPrivate;

typedef struct
{
        GObject          parent;
        KpmPrefsPrivate *priv;
} KpmPrefs;

typedef struct
{
        GObjectClass    parent_class;
        void            (* action_help)                 (KpmPrefs       *prefs);
        void            (* action_close)                (KpmPrefs       *prefs);
} KpmPrefsClass;

GType            kpm_prefs_get_type                     (void);
KpmPrefs        *kpm_prefs_new                          (void);
void             kpm_prefs_activate_window              (KpmPrefs       *prefs);

void init_power();

G_END_DECLS

#endif  /* __KPMPREFS_H */
                                                    
