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
#ifndef CUSTDOMITEMMODEL_H
#define CUSTDOMITEMMODEL_H

#include <QAbstractTableModel>
#include <QStandardItem>
//#include <QMap>

class CustdomItemModel : public QAbstractTableModel
{
public:
    CustdomItemModel();
    ~CustdomItemModel();

//    virtual QModelIndex index(int row, int column = 0, const QModelIndex & parent = QModelIndex()) const;
//    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) const;
//    void setCurrentMap(const QMap<QString, QMap<QString, QString> > &map);
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

//    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());

private:
//    QString current_key(int offset) const;
    QStringList rowStringList;
//    QMap<QString, QMap<QString, QString> > currentMap;
    QList<QStandardItem *> itemList;

};

#endif // CUSTDOMITEMMODEL_H
