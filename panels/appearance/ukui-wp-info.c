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
#include "ukui-wp-info.h"
#include <libintl.h>

UkuiWPInfo* ukui_wp_info_new(const char* uri, MateDesktopThumbnailFactory* thumbs)
{
	UkuiWPInfo* wp;

	GFile* file = g_file_new_for_commandline_arg(uri);

	GFileInfo* info = g_file_query_info(file,
					    G_FILE_ATTRIBUTE_STANDARD_NAME ","
					    G_FILE_ATTRIBUTE_STANDARD_SIZE ","
					    G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE ","
					    G_FILE_ATTRIBUTE_TIME_MODIFIED,
					    G_FILE_QUERY_INFO_NONE,
					    NULL, NULL);

	g_object_unref(file);

	if (info == NULL || g_file_info_get_content_type (info) == NULL)
	{
		if (!strcmp (uri, "(none)"))
		{
			wp = g_new0(UkuiWPInfo, 1);

			wp->mime_type = g_strdup("image/x-no-data");
			wp->uri = g_strdup(uri);
            wp->name = g_strdup(gettext("No Desktop Background"));
			wp->size = 0;
		}
		else
		{
			wp = NULL;
		}
	}
	else
	{
		wp = g_new0 (UkuiWPInfo, 1);

		wp->uri = g_strdup(uri);

		wp->name = g_strdup(g_file_info_get_name(info));

		if (g_file_info_get_content_type(info) != NULL)
		{
			wp->mime_type = g_strdup(g_file_info_get_content_type(info));
		}

		wp->size = g_file_info_get_size(info);
		wp->mtime = g_file_info_get_attribute_uint64(info, G_FILE_ATTRIBUTE_TIME_MODIFIED);

		wp->thumburi = mate_desktop_thumbnail_factory_lookup(thumbs, uri, wp->mtime);
	}

	if (info != NULL)
	{
		g_object_unref(info);
	}

	return wp;
}

void ukui_wp_info_free(UkuiWPInfo* info)
{
	if (info == NULL)
	{
		return;
	}

	g_free(info->uri);
	g_free(info->thumburi);
	g_free(info->name);
	g_free(info->mime_type);
}
