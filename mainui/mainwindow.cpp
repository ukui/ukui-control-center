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
#include "ui_mainwindow.h"

#include <QProcess>

#include "../plugins/pluginsComponent/publicdata.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    double scaleWidth = 1004 / 1920; // 1920*1080 下 标准大小  1004 * 744
//    double scaleHeight = 744 / 1080;

//    resize(QGuiApplication::primaryScreen()->availableSize() * 4 / 5);
    //主屏幕分辨率
    QScreen * screen = QGuiApplication::primaryScreen();
    QRect totalRect = screen->geometry();
    int screenWidth = totalRect.width();
    int screenHeight = totalRect.height();
    qDebug() << screenHeight << screenWidth;
    resize(screenWidth * 4 / 5, screenHeight * 9 / 10); //非标准

    //设置panel图标
    QIcon panelicon;
    if (QIcon::hasThemeIcon("applications-system"))
        panelicon = QIcon::fromTheme("applications-system");
    else
        panelicon = QIcon("://applications-system.svg");
    this->setWindowIcon(panelicon);

    //background color
    QPalette bgPalette;
    bgPalette.setBrush(QPalette::Window, QBrush(Qt::gray));
//    this->setPalette(bgPalette);

    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Widget);
//    this->setStyleSheet("QWidget#MainWindow{border: 1px solid #2d7d9a; border-radius: 5px;}");
    this->setStyleSheet("QWidget#titleWidget{background-color: #f5f6f7; border-bottom: none}");
    this->setStyleSheet("QWidget#MainWindow{border: 1px solid #2d7d9a;}");
//    this->setStyleSheet("QToolButton#backBtn#forwardBtn#minBtn#closeBtn{border: none}");
    //设置左上角样式
    ui->ltBtn->setIcon(QIcon(":/title/logo.svg"));
    ui->ltBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->ltBtn->setStyleSheet("border:none; font-size: 14px; background-color: #2669b5; color: #ffffff");
    //设置toolbutton样式
    ui->minBtn->setStyleSheet("border: none");
    ui->closeBtn->setStyleSheet("border: none");


    ui->lineEdit->setPlaceholderText(tr("Find Devices"));
    ui->lineEdit->hide();

    connect(ui->minBtn, SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(ui->closeBtn, SIGNAL(clicked(bool)), this, SLOT(close()));

    loadPlugins();

    mainpageWidget = new MainPageWidget(this);
    ui->stackedWidget->addWidget(mainpageWidget);

    modulepageWidget = new ModulePageWidget(this);
    ui->stackedWidget->addWidget(modulepageWidget);

    PublicData * publicdata = new PublicData();

    if (QApplication::arguments().length() > 1 && QApplication::arguments().at(1) == "-a"){ //桌面背景
        QStringList keyStringList = publicdata->subfuncList[PERSONALIZED];
        ui->stackedWidget->setCurrentIndex(1);
        //switch_modulepage(QObject * plugin, int page) arg1: plugin obj; arg2: plugin page;
        modulepageWidget->switch_modulepage(modulesList.at(PERSONALIZED).value(keyStringList.at(0)),0);
    }

    if (QApplication::arguments().length() > 1 && QApplication::arguments().at(1) == "-d"){ //显示器
        QStringList keyStringList = publicdata->subfuncList[SYSTEM];
        ui->stackedWidget->setCurrentIndex(1);
        modulepageWidget->switch_modulepage(modulesList.at(SYSTEM).value(keyStringList.at(0)),0);
    }

    delete publicdata;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *){
    qDebug() << "******close Event***";
    qApp->exit();
}

//void MainWindow::mousePressEvent(QMouseEvent * event){
//    mousePress = false;
//    if (event->button() == Qt::LeftButton){
//        mousePress = true;
//        dragPos = event->globalPos() - pos();
//    }
//}

//void MainWindow::mouseReleaseEvent(QMouseEvent *event){
//    if (event->button() & Qt::LeftButton){
//        mousePress = false;
////        setWindowOpacity(1); //设置透明度,1不透明
//    }
//}

//void MainWindow::mouseMoveEvent(QMouseEvent *event){
//    if (event->buttons() & Qt::LeftButton){
//        if (mousePress){
//            move(event->globalPos() - dragPos);
////            setWindowOpacity(0.5);
//        }
//    }
//}

void MainWindow::backToMain(){
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::pluginClicked_cb(QObject * plugin){
    ui->stackedWidget->setCurrentIndex(1);
    modulepageWidget->switch_modulepage(plugin);
}

void MainWindow::loadPlugins(){
//    moduleDir = QDir(qApp->applicationDirPath());
//    moduleDir.cd("plugins");
//    foreach (QString dirname, moduleDir.entryList(QDir::Dirs)) {
//        if (dirname == "." || dirname == "..")
//            continue;
//        pluginDir = QDir(moduleDir.currentPath());
//        pluginDir.cd("plugins");
//        pluginDir.cd(dirname);

//        //遍历
//        QMap<QString, QObject *> pluginsMaps;
//        foreach (QString fileName, pluginDir.entryList(QDir::Files)){
//            QPluginLoader loader(pluginDir.absoluteFilePath(fileName));
//            QObject * plugin = loader.instance();
//            if (plugin){
//                CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
//                pluginsMaps.insert(pluginInstance->get_plugin_name(), plugin);
//            }
//        }
//        modulesList.append(pluginsMaps);
//    }

    for (int index = 0; index < FUNCTOTALNUM; index++){
        QMap<QString, QObject *> pluginsMaps;
        QStringList pluginStringList;
        modulesList.append(pluginsMaps);
    }

    static bool installed = (QCoreApplication::applicationDirPath() == QDir(("/usr/bin")).canonicalPath());

    if (installed)
        pluginsDir = QDir("/usr/lib/control-center/plugins/");
    else {
        pluginsDir = QDir(qApp->applicationDirPath() + "/plugins/");
    }

//    pluginsDir = QDir(qApp->applicationDirPath());
//    pluginsDir.cd("plugins");


    QString version = pkgversion("ukui-screensaver");

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)){
        //低版本ukui-screensaver不存在锁屏背景的设置，跳过
        if (version.startsWith("1") && fileName == "libscreenlock.so")
            continue;


        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject * plugin = loader.instance();
        if (plugin){
            CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
            modulesList[pluginInstance->get_plugin_type()].insert(pluginInstance->get_plugin_name(), plugin);
        }
        else {
            if (fileName.endsWith(".so"))
                qDebug() << fileName << "Load Failed: " << loader.errorString() << "\n";
        }
    }

//    QMap<QString, QObject *> tmpMaps;
//    QMap<QString, QObject *>::iterator it;
//    tmpMaps = modulesList[DEVICES];
//    for (it = tmpMaps.begin(); it != tmpMaps.end(); it++){
//        qDebug() << it.key() << "------";
//    }
}

QMap<QString, QObject *> MainWindow::export_module(int type){
    QMap<QString, QObject *> emptyMaps;
    if (type < modulesList.length())
        return modulesList[type];
    else
        return emptyMaps;
}

QString MainWindow::pkgversion(QString pkgname){
    QProcess process;
    QString cmd = QString("dpkg -s %1").arg(pkgname);
    process.start(cmd);
    process.waitForFinished();
    QString output = QString(process.readAllStandardOutput());
    QStringList pkgstatus = output.split("\n");
    QString version;
    for (QString line : pkgstatus){
        if (line.startsWith("Version"))
            version = line.split(" ").at(1);
        else
            continue;
    }
    return version;

}
