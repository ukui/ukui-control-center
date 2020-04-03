/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>
#include <QtPlugin>

#include <QMouseEvent>

#include "shell/interface.h"

#include "qtdbus/systemdbusdispatcher.h"
#include "qtdbus/userdispatcher.h"

#include "changepwddialog.h"
#include "changefacedialog.h"
#include "changetypedialog.h"
#include "deluserdialog.h"
#include "createuserdialog.h"

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
    bool noPwdLogin;
    qint64 uid;
}UserInfomation;

namespace Ui {
class UserInfo;
}

class QDBusInterface;
class SwitchButton;

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
    QWidget *get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;

public:
    void initComponent();
    void initAllUserStatus();

    void _acquireAllUsersInfo();
    UserInfomation _acquireUserInfo(QString objpath);
    QString _accountTypeIntToString(int type);
    void _buildWidgetForItem(UserInfomation user);
    void _resetListWidgetHeigh();

    void _refreshUserInfoUI();

    void showCreateUserDialog();
    void createUser(QString username, QString pwd, QString pin, int atype);
    void createUserDone(QString objpath);

    void showDeleteUserDialog(QString username);
    void deleteUser(bool removefile, QString username);
    void deleteUserDone(QString objpath);

    void showChangePwdDialog(QString username);
    void changeUserPwd(QString pwd, QString username);

    void showChangeTypeDialog(QString username);
    void changeUserType(int atype, QString username);

    void showChangeFaceDialog(QString username);
    void changeUserFace(QString facefile, QString username);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::UserInfo *ui;

    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;

private:
    SwitchButton * nopwdSwitchBtn;
    SwitchButton * autoLoginSwitchBtn;

    SystemDbusDispatcher * sysdispatcher;

private:
    QMap<QString, UserInfomation> allUserInfoMap;
    QMap<QString, QListWidgetItem *> otherUserItemMap;

    int adminnum;

    QString _newUserPwd;

    QString pwdcreate;

    QDBusInterface * sysinterface;

};

#endif // USERINFO_H
