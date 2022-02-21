#ifndef UPDATESOURCE_H
#define UPDATESOURCE_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDebug>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDBusReply>

#include "updatedbus.h"
const QString Symbol = "control";
class UpdateSource : public QObject
{
    Q_OBJECT
public:
    explicit UpdateSource(QObject *parent = nullptr);
    QDBusInterface *serviceInterface;
    void getReply(QDBusPendingCallWatcher *call);
    void callDBusUpdateTemplate();
    QString getFailInfo(int statusCode);
    QString getOrSetConf(QString type, QStringList name);
    void killProcessSignal(int pid, int signal);
signals:
    void getReplyFalseSignal();
    void startDbusFinished();
    void sigReconnTimes(int);
public slots:
    void startDbus();
};

#endif // UPDATESOURCE_H
