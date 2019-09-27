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
#include "listwidgetitem.h"

ListWidgetItem::ListWidgetItem(QWidget *parent) :
    QWidget(parent)
{
//    this->setFixedSize(QSize(198,50));
    this->setStyleSheet("background: none;");

    iconLabel = new QLabel(this);
    QSizePolicy policy = iconLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Fixed);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    iconLabel->setSizePolicy(policy);

    textLabel = new QLabel(this);
    QSizePolicy policy1 = textLabel->sizePolicy();
    policy1.setHorizontalPolicy(QSizePolicy::Fixed);
    policy1.setVerticalPolicy(QSizePolicy::Fixed);
    textLabel->setSizePolicy(policy1);
    textLabel->setScaledContents(true);

    QHBoxLayout * mainlayout = new QHBoxLayout(this);
    mainlayout->addWidget(iconLabel);
    mainlayout->addWidget(textLabel);
    mainlayout->addStretch();
    mainlayout->setSpacing(10);
    mainlayout->setContentsMargins(20, 0, 0, 0);

    setLayout(mainlayout);
}

ListWidgetItem::~ListWidgetItem()
{
}

void ListWidgetItem::setLabelPixmap(QString filename){
    iconLabel->setPixmap(QPixmap(filename));
}

void ListWidgetItem::setLabelText(QString text){
    textLabel->setText(text);
}

QString ListWidgetItem::text(){
    return textLabel->text();
}
