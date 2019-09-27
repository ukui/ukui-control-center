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

#include <QTranslator>

#include "kpplication.h"

int main(int argc, char *argv[])
{
//    QApplication a(argc, argv);

    Kpplication app("ukui-control-center", argc, argv);
    app.setQuitOnLastWindowClosed(false);

    app.setOrganizationName("KYLIN");
    app.setApplicationName("ukui-control-center");
    app.setApplicationVersion("2.0.0");


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

//    Kpplication * app_instance = Kpplication::instance();
//    if (app_instance->isRunning()){
//        app_instance->sendMessage("Already Running!");
//        return 1;
//    }
    if (app.isRunning()){
        return !app.sendMessage("Already Running!");
    }

    MainWindow w;
    app.setActivationWindow(&w);
    w.show();

    return app.exec();
}
