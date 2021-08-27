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
#include "userinfo.h"
#include "ui_userinfo.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QListView>

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDebug>
#include <QLineEdit>
#include <QMessageBox>

#include <polkit-qt5-1/polkitqt1-authority.h>


#include "ImageUtil/imageutil.h"
#include "elipsemaskwidget.h"
#include "passwdcheckutil.h"
#include "loginedusers.h"

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

#ifdef WITHKYSEC
#include <kysec/libkysec.h>
#include <kysec/status.h>
#endif

#define DEFAULTFACE "/usr/share/ukui/faces/default.png"
#define ITEMHEIGH 60

UserInfo::UserInfo() : mFirstLoad(true)
{
    pluginName = tr("User Info");
    pluginType = ACCOUNT;
}

UserInfo::~UserInfo()
{
    if (!mFirstLoad) {
        delete ui;
        ui = nullptr;
        delete autoSettings;
        autoSettings = nullptr;
    }
}

QString UserInfo::get_plugin_name() {
    return pluginName;
}

int UserInfo::get_plugin_type() {
    return pluginType;
}

QWidget *UserInfo::get_plugin_ui() {
    if (mFirstLoad) {
        mFirstLoad = false;

        ui = new Ui::UserInfo;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        // 构建System dbus调度对象
        sysdispatcher = new SystemDbusDispatcher(this);

        // 获取系统全部用户信息，用户Uid大于等于1000的
        _acquireAllUsersInfo();

        readCurrentPwdConf();
        initComponent();
        initAllUserStatus();
        // 设置界面用户信息
        _refreshUserInfoUI();

        /**/
        pluginWidget2 = new QWidget;
        pluginWidget2->setAttribute(Qt::WA_DeleteOnClose);
        initUI();
        initSearchText();

        buildAndSetupUsers();
        setUserConnect();
        /**/


    }
    return pluginWidget;
}

void UserInfo::plugin_delay_control() {

}

const QString UserInfo::name() const {

    return QStringLiteral("userinfo");
}

void UserInfo::initSearchText() {
    //~ contents_path /userinfo/Password
    ui->changePwdBtn->setText(tr("Password"));
    //~ contents_path /userinfo/Type
    ui->changeTypeBtn->setText(tr("Type"));
    //~ contents_path /userinfo/Login no passwd
    ui->loginpwdLabel->setText(tr("Login no passwd"));
    //~ contents_path /userinfo/enable autoLogin
    ui->autologinLabel->setText(tr("enable autoLogin"));


    currentLabel->setText(tr("CurrentUser"));
    othersLabel->setText(tr("OthersUser"));

}

/**3.1****begin************/

void UserInfo::initUI(){

    //标题
    currentLabel = new TitleLabel();
    othersLabel = new TitleLabel();

    //当前用户区域
    currentFrame = new QFrame();

    ////当前用户
    currentUserFrame = new QFrame();
    //////分割线
    splitVLine1 = createVLine(currentUserFrame, 10);
    splitVLine2 = createVLine(currentUserFrame, 10);

    currentUserlogoBtn = new QPushButton();
    currentUserlogoBtn->setFixedSize(QSize(104, 104));
    currentUserlogoBtn->setIconSize(QSize(96, 96));

    changeCurrentPwdBtn = new QPushButton();
    changeCurrentPwdBtn->setFlat(true);
    changeCurrentPwdBtn->setText(tr("Passwd"));

    changeCurrentTypeBtn = new QPushButton();
    changeCurrentTypeBtn->setFlat(true);
    changeCurrentTypeBtn->setText(tr("Type"));

    changeCurrentGroupsBtn = new QPushButton();
    changeCurrentGroupsBtn->setFlat(true);
    changeCurrentGroupsBtn->setText(tr("Groups"));

    currentNickNameLabel = new QLabel();
    currentNickNameLabel->setFixedHeight(27);
    currentUserTypeLabel = new QLabel();
    currentUserTypeLabel->setFixedHeight(20);

    currentUserinfoVerLayout = new QVBoxLayout();
    currentUserinfoVerLayout->setSpacing(4);
    currentUserinfoVerLayout->setContentsMargins(0, 0, 0, 0);
    currentUserinfoVerLayout->addStretch();
    currentUserinfoVerLayout->addWidget(currentNickNameLabel, Qt::AlignHCenter);
    currentUserinfoVerLayout->addWidget(currentUserTypeLabel, Qt::AlignHCenter);
    currentUserinfoVerLayout->addStretch();

    currentUserHorLayout = new QHBoxLayout();
    currentUserHorLayout->setSpacing(16);
    currentUserHorLayout->setContentsMargins(16, 0, 16, 0);
    currentUserHorLayout->addWidget(currentUserlogoBtn);
    currentUserHorLayout->addLayout(currentUserinfoVerLayout);
    currentUserHorLayout->addStretch();
    currentUserHorLayout->addWidget(changeCurrentPwdBtn);
    currentUserHorLayout->addWidget(splitVLine1);
    currentUserHorLayout->addWidget(changeCurrentTypeBtn);
    currentUserHorLayout->addWidget(splitVLine2);
    currentUserHorLayout->addWidget(changeCurrentGroupsBtn);

    currentUserFrame->setMinimumSize(QSize(550, 127));
    currentUserFrame->setMaximumSize(QSize(16777215, 127));
    currentUserFrame->setFrameShape(QFrame::NoFrame);
    currentUserFrame->setLayout(currentUserHorLayout);


    ////分割线
    splitHLine1 = createHLine(currentFrame);
    splitHLine2 = createHLine(currentFrame);

    ////免密登录
    nopwdLoginFrame = new QFrame();

    nopwdLoginLabel = new QLabel();
    nopwdLoginLabel->setText(tr("AutoLoginOnBoot"));
//    nopwdLoginLabel->setFixedWidth(550);

    nopwdLoginSBtn = new SwitchButton(nopwdLoginFrame);

    nopwdLoginHorLayout = new QHBoxLayout();
    nopwdLoginHorLayout->setSpacing(8);
    nopwdLoginHorLayout->setContentsMargins(16, 0, 16, 0);
    nopwdLoginHorLayout->addWidget(nopwdLoginLabel);
    nopwdLoginHorLayout->addStretch();
    nopwdLoginHorLayout->addWidget(nopwdLoginSBtn);

    nopwdLoginFrame->setMinimumSize(QSize(550, 60));
    nopwdLoginFrame->setMaximumSize(QSize(16777215, 60));
    nopwdLoginFrame->setFrameShape(QFrame::NoFrame);
    nopwdLoginFrame->setLayout(nopwdLoginHorLayout);

    ////开机自动登录
    autoLoginFrame = new QFrame();

    autoLoginLabel = new QLabel();
    autoLoginLabel->setText(tr("LoginWithoutPwd"));
//    autoLoginLabel->setFixedWidth(550);

    autoLoginSBtn = new SwitchButton(autoLoginFrame);

    autoLoginHorLayout = new QHBoxLayout();
    autoLoginHorLayout->setSpacing(8);
    autoLoginHorLayout->setContentsMargins(16, 0, 16, 0);
    autoLoginHorLayout->addWidget(autoLoginLabel);
    autoLoginHorLayout->addStretch();
    autoLoginHorLayout->addWidget(autoLoginSBtn);

    autoLoginFrame->setMinimumSize(QSize(550, 60));
    autoLoginFrame->setMaximumSize(QSize(16777215, 60));
    autoLoginFrame->setFrameShape(QFrame::NoFrame);
    autoLoginFrame->setLayout(autoLoginHorLayout);


    currentVerLayout = new QVBoxLayout();
    currentVerLayout->setSpacing(0);
    currentVerLayout->setContentsMargins(0, 0, 0, 0);
    currentVerLayout->addWidget(currentUserFrame);
    currentVerLayout->addWidget(splitHLine1);
    currentVerLayout->addWidget(nopwdLoginFrame);
    currentVerLayout->addWidget(splitHLine2);
    currentVerLayout->addWidget(autoLoginFrame);

    currentFrame->setMinimumSize(QSize(550, 248));
    currentFrame->setMaximumSize(QSize(16777215, 248));
    currentFrame->setFrameShape(QFrame::Box);
    currentFrame->setLayout(currentVerLayout);

    //其他用户区域
    addUserBtn = new AddBtn;

    addUserHorLayout = new QHBoxLayout();
    addUserHorLayout->setSpacing(8);
    addUserHorLayout->setContentsMargins(0, 0, 0, 0);
    addUserHorLayout->addWidget(addUserBtn);

    addUserFrame = new QFrame();
    addUserFrame->setMinimumSize(QSize(550, 60));
    addUserFrame->setMaximumSize(QSize(16777215, 60));
    addUserFrame->setFrameShape(QFrame::NoFrame);
    addUserFrame->setLayout(addUserHorLayout);

    otherVerLayout = new QVBoxLayout();
    otherVerLayout->setSpacing(0);
    otherVerLayout->setContentsMargins(0, 0, 0, 0);
    otherVerLayout->addWidget(addUserFrame);
    otherVerLayout->addStretch();

    othersFrame = new QFrame();
    othersFrame->setMinimumSize(QSize(550, 60));
    othersFrame->setMaximumSize(QSize(16777215, 16777215));
    othersFrame->setFrameShape(QFrame::Box);
    othersFrame->setLayout(otherVerLayout);


    //界面主布局
    mainVerLayout = new QVBoxLayout(pluginWidget2);
    mainVerLayout->setSpacing(8);
    mainVerLayout->setContentsMargins(0, 0, 40, 40);

    mainVerLayout->addWidget(currentLabel);
    mainVerLayout->addWidget(currentFrame);

    mainVerLayout->addWidget(othersLabel);
    mainVerLayout->addWidget(othersFrame);


    pluginWidget2->setLayout(mainVerLayout);

}

