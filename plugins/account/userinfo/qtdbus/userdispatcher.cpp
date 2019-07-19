#include "userdispatcher.h"

#include <QDebug>

UserDispatcher::UserDispatcher(QString objpath, QObject *parent) :
    QObject(parent)
{
    this->setParent(parent);
    useriface = new QDBusInterface("org.freedesktop.Accounts",
                                     objpath,
                                     "org.freedesktop.Accounts.User",
                                     QDBusConnection::systemBus());
    pUserInfo = (UserInfo *)QObject::parent();
}

UserDispatcher::~UserDispatcher()
{
    delete useriface;
    useriface = NULL;
}

/*来自gtk控制面板的加密代码*/
QString UserDispatcher::make_crypted (const gchar *plain){
    GString *salt;
    gchar *result;
    GRand *rand;
    gint i;

    rand = g_rand_new ();
    salt = g_string_sized_new (21);

    gchar salt_char[] = "ABCDEFGHIJKLMNOPQRSTUVXYZ"
                   "abcdefghijklmnopqrstuvxyz"
                   "./0123456789";

//    /* SHA 256 */
    g_string_append (salt, "$6$");
    for (i = 0; i < 16; i++) {
        g_string_append_c (salt, salt_char[g_rand_int_range(rand, 0, G_N_ELEMENTS (salt_char) )]);
    }
    g_string_append_c (salt, '$');

//    result = g_strdup ((const gchar *)crypt(plain, salt->str)); //运行后找不到crypt undefined symbol: crypt

    g_string_free (salt, TRUE);
    g_rand_free (rand);

//    return QString(result);
    return "";
}

QString UserDispatcher::change_user_pwd(QString pwd, QString hint){

       qDebug() << "need make_crypt, so run nothing";
       qDebug() << pwd << hint;

//    QDBusReply<QString> reply = useriface->call("SetPassword", pwd, hint);
//    if (reply.isValid())
//        qDebug() << reply.value();
//    else
//        qDebug() << reply.error().message();
    return "";
}

void UserDispatcher::change_user_type(int atype){
    useriface->call("SetAccountType", QVariant(atype));
}

void UserDispatcher::change_user_face(QString facefile){
    useriface->call("SetIconFile", QVariant(facefile));
}
