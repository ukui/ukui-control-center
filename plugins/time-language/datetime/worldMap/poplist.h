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

#ifndef POPLIST_H
#define POPLIST_H

#include "zoneinfo.h"

#include <QFrame>
#include <QListView>
#include <QStringListModel>
#include <QStringList>

class PopList : public QFrame
{
    Q_OBJECT
public:
    explicit PopList(QWidget* parent = nullptr);

    // 返回模型中视图列表
    QStringList stringList();

Q_SIGNALS:
    //隐藏信号
    void listHide();

    // 列表项点击
    void listAactive(int index);

public slots:
    void showPopLists(QPoint pos);
    // 模型视图内容
    void setStringList(QStringList strings);

protected:
    // esc按键
    void keyPressEvent(QKeyEvent* event) ;

    bool eventFilter(QObject* obj, QEvent* event);
    void hideEvent(QHideEvent* event);
    void showEvent(QShowEvent* event);

private:
    void initUI();
private:
    ZoneInfo* m_zoneinfo;

    // model/view design
    QListView* m_listView;  
    QStringListModel* m_listModel;

private slots:
    // 列表项激活
    void listActiveSlot(QModelIndex idnex);

};

#endif // POPLIST_H