void UserInfo::buildAndSetupUsers(){

    QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
    for (; it != allUserInfoMap.end(); it++){
        UserInfomation user = it.value();

        //用户头像为.face且.face文件不存在
        char * iconpath = user.iconfile.toLatin1().data();
        if (!g_file_test(iconpath, G_FILE_TEST_EXISTS)){
            user.iconfile = DEFAULTFACE;
            //更新用户数据
            allUserInfoMap.find(it.key()).value().iconfile = DEFAULTFACE;
        }

        //当前用户
        if (user.username == QString(g_get_user_name())){
            //设置用户头像
            currentUserlogoBtn->setIcon(QIcon(user.iconfile));
            ////圆形头像
            ElipseMaskWidget * currentElipseMaskWidget = new ElipseMaskWidget(currentUserlogoBtn);
            currentElipseMaskWidget->setGeometry(0, 0, currentUserlogoBtn->width(), currentUserlogoBtn->height());

            //设置用户昵称
            if (setTextDynamic(currentNickNameLabel, user.realname)){
                currentNickNameLabel->setToolTip(user.realname);
            }

            //用户类型
            QString cType = _accountTypeIntToString(user.accounttype);
            if (setTextDynamic(currentUserTypeLabel, cType)){
                currentUserTypeLabel->setToolTip(cType);
            }

        } else {
            otherVerLayout->insertWidget(0, buildItemForOthers(user));
        }
    }
}

void UserInfo::showChangeUserTypeDialog(QString u){
    if (allUserInfoMap.keys().contains(u)){
        UserInfomation user = allUserInfoMap.value(u);

        ChangeUserType * dialog = new ChangeUserType(user.objpath);
        dialog->exec();

    } else {
        qDebug() << "User Data Error When Change User type";
    }
}


