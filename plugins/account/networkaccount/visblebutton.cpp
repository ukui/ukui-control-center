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

#include "visblebutton.h"

VisbleButton::VisbleButton(QWidget *parent) : QLabel(parent)
{
    status = false;
    setFixedSize(16,16);
}

void VisbleButton::enterEvent(QEvent *event) {

}

void VisbleButton::leaveEvent(QEvent *event) {

}

void VisbleButton::mousePressEvent(QMouseEvent *event) {
    status = !status;
    emit clicked(status);
    emit toggled(status);
}

void VisbleButton::setChecked(bool checked) {
    status = checked;
    emit clicked(status);
    emit toggled(status);
}
