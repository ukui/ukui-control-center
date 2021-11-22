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

#include "toolpop.h"

#include <QPaintEvent>
#include <QPainterPath>
#include <QPainter>

ToolPop::ToolPop(QWidget* parent) : QLabel (parent)
{
    this->setAlignment(Qt::AlignCenter);
    this->setStyleSheet("margin: 0 15");
    this->setStyleSheet("background-color: #3D6BE5;border-radius:4px;");

}

void ToolPop::popupSlot(QPoint point) {
    this->move(point.x() - this->width() / 2 -4,
               point.y() - this->height()-8);
    this->show();
}

void ToolPop::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing,
                           true);

    QFont font;
    font.setPixelSize(12);
    QFontMetrics fontMetrics(font);
    int labelLen = fontMetrics.width(this->text());
    painter.setPen(QPen(Qt::white));
    painter.setFont(font);
    painter.drawText((this->width() - labelLen) / 2, 16, this->text());
}


