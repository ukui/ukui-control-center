#include "systemdbusdispatcher.h"

#include <QDebug>

SystemDbusDispatcher::SystemDbusDispatcher(QObject *parent) :
    QObject(parent)
{
    systemiface = new QDBusInterface("org.freedesktop.Accounts",
                                     "/org/freedesktop/Accounts",
                                     "org.freedesktop.Accounts",
                                     QDBusConnection::systemBus());
    connect(systemiface, SIGNAL(UserAdded(QDBusObjectPath)), this, SLOT(create_user_success(QDBusObjectPath)));
    connect(systemiface, SIGNAL(UserDeleted(QDBusObjectPath)), this, SLOT(delete_user_success(QDBusObjectPath)));
}

SystemDbusDispatcher::~SystemDbusDispatcher()
{
    delete systemiface;
    systemiface = NULL;
}

QStringList SystemDbusDispatcher::list_cached_users(){
    QStringList users;
    QDBusReply<QList<QDBusObjectPath> > reply = systemiface->call("ListCachedUsers");
    if (reply.isValid()){
        for (QDBusObjectPath op : reply.value())
            users << op.path();
    }
    return users;
}

void SystemDbusDispatcher::create_user(QString name, QString fullname, int accounttype){
    QDBusReply<QDBusObjectPath> repley = systemiface->call("CreateUser", name, fullname, accounttype);
    if (!repley.isValid())
        qDebug() << "Create User Error" << repley.error().message();
//        QMessageBox::information(0, "Create User Error", repley.error().message());
}

void SystemDbusDispatcher::delete_user(qint64 uid, bool removefile){
    systemiface->call("DeleteUser", uid, removefile);
}

void SystemDbusDispatcher::create_user_success(QDBusObjectPath objpath){
    emit createuserdone(objpath.path());
}

void SystemDbusDispatcher::delete_user_success(QDBusObjectPath objpath){
    emit deleteuserdone(objpath.path());
}
