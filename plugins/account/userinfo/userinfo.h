#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>
#include <QtPlugin>
#include "mainui/interface.h"

#include <QWidget>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QDBusInterface>
#include <QSignalMapper>

#include "qtdbus/systemdbusdispatcher.h"
#include "qtdbus/userdispatcher.h"

#include "changepwddialog.h"
#include "changefacedialog.h"
#include "changetypedialog.h"
#include "deluserdialog.h"
#include "createuserdialog.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
}

enum {
    STANDARDUSER,
    ADMINISTRATOR,
    ROOT
};

typedef struct _UserInfomation {
    QString objpath;
    QString username;
    QString iconfile;
    QString passwd;
    int accounttype;
    int passwdtype;
    bool current;
    bool logined;
    bool autologin;
    qint64 uid;
}UserInfomation;

namespace Ui {
class UserInfo;
}

class UserInfo : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    UserInfo();
    ~UserInfo();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;

    void get_all_users();
    UserInfomation init_user_info(QString objpath);
    void init_root_info();
    void setup_otherusers_ui();
    void build_item_with_widget(UserInfomation user);
    void ui_component_init();
    void ui_status_init();

    QString accounttype_enum_to_string(int id);
    QString login_status_bool_to_string(bool status);

private:
    Ui::UserInfo *ui;

    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

    SystemDbusDispatcher * sysdispatcher;

    QMap<QString, UserInfomation> allUserInfoMap;
    QMap<QString, QToolButton *> otherbtnMap;
    QMap<QString, QListWidgetItem *> otherItemMap;

    QSignalMapper * pwdSignalMapper;
    QSignalMapper * faceSignalMapper;
    QSignalMapper * typeSignalMapper;
    QSignalMapper * delSignalMapper;

    QSize faceSize;
    QSize itemSize;
    QSize btnSize;

    int adminnum;

private slots:
    void show_change_pwd_dialog_slot(QString username);
    void change_pwd_slot(QString pwd, QString username);
    void change_pwd_done_slot();

    void show_change_face_dialog_slot(QString username);
    void change_face_slot(QString facefile, QString username);
    void change_face_done_slot();

    void show_change_accounttype_dialog_slot(QString username);
    void change_accounttype_slot(int atype, QString username, bool status);
    void change_accounttype_done_slot();

    void show_del_user_dialog_slot(QString username);
    void delete_user_slot(bool removefile, QString username);
    void delete_user_done_slot(QString objpath);

    void show_create_user_dialog_slot();
    void create_user_slot(QString username, QString pwd, QString pin, int atype, bool autologin);
    void create_user_done_slot(QString objpath);
};

#endif // USERINFO_H
