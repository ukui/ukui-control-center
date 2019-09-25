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
