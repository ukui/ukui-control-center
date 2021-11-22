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

#include "checkbutton.h"
#include <QDebug>

m_button::m_button(QWidget *parent) : QPushButton(parent)
{
    m_cTimer = new QTimer(this);
    states =true;
    m_cTimer->setInterval(140);//设置动画速度
    buttonshow();

}

void m_button::buttonshow()
{
    //通过定时器实现循环插入载入图片
    connect(m_cTimer,&QTimer::timeout, [this] () {
            QIcon icon;
            QString str = QString(":/img/plugins/upgrade/loading%1.svg").arg(i);
            icon.addFile(str);
            this->setIcon(icon);
            i--;
            if(i==9){
                i=17;
            }
        });
}

void m_button::start()
{
    this->setText("");
    m_cTimer->start();
    states = false;
}

void m_button::stop()
{
    QIcon icon;
    this->setIcon(icon);
    m_cTimer->stop();
    states = true;
}

void m_button::buttonswitch()
{
    //两态开关，实现一个按钮开始暂停功能
    qDebug()<<states;
    if(states){
        start();
    }else{
        stop();
    }
}