QFrame * UserInfo::createHLine(QFrame *f, int len){
    QFrame *line = new QFrame(f);
    if (len == 0){
        line->setMinimumSize(QSize(0, 1));
        line->setMaximumSize(QSize(16777215, 1));
    } else {
        line->setMinimumSize(QSize(len, 1));
        line->setMaximumSize(QSize(len, 1));
    }

    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

QFrame * UserInfo::createVLine(QFrame *f, int len){
    QFrame *line = new QFrame(f);
    if (len == 0){
        line->setMinimumSize(QSize(1, 0));
        line->setMaximumSize(QSize(1, 16777215));
    } else {
        line->setMinimumSize(QSize(1, len));
        line->setMaximumSize(QSize(1, len));
    }

    line->setLineWidth(0);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

QFrame * UserInfo::buildItemForOthers(UserInfomation user){

    QFrame * otherUserFrame = new QFrame();

    //设置头像
    QPushButton * otherUserLogoBtn = new QPushButton();
    otherUserLogoBtn->setFixedSize(QSize(56, 56));
    otherUserLogoBtn->setIcon(QIcon(user.iconfile));
    otherUserLogoBtn->setIconSize(QSize(48, 48));
    ////圆形头像
    ElipseMaskWidget * otherElipseMaskWidget = new ElipseMaskWidget(otherUserLogoBtn);
    otherElipseMaskWidget->setGeometry(0, 0, otherUserLogoBtn->width(), otherUserLogoBtn->height());

    QPushButton * changeOtherPwdBtn = new QPushButton();
    changeOtherPwdBtn->setFlat(true);
    changeOtherPwdBtn->setText(tr("Passwd"));
    QPushButton * changeOtherTypeBtn = new QPushButton();
    changeOtherTypeBtn->setFlat(true);
    changeOtherTypeBtn->setText(tr("Type"));
    QPushButton * deleteOtherBtn = new QPushButton();
    deleteOtherBtn->setFlat(true);
    deleteOtherBtn->setText(tr("Del"));

    QLabel * otherNickNameLabel = new QLabel();
    otherNickNameLabel->setFixedHeight(20);
    if (setTextDynamic(otherNickNameLabel, user.realname)){
        otherNickNameLabel->setToolTip(user.realname);
    }
    QLabel * otherUserTypeLable = new QLabel();
    otherUserTypeLable->setFixedHeight(20);
    QString otype = _accountTypeIntToString(user.accounttype);
    if (setTextDynamic(otherUserTypeLable, otype)){
        otherUserTypeLable->setToolTip(otype);
    }


    QVBoxLayout * otherUserinfoVerLayout = new QVBoxLayout();
    otherUserinfoVerLayout->setSpacing(4);
    otherUserinfoVerLayout->setContentsMargins(0, 0, 0, 0);
    otherUserinfoVerLayout->addStretch();
    otherUserinfoVerLayout->addWidget(otherNickNameLabel, Qt::AlignHCenter);
    otherUserinfoVerLayout->addWidget(otherUserTypeLable, Qt::AlignHCenter);
    otherUserinfoVerLayout->addStretch();

    QFrame * splitDynamicVLine1 = createVLine(otherUserFrame, 10);
    QFrame * splitDynamicVLine2 = createVLine(otherUserFrame, 10);
    QFrame * splitDynamicHLine1 = createHLine(otherUserFrame);

    QHBoxLayout * otherUserHorLayout = new QHBoxLayout();
    otherUserHorLayout->setSpacing(16);
    otherUserHorLayout->setContentsMargins(16, 0, 16, 0);
    otherUserHorLayout->addWidget(otherUserLogoBtn);
    otherUserHorLayout->addLayout(otherUserinfoVerLayout);
    otherUserHorLayout->addStretch();
    otherUserHorLayout->addWidget(changeOtherPwdBtn);
    otherUserHorLayout->addWidget(splitDynamicVLine1);
    otherUserHorLayout->addWidget(changeOtherTypeBtn);
    otherUserHorLayout->addWidget(splitDynamicVLine2);
    otherUserHorLayout->addWidget(deleteOtherBtn);

    QVBoxLayout * mOtherUserVerLayout = new QVBoxLayout();
    mOtherUserVerLayout->setSpacing(0);
    mOtherUserVerLayout->setContentsMargins(0, 0, 0, 0);
    mOtherUserVerLayout->addLayout(otherUserHorLayout);
    mOtherUserVerLayout->addWidget(splitDynamicHLine1);

    otherUserFrame->setMinimumSize(QSize(550, 60));
    otherUserFrame->setMaximumSize(QSize(16777215, 60));
    otherUserFrame->setFrameShape(QFrame::NoFrame);
    otherUserFrame->setLayout(mOtherUserVerLayout);


    return otherUserFrame;
}

bool UserInfo::setTextDynamic(QLabel *label, QString string){

    bool isOverLength = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);

    QString str = string;
    if (fontSize > 100) {
        label->setFixedWidth(100);
        str = fontMetrics.elidedText(string, Qt::ElideRight, 100);
        isOverLength = true;
    } else {
        label->setFixedWidth(fontSize);
    }
    label->setText(str);
    return isOverLength;

}

void UserInfo::setUserConnect(){

    connect(changeCurrentTypeBtn, &QPushButton::clicked, [=]{
        showChangeUserTypeDialog(QString(g_get_user_name()));
    });
}

void UserInfo::setUserDBusPropertyConnect(const QString pObjPath){
    QDBusInterface iproperty("org.freedesktop.Accounts",
                             pObjPath,
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());

    iproperty.connection().connect("org.freedesktop.Accounts", pObjPath, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                    this, SLOT(userPropertyChangedSlot(QString, QMap<QString, QVariant>, QStringList)));
}


void UserInfo::userPropertyChangedSlot(QString property, QMap<QString, QVariant> propertyMap, QStringList propertyList){
    Q_UNUSED(property);
    Q_UNUSED(propertyList);


}


/**3.1****end**************/


QString UserInfo::_accountTypeIntToString(int type){
    QString atype;
    if (type == STANDARDUSER)
        atype = tr("Standard");
    else if (type == ADMINISTRATOR)
        atype = tr("Admin");
    else if (type == ROOT)
        atype = tr("root");

    return atype;
}

void UserInfo::_acquireAllUsersInfo(){

    mUserName = qgetenv("USER");
    if (mUserName.isEmpty()) {
        mUserName = qgetenv("USERNAME");
    }

    QStringList objectpaths = sysdispatcher->list_cached_users();

    //初始化用户信息QMap
    allUserInfoMap.clear();
    //初始化管理员数目为0
//    adminnum = 0;

    //root
    if (!getuid()){
        UserInfomation root;
        root.username = g_get_user_name();
        root.realname = g_get_real_name();
        root.current = true;
        root.logined = true;
        root.autologin = false;
        root.uid = 0;
        root.accounttype = ADMINISTRATOR;
        root.iconfile = DEFAULTFACE;
        allUserInfoMap.insert(root.username, root);
    }

    for (QString objectpath : objectpaths){
        UserInfomation user;
        user = _acquireUserInfo(objectpath);
        allUserInfoMap.insert(user.username, user);
    }

    if (allUserInfoMap.isEmpty()) {
        ui->currentUserFrame->setVisible(false);
        ui->autoLoginFrame->setVisible(false);
    } else {
        ui->currentUserFrame->setVisible(true);
        ui->autoLoginFrame->setVisible(true);
    }
    initUserPropertyConnection(objectpaths);
}

int UserInfo::_userCanDel(QString user){
    QString cmd = QString("cat /etc/group | grep sudo | awk -F: '{ print $NF}'");
    QString output;

    FILE   *stream;
    char buf[256];

    if ((stream = popen(cmd.toLatin1().data(), "r" )) == NULL){
        return -1;
    }

    while(fgets(buf, 256, stream) != NULL){
        output = QString(buf).simplified();
    }

    pclose(stream);

    QStringList users = output.split(",");
    int num = users.length();

    if (users.contains(user)){
        if (num > 1){
            return 1;
        } else {
            return 0;
        }
    } else {
        return 1;
    }
}

UserInfomation UserInfo::_acquireUserInfo(QString objpath){
    UserInfomation user;

    //默认值
    user.current = false;
    user.logined = false;
    user.autologin = false;

    QDBusInterface * iproperty = new QDBusInterface("org.freedesktop.Accounts",
                                            objpath,
                                            "org.freedesktop.DBus.Properties",
                                            QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply = iproperty->call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()){
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        user.username = propertyMap.find("UserName").value().toString();
        user.realname = propertyMap.find("RealName").value().toString();

        if (user.realname.isEmpty()){
            user.realname = propertyMap.find("UserName").value().toString();
        }

        if (user.username == QString(g_get_user_name())) {
            user.current = true;
            user.logined = true;
            user.noPwdLogin = getNoPwdStatus();
        }
        user.accounttype = propertyMap.find("AccountType").value().toInt();
        user.iconfile = propertyMap.find("IconFile").value().toString();
        user.passwdtype = propertyMap.find("PasswordMode").value().toInt();
        user.uid = propertyMap.find("Uid").value().toInt();
        user.autologin = getAutomaticLogin().contains(user.username, Qt::CaseSensitive);
        user.objpath = objpath;
    }
    else
        qDebug() << "reply failed";

    delete iproperty;
    iproperty = nullptr;

    return user;
}

void UserInfo::readCurrentPwdConf(){
#ifdef ENABLEPQ
    int ret, status;
    void *auxerror;
    char buf[255];

    pwdMsg = "";

    pwdconf = pwquality_default_settings();
    if (pwdconf == NULL) {
        enablePwdQuality = false;
        qDebug() << "init pwquality settings failed";
    } else {
        enablePwdQuality = true;
    }

    ret = pwquality_read_config(pwdconf, PWCONF, &auxerror);
    if (ret != 0){
        enablePwdQuality = false;
        qDebug() << "Reading pwquality configuration file failed: " << pwquality_strerror(buf, sizeof(buf), ret, auxerror);
    } else {
        enablePwdQuality = true;
    }

    if (PasswdCheckUtil::getCurrentPamState()){
        enablePwdQuality = true;
    } else {
        enablePwdQuality = false;
    }

    if (enablePwdQuality){
        int minLen;
        status = pwquality_get_int_value(pwdconf, PWQ_SETTING_MIN_LENGTH, &minLen);
        if (!status && minLen > 0){
            pwdOption.min_length = minLen;
            pwdMsg += QObject::tr("min length %1\n").arg(minLen);

        } else {
            pwdMsg += "";
        }

        int digCredit;
        status = pwquality_get_int_value(pwdconf, PWQ_SETTING_DIG_CREDIT, &digCredit);
        if (!status && digCredit > 0){
            pwdOption.dig_credit = digCredit;
            pwdMsg += QObject::tr("min digit num %1\n").arg(digCredit);
        } else {
            pwdMsg += "";
        }

        int upCredit;
        status = pwquality_get_int_value(pwdconf, PWQ_SETTING_UP_CREDIT, &upCredit);
        if (!status && upCredit > 0){
            pwdOption.up_credit = upCredit;
            pwdMsg += QObject::tr("min upper num %1\n").arg(upCredit);
        } else {
            pwdMsg += "";
        }

        int lowCredit;
        status = pwquality_get_int_value(pwdconf, PWQ_SETTING_LOW_CREDIT, &lowCredit);
        if (!status && lowCredit > 0){
            pwdOption.low_credit = lowCredit;
            pwdMsg += QObject::tr("min lower num %1\n").arg(lowCredit);
        } else {
            pwdMsg += "";
        }

        int othCredit;
        status = pwquality_get_int_value(pwdconf, PWQ_SETTING_OTH_CREDIT, &othCredit);
        if (!status && othCredit > 0){
            pwdOption.oth_credit = othCredit;
            pwdMsg += QObject::tr("min other num %1\n").arg(othCredit);
        } else {
            pwdMsg += "";
        }


        int minClass;
        status = pwquality_get_int_value(pwdconf, PWQ_SETTING_MIN_CLASS, &minClass);
        if (!status && minClass > 0){
            pwdOption.min_class = minClass;
            pwdMsg += QObject::tr("min char class %1\n").arg(minClass);
        } else {
            pwdMsg += "";
        }

        int maxRepeat;
        status = pwquality_get_int_value(pwdconf, PWQ_SETTING_MAX_REPEAT, &maxRepeat);
        if (!status && maxRepeat > 0){
            pwdOption.max_repeat = maxRepeat;
            pwdMsg += QObject::tr("max repeat %1\n").arg(maxRepeat);
        } else {
            pwdMsg += "";
        }

        int maxClassRepeat;
        status = pwquality_get_int_value(pwdconf, PWQ_SETTING_MAX_CLASS_REPEAT, &maxClassRepeat);
        if (!status && maxClassRepeat > 0){
            pwdOption.max_class_repeat = maxClassRepeat;
            pwdMsg += QObject::tr("max class repeat %1\n").arg(maxClassRepeat);
        } else {
            pwdMsg += "";
        }

        int maxSequence;
        status = pwquality_get_int_value(pwdconf, PWQ_SETTING_MAX_SEQUENCE, &maxSequence);
        if (!status && maxSequence > 0){
            pwdOption.max_class_repeat = maxSequence;
            pwdMsg += QObject::tr("max sequence %1\n").arg(maxSequence);
        } else {
            pwdMsg += "";
        }
    }

    qDebug() << "pwquality:" << pwdOption.min_length << pwdOption.min_class << pwdOption.dig_credit << pwdOption.low_credit << pwdOption.up_credit;
    qDebug() << "pwquality msg:" << pwdMsg;
#endif
}

void UserInfo::initComponent(){

    //root需要屏蔽部分功能
    if (!getuid()){
        ui->changeTypeBtn->setEnabled(false);
//        ui->changeGroupBtn->setEnabled(false);
        ui->autoLoginFrame->setVisible(false);
        ui->noPwdLoginFrame->setVisible(false);
    }

#ifdef WITHKYSEC
    if (!kysec_is_disabled() && kysec_get_3adm_status() && (getuid() || geteuid())){
        ui->addUserWidget->hide();
    }
#endif

    ui->listWidget->setStyleSheet("QListWidget::Item:hover{background:palette(base);}");

    addWgt = new HoverWidget("");
    addWgt->setObjectName("addwgt");
    addWgt->setMinimumSize(QSize(580, 50));
    addWgt->setMaximumSize(QSize(960, 50));
    QPalette pal;
    QBrush brush = pal.highlight();  //获取window的色值
    QColor highLightColor = brush.color();
    QString stringColor = QString("rgba(%1,%2,%3)") //叠加20%白色
           .arg(highLightColor.red()*0.8 + 255*0.2)
           .arg(highLightColor.green()*0.8 + 255*0.2)
           .arg(highLightColor.blue()*0.8 + 255*0.2);

    addWgt->setStyleSheet(QString("HoverWidget#addwgt{background: palette(button);\
                                   border-radius: 4px;}\
                                   HoverWidget:hover:!pressed#addwgt{background: %1;\
                                   border-radius: 4px;}").arg(stringColor));
    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel * iconLabel = new QLabel();
    QLabel * textLabel = new QLabel(tr("Add new user"));
    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
    iconLabel->setPixmap(pixgray);
    iconLabel->setProperty("useIconHighlightEffect", true);
    iconLabel->setProperty("iconHighlightEffectMode", 1);
    addLyt->addWidget(iconLabel);
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    addWgt->setLayout(addLyt);

    connect(addWgt, &HoverWidget::widgetClicked, this, [=](QString mname) {
        Q_UNUSED(mname);
        showCreateUserDialog();
    });

    addUserTmpBtn = new AddBtn;
    ui->horizontalLayout_3->addWidget(addUserTmpBtn);
    connect(addUserTmpBtn, &AddBtn::clicked, this, [=]{
        showCreateUserDialog();
    });

    // 悬浮改变Widget状态
    connect(addWgt, &HoverWidget::enterWidget, this, [=](){

        iconLabel->setProperty("useIconHighlightEffect", false);
        iconLabel->setProperty("iconHighlightEffectMode", 0);
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
        iconLabel->setPixmap(pixgray);
        textLabel->setStyleSheet("color: white;");
    });

    // 还原状态
    connect(addWgt, &HoverWidget::leaveWidget, this, [=](){

        iconLabel->setProperty("useIconHighlightEffect", true);
        iconLabel->setProperty("iconHighlightEffectMode", 1);
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
        iconLabel->setPixmap(pixgray);
        textLabel->setStyleSheet("color: palette(windowText);");
    });

    ui->nopwdHorLayout->setSpacing(0);
    ui->nopwdHorLayout->setMargin(0);

    nopwdSwitchBtn = new SwitchButton(ui->noPwdLoginFrame);
    ui->nopwdHorLayout->addWidget(nopwdSwitchBtn);

    autoLoginSwitchBtn = new SwitchButton(ui->autoLoginFrame);
    ui->autoLoginHorLayout->addWidget(autoLoginSwitchBtn);

    ui->listWidget->setSpacing(0);

    ElipseMaskWidget * mainElipseMaskWidget = new ElipseMaskWidget(ui->currentUserFaceLabel);
    mainElipseMaskWidget->setGeometry(0, 0, ui->currentUserFaceLabel->width(), ui->currentUserFaceLabel->height());

    if (getuid()){
        ui->currentUserFaceLabel->installEventFilter(this);
        ui->nameChangeWidget->installEventFilter(this);
    }

    //修改当前用户密码的回调
    connect(ui->changePwdBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        UserInfomation user = allUserInfoMap.value(g_get_user_name());

        showChangePwdDialog(user.username);
    });

    //修改当前用户类型的回调
    if (getuid())
        connect(ui->changeTypeBtn, &QPushButton::clicked, this, [=](bool checked){
            Q_UNUSED(checked)
            UserInfomation user = allUserInfoMap.value(g_get_user_name());

            showChangeTypeDialog(user.username);
        });

    connect(ui->changeGroupBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        showChangeGroupDialog();
    });

    //修改当前用户免密登录
    if (getuid())
        connect(nopwdSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked){

            UserInfomation user = allUserInfoMap.value(g_get_user_name());
            //免密登录状态改变

            QDBusInterface * tmpSysinterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                                                  "/",
                                                                  "com.control.center.interface",
                                                                  QDBusConnection::systemBus());

            if (!tmpSysinterface->isValid()){
                qCritical() << "Create Client Interface Failed When execute gpasswd: " << QDBusConnection::systemBus().lastError();
                return;
            }
            tmpSysinterface->call("setNoPwdLoginStatus", checked, user.username);

            delete tmpSysinterface;
            tmpSysinterface = nullptr;

        });

    //修改当前用户自动登录
    if (getuid())
        connect(autoLoginSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked) {

            UserInfomation user = allUserInfoMap.value(g_get_user_name());

            UserDispatcher * userdispatcher  = new UserDispatcher(user.objpath);

            bool status = getAutomaticLogin().contains(user.username, Qt::CaseSensitive);

            if (checked && !isOpenAutoLogin(user.username)) {
                autoLoginSwitchBtn->blockSignals(true);
                autoLoginSwitchBtn->setChecked(false);
                autoLoginSwitchBtn->blockSignals(false);
                return ;
            }

            if ((checked != status)) {
                if (checked) {
                    userdispatcher->change_user_autologin(user.username);
                } else {
                    userdispatcher->change_user_autologin("");
                }
            }
        });

    //成功删除用户的回调
    connect(sysdispatcher, &SystemDbusDispatcher::deleteuserdone, this, [=](QString objPath){
        deleteUserDone(objPath);
    });

    //新建用户的回调
