#include "mainwindow.h"
#include <QApplication>

#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //i18n
    QString locale = QLocale::system().name();
    QTranslator translator;
    if (locale == "zh_CN"){
        if (translator.load("zh_CN.qm", "://i18n/"))
            a.installTranslator(&translator);
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

    MainWindow w;
    w.show();

    return a.exec();
}
