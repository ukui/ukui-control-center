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

#include "slider.h"

#include <QStyle>
#include <QPainter>
#include <QStyleOptionSlider>
#include <QDebug>

Slider::Slider() : QSlider(Qt::Horizontal)
{
    scaleList << "1.0" << "1.25" << "1.5" << "1.75" << "2.0";
    this->setMinimumHeight(50);
}

void Slider::paintEvent(QPaintEvent *e)
{
    QSlider::paintEvent(e);

    auto painter = new QPainter(this);
    painter->setPen(QPen(Qt::black));

    auto rect = this->geometry();

    int numTicks = (maximum() - minimum())/tickInterval();

    QFontMetrics fontMetrics = QFontMetrics(this->font());

    if (this->orientation() == Qt::Horizontal) {
        int fontHeight = fontMetrics.height();

        for (int i = 0; i <= numTicks; i++) {
            int tickNum = minimum() + (tickInterval() * i);

            int tickX = (((rect.width()/numTicks) * i) - (fontMetrics.width(QString::number(tickNum))/2));
            int tickY = rect.height()/2 + fontHeight + 2;

            painter->drawText(QPoint(tickX + 0.1, tickY),
                              this->scaleList.at(i));
        }
    }

    painter->end();
}
