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
#ifndef MODULEPAGEWIDGET_H
#define MODULEPAGEWIDGET_H

#include <QWidget>
#include <QMap>
#include <QGSettings>
#include <QListWidget>
#include <QPalette>
class MainWindow;
class CommonInterface;
class KeyValueConverter;

class QListWidgetItem;

namespace Ui {
class ModulePageWidget;
}

class LeftWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit LeftWidget(QWidget *parent = 0);
    ~LeftWidget();

private:
    const int perStepDelta = 60;
    const int maxDeltaPerCall = 600;
protected:
    void wheelEvent(QWheelEvent* event);
};

class ModulePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModulePageWidget(QWidget *parent = 0);
    ~ModulePageWidget();

public:
    void initUI();
    void switchPage(QObject * plugin, bool recorded = true);
    void refreshPluginWidget(CommonInterface * plu);
    void highlightItem(QString text);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::ModulePageWidget *ui;

private:
    MainWindow * pmainWindow;
    KeyValueConverter * mkvConverter;
    LeftWidget * leftListWidget;

private:
    QMap<QString, CommonInterface*> pluginInstanceMap;
    QMultiMap<QString, QListWidgetItem*> strItemsMap;//存储功能名与二级菜单item的Map,为了实现高亮
    QListWidgetItem *firstItem;
    QListWidgetItem *mSelectItem;
    bool flagBit;
    QString currentThemeMode;
    QGSettings *qtSettings;
public slots:
    void changePageSlot(QListWidgetItem * cur);

Q_SIGNALS:
    void pageChangeSignal();
};

#endif // MODULEPAGEWIDGET_H
