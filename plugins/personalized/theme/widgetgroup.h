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
#ifndef WIDGETGROUP_H
#define WIDGETGROUP_H

#include <QObject>

class ThemeWidget;

class WidgetGroup : public QObject
{
    Q_OBJECT

public:
    explicit WidgetGroup(QObject *parent = nullptr);
    ~WidgetGroup();

public:
    void addWidget(ThemeWidget * widget, int id = -1);
    void removeWidget(ThemeWidget * widget);

    void setCurrentWidget(ThemeWidget * widget);


private:
    ThemeWidget * _preWidget;
    ThemeWidget * _curWidget;

    QList<ThemeWidget *> widgets;


Q_SIGNALS:
    void widgetChanged(ThemeWidget * preWidget, ThemeWidget * curWidget);

};

#endif // WIDGETGROUP_H
