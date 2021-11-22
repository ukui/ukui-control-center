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

#include "loadinglabel.h"

LoadingLabel::LoadingLabel(QObject *parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(100);
    connect(m_timer,SIGNAL(timeout()),this,SLOT(Refresh_icon()));
    this->setPixmap(QIcon::fromTheme("ukui-loading-"+QString("%1").arg(i)).pixmap(this->width(),this->height()));
}

LoadingLabel::~LoadingLabel()
{
//    delete m_timer;
}

void LoadingLabel::setTimerStop()
{
    m_timer->start();
}

void LoadingLabel::setTimerStart()
{
    m_timer->stop();
}

void LoadingLabel::setTimeReresh(int m)
{
    m_timer->setInterval(m);
}

void LoadingLabel::Refresh_icon()
{
    qDebug() << Q_FUNC_INFO;
    if(i == 8)
        i = 0;
    this->setPixmap(QIcon::fromTheme("ukui-loading-"+QString::number(i,10)).pixmap(this->width(),this->height()));
    this->update();
    i++;
}