//    connect(ui->addBtn, &QPushButton::clicked, this, [=](bool checked){
//        Q_UNUSED(checked)
//        showCreateUserDialog();
//    });

    //成功新建用户的回调
    connect(sysdispatcher, &SystemDbusDispatcher::createuserdone, this, [=](QString objPath){
        createUserDone(objPath);
    });

}

void UserInfo::_resetListWidgetHeigh(){
    //设置其他用户控件的总高度
    ui->listWidget->setFixedHeight((allUserInfoMap.count() - 1 ) * (ITEMHEIGH) + 4);
}

void UserInfo::initAllUserStatus(){
    _resetListWidgetHeigh();

    //每次初始化用户状态清空其他用户QMap
    otherUserItemMap.clear();

    QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
    for (; it != allUserInfoMap.end(); it++){
        UserInfomation user = it.value();

        //当前用户
        if (user.username == QString(g_get_user_name())){

        } else { //其他用户
            _buildWidgetForItem(user);

        }
    }
}

QStringList UserInfo::getLoginedUsers() {
    m_loginedUser.clear();
    qRegisterMetaType<LoginedUsers>("LoginedUsers");
    qDBusRegisterMetaType<LoginedUsers>();
    QDBusInterface loginInterface("org.freedesktop.login1",
                                  "/org/freedesktop/login1",
                                  "org.freedesktop.login1.Manager",
                                  QDBusConnection::systemBus());

    if (loginInterface.isValid()) {
//        qDebug() << "create interface sucess";
    }

    QDBusMessage result = loginInterface.call("ListUsers");
    QList<QVariant> outArgs = result.arguments();
    QVariant first = outArgs.at(0);
    QDBusArgument dbvFirst = first.value<QDBusArgument>();
    QVariant vFirst = dbvFirst.asVariant();
    const QDBusArgument &dbusArgs = vFirst.value<QDBusArgument>();

    QVector<LoginedUsers> loginedUsers;

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        LoginedUsers user;
        dbusArgs >> user;
        loginedUsers.push_back(user);
    }
    dbusArgs.endArray();

    for (LoginedUsers user : loginedUsers) {

        QDBusInterface userPertyInterface("org.freedesktop.login1",
                                          user.objpath.path(),
                                          "org.freedesktop.DBus.Properties",
                                          QDBusConnection::systemBus());

        QDBusReply<QVariant> reply = userPertyInterface.call("Get", "org.freedesktop.login1.User", "State");
        if (reply.isValid()) {
            QString status = reply.value().toString();
            if ("closing" != status) {
                m_loginedUser.append(user.userName);
            }
        }
    }
    return m_loginedUser;
}

