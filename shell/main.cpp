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
#include "framelessExtended/framelesshandle.h"
#include "customstyle.h"

#include <QtSingleApplication>
#include <QTranslator>
#include <fcntl.h>
#include <syslog.h>
#include <QObject>
#include <QDesktopWidget>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QTimer>
#include <QDebug>
#include <QGSettings>
#include <QSharedPointer>
#include <memory>
#include <X11/Xlib.h>
#include "xatom-helper.h"

void centerToScreen(QWidget* widget) {
    if (!widget)
      return;
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = widget->width();
    int y = widget->height();
    widget->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
}

int main(int argc, char *argv[])
{

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QString id = QString("ukui-control-center" + QLatin1String(getenv("DISPLAY")));
    QtSingleApplication a(id, argc, argv);

    if (a.isRunning()) {
        a.sendMessage(QApplication::arguments().length() > 1 ? QApplication::arguments().at(1) : a.applicationFilePath());
        qDebug() << QObject::tr("ukui-control-center is already running!");
        return EXIT_SUCCESS;
    } else {
        //控制面板是否被禁用
        QString m_initPath =  QString("%1/%2/%3").arg(QDir::homePath()).arg(".cache/ukui-menu").arg("ukui-menu.ini");
        QSettings settings(m_initPath, QSettings::IniFormat);
        settings.beginGroup("application");
        if (settings.contains("ukui-control-center")){
            bool isRun = settings.value("application/ukui-control-center").toBool();
            if (!isRun) {
                qDebug() << QObject::tr("ukui-control-center is disabled！");
                return EXIT_SUCCESS;
            }
        }
        settings.sync();
        settings.endGroup();

        // 加载国际化文件
        QTranslator translator;
        translator.load("/usr/share/ukui-control-center/shell/res/i18n/" + QLocale::system().name());
        a.installTranslator(&translator);

        //命令行参数
        //暂时隐藏了不必要的参数
        QCoreApplication::setApplicationName("ukui-control-center");
        QCoreApplication::setApplicationVersion("2.0");
        QCommandLineParser parser;
        QCommandLineOption monitorRoleOption("m", "Go to monitor settings page");
        QCommandLineOption backgroundRoleOption("b", "Go to background settings page");
        QCommandLineOption userinfoRoleOption("u", "Go to userinfo settings page");
        QCommandLineOption aboutRoleOption("a", "Go to about settings page");
        QCommandLineOption powerRoleOption("p", "Go to power settings page");
        QCommandLineOption datetimeRoleOption("t", "Go to datetime settings page");
//        QCommandLineOption desktopRoleOption("d", "Go to desktop settings page");
        QCommandLineOption audioRoleOption("s", "Go to audio settings page");
//        QCommandLineOption noticeRoleOption("n", "Go to notice settings page");
        QCommandLineOption userinfoOption("userinfo", "Go to userinfo settings page");
        QCommandLineOption netOption("network", "Go to netconnect settings page");
        QCommandLineOption bluetoothOption("bluetooth", "Go to bluetooth settings page");
//        QCommandLineOption vpnOption("vpn", "Go to vpn settings page");
        QCommandLineOption proxyOption("proxy", "Go to proxy settings page");
        QCommandLineOption themeOption("theme", "Go to theme settings page");
        QCommandLineOption wallpaperOption("wallpaper", "Go to wallpaper settings page");
        QCommandLineOption screenlockOption("screenlock", "Go to screenlock settings page");
        QCommandLineOption fontOption("font", "Go to font settings page");
        QCommandLineOption autobootOption("startup", "Go to startup settings page");
        QCommandLineOption powerOption("battery", "Go to battery settings page");
        QCommandLineOption opmodeOption("opmode", "Go to operating mode settings page");
        QCommandLineOption audioOption("audio", "Go to audio settings page");
        QCommandLineOption datetimeOption("datetime", "Go to datetime settings page");
        QCommandLineOption areaOption("language", "Go to language and region settings page");
        QCommandLineOption backupOption("recovery", "Go to recovery settings page");
//        QCommandLineOption saveeyeOption("saveeye", "Go to saveeye settings page");
        QCommandLineOption displayOption("displays", "Go to displays settings page");
        QCommandLineOption printerOption("printer", "Go to printer settings page");
        QCommandLineOption mouseOption("mouse", "Go to mouse settings page");
        QCommandLineOption touchpadOption("trackpad", "Go to trackpad settings page");
        QCommandLineOption gestureOption("gesture", "Go to gesture settings page");
        QCommandLineOption printersOption("pen", "Go to pen settings page");
//        QCommandLineOption keyboardOption("keyboard", "Go to keyboard settings page");
        QCommandLineOption aboutOption("about", "Go to about settings page");

        parser.addHelpOption();
        parser.addVersionOption();
        parser.addOption(monitorRoleOption);
        parser.addOption(backgroundRoleOption);
        parser.addOption(userinfoRoleOption);
        parser.addOption(aboutRoleOption);
        parser.addOption(powerRoleOption);
        parser.addOption(datetimeRoleOption);
//        parser.addOption(desktopRoleOption);
        parser.addOption(audioRoleOption);
//        parser.addOption(noticeRoleOption);
        parser.addOption(userinfoOption);
        parser.addOption(netOption);
//        parser.addOption(vpnOption);
        parser.addOption(bluetoothOption);
        parser.addOption(proxyOption);
        parser.addOption(themeOption);
        parser.addOption(wallpaperOption);
        parser.addOption(screenlockOption);
        parser.addOption(fontOption);
        parser.addOption(autobootOption);
        parser.addOption(powerOption);
        parser.addOption(opmodeOption);
        parser.addOption(audioOption);
        parser.addOption(datetimeOption);
        parser.addOption(areaOption);
        parser.addOption(backupOption);
//        parser.addOption(saveeyeOption);
        parser.addOption(displayOption);
        parser.addOption(printerOption);
        parser.addOption(mouseOption);
        parser.addOption(touchpadOption);
        parser.addOption(gestureOption);
        parser.addOption(printersOption);
//        parser.addOption(keyboardOption);
        parser.addOption(aboutOption);
        parser.process(a);

        MainWindow * w = new MainWindow;
        centerToScreen(w);
        w->setAttribute(Qt::WA_DeleteOnClose);

        a.setActivationWindow(w);
        QObject::connect(&a, SIGNAL(messageReceived(const QString&)),w, SLOT(sltMessageReceived(const QString&)));

//        auto style = new InternalStyle(nullptr);
//        a.setStyle(style);

        //使用窗管的无边框策略
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(w->winId(), hints);
        w->show();
        return a.exec();
    }
}

