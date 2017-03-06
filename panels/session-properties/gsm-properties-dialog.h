/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*-
 *
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

#ifndef __GSM_PROPERTIES_DIALOG_H
#define __GSM_PROPERTIES_DIALOG_H

#include <glib-object.h>
#include <gtk/gtk.h>

typedef struct GsmPropertiesDialogPrivate GsmPropertiesDialogPrivate;

typedef struct
{
        GtkViewport                   parent;
        GsmPropertiesDialogPrivate *priv;
} GsmPropertiesDialog;


void gsm_properties_dialog_new                (GtkBuilder * builder);
void gsm_properties_dialog_destroy();

#define GSM_PROPERTIES_ICON_SIZE GTK_ICON_SIZE_LARGE_TOOLBAR


#endif /* __GSM_PROPERTIES_DIALOG_H */