void UserInfo::_refreshUserInfoUI(){
    QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
    for (; it != allUserInfoMap.end(); it++){
        UserInfomation user = it.value();

        //用户头像为.face且.face文件不存在
        char * iconpath = user.iconfile.toLatin1().data();
        if (!g_file_test(iconpath, G_FILE_TEST_EXISTS)){
            user.iconfile = DEFAULTFACE;
            //更新用户数据
            allUserInfoMap.find(it.key()).value().iconfile = DEFAULTFACE;
        }

        //当前用户
        if (user.username == QString(g_get_user_name())){
            //设置用户头像
            QPixmap iconPixmap = QPixmap(user.iconfile).scaled(ui->currentUserFaceLabel->size());
            ui->currentUserFaceLabel->setPixmap(iconPixmap);

            //设置用户名
            ui->userNameLabel->setText(user.realname);
            ui->userNameChangeLabel->setProperty("useIconHighlightEffect", 0x8);
            ui->userNameChangeLabel->setPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(ui->userNameChangeLabel->size()));
            //设置用户类型
            ui->userTypeLabel->setText(_accountTypeIntToString(user.accounttype));
            //设置登录状态
            autoLoginSwitchBtn->blockSignals(true);
            autoLoginSwitchBtn->setChecked(user.autologin);
            autoLoginSwitchBtn->blockSignals(false);
            //设置免密登录状态
            nopwdSwitchBtn->setChecked(user.noPwdLogin);

        } else { //其他用户
            QListWidgetItem * item = otherUserItemMap.value(user.objpath); //是否需要判断？？

            QWidget * widget = ui->listWidget->itemWidget(item);

            QPushButton * faceBtn = widget->findChild<QPushButton *>("faceBtn");
            faceBtn->setIcon(QIcon(user.iconfile));

        }
    }
}

