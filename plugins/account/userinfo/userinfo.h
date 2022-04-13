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

#include <ukcc/widgets/titlelabel.h>
#include <ukcc/widgets/addbtn.h>
#include <ukcc/widgets/switchbutton.h>

#include <QFrame>
#include <QPushButton>
#include <QDBusObjectPath>

#include "changeusertype.h"
#include "changeusernickname.h"
#include "changeuserpwd.h"
#include "changeuserlogo.h"
#include "createusernew.h"
#include "deleteuserexists.h"
#include "utilsforuserinfo.h"

#include <QObject>
#include <QtPlugin>
#include <QFileSystemWatcher>
#include <QMouseEvent>
#include <QSettings>

#include "shell/interface.h"

#include "qtdbus/systemdbusdispatcher.h"
#include "qtdbus/userdispatcher.h"

#include "changegroupdialog.h"

#ifdef ENABLEPQ
extern "C" {
#include <pwquality.h>
}
#endif

#define STYLE_FONT_SCHEMA  "org.ukui.style"

enum {
    STANDARDUSER,
    ADMINISTRATOR,
    ROOT
};

typedef struct _UserInfomation {
    QString objpath;
    QString username;
    QString realname;
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

typedef struct _PwdQualityOption {

    int diff_ok;
    int min_length;
    int dig_credit;
    int up_credit;
    int low_credit;
    int oth_credit;
    int min_class;
    int max_repeat;
    int max_class_repeat;
    int max_sequence;
    int gecos_check;
    int dict_check;
    int user_check;
    int enforcing;
    int retry_times;
    int enforce_for_root;
    int local_users_only;
    int palindrome;
    int no_similar_check;
    char *bad_words;
    char *dict_path;

}PwdQualityOption;

namespace Ui {
class UserInfo;
}

class QDBusInterface;

class UserInfo : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    UserInfo();
    ~UserInfo();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

public:
    AddBtn * addUserBtn;

    SwitchButton * nopwdLoginSBtn;
    SwitchButton * autoLoginSBtn;

    TitleLabel * currentLabel;
    TitleLabel * othersLabel;
    QLabel * currentNickNameLabel;
    QLabel * currentNickNameChangeLabel;
    QLabel * currentUserTypeLabel;
    QLabel * nopwdLoginLabel;
    QLabel * autoLoginLabel;

    QPushButton * currentUserlogoBtn;
    QPushButton * changeCurrentPwdBtn;
    QPushButton * changeCurrentTypeBtn;
    QPushButton * changeCurrentGroupsBtn;

    QVBoxLayout * mainVerLayout;
    QVBoxLayout * currentVerLayout;
    QHBoxLayout * currentUserHorLayout;
    QVBoxLayout * currentUserinfoVerLayout;
    QHBoxLayout * currentNickNameHorLayout;
    QHBoxLayout * nopwdLoginHorLayout;
    QHBoxLayout * autoLoginHorLayout;
    QVBoxLayout * otherVerLayout;
    QHBoxLayout * addUserHorLayout;

    QFrame * currentFrame;
    QFrame * currentUserFrame;
    QFrame * nopwdLoginFrame;
    QFrame * autoLoginFrame;
    QFrame * othersFrame;
    QFrame * addUserFrame;

    QFrame * splitHLine1;
    QFrame * splitHLine2;
    QFrame * splitVLine1;
    QFrame * splitVLine2;

public:
    void initUI();
    void buildAndSetupUsers();
    int isDomainUser(const char* username);
    void setUserConnect();
    void setUserDBusPropertyConnect(const QString pObjPath);
    bool authoriyLogin();
    QString getAutomaticLogin();

    void showChangeUserTypeDialog(QString u);
    void showChangeUserNicknameDialog();
    void showChangeUserPwdDialog(QString pName);
    void showChangeUserLogoDialog(QString pName, UtilsForUserinfo *utilsUser);
    void showCreateUserNewDialog();
    void showDeleteUserExistsDialog(QString pName);

    void buildItemForUsersAndSetConnect(UserInfomation user);

protected:
    QFrame * createHLine(QFrame * f, int len = 0);
    QFrame * createVLine(QFrame * f, int len = 0);
    bool setTextDynamic(QLabel * label, QString string);

private:
    bool isLastAdmin(QString uname);
    bool openAutoLoginMsg(const QString &userName);
    void showMessageBox();

public slots:
    void currentUserPropertyChangedSlot(QString, QMap<QString, QVariant>, QStringList);

    void newUserCreateDoneSlot(QDBusObjectPath op);
    void existsUserDeleteDoneSlot(QDBusObjectPath op);
    void changeUserFace(QString facefile, QString username, UtilsForUserinfo *utilsUser);

Q_SIGNALS:
    void userTypeChanged(QString n);

    /**************/

public:
    void initSearchText();

    QStringList getLoginedUsers();
    void _acquireAllUsersInfo();
    UserInfomation _acquireUserInfo(QString objpath);
    QString _accountTypeIntToString(int type);
    void showChangeGroupDialog();

    void readCurrentPwdConf();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::UserInfo *ui;

    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;
    QWidget * pluginWidget2;

    SystemDbusDispatcher * sysdispatcher;
    QSettings * autoSettings = nullptr;

    QMap<QString, UserInfomation> allUserInfoMap;

    QString mUserName;

    QStringList m_loginedUser;

    QDBusInterface *sysinterface;

    bool enablePwdQuality;
    bool mFirstLoad;

#ifdef ENABLEPQ
    pwquality_settings_t * pwdconf;
#endif

    PwdQualityOption pwdOption;

    QString pwdMsg;

private:
    bool getNoPwdStatus();
    void fontSizeChange(UserInfomation user, UtilsForUserinfo * utils);
    QPixmap makeRoundLogo(QString logo, int wsize, int hsize, int radius);
};

#endif // USERINFO_H
