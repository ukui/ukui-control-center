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

#ifndef _UKUI_WP_INFO_H_
#define _UKUI_WP_INFO_H_

#include <glib.h>
#include <libmate-desktop/mate-desktop-thumbnail.h>

typedef struct _UkuiWPInfo {
	char* uri;
	char* thumburi;
	char* name;
	char* mime_type;

	goffset size;

	time_t mtime;
} UkuiWPInfo;

UkuiWPInfo* ukui_wp_info_new(const char* uri, MateDesktopThumbnailFactory* thumbs);
void ukui_wp_info_free(UkuiWPInfo* info);

#endif

