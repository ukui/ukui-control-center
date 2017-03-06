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
#ifndef __USER_ACCOUNTS_H
#define __USER_ACCOUNTS_H

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gio/gio.h>
#include <gdk/gdk.h>

#include "mainwindow.h"

#define FACEHEIGHT 72
#define FACEWIDTH  72

#define STANDARDUSER 0
#define ADMINISTRATOR 1

typedef struct
{
    GtkNotebook	*notebook;
	GtkWidget 	*labelname0;
	GtkWidget 	*labelname1;
	GtkWidget	*labeltype0;
	GtkWidget	*labeltype1;
	GtkWidget	*image0;
	GtkWidget	*image1;
    gchar 		*username;
	gchar		*iconfile;
	gchar 		*password;
	gint		accounttype;
	gint 		passwdtype;
	gboolean 	currentuser;
	gboolean	logined;
	gboolean	autologin;
	gint 		uid;
	GDBusProxy 	*proxy;
} UserInfo;


void init_user_accounts();


#endif  /* __USER_ACCOUNTS_H */
