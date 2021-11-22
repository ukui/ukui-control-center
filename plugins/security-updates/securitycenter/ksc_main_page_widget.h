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

#ifndef KSC_MAIN_PAGE_WIDGET_H
#define KSC_MAIN_PAGE_WIDGET_H

#include <QWidget>
#include "defender_interface.h"
#include "FlowLayout/flowlayout.h"
#define _(STRING) gettext(STRING)

class ksc_module_func_widget;

namespace Ui {
class ksc_main_page_widget;
}

class ksc_main_page_widget : public QWidget
{
    Q_OBJECT

public:
    explicit ksc_main_page_widget(QWidget *parent = nullptr);
    ~ksc_main_page_widget();

private:
    void init_list_widget();
    void auto_set_main_icon(ksc_defender_module &module);
    void set_ksc_defender_text(ksc_defender_module module);

public:
    void refresh_data();

private slots:
    void slot_recv_ksc_defender_module_change(ksc_defender_module module);

    void on_pushButton_clicked();

private:
    Ui::ksc_main_page_widget *ui;

    defender_interface *m_pInterface;

    QMap<int, ksc_module_func_widget*> m_map;

    FlowLayout * flowLayout;
};

#endif // KSC_MAIN_PAGE_WIDGET_H
