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

#ifndef FONTWATCHER_H
#define FONTWATCHER_H

#include <QObject>
#include <QWidget>
#include <QGSettings>
#include <QList>

struct widgetcontent
{
    int n_font_size;
    int n_font_point_size;
    QWidget* contentwidget;
    int limit;
};

class FontWatcher : public QWidget
{
    Q_OBJECT
public:
    explicit FontWatcher(QWidget *parent = nullptr);
    //virtual ~FontWatcher();
public:


    void               init();
    void               Set_Special_Font_Size(int size);              //设置特殊字体
    void               Add_Item_Contents_Widget(QWidget *obj);
    void               SetGsetting(QGSettings *gid);
    void               Set_Single_Content_Special(widgetcontent *obj , float add_percent , int n_size,QFont ft);
    int                Get_Init_Font_Size();
    widgetcontent*     Font_Special(QWidget *obj , int limit);
signals:
    void updatefontsize(QWidget *watcher);

private:
    QList<widgetcontent*>       m_Contents_list;                  //需要监听的事件列表
    QGSettings*                 m_FontSettings;                   //监听字体大小
    int                         m_fontsize;                       //变化字体大小
    int                         m_init_fontsize;                  //初始字体大小
    int                         m_SpecialFontsize;                //特殊字体大小

};

#endif // FONTWATCHER_H