void UserInfo::_buildWidgetForItem(UserInfomation user){
    HoverWidget * baseWidget = new HoverWidget(user.username);
    baseWidget->setMinimumSize(550,50);
    baseWidget->setMaximumSize(960,50);
    baseWidget->setAttribute(Qt::WA_DeleteOnClose);

    //ui->currentUserFrame->setContentsMargins(16,0,16,0);

    QHBoxLayout * baseVerLayout = new QHBoxLayout(baseWidget);
    baseVerLayout->setSpacing(0);
    baseVerLayout->setMargin(0);

    QHBoxLayout * baseHorLayout = new QHBoxLayout();
    baseHorLayout->setSpacing(16);
    baseHorLayout->setMargin(0);

    QFrame * widget = new QFrame(baseWidget);
    widget->setFrameShape(QFrame::Shape::Box);
    widget->setFixedHeight(50);

    QHBoxLayout * mainHorLayout = new QHBoxLayout(widget);
    mainHorLayout->setSpacing(16);
    mainHorLayout->setContentsMargins(16, 0, 16, 0);

    QPushButton * faceBtn = new QPushButton(widget);
    faceBtn->setObjectName("faceBtn");
    faceBtn->setFixedSize(40, 40);
    faceBtn->setIcon(QIcon(user.iconfile));
    faceBtn->setIconSize(QSize(32, 32));
    connect(faceBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        showChangeFaceDialog(user.username);
    });

    ElipseMaskWidget * otherElipseMaskWidget = new ElipseMaskWidget(faceBtn);
    otherElipseMaskWidget->setGeometry(0, 0, faceBtn->width(), faceBtn->height());

    QLabel * nameLabel = new QLabel(widget);
    QSizePolicy nameSizePolicy = nameLabel->sizePolicy();
    nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    nameLabel->setSizePolicy(nameSizePolicy);
    nameLabel->setText(user.realname);

    QString btnQss = QString("QPushButton{background: #ffffff; border-radius: 4px;}");

    QPushButton * typeBtn = new QPushButton(widget);
//    typeBtn->setFixedSize(88, 36);
    typeBtn->setFixedHeight(36);
    typeBtn->setMinimumWidth(88);
    typeBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    typeBtn->setText(tr("Change type"));
//    typeBtn->setStyleSheet(btnQss);
    connect(typeBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        showChangeTypeDialog(user.username);
    });
    typeBtn->hide();

    QPushButton * pwdBtn = new QPushButton(widget);
//    pwdBtn->setFixedSize(88, 36);
    pwdBtn->setFixedHeight(36);
    pwdBtn->setMinimumWidth(88);
    pwdBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    pwdBtn->setText(tr("Change pwd"));
//    pwdBtn->setStyleSheet(btnQss);
    connect(pwdBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        showChangePwdDialog(user.username);
    });
    pwdBtn->hide();

    mainHorLayout->addWidget(faceBtn);
    mainHorLayout->addWidget(nameLabel);
    mainHorLayout->addStretch();
    mainHorLayout->addWidget(typeBtn);
    mainHorLayout->addWidget(pwdBtn);

    widget->setLayout(mainHorLayout);

    QPushButton * delBtn = new QPushButton(baseWidget);
    delBtn->setFixedSize(60, 36);
    delBtn->setText(tr("Del"));
//    delBtn->setStyleSheet("QPushButton{background: #FA6056; border-radius: 4px}");
    delBtn->hide();
    connect(delBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        showDeleteUserDialog(user.username);
    });

    connect(baseWidget, &HoverWidget::enterWidget, this, [=](QString name){

        //不允许删除最后一个管理员
        if (_userCanDel(name) == 1){
            delBtn->setEnabled(true);
        } else if (_userCanDel(name) == 0) {
            delBtn->setEnabled(false);
        }

#ifdef WITHKYSEC
        if (!kysec_is_disabled() && kysec_get_3adm_status()){
            if (user.username == "secadm" || user.username == "auditadm"){
                delBtn->setEnabled(false);
            }
        }
#endif

        typeBtn->show();
        pwdBtn->show();
        delBtn->show();
    });
    connect(baseWidget, &HoverWidget::leaveWidget, this, [=](QString name){
        Q_UNUSED(name)
        typeBtn->hide();
        pwdBtn->hide();
        delBtn->hide();
    });

    baseHorLayout->addWidget(widget);
    baseHorLayout->addWidget(delBtn, Qt::AlignVCenter);
    //baseHorLayout->addSpacing(4);

    baseVerLayout->addLayout(baseHorLayout);
//    baseVerLayout->addStretch();

    baseWidget->setLayout(baseVerLayout);

#ifdef WITHKYSEC
    if (!kysec_is_disabled() && kysec_get_3adm_status()){
        if (user.username == "secadm" || user.username == "auditadm"){
            pwdBtn->setEnabled(false);
            typeBtn->setEnabled(false);
        }
    }
#endif

    QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
