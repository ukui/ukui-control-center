#include "mainwindow.h"
#include <QApplication>

#include <QTranslator>

#include "framelessExtended/framelesshandle.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //加载国际化文件
    QString locale = QLocale::system().name();
    QTranslator translator;
    if (locale == "zh_CN"){
        if (translator.load("zh_CN.qm", "://i18n/"))
            a.installTranslator(&translator);
        else
            qDebug() << "Load translations file" << locale << "failed!";
    }


    //加载qss样式文件
    QString qss;
    QFile QssFile("://global.qss");
    QssFile.open(QFile::ReadOnly);

    if (QssFile.isOpen()){
        qss = QLatin1String(QssFile.readAll());
        qApp->setStyleSheet(qss);
        QssFile.close();
    }


    MainWindow * w = new MainWindow;
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();

    FramelessHandle * pHandle = new FramelessHandle(w);
    pHandle->activateOn(w);

    return a.exec();
}
