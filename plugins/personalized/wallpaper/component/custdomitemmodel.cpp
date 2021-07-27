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
#include "custdomitemmodel.h"

#include <QDebug>


CustdomItemModel::CustdomItemModel()
{
}

CustdomItemModel::~CustdomItemModel()
{
}

QModelIndex CustdomItemModel::index(int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(parent);
    if (row >= 0 && row < rowCount() && column >= 0 && column < columnCount()){
        QStandardItem * item = itemList.at(row);
        return createIndex(row, column, (void *)(item));
    }
    return QModelIndex();
}

int CustdomItemModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

int CustdomItemModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return itemList.count();
}

QVariant CustdomItemModel::data(const QModelIndex &index, int role) const {
    qDebug() << role << "**********role***";
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole){
        QStandardItem * item = itemList.at(index.row());
        return QVariant(item->text());
    }
    else if (role == Qt::DecorationRole){
        QStandardItem * item = itemList.at(index.row());
        return QVariant(item->icon());
    }
    else if (role == Qt::ToolTipRole){
        QStandardItem * item = itemList.at(index.row());
        return item->toolTip();
    }
    else
        return QVariant();
}

bool CustdomItemModel::setData(const QModelIndex &index, const QVariant &value, int role){
    if (index.isValid() && role == Qt::EditRole){
        QStandardItem * item = itemList.at(index.row());
        item->setText(value.toString());
        emit dataChanged(index, index);
        return true;
    }
    else if (role == Qt::DecorationRole){
        QStandardItem * item = itemList.at(index.row());
        item->setIcon(value.value<QIcon>());
        return true;
    }
    else if (role == Qt::ToolTipRole){
        QStandardItem * item = itemList.at(index.row());
        item->setToolTip(value.toString());
        return true;
    }
    else
        return false;
}

bool CustdomItemModel::insertRows(int row, int count, const QModelIndex &parent){
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; i++){
        QStandardItem * item = new QStandardItem();
        item->setSizeHint(QSize(160, 160));
        itemList.insert(row, item);
    }
    endInsertRows();
    return true;
}
