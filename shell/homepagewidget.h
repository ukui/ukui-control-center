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
#ifndef HOMEPAGEWIDGET_H
#define HOMEPAGEWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QVariantMap>
#include <QGSettings>

enum COLOR{
    BLUE,
    WHITE,
    BLACK,
    GRAY,
    HIGHLIGHT,
};

class MainWindow;
class QListWidgetItem;

#define PLUGIN_STATUS_SCHEMA  "org.ukui.control-center.pluginstatus"
#define PLUGINS_SCHEMA                  "org.ukui.control-center.plugins"
#define PLUGINS_PATH                         "/org/ukui/control-center/plugins/"
#define PLUGIN_NAME                          "plugin-name"
#define SHOW_KEY                                 "show"

namespace Ui {
class HomePageWidget;
}

class HomePageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HomePageWidget(QWidget *parent = 0);
    ~HomePageWidget();

public:
    void initUI();
    QGSettings * setGsettingsPath(QList<char *> list , QString name);

private:
    // load svg picture
    const QPixmap loadSvg(const QString &fileName, COLOR color);
    // chang svg picture's color
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source, COLOR color);
    bool eventFilter(QObject *watched, QEvent *event);
public:
     QList<char *> list_path;
     QMap<QString, QGSettings *> vecGsettins;
private:
    Ui::HomePageWidget *ui;
    MainWindow * pmainWindow;
    QVariantMap mModuleMap;
};

#endif // HOMEPAGEWIDGET_H
