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
#include "widgetgroup.h"

#include "themewidget.h"

#include <QDebug>

WidgetGroup::WidgetGroup(QObject *parent) :
    QObject(parent)
{
    _preWidget = nullptr;
    _curWidget = nullptr;
}

WidgetGroup::~WidgetGroup()
{
}

void WidgetGroup::addWidget(ThemeWidget *widget, int id){
    Q_UNUSED(id)
    connect(widget->radioBtn,&QRadioButton::clicked,[=]{
        _preWidget = _curWidget;
        _curWidget = widget;
        emit widgetChanged(_preWidget, _curWidget);
    });
    connect(widget,&ThemeWidget::clicked,[=]{
        _preWidget = _curWidget;
        _curWidget = widget;
        emit widgetChanged(_preWidget, _curWidget);
    });

}

void WidgetGroup::removeWidget(ThemeWidget *widget){
    disconnect(widget, 0, 0, 0);
}

void WidgetGroup::setCurrentWidget(ThemeWidget *widget){
    _curWidget = widget;
}
