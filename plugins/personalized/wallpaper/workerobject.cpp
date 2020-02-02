#include "workerobject.h"

#include "xmlhandle.h"

#include <QDebug>

WorkerObject::WorkerObject()
{
}

WorkerObject::~WorkerObject()
{
    delete xmlHandleObj;
}

void WorkerObject::run(){
    //构建xmlhandle对象
    xmlHandleObj = new XmlHandle();

    //解析壁纸数据，如果本地xml文件不存在则自动构建
    xmlHandleObj->init();

    //获取壁纸数据
    wallpaperinfosMap = xmlHandleObj->requireXmlData();

    //压缩壁纸
    QSize IMAGE_SIZE(166, 110);
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

        emit pixmapGenerate(pixmap, filename);
    }

    emit workComplete();

}
