#ifndef WORKEROBJECT_H
#define WORKEROBJECT_H

#include <QObject>
#include <QPixmap>

#include "xmlhandle.h"

class QPixmap;

class WorkerObject : public QObject
{
    Q_OBJECT

public:
    explicit WorkerObject();
    ~WorkerObject();

public:
    void run();

private:
    XmlHandle * xmlHandleObj;

    QMap<QString, QMap<QString, QString> > wallpaperinfosMap;

Q_SIGNALS:
    void pixmapGenerate(QPixmap pixmap, QString filename);
    void workComplete();

};

#endif // WORKEROBJECT_H
