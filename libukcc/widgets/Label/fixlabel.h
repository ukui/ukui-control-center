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

#ifndef FIXLABEL_H
#define FIXLABEL_H
#include <QLabel>

#include "libukcc_global.h"

//文本长自动省略并添加悬浮
class LIBUKCC_EXPORT FixLabel : public QLabel
{

    Q_OBJECT
public:
    explicit FixLabel(QWidget *parent = nullptr);
    explicit FixLabel(QString text , QWidget *parent = nullptr);
    ~FixLabel();
    void setText(const QString &text, bool saveTextFlag = true);
private:
    void paintEvent(QPaintEvent *event);

private:
    QString mStr;
};


#endif // FIXLABEL_H
