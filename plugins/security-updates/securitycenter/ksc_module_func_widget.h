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

#ifndef KSC_MODULE_FUNC_WIDGET_H
#define KSC_MODULE_FUNC_WIDGET_H
#include<QGSettings>
#include <QWidget>
#include <QStyleOption>
#include <QPainter>

#include "ksc_business_def.h"

namespace Ui {
class ksc_module_func_widget;
}

class ksc_module_func_widget : public QWidget
{
    Q_OBJECT

public:
    explicit ksc_module_func_widget(QWidget *parent = nullptr);
    ~ksc_module_func_widget();

    void set_module_data(const ksc_defender_module module);
    void update_module_data(ksc_defender_module module);
    void update_module_icon();

private:
    void runExternalApp(QString cmd) {
        QProcess process(this);
        process.startDetached(cmd);
    }

protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    virtual void paintEvent(QPaintEvent * event);
    void mousePressEvent(QMouseEvent *event);

private:
    Ui::ksc_module_func_widget *ui;

    ksc_defender_module m_module;

    QString m_interval_icon_hover;
    QString m_interval_icon_normal;
};

#endif // KSC_MODULE_FUNC_WIDGET_H
