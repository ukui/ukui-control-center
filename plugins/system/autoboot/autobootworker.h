#ifndef AUTOBOOTWORKER_H
#define AUTOBOOTWORKER_H

#include <QObject>

//#include "datadefined.h"

class AutobootWorker : public QObject
{
    Q_OBJECT

public:
    explicit AutobootWorker(QObject * parent = 0);
//    ~AutobootWorker();

public:


Q_SIGNALS:
    void threadComplete();

public slots:
    void doWork();

};

#endif // AUTOBOOTWORKER_H
