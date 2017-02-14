/*
 *  Authors: Rodney Dawes <dobey@ximian.com>
 *  Modified by: zhangshuhao <zhangshuhao@kylinos.cn>
 *  Copyright 2003-2006 Novell, Inc. (www.novell.com)
 *  Copyright (C) 2016,Tianjin KYLIN Information Technology Co., Ltd.
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 2 of the GNU General Public License
 *  as published by the Free Software Foundation
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02110-1301, USA.
 *
 */

#include <glib.h>
#include <gio/gio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtk.h>
#include <libmate-desktop/mate-desktop-thumbnail.h>
#include <libmate-desktop/mate-bg.h>

#include "ukui-wp-info.h"

#ifndef _UKUI_WP_ITEM_H_
#define _UKUI_WP_ITEM_H_

typedef struct _UkuiWPItem UkuiWPItem;

struct _UkuiWPItem {
  UkuiBG *bg;

  gchar * name;
  gchar * filename;
  gchar * description;
  UkuiBGPlacement options;
  UkuiBGColorType shade_type;

  /* Where the Item is in the List */
  GtkTreeRowReference * rowref;

  /* Real colors */
#if GTK_CHECK_VERSION (3, 0, 0)
  GdkRGBA * pcolor;
  GdkRGBA * scolor;
#else
  GdkColor * pcolor;
  GdkColor * scolor;
#endif

  UkuiWPInfo * fileinfo;

  /* Did the user remove us? */
  gboolean deleted;

  /* Wallpaper author, if present */
  gchar *artist;

  /* Width and Height of the original image */
  gint width;
  gint height;
};

UkuiWPItem * ukui_wp_item_new (const gchar *filename,
				 GHashTable *wallpapers,
				 UkuiDesktopThumbnailFactory *thumbnails);

void ukui_wp_item_free (UkuiWPItem *item);
GdkPixbuf * ukui_wp_item_get_thumbnail (UkuiWPItem *item,
					 UkuiDesktopThumbnailFactory *thumbs,
                                         gint width,
                                         gint height);
GdkPixbuf * ukui_wp_item_get_frame_thumbnail (UkuiWPItem *item,
                                               UkuiDesktopThumbnailFactory *thumbs,
                                               gint width,
                                               gint height,
                                               gint frame);
void ukui_wp_item_update (UkuiWPItem *item);
void ukui_wp_item_update_description (UkuiWPItem *item);
void ukui_wp_item_ensure_ukui_bg (UkuiWPItem *item);

const gchar *wp_item_option_to_string (UkuiBGPlacement type);
const gchar *wp_item_shading_to_string (UkuiBGColorType type);
UkuiBGPlacement wp_item_string_to_option (const gchar *option);
UkuiBGColorType wp_item_string_to_shading (const gchar *shade_type);

#endif
