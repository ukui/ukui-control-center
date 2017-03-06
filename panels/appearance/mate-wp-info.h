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
#ifndef _MATE_WP_INFO_H_
#define _MATE_WP_INFO_H_
#include <string.h>
#include <gio/gio.h>
#include <glib.h>
#include <gtk/gtk.h>
#define MATE_DESKTOP_USE_UNSTABLE_API
#include <libmate-desktop/mate-desktop-thumbnail.h>

typedef struct _MateWPInfo {
    char* uri;
    char* thumburi;
    char* name;
    char* mime_type;

    goffset size;

    time_t mtime;
} MateWPInfo;

MateWPInfo* mate_wp_info_new(const char* uri, MateDesktopThumbnailFactory* thumbs);
void mate_wp_info_free(MateWPInfo* info);

#endif

