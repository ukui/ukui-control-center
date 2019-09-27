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
﻿#include "keybinding_view.h"
#include <QFile>

KeybindingView::KeybindingView(QTreeView *view):view(view){

    QStringList headers;
    headers << ("Title") << ("Description");

    QFile file("/home/kylin/default.txt");
    file.open(QIODevice::ReadOnly);
    model = new TreeModel();
    view->setModel(model);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setFocusPolicy(Qt::NoFocus);


    updatemodeltree();
    for (int column = 0; column < model->columnCount(); ++column)
        view->resizeColumnToContents(column);

}

void KeybindingView::updatemodeltree()
{
    QStringList desktop_shortcuts,system_shortcuts;
    desktop_shortcuts<<"desktop shortcuts"<<"";
    system_shortcuts<<"system shortcuts"<<"";
    insertRow(desktop_shortcuts);
    insertRow(system_shortcuts);


}
void KeybindingView::insertChild()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();

    if (model->columnCount(index) == 0) {
        if (!model->insertColumn(0, index))
            return;
    }

    if (!model->insertRow(0, index))
        return;

    for (int column = 0; column < model->columnCount(index); ++column) {
        QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant("[No data]"), Qt::EditRole);
        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
    }

    view->selectionModel()->setCurrentIndex(model->index(0, 0, index),
                                            QItemSelectionModel::ClearAndSelect);

}

void KeybindingView::insertRow(QStringList list)
{
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();

    if (!model->insertRow(index.row()+1, index.parent()))
        return;

    for (int column = 0; column < model->columnCount(index.parent()); ++column) {
        QModelIndex child = model->index(index.row()+1, column, index.parent());
        model->setData(child, QVariant(list[column]), Qt::EditRole);
    }
}

void KeybindingView::removeRow()
{
    QModelIndex index = view->selectionModel()->currentIndex();
    QAbstractItemModel *model = view->model();
    if (model->removeRow(index.row(), index.parent()));
}

