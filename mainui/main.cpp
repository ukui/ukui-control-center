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
#include "mainwindow.h"
#include <QApplication>
#include <QtSingleApplication>

#include <QTranslator>

#include "kpplication.h"
#include "framelessExtended/framelesshandle.h"

int main(int argc, char *argv[])
{
//    QApplication a(argc, argv);

    QtSingleApplication app(argc, argv);
    if (app.isRunning()){
        qDebug() << QObject::tr("ukui-control-center had already running!");
        return 0;
    }

    //来自网络的单例模式模块
//    Kpplication kapp("ukui-control-center", argc, argv);
//    kapp.setQuitOnLastWindowClosed(false);

//    kapp.setOrganizationName("KYLIN");
//    kapp.setApplicationName("ukui-control-center");
//    kapp.setApplicationVersion("2.0.0");


    //i18n
    QString locale = QLocale::system().name();
    QTranslator translator;
    if (locale == "zh_CN"){
        if (translator.load("zh_CN.qm", "://i18n/"))
            app.installTranslator(&translator);
        else
            qDebug() << "Load translations file" << locale << "failed!";
    }

    //load qss file
    QString qss;
    QFile QssFile("://main.qss");
    QssFile.open(QFile::ReadOnly);

    if (QssFile.isOpen()){
        qss = QLatin1String(QssFile.readAll());
        qApp->setStyleSheet(qss);
        QssFile.close();
    }

//    if (kapp.isRunning()){
//        return !kapp.sendMessage("Already Running!");
//    }

    MainWindow * w = new MainWindow;
//    kapp.setActivationWindow(&w);
    w->show();

    FramelessHandle * pHandle = new FramelessHandle(w);
    pHandle->activateOn(w);

    return app.exec();
}
