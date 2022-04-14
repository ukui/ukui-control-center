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
#ifndef UKCCABOUT_H
#define UKCCABOUT_H

#include <QWidget>
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>

#include "ukcc/widgets/fixlabel.h"
#include <ukcc/widgets/lightlabel.h>

class UkccAbout : public QDialog
{
    Q_OBJECT
public:
    explicit UkccAbout(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watch, QEvent *event);

private:
    QLabel *mIconLabel = nullptr;
    QLabel *mAppLabel = nullptr;
    LightLabel *mVersionLabel_1 = nullptr;
    LightLabel *mVersionLabel_2 = nullptr;
    LightLabel *mTipLabel_1 = nullptr;
    LightLabel *mTipLabel_2 = nullptr;

private:
    void initUI();
    QString getUkccVersion();

signals:

};

#endif // UKCCABOUT_H
