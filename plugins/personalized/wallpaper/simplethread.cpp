#include "simplethread.h"

#include <QHBoxLayout>
#include <QLabel>

#include <QDebug>

SimpleThread::SimpleThread(QMap<QString, QMap<QString, QString> > _wpinfo, QObject *parent) : QThread(parent),
    wallpaperinfosMap(_wpinfo)
{
}

SimpleThread::~SimpleThread()
{
}

void SimpleThread::run(){
    QSize IMAGE_SIZE(160, 120);
    QMap<QString, QMap<QString, QString> >::iterator iters = wallpaperinfosMap.begin();
    for (int row = 0; iters != wallpaperinfosMap.end(); iters++, row++){
        //跳过xml的头部信息
        if (QString(iters.key()) == "head")
            continue;

        //跳过被删除的壁纸
        QMap<QString, QString> wpMap = (QMap<QString, QString>)iters.value();
        QString delstatus = QString(wpMap.find("deleted").value());
        if (delstatus == "true")
            continue;

        QString filename = QString(iters.key());
        QPixmap pixmap = QPixmap(filename).scaled(IMAGE_SIZE);

        emit widgetItemCreate(pixmap, filename);
//        QPixmap pixmap(filename);
//        QListWidgetItem * item = new QListWidgetItem(QIcon(pixmap.scaled(IMAGE_SIZE)), "");
//        item->setSizeHint(ITEM_SIZE);
//        item->setData(Qt::UserRole, filename);
    }

}
