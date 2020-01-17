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
#include "comboboxitem.h"

ComboboxItem::ComboboxItem(QWidget *parent) :
    QWidget(parent)
{
    mpress = false;

    imgLabel = new QLabel(this);
    textLabel = new QLabel(this);

    mainlayout = new QHBoxLayout(this);
//    mainlayout->addStretch();
    mainlayout->addWidget(textLabel);
    mainlayout->addStretch();
    mainlayout->addWidget(imgLabel);
    mainlayout->addStretch();

    mainlayout->setSpacing(5);
    mainlayout->setContentsMargins(5, 5, 5, 5);

    setLayout(mainlayout);
}

ComboboxItem::~ComboboxItem()
{
}

void ComboboxItem::setLabelContent(QString content){
    textLabel->setText(content);
}

QString ComboboxItem::getLabelContent(){
    return textLabel->text();
}

void ComboboxItem::mousePressEvent(QMouseEvent * event){
    if (event->button() == Qt::LeftButton){
        mpress = true;
//        emit chooseItem(textLabel->text());
    }
//    QWidget::mousePressEvent(event);
}

void ComboboxItem::mouseReleaseEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        ;//un used warning
    }
    if (mpress){
        emit chooseItem(textLabel->text());
        mpress = false;
    }
}
