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


#include <ukcc/widgets/imageutil.h>
#include "elipsemaskwidget.h"
#include "passwdcheckutil.h"
#include "loginedusers.h"
#include "../../../shell/utils/utils.h"
/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <gio/gio.h>

#ifdef WITHKYSEC
#include <kysec/libkysec.h>
#include <kysec/status.h>
#endif
#define DEFAULTFACECOMMUNITY "/usr/share/ukui/faces/01-default-community.png"
#define DEFAULTFACECOMMERCIAL "/usr/share/ukui/faces/01-default-commercial.png"
#define DEFAULTFACE (Utils::isCommunity())?DEFAULTFACECOMMUNITY:DEFAULTFACECOMMERCIAL
#define ITEMHEIGH 60

UserInfo::UserInfo() : mFirstLoad(true)
{
    pluginName = QObject::tr("User Info");
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

QString UserInfo::plugini18nName() {
    return pluginName;
}

int UserInfo::pluginTypes() {
    return pluginType;
}

QWidget *UserInfo::pluginUi() {
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

        /**/
        pluginWidget2 = new QWidget;
        pluginWidget2->setAttribute(Qt::WA_DeleteOnClose);
        initUI();
        initSearchText();

        buildAndSetupUsers();
        setUserConnect();
        /**/


    }
    return pluginWidget2;
}

const QString UserInfo::name() const {

    return QStringLiteral("Userinfo");
}

bool UserInfo::isShowOnHomePage() const
{
    return !isIntel();
}

QIcon UserInfo::icon() const
{
    return QIcon::fromTheme("user-available-symbolic");
}

bool UserInfo::isEnable() const
{
    return !isIntel();
}

void UserInfo::initSearchText() {
    //~ contents_path /Userinfo/Password
    ui->changePwdBtn->setText(tr("Password"));
    //~ contents_path /Userinfo/Type
    ui->changeTypeBtn->setText(tr("Type"));
    //~ contents_path /Userinfo/Login no passwd
    ui->loginpwdLabel->setText(tr("Login no passwd"));
    //~ contents_path /Userinfo/enable autoLogin
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
    currentNickNameChangeLabel = new QLabel();
    currentNickNameChangeLabel->setFixedSize(QSize(15, 22));
    currentNickNameChangeLabel->setProperty("useIconHighlightEffect", 0x8);
    currentNickNameChangeLabel->setPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(currentNickNameChangeLabel->size()));
    currentUserTypeLabel = new QLabel();
    currentUserTypeLabel->setFixedHeight(20);
    currentUserTypeLabel->setFixedWidth(100);

    currentNickNameHorLayout = new QHBoxLayout();
    currentNickNameHorLayout->setSpacing(0);
    currentNickNameHorLayout->setContentsMargins(0, 0, 0, 0);
    currentNickNameHorLayout->addWidget(currentNickNameLabel);
    currentNickNameHorLayout->addWidget(currentNickNameChangeLabel);
    currentNickNameHorLayout->addStretch();

    currentUserinfoVerLayout = new QVBoxLayout();
    currentUserinfoVerLayout->setSpacing(4);
    currentUserinfoVerLayout->setContentsMargins(0, 0, 0, 0);
    currentUserinfoVerLayout->addStretch();
    currentUserinfoVerLayout->addLayout(currentNickNameHorLayout);
    currentUserinfoVerLayout->addWidget(currentUserTypeLabel, Qt::AlignHCenter);
    currentUserinfoVerLayout->addStretch();

    currentUserHorLayout = new QHBoxLayout();
    currentUserHorLayout->setSpacing(8);
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
    nopwdLoginLabel->setText(tr("LoginWithoutPwd"));

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
    autoLoginLabel->setText(tr("AutoLoginOnBoot"));
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
    addUserFrame->setObjectName("continue");
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
    othersFrame->setFixedHeight(60);
    othersFrame->setFrameShape(QFrame::Box);
    othersFrame->setLayout(otherVerLayout);

    //界面主布局
    mainVerLayout = new QVBoxLayout(pluginWidget2);
    mainVerLayout->setSpacing(8);
    mainVerLayout->setContentsMargins(0, 0, 0, 0);

    mainVerLayout->addWidget(currentLabel);
    mainVerLayout->addWidget(currentFrame);

    mainVerLayout->addSpacing(40);

    mainVerLayout->addWidget(othersLabel);
    mainVerLayout->addWidget(othersFrame);

    mainVerLayout->addStretch();


    pluginWidget2->setLayout(mainVerLayout);

