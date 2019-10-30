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
#ifndef CUSTOMCOMBOBOX_H
#define CUSTOMCOMBOBOX_H

#include <QWidget>
#include <QComboBox>

#include "support-customcombobox/comboboxitem.h"
#include "support-customcombobox/nofocusframedelegate.h"

class QListWidget;
class QListWidgetItem;

class CustomComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit CustomComboBox(QWidget *parent = 0);
    ~CustomComboBox();

//    void setCurrentItem(QString text);
    void addwidgetItem(QString text);
    void removewidgetItems();
    void setcurrentwidgetIndex(int index);

private:
//    QString currentitemtext;

    QListWidget * partListWidget;

public slots:
//    void currentItemChanged(QListWidgetItem * current, QListWidgetItem * previous);
    void onChooseItem(QString text);

};

#endif // CUSTOMCOMBOBOX_H
