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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QLabel>
#include <QListWidgetItem>

#include <QProcess>

#include <QDir>
#include <QPluginLoader>

#include <QDebug>

#include "interface.h"
#include "mainpagewidget.h"
#include "modulepagewidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QStringList systemStringList;

    void initUI();
    void backToMain();

    QMap<QString, QObject *> export_module(int);


private:
    Ui::MainWindow *ui;

    QPoint dragPos;
    bool mousePress;

    QDir moduleDir;
    QDir pluginDir;
    QDir pluginsDir;

    ModulePageWidget * modulepageWidget;
    MainPageWidget * mainpageWidget;

    QList <QMap<QString, QObject *>> modulesList;

    void loadPlugins();

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void closeEvent(QCloseEvent *);

public slots:
    void pluginClicked_cb(QObject * plugin);
};

#endif // MAINWINDOW_H
