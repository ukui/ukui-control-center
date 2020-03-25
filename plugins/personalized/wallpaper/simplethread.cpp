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
#include "simplethread.h"

#include <QHBoxLayout>
#include <QLabel>

#include <QDebug>

SimpleThread::SimpleThread(QMap<QString, QMap<QString, QString> > _wpinfo, QObject *parent) : QThread(parent),
    wallpaperinfosMap(_wpinfo)
{
}

SimpleThread::~SimpleThread()
{
}

void SimpleThread::run(){
    QSize IMAGE_SIZE(160, 120);
    QMap<QString, QMap<QString, QString> >::iterator iters = wallpaperinfosMap.begin();
    for (int row = 0; iters != wallpaperinfosMap.end(); iters++, row++){
        //跳过xml的头部信息
        if (QString(iters.key()) == "head")
            continue;

        //跳过被删除的壁纸
        QMap<QString, QString> wpMap = (QMap<QString, QString>)iters.value();
        QString delstatus = QString(wpMap.find("deleted").value());
        if (delstatus == "true")
            continue;

        QString filename = QString(iters.key());
        QPixmap pixmap = QPixmap(filename)/*.scaled(IMAGE_SIZE)*/;

        emit widgetItemCreate(pixmap, filename);
//        QPixmap pixmap(filename);
//        QListWidgetItem * item = new QListWidgetItem(QIcon(pixmap.scaled(IMAGE_SIZE)), "");
//        item->setSizeHint(ITEM_SIZE);
//        item->setData(Qt::UserRole, filename);
    }

}
