/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2008 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __KPM_BRIGHTNESS_H
#define __KPM_BRIGHTNESS_H

#include <glib-object.h>

G_BEGIN_DECLS

#define KPM_TYPE_BRIGHTNESS		(kpm_brightness_get_type ())
#define KPM_BRIGHTNESS(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), KPM_TYPE_BRIGHTNESS, KpmBrightness))
#define KPM_BRIGHTNESS_CLASS(k)		(G_TYPE_CHECK_CLASS_CAST((k), KPM_TYPE_BRIGHTNESS, KpmBrightnessClass))
#define KPM_IS_BRIGHTNESS(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), KPM_TYPE_BRIGHTNESS))
#define KPM_IS_BRIGHTNESS_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), KPM_TYPE_BRIGHTNESS))
#define KPM_BRIGHTNESS_GET_CLASS(o)	(G_TYPE_INSTANCE_GET_CLASS ((o), KPM_TYPE_BRIGHTNESS, KpmBrightnessClass))

#define KPM_BRIGHTNESS_DIM_INTERVAL	5 /* ms */

typedef struct KpmBrightnessPrivate KpmBrightnessPrivate;

typedef struct
{
	GObject		         parent;
	KpmBrightnessPrivate	*priv;
} KpmBrightness;

typedef struct
{
	GObjectClass	parent_class;
	void		(* brightness_changed)	(KpmBrightness		*brightness,
						 guint			 percentage);
} KpmBrightnessClass;

GType		 kpm_brightness_get_type	(void);
KpmBrightness	*kpm_brightness_new		(void);

gboolean	 kpm_brightness_has_hw		(KpmBrightness		*brightness);
gboolean	 kpm_brightness_up		(KpmBrightness		*brightness,
						 gboolean		*hw_changed);
gboolean	 kpm_brightness_down		(KpmBrightness		*brightness,
						 gboolean		*hw_changed);
gboolean	 kpm_brightness_get		(KpmBrightness		*brightness,
						 guint			*percentage);
gboolean	 kpm_brightness_set		(KpmBrightness		*brightness,
						 guint			 percentage,
						 gboolean		*hw_changed);

G_END_DECLS

#endif /* __KPM_BRIGHTNESS_H */