#ifdef WITHKYSEC
    if (!kysec_is_disabled() && kysec_get_3adm_status() && (getuid() || geteuid())){
        addUserBtn->setEnabled(false);
    }
#endif

    //root需要屏蔽部分功能
    if (!getuid()){
        currentNickNameChangeLabel->setEnabled(false);
        currentNickNameLabel->setEnabled(false);
        changeCurrentPwdBtn->setEnabled(false);
        changeCurrentTypeBtn->setEnabled(false);
        currentUserlogoBtn->setEnabled(false);
        autoLoginFrame->hide();
        nopwdLoginFrame->hide();
    }

}

void UserInfo::buildAndSetupUsers(){

    QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
    for (; it != allUserInfoMap.end(); it++){
        UserInfomation user = it.value();

        //当前用户
        if (user.username == QString(g_get_user_name())){

            //设置用户头像
            QPixmap iconfile = makeRoundLogo(user.iconfile, currentUserlogoBtn->width(), currentUserlogoBtn->height(), currentUserlogoBtn->width()/2);
            currentUserlogoBtn->setIcon(iconfile);
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
            fontSizeChange(user, nullptr);
            if (user.accounttype && getuid()){
                changeCurrentTypeBtn->setEnabled(!isLastAdmin(user.username));
            }

            connect(this, &UserInfo::userTypeChanged, [=](QString n){
                if (user.accounttype){
                    changeCurrentTypeBtn->setEnabled(!isLastAdmin(user.username));
                } else {
                    changeCurrentTypeBtn->setEnabled(true);
                }
            });

            QProcess *process = new QProcess;
            process->start("dpkg -l | grep kim-client");
            process->waitForFinished();

            QByteArray ba = process->readAllStandardOutput();
            delete process;
            QString mOutput = QString(ba.data());
            // 域用户用户信息不可设置
            if (isDomainUser(user.username.toLatin1().data())) {
                // 工行修改密码按钮置灰
                if (mOutput.contains("icbc")) {
                    changeCurrentPwdBtn->setEnabled(false);
                }
                currentNickNameChangeLabel->setEnabled(false);
                currentNickNameLabel->setEnabled(false);   
                changeCurrentGroupsBtn->setEnabled(false);
                changeCurrentTypeBtn->setEnabled(false);
                nopwdLoginSBtn->setEnabled(false);
                autoLoginSBtn->setEnabled(false);
                addUserBtn->setEnabled(false);
            }

            //设置自动登录状态
            autoLoginSBtn->blockSignals(true);
            autoLoginSBtn->setChecked(user.autologin);
            autoLoginSBtn->blockSignals(false);

            //设置免密登录状态
            nopwdLoginSBtn->blockSignals(true);
            nopwdLoginSBtn->setChecked(user.noPwdLogin);
            nopwdLoginSBtn->blockSignals(false);

            //绑定当前用户的属性改变回调
            setUserDBusPropertyConnect(user.objpath);

        } else {

            buildItemForUsersAndSetConnect(user);
        }
    }

    QDBusConnection::systemBus().connect(QString(), QString(), "org.freedesktop.Accounts", "UserAdded", this, SLOT(newUserCreateDoneSlot(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(QString(), QString(), "org.freedesktop.Accounts", "UserDeleted", this, SLOT(existsUserDeleteDoneSlot(QDBusObjectPath)));
}

/*
* 判断用户是否为域用户
* 和/etc/passwd文件中用户做对比
* 1：域用户，0：非域用户
*/
int UserInfo::isDomainUser(const char* username)
{
    FILE *fp;
    fp=fopen("/etc/passwd","r");
    if(fp == NULL)
    {
        return 1;
    }
    char buf[1024], name[128];
    while(!feof(fp))
    {
        if(fgets(buf,sizeof (buf),fp) == NULL)
        {
            break;
        }
        sscanf(buf,"%[^:]",name);
        if(strcmp(name,username) == 0)
        {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return 1;
}

void UserInfo::buildItemForUsersAndSetConnect(UserInfomation user){

    UtilsForUserinfo * utils = new UtilsForUserinfo;
    utils->refreshUserLogo(user.iconfile);
    utils->refreshUserNickname(user.realname);
    utils->refreshUserType(user.accounttype);
    utils->setObjectPathData(user.objpath);
    fontSizeChange(user, utils);

    if (user.accounttype){
        utils->refreshDelStatus(!isLastAdmin(user.username));
        utils->refreshTypeStatus(!isLastAdmin(user.username));
    }

    // 域用户按钮不可设置
    UserInfomation curruser = allUserInfoMap.value(g_get_user_name());
    if (isDomainUser(curruser.username.toLatin1().data())) {
        utils->refreshDelStatus(false);
        utils->refreshPwdStatus(false);
        utils->refreshTypeStatus(false);
    }

#ifdef WITHKYSEC
    if (!kysec_is_disabled() && kysec_get_3adm_status()){
        if (user.username == "secadm" || user.username == "auditadm"){
            utils->refreshDelStatus(false);
            utils->refreshPwdStatus(false);
            utils->refreshTypeStatus(false);
        }
    }
#endif

    connect(utils, &UtilsForUserinfo::changeLogoBtnPressed, this, [=]{
        showChangeUserLogoDialog(user.username, utils);
    });
    connect(utils, &UtilsForUserinfo::changePwdBtnPressed, this, [=]{
        showChangeUserPwdDialog(user.username);
    });
    connect(utils, &UtilsForUserinfo::changeTypeBtnPressed, this, [=]{
        showChangeUserTypeDialog(user.username);
    });

    connect(utils, &UtilsForUserinfo::deleteUserBtnPressed, this, [=]{
        showDeleteUserExistsDialog(user.username);
    });

    //用户类型发生变化，重置状态
    connect(this, &UserInfo::userTypeChanged, utils, [=](QString n){
        utils->refreshDelStatus(!isLastAdmin(user.username));
        utils->refreshTypeStatus(!isLastAdmin(user.username));
#ifdef WITHKYSEC
        if (!kysec_is_disabled() && kysec_get_3adm_status()){
            if (user.username == "secadm" || user.username == "auditadm"){
                utils->refreshDelStatus(false);
                utils->refreshPwdStatus(false);
                utils->refreshTypeStatus(false);
            }
        }
#endif
    });

    QDBusInterface tmpProperty("org.freedesktop.Accounts",
                             user.objpath,
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());

    tmpProperty.connection().connect("org.freedesktop.Accounts", user.objpath, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                    utils, SLOT(userPropertyChangedSlot(QString, QMap<QString, QVariant>, QStringList)));

    QFrame * newUserFrame = utils->buildItemForUsers();
    othersFrame->setFixedHeight(othersFrame->height() + newUserFrame->height());

    otherVerLayout->insertWidget(0, newUserFrame);
}

void UserInfo::showCreateUserNewDialog(){
    //获取系统所有用户名列表，创建时判断重名
    QStringList usersStringList;
    QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
    for (; it != allUserInfoMap.end(); it++){
        UserInfomation user = it.value();

        usersStringList.append(user.username);
        usersStringList.append(user.realname);
    }

    CreateUserNew dialog(usersStringList, pluginWidget2);
    dialog.exec();
}

void UserInfo::showDeleteUserExistsDialog(QString pName){
    QStringList loginedusers = getLoginedUsers();
    if (loginedusers.contains(pName)) {
        QMessageBox::warning(pluginWidget2, tr("Warning"), tr("The user is logged in, please delete the user after logging out"));
        return;
    }

    if (allUserInfoMap.keys().contains(pName)){
        UserInfomation user = allUserInfoMap.value(pName);

        DeleteUserExists dialog(user.username, user.realname, user.uid, pluginWidget2);
        dialog.exec();
    }
}

void UserInfo::showChangeUserNicknameDialog(){
    if (allUserInfoMap.keys().contains(g_get_user_name())){
        QStringList names;
        QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
        for (; it != allUserInfoMap.end(); it++){
            UserInfomation user = it.value();

            if (QString::compare(user.username, g_get_user_name()) == 0)
                continue;

            names.append(user.username);
            names.append(user.realname);
        }

        UserInfomation user = allUserInfoMap.value(g_get_user_name());

        ChangeUserNickname dialog(user.realname, names, user.objpath , pluginWidget2);
        dialog.exec();

    } else {
        qWarning() << "User Data Error When Change User Type";
    }

    _acquireAllUsersInfo();
}

void UserInfo::showChangeUserLogoDialog(QString pName, UtilsForUserinfo *utilsUser){
    if (allUserInfoMap.keys().contains(pName)){
        UserInfomation user = allUserInfoMap.value(pName);

        ChangeUserLogo *dialog = new ChangeUserLogo(pName, user.objpath, pluginWidget2);
        qDebug() << user.iconfile << ";" << __LINE__;
        dialog->requireUserInfo(user.iconfile, _accountTypeIntToString(user.accounttype));
        connect(dialog, &ChangeUserLogo::face_file_send, this, [=](QString faceFile){
            changeUserFace(faceFile, user.username, utilsUser);
        });
        dialog->exec();

    } else {
        qWarning() << "User Info Data Error When Change User Pwd";
    }

    _acquireAllUsersInfo();
}

void UserInfo::changeUserFace(QString facefile, QString username, UtilsForUserinfo *utilsUser)
{
    if (utilsUser != nullptr) {
        QPixmap iconfile = makeRoundLogo(facefile, utilsUser->logoBtn->width(), utilsUser->logoBtn->height(), utilsUser->logoBtn->width()/2);
        utilsUser->logoBtn->setIcon(iconfile);
    } else {
        QPixmap iconfile = makeRoundLogo(facefile, currentUserlogoBtn->width(), currentUserlogoBtn->height(), currentUserlogoBtn->width()/2);
        currentUserlogoBtn->setIcon(iconfile);
    }
}

QPixmap UserInfo::makeRoundLogo(QString logo, int wsize, int hsize, int radius)
{
    QPixmap rectPixmap;
    QPixmap iconcop = QPixmap(logo);

    if (iconcop.width() > iconcop.height()) {
        QPixmap iconPixmap = iconcop.copy((iconcop.width() - iconcop.height())/2, 0, iconcop.height(), iconcop.height());
        // 根据label高度等比例缩放图片
        rectPixmap = iconPixmap.scaledToHeight(hsize);
    } else {
        QPixmap iconPixmap = iconcop.copy(0, (iconcop.height() - iconcop.width())/2, iconcop.width(), iconcop.width());
        // 根据label宽度等比例缩放图片
        rectPixmap = iconPixmap.scaledToWidth(wsize);
    }

    if (rectPixmap.isNull()) {
        return QPixmap();
    }
    QPixmap pixmapa(rectPixmap);
    QPixmap pixmap(radius*2,radius*2);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QPainterPath path;
    path.addEllipse(0, 0, radius*2, radius*2);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, radius*2, radius*2, pixmapa);
    return pixmap;
}

void UserInfo::showChangeUserPwdDialog(QString pName){
    if (allUserInfoMap.keys().contains(pName)){
        UserInfomation user = allUserInfoMap.value(pName);

        ChangeUserPwd dialog(pName, pluginWidget2);
        dialog.exec();

    } else {
        qWarning() << "User Info Data Error When Change User Pwd";
    }
}

void UserInfo::showChangeUserTypeDialog(QString u){
    if (allUserInfoMap.keys().contains(u)){
        UserInfomation user = allUserInfoMap.value(u);

        ChangeUserType dialog(user.objpath, pluginWidget2);
        dialog.requireUserInfo(user.iconfile, user.realname, user.accounttype, _accountTypeIntToString(user.accounttype));
        if (dialog.exec() == QDialog::Accepted){
            //告知有用户的类型发生变化
            emit userTypeChanged(u);
        }

    } else {
        qWarning() << "User Data Error When Change User Nickname";
    }

    _acquireAllUsersInfo();
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

bool UserInfo::setTextDynamic(QLabel *label, QString string){

    bool isOverLength = false;
    QFontMetrics fontMetrics(label->font());
    int labelValueSize = 150;
    int fontSize = fontMetrics.width(string);
    QString str = string;
    qDebug() << "fontSize:" << fontSize << ";labelValueSize:" << labelValueSize;
    if (fontSize > labelValueSize) {
        str = fontMetrics.elidedText(string, Qt::ElideRight, labelValueSize);
        isOverLength = true;
    } else {

    }
    label->setText(str);
    return isOverLength;

}

void UserInfo::setUserConnect(){

    currentNickNameLabel->installEventFilter(this);
    currentNickNameChangeLabel->installEventFilter(this);

    connect(currentUserlogoBtn, &QPushButton::clicked, this, [=]{
        showChangeUserLogoDialog(QString(g_get_user_name()), nullptr);
    });

    connect(changeCurrentPwdBtn, &QPushButton::clicked, this, [=]{
        showChangeUserPwdDialog(QString(g_get_user_name()));
    });

    connect(changeCurrentTypeBtn, &QPushButton::clicked, [=]{
        showChangeUserTypeDialog(QString(g_get_user_name()));
    });

    connect(changeCurrentGroupsBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        showChangeGroupDialog();
    });

    //自动登录登录
    connect(autoLoginSBtn, &SwitchButton::checkedChanged, autoLoginSBtn, [=](bool checked){
        UserInfomation user = allUserInfoMap.value(g_get_user_name());

        QString autoUser = getAutomaticLogin();
        qDebug() << "Current Auto User:" << autoUser;

        //冲突，弹出提示窗口由用户选择
        if (checked && !autoUser.isEmpty()){

            if (!openAutoLoginMsg(user.username)){
                autoLoginSBtn->blockSignals(true);
                autoLoginSBtn->setChecked(false);
                autoLoginSBtn->blockSignals(false);
                return;
            }
        }

        QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.Accounts",
                                                              user.objpath,
                                                              "org.freedesktop.Accounts.User",
                                                              "SetAutomaticLogin");
        message << checked;
        QDBusMessage response = QDBusConnection::systemBus().call(message);

        if (response.type() == QDBusMessage::ErrorMessage){

            autoLoginSBtn->blockSignals(true);
            autoLoginSBtn->setChecked(!checked);
            autoLoginSBtn->blockSignals(false);
        }
    });

    //免密登录
    connect(nopwdLoginSBtn, &SwitchButton::checkedChanged, [=](bool checked){
        UserInfomation user = allUserInfoMap.value(g_get_user_name());

        QDBusInterface piface("com.control.center.qt.systemdbus",
                              "/",
                              "com.control.center.interface",
                              QDBusConnection::systemBus());
        if (!piface.isValid()){
            nopwdLoginSBtn->blockSignals(true);
            nopwdLoginSBtn->setChecked(!checked);
            nopwdLoginSBtn->blockSignals(false);
            qCritical() << "Create Client Interface Failed When execute gpasswd: " << QDBusConnection::systemBus().lastError();
            return;
        }

        QDBusReply<int> ret = piface.call("setNoPwdLoginStatus", checked, user.username);
        if (ret == 0) {
            nopwdLoginSBtn->blockSignals(true);
            nopwdLoginSBtn->setChecked(!checked);
            nopwdLoginSBtn->blockSignals(false);
        }

    });

    connect(addUserBtn, &AddBtn::clicked, [=]{
        showCreateUserNewDialog();
    });
}

bool UserInfo::authoriyLogin()
{
    PolkitQt1::Authority::Result result;

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
                "org.control.center.qt.systemdbus.action.login",
                PolkitQt1::UnixProcessSubject(QCoreApplication::applicationPid()),
                PolkitQt1::Authority::AllowUserInteraction);

    if (result == PolkitQt1::Authority::Yes) { //认证通过
        qDebug() << QString("operation authorized") << result;
        return true;
    } else {
        qDebug() << QString("not authorized") << result;
        return false;
    }
}

void UserInfo::setUserDBusPropertyConnect(const QString pObjPath){
    QDBusInterface iproperty("org.freedesktop.Accounts",
                             pObjPath,
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());

    iproperty.connection().connect("org.freedesktop.Accounts", pObjPath, "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                    this, SLOT(currentUserPropertyChangedSlot(QString, QMap<QString, QVariant>, QStringList)));
}


void UserInfo::currentUserPropertyChangedSlot(QString property, QMap<QString, QVariant> propertyMap, QStringList propertyList){
    Q_UNUSED(property);
    Q_UNUSED(propertyList);
    if (propertyMap.keys().contains("AutomaticLogin") && getuid()){
        bool current = propertyMap.value("AutomaticLogin").toBool();
        if (current != autoLoginSBtn->isChecked()){
            autoLoginSBtn->blockSignals(true);
            autoLoginSBtn->setChecked(current);
            autoLoginSBtn->blockSignals(false);
        }
    }

    if (propertyMap.keys().contains("RealName") && getuid()){
        QString current = propertyMap.value("RealName").toString();
        if (QString::compare(current, currentNickNameLabel->text()) != 0){
            //更新用户昵称
            if (setTextDynamic(currentNickNameLabel, current)){
                currentNickNameLabel->setToolTip(current);
            } else {
                currentNickNameLabel->setToolTip("");
            }
        }
    }

    if (propertyMap.keys().contains("IconFile") && getuid()){
        QString current = propertyMap.value("IconFile").toString();
        QPixmap iconfile = makeRoundLogo(current, currentUserlogoBtn->width(), currentUserlogoBtn->height(), currentUserlogoBtn->width()/2);
        currentUserlogoBtn->setIcon(iconfile);
    }

    if (propertyMap.keys().contains("AccountType") && getuid()){
        bool current = propertyMap.value("AccountType").toBool();
        //刷新当前用户类型
        QString cType = _accountTypeIntToString(current);
        if (setTextDynamic(currentUserTypeLabel, cType)){
            currentUserTypeLabel->setToolTip(cType);
        }
        showMessageBox();
    }
}

void UserInfo::showMessageBox()
{
    QMessageBox msg(qApp->activeWindow());
    msg.setIcon(QMessageBox::Warning);

    msg.setText(tr("Modify the account type need to logout to take effect, whether to logout?"));
    msg.addButton(tr("logout later"), QMessageBox::NoRole);
    msg.addButton(tr("logout now"), QMessageBox::ApplyRole);

    int ret = msg.exec();

    if (ret == 1) {
        system("ukui-session-tools --logout");
    }
    return;
}

void UserInfo::newUserCreateDoneSlot(QDBusObjectPath op){
    //刷新用户数据
    _acquireAllUsersInfo();

    UserInfomation user;
    user = _acquireUserInfo(op.path());

    buildItemForUsersAndSetConnect(user);

    //
    emit userTypeChanged(user.username);
}

void UserInfo::existsUserDeleteDoneSlot(QDBusObjectPath op){
    //刷新用户数据
    _acquireAllUsersInfo();

    QObjectList list = othersFrame->children();
    foreach (QObject * obj, list) {
        if (obj->objectName() == op.path()){
            QFrame * f = qobject_cast<QFrame *>(obj);
            f->setParent(NULL);
            otherVerLayout->removeWidget(f);

            //重置高度
            othersFrame->setFixedHeight(othersFrame->height() - f->height());
        }
    }

    emit userTypeChanged("");
}

bool UserInfo::isLastAdmin(QString uname){
    QString cmd = QString("cat /etc/group | grep sudo | awk -F: '{ print $NF}'");
    QString output;

    FILE   *stream;
    char buf[256];

    if ((stream = popen(cmd.toLatin1().data(), "r" )) == NULL){
        return false;
    }

    while(fgets(buf, 256, stream) != NULL){
        output = QString(buf).simplified();
    }

    pclose(stream);

    QStringList users = output.split(",");
    int num = users.length();

    if (users.contains(uname)){
        if (num > 1){
            return false;
        } else {
            return true;
        }
    } else {
        return false;
    }
}

QString UserInfo::getAutomaticLogin() {

    QString filename = "/etc/lightdm/lightdm.conf";
    autoSettings = new QSettings(filename, QSettings::IniFormat);
    autoSettings->beginGroup("SeatDefaults");

    QString autoUser = autoSettings->value("autologin-user", "").toString();

    autoSettings->endGroup();

    return autoUser;
}

bool UserInfo::openAutoLoginMsg(const QString &userName){
    QString autoLoginedUser = this->getAutomaticLogin();
    bool res = true;
    int  ret;
    if (!autoLoginedUser.isEmpty() && userName != autoLoginedUser) {
        QMessageBox msg(this->pluginWidget2);
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
        user.autologin = propertyMap.find("AutomaticLogin").value().toBool();
        user.objpath = objpath;

        //用户头像为.face且.face文件不存在
        char * iconpath = user.iconfile.toLatin1().data();
        if (!g_file_test(iconpath, G_FILE_TEST_EXISTS)){
            user.iconfile = DEFAULTFACE;
        }
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

void UserInfo::showChangeGroupDialog(){
    ChangeGroupDialog * dialog = new ChangeGroupDialog(pluginWidget2);
    dialog->exec();
}

bool UserInfo::eventFilter(QObject *watched, QEvent *event){

    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton ){
            if ((watched == currentNickNameChangeLabel && currentNickNameChangeLabel->isEnabled())
                    || (watched == currentNickNameLabel && currentNickNameLabel->isEnabled())){
                showChangeUserNicknameDialog();
            }
        }
    }

    return QObject::eventFilter(watched, event);
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

void UserInfo::fontSizeChange(UserInfomation user, UtilsForUserinfo * utils)
{
    const QByteArray styleID(STYLE_FONT_SCHEMA);
    QGSettings *stylesettings = new QGSettings(styleID, QByteArray(), this);
    connect(stylesettings, &QGSettings::changed, this, [=](const QString &key){
        if (key == "systemFontSize" || key == "systemFont") {
            if (utils == nullptr) {
                QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
                for (; it != allUserInfoMap.end(); it++){
                    UserInfomation currentUser = it.value();
                    //当前用户
                    if (currentUser.username == QString(g_get_user_name())){
                        //设置用户昵称
                        if (setTextDynamic(currentNickNameLabel, currentUser.realname)){
                            currentNickNameLabel->setToolTip(currentUser.realname);
                        }

                        //用户类型
                        QString cType = _accountTypeIntToString(currentUser.accounttype);
                        if (setTextDynamic(currentUserTypeLabel, cType)){
                            currentUserTypeLabel->setToolTip(cType);
                        }
                    }
                }
            } else {
                utils->refreshUserNickname(user.realname);
                utils->refreshUserType(user.accounttype);
            }
        }
    });

}
