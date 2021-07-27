/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef DATADEFINED_H
#define DATADEFINED_H

#include <QPixmap>
#include <QMap>

#define SYSTEMPOS 2
#define ALLPOS 1
#define LOCALPOS 0

#define APP_KEY_FILE_DESKTOP_KEY_AUTOSTART_ENABLE "X-UKUI-Autostart-enabled"

typedef struct _AutoApp{

    QString bname;
    QString path;

//    bool enable;
    bool no_display;
    bool shown;
    bool hidden;

    QString name;
    QString comment;
    QPixmap pixmap;
    QString exec;
    QString description;

    int xdg_position;
}AutoApp;

#endif // DATADEFINED_H