//    item->setSizeHint(QSize(ui->listWidget->width() - 4, ITEMHEIGH));
    item->setSizeHint(QSize(QSizePolicy::Expanding, ITEMHEIGH));
    item->setData(Qt::UserRole, QVariant(user.objpath));
    ui->listWidget->setItemWidget(item, baseWidget);

    otherUserItemMap.insert(user.objpath, item);

}

void UserInfo::showCreateUserDialog(){
    //获取系统所有用户名列表，创建时判断重名
    QStringList usersStringList;
    for (QVariant tmp : allUserInfoMap.keys()){
        usersStringList << tmp.toString();
    }

    CreateUserDialog * dialog = new CreateUserDialog(usersStringList,pluginWidget);
    dialog->setRequireLabel(pwdMsg);
    connect(dialog, &CreateUserDialog::newUserWillCreate, this, [=](QString uName, QString pwd, QString pin, int aType){
        createUser(uName, pwd, pin, aType);
    });
    dialog->exec();
}

QStringList UserInfo::getUsersList()
{
    QStringList usersStringList;
    for (QVariant tmp : allUserInfoMap.keys()){
        usersStringList << tmp.toString();
    }
    return usersStringList;
}

void UserInfo::createUser(QString username, QString pwd, QString pin, int atype){
    Q_UNUSED(pin);
    sysdispatcher->create_user(username, "", atype);

    //使用全局变量传递新建用户密码
    _newUserPwd = pwd;
    _newUserName = username;
}

void UserInfo::createUserDone(QString objpath){
    UserDispatcher * userdispatcher  = new UserDispatcher(objpath);
    //设置默认头像
    userdispatcher->change_user_face(DEFAULTFACE);
    //设置默认密码
//    userdispatcher->change_user_pwd(_newUserPwd, "");
    QDBusInterface * tmpSysinterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                                          "/",
                                                          "com.control.center.interface",
                                                          QDBusConnection::systemBus());

    if (!tmpSysinterface->isValid()){
        qCritical() << "Create Client Interface Failed When : " << QDBusConnection::systemBus().lastError();
        return;
    }
    tmpSysinterface->call("changeOtherUserPasswd", _newUserName, _newUserPwd);

    delete tmpSysinterface;
    tmpSysinterface = nullptr;

    //刷新全部用户信息
    _acquireAllUsersInfo();
    //重建其他用户ListWidget高度
    _resetListWidgetHeigh();

    //获取新建用户信息
    UserInfomation user;
    user = _acquireUserInfo(objpath);

    //构建Item
    _buildWidgetForItem(user);
}

void UserInfo::showDeleteUserDialog(QString username){

    QStringList loginedusers = getLoginedUsers();
    if (loginedusers.contains(username)) {
        QMessageBox::warning(pluginWidget, tr("Warning"), tr("The user is logged in, please delete the user after logging out"));
        return;
    }
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    DelUserDialog * dialog = new DelUserDialog(pluginWidget);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setUsername(user.username);
    connect(dialog, &DelUserDialog::removefile_send, this, [=](bool removeFile){
        deleteUser(removeFile, user.username);
    });
    dialog->exec();
}

void UserInfo::deleteUser(bool removefile, QString username){
//    qDebug() << allUserInfoMap.keys() << username;

    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    // hidden the item when click delete user button
//    QListWidgetItem *item =  otherUserItemMap.find(user.objpath).value();
//    ui->listWidget->setItemHidden(item, true);

    sysdispatcher->delete_user(user.uid, removefile);
}

void UserInfo::delete_user_slot(bool removefile, QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    sysdispatcher->delete_user(user.uid, removefile);
}

void UserInfo::pwdAndAutoChangedSlot(QString key) {
    if ("option" == key) {
        autoLoginSwitchBtn->blockSignals(true);
        autoLoginSwitchBtn->setChecked(getAutomaticLogin().contains(mUserName, Qt::CaseSensitive));
        autoLoginSwitchBtn->blockSignals(false);
        nopwdSwitchBtn->setChecked(getNoPwdStatus());
    } else if( "avatar" == key) {
        //重新获取全部用户QMap
        _acquireAllUsersInfo();

        //更新界面显示
        _refreshUserInfoUI();
    }
}

void UserInfo::propertyChangedSlot(QString property, QMap<QString, QVariant> propertyMap, QStringList propertyList) {
    Q_UNUSED(property);
    Q_UNUSED(propertyList);
    if (propertyMap.keys().contains("IconFile") && getuid() &&
            propertyMap.value("RealName").toString() == mUserName) {
        if (propertyMap.keys().contains("AccountType")) {
            int type = propertyMap.value("AccountType").toInt();
            ui->userTypeLabel->setText(_accountTypeIntToString(type));
        }
    }
}

void UserInfo::deleteUserDone(QString objpath){
    QListWidgetItem * item = otherUserItemMap.value(objpath);

    //删除Item
    ui->listWidget->takeItem(ui->listWidget->row(item));

    //更新其他用户QMap
    otherUserItemMap.remove(objpath);

    //更新所有用户信息
    _acquireAllUsersInfo();

    //重置其他用户ListWidget高度
    _resetListWidgetHeigh();
}

void UserInfo::showChangeGroupDialog(){
    ChangeGroupDialog * dialog = new ChangeGroupDialog(pluginWidget);
    dialog->exec();
}

void UserInfo::showChangeTypeDialog(QString username){
    if (allUserInfoMap.keys().contains(username)){
        UserInfomation user = allUserInfoMap.value(username);

        ChangeTypeDialog * dialog = new ChangeTypeDialog(pluginWidget);
        dialog->setFace(user.iconfile);
        dialog->setUsername(user.realname);
        dialog->setCurrentAccountTypeLabel(_accountTypeIntToString(user.accounttype));
        dialog->setCurrentAccountTypeBtn(user.accounttype);
        dialog->forbidenChange(_userCanDel(username));
//        connect(dialog, SIGNAL(type_send(int,QString,bool)), this, SLOT(change_accounttype_slot(int,QString,bool)));
        connect(dialog, &ChangeTypeDialog::type_send, this, [=](int atype){
            changeUserType(atype, username);
        });
        dialog->exec();

    } else {
        qDebug() << "User Data Error When Change User type";
    }
}

void UserInfo::changeUserType(int atype, QString username){
    UserInfomation user = allUserInfoMap.value(username);

    //构建dbus调度对象
    UserDispatcher * userdispatcher  = new UserDispatcher(user.objpath); //继承QObject不再删除

    //更改用户类型
    userdispatcher->change_user_type(atype);

    //重新获取全部用户QMap
    _acquireAllUsersInfo();

    //更新界面显示
    _refreshUserInfoUI();
}

void UserInfo::showChangeNameDialog(){
    QStringList usernames;
    QStringList realnames;
    QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
    for (; it != allUserInfoMap.end(); it++){
        UserInfomation user = it.value();

        usernames.append(user.username);
        realnames.append(user.realname);
    }

    ChangeUserName * dialog = new ChangeUserName(usernames, realnames);
    connect(dialog, &ChangeUserName::sendNewName, [=](QString name){
        changeUserName(name);
    });
    dialog->exec();
}

