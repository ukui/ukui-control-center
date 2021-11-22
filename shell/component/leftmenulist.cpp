/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "leftmenulist.h"

#include <QDebug>

LeftMenuList::LeftMenuList(QWidget *parent) : QListWidget(parent) {

}

void LeftMenuList::resizeEvent(QResizeEvent *event) {
    int maxItemWidth = 0;
    for (int i = 0; i < this->count(); i++) {
        QWidget *item = this->itemWidget(this->item(i));
        if (item->width() > maxItemWidth) {
            maxItemWidth = item->width();
        }
        item->setFixedWidth(this->width() - 24);
    };
    if (maxItemWidth < 100) {
        this->setMinimumWidth(maxItemWidth);
    }

    QListWidget::resizeEvent(event);
}

void LeftMenuList::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QListWidget::mousePressEvent(event);
    }
}
