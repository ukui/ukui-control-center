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

#ifndef LOADINGLABEL_H
#define LOADINGLABEL_H

#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QIcon>
#include <QDebug>
#include <QPaintEvent>

class LoadingLabel : public QLabel
{
public:
    explicit LoadingLabel(QObject *parent = nullptr);
    ~LoadingLabel();
    void setTimerStop();
    void setTimerStart();
    void setTimeReresh(int);

private slots:
    void Refresh_icon();
private:
    QTimer *m_timer;
    int i;
};

#endif // LOADINGLABEL_H
