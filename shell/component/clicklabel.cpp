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
#include "clicklabel.h"

ClickLabel::ClickLabel(const QString &text, QWidget *parent)
{
    setText(abridge(text));
    adjustSize();
}

ClickLabel::~ClickLabel()
{
}

QString ClickLabel::abridge(QString text)
{
    /* 设计要求，部分首页显示插件名和导航显示名不一致*/
    if (text == "时间和日期") {
        text = "时间日期";
    } else if (text == "区域语言") {
        text = "语言";
    }

    return text;
}

void ClickLabel::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton)
        emit clicked();
//    QLabel::mousePressEvent(event);
}
