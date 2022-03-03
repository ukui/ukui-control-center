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
#include "gesture.h"
#include "ui_gesture.h"
#include <QProcess>
#include <QDebug>
#include <QMouseEvent>
#include <QMap>
#define ITEMFIXEDHEIGH 58

Gesture::Gesture() : mFirstLoad(true)
{
    pluginName = tr("Gesture Guidance");
    pluginType = OTHER;
    QString path=":/img/plugins/gesture/gesture.ini";
    gesturesetting=new QSettings(path,QSettings::IniFormat);
}

Gesture::~Gesture()
{
    if (!mFirstLoad) {
        delete ui;
    }
    delete gesturesetting;
}

QString Gesture::get_plugin_name(){
   return pluginName;
}

int Gesture::get_plugin_type(){
   return pluginType;
}

QWidget *Gesture::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Gesture;
        pluginWidget = new QWidget;

        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        //~ contents_path /gesture/Gesture Guidance
        ui->label->setText(tr("Gesture Guidance"));
        ui->label->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
        gifFlowLayout=new FlowLayout(ui->frame,0,20,20);
        gifFlowLayout->setContentsMargins(0, 0, 0, 0);
        gifFlowLayout->setSpacing(100);
        ui->frame->setLayout(gifFlowLayout);
        additem();
    }
   return pluginWidget;
}
void Gesture::plugin_delay_control(){

}

const QString Gesture::name() const {

   return QStringLiteral("gesture");
}


void Gesture::additem()
{
    gesturesetting->beginGroup("gesture");
    QStringList giflist= gesturesetting->allKeys();
    QMap<QString,int> list;
    for (int i = 0;  i< giflist.count();i ++) {
        list.insert(giflist.at(i),gesturesetting->value(giflist.at(i)).toInt());
    }
    gesturesetting->sync();
    gesturesetting->endGroup();
    for(int i=0;i<giflist.count();i++) {
        Itemwidget * item=new Itemwidget(list.key(i), ui->frame);
        connect(item, &Itemwidget::picClicked, this, [=](){
            if (mCurrentItem != nullptr) {
                mCurrentItem->stopMovieSlot();
            }
            mCurrentItem = item;
            item->startMovieSlot();
        });
        gifFlowLayout->addWidget(item);
    }
}
