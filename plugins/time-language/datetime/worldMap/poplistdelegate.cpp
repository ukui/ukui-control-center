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

#include "poplistdelegate.h"

PopListDelegate::PopListDelegate(QWidget* parent) : QStyledItemDelegate (parent)
{

}

// 委托类，主要绘制鼠标划过时候的状态
void PopListDelegate::paint(QPainter *painter, const QStyleOptionViewItem& option,const QModelIndex& index) const{
    painter->save();

    QRect rect(option.rect);

    if(option.state.testFlag(QStyle::State_MouseOver)) {
        QColor hoverColor(44, 167, 248);
        painter->fillRect(rect, QBrush(hoverColor));
    }

    QColor textColor(48, 48, 48);
    if(option.state.testFlag(QStyle::State_MouseOver)) {
        textColor = Qt::white;
    }

    painter->setPen(QPen(textColor));
    QString text = index.model()->data(index, Qt::DisplayRole).toString();

      painter->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, text);
    painter->restore();
}