void UserInfo::changeUserName(QString newName){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(QString(g_get_user_name())).value());

    UserDispatcher * userdispatcher  = new UserDispatcher(user.objpath);

    userdispatcher->change_user_name(newName);

    //重新获取全部用户QMap
    _acquireAllUsersInfo();

    //更新界面显示
    _refreshUserInfoUI();
}


void UserInfo::showChangeFaceDialog(QString username){
    if (allUserInfoMap.keys().contains(username)){
        UserInfomation user = allUserInfoMap.value(username);

        ChangeFaceDialog * dialog = new ChangeFaceDialog(pluginWidget);
        dialog->setFace(user.iconfile);
        dialog->setUsername(user.realname);
        dialog->setAccountType(_accountTypeIntToString(user.accounttype));
    //    dialog->set_face_list_status(user.iconfile);
        connect(dialog, &ChangeFaceDialog::face_file_send, [=](QString faceFile){
            changeUserFace(faceFile, user.username);
        });
        dialog->exec();
    } else {
        qDebug() << "User Data Error When Change User Face!";
    }

}

void UserInfo::changeUserFace(QString facefile, QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    UserDispatcher * userdispatcher  = new UserDispatcher(user.objpath);
    userdispatcher->change_user_face(facefile);

    //拷贝设置的头像文件到~/.face
    sysinterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                     "/",
                                     "com.control.center.interface",
                                     QDBusConnection::systemBus());

    if (!sysinterface->isValid()){
        qCritical() << "Create Client Interface Failed When Copy Face File: " << QDBusConnection::systemBus().lastError();
        return;
    }

    QString cmd = QString("cp %1 /home/%2/.face").arg(facefile).arg(user.username);

    QProcess::execute(cmd);

    //重新获取全部用户QMap
    _acquireAllUsersInfo();

    //更新界面显示
    _refreshUserInfoUI();
}

void UserInfo::showChangePwdDialog(QString username){

    if (allUserInfoMap.keys().contains(username)){
        UserInfomation user = allUserInfoMap.value(username);
        ChangePwdDialog * dialog = new ChangePwdDialog(user.current, user.username,pluginWidget);
        if (!getuid() || !user.current)
            dialog->haveCurrentPwdEdit(false);

        connect(dialog, &ChangePwdDialog::passwd_send, this, [=](QString pwd){

                changeUserPwd(pwd, username);

        });
        connect(dialog, &ChangePwdDialog::passwd_send2, this, [=](QString pwd){

            PolkitQt1::Authority::Result result;

            result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                        "org.control.center.qt.systemdbus.action",
                        PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
                        PolkitQt1::Authority::AllowUserInteraction);

            if (result == PolkitQt1::Authority::Yes){
                changeUserPwd(pwd, username);
            }


        });
        dialog->exec();

    } else {
        qDebug() << "User Info Data Error When Change User type";
    }
}


void UserInfo::changeUserPwd(QString pwd, QString username){
    //上层已做判断，这里不去判断而直接获取
    UserInfomation user = allUserInfoMap.value(username);

//    UserDispatcher * userdispatcher  = new UserDispatcher(user.objpath); //继承QObject不再删除
//    QString result = userdispatcher->change_user_pwd(pwd, "");

    QDBusInterface * tmpSysinterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                                          "/",
                                                          "com.control.center.interface",
                                                          QDBusConnection::systemBus());

    if (!tmpSysinterface->isValid()){
        qCritical() << "Create Client Interface Failed When : " << QDBusConnection::systemBus().lastError();
        return;
    }
    tmpSysinterface->call("changeOtherUserPasswd", username, pwd);

    delete tmpSysinterface;
    tmpSysinterface = nullptr;
}


bool UserInfo::eventFilter(QObject *watched, QEvent *event){
    if (watched == ui->currentUserFaceLabel){
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton ){
                if(watched == ui->currentUserFaceLabel){
                    showChangeFaceDialog(g_get_user_name());
                }
                return true;
            } else {
                return false;
            }
        }
    } else if (watched == ui->nameChangeWidget){
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton ){
                showChangeNameDialog();
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

QString UserInfo::getAutomaticLogin() {

    QString filename = "/etc/lightdm/lightdm.conf";
    autoSettings = new QSettings(filename, QSettings::IniFormat);
    autoSettings->beginGroup("SeatDefaults");

    QString autoUser = autoSettings->value("autologin-user", "").toString();

    autoSettings->endGroup();

    return autoUser;
}

bool UserInfo::getNoPwdStatus() {
    // 获取当前用户免密登录属性
    QDBusInterface tmpSysinterface("com.control.center.qt.systemdbus",
                                     "/",
                                     "com.control.center.interface",
                                     QDBusConnection::systemBus());
    // 获取免密登录状态
    QDBusReply<QString> noPwdres;
    noPwdres  = tmpSysinterface.call("getNoPwdLoginStatus");
    if (!noPwdres.isValid()) {
        qDebug() << noPwdres.error();
    }
    return (noPwdres.value().contains(mUserName) ? true : false);

}

bool UserInfo::isOpenAutoLogin(const QString &userName) {
    QString autoLoginedUser = this->getAutomaticLogin();
    bool res = true;
    int  ret;
    if (!autoLoginedUser.isEmpty() && userName != autoLoginedUser) {
        QMessageBox msg(this->pluginWidget);
        msg.setWindowTitle(tr("Hint"));
        msg.setText(tr("The system only allows one user to log in automatically."
                       "After it is turned on, the automatic login of other users will be turned off."
                       "Is it turned on?"));
        msg.addButton(tr("Trun on"), QMessageBox::AcceptRole);
        msg.addButton(tr("Close on"), QMessageBox::RejectRole);

        ret = msg.exec();

        switch (ret) {
          case QMessageBox::AcceptRole:
              res = true;
              break;
          case QMessageBox::RejectRole:
              res = false;
              break;
        }
    }
    return res;
}

void UserInfo::initUserPropertyConnection(const QStringList &objPath) {

    foreach (QString userPath, objPath) {
        QDBusInterface iproperty("org.freedesktop.Accounts",
                                 userPath,
                                 "org.freedesktop.DBus.Properties",
                                 QDBusConnection::systemBus());

        iproperty.connection().connect("org.freedesktop.Accounts", userPath, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                        this, SLOT(propertyChangedSlot(QString, QMap<QString, QVariant>, QStringList)));
    }

    QDBusConnection::sessionBus().connect(QString(), QString("/org/kylinssoclient/path"), "org.freedesktop.kylinssoclient.interface", "keyChanged", this, SLOT(pwdAndAutoChangedSlot(QString)));
}
