#ifndef USERDISPATCHER_H
#define USERDISPATCHER_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>

#include <unistd.h>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
}

class UserInfo;

class UserDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit UserDispatcher(QString objpath, QObject *parent = 0);
    ~UserDispatcher();

    QString change_user_pwd(QString pwd, QString hint);
    void change_user_type(int atype);
    void change_user_face(QString facefile);

private:
    QDBusInterface * useriface;

    UserInfo * pUserInfo;

    /***加密,来自gtk控制面板***/
    QString make_crypted(const gchar * plain);
    /*****/
};

#endif // USERDISPATCHER_H
