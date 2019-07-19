#ifndef SYSTEMDBUSDISPATCHER_H
#define SYSTEMDBUSDISPATCHER_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>

#include<QMessageBox>

class SystemDbusDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit SystemDbusDispatcher(QObject * parent = 0);
    ~SystemDbusDispatcher();

    QStringList list_cached_users();

    void create_user(QString name, QString fullname, int accounttype);
    void delete_user(qint64 uid, bool removefile);

private:
    QDBusInterface * systemiface;

private slots:
    void create_user_success(QDBusObjectPath objpath);
    void delete_user_success(QDBusObjectPath objpath);

signals:
    void createuserdone(QString path);
    void deleteuserdone(QString path);

};

#endif // SYSTEMDBUSDISPATCHER_H
