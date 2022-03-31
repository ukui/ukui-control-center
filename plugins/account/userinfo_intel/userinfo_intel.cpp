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
#include "userinfo_intel.h"
#include "ui_userinfo_intel.h"
#include "changepininteldialog.h"
#include "changepwdinteldialog.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>

#include <QDebug>
#include <QDir>
#include <QFrame>
#include <QTextCodec>
#include <QByteArray>


#include <ukcc/widgets/switchbutton.h>
#include <ukcc/widgets/imageutil.h>
#include "elipsemaskwidget.h"
#include "passwdcheckutil.h"


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


#define DEFAULTFACE "/usr/share/ukui/faces/default.png"
#define ITEMHEIGH 72

UserInfoIntel::UserInfoIntel() : mFirstLoad(true)
{
    pluginName = tr("User Info Intel");
    pluginType = ACCOUNT;
}

UserInfoIntel::~UserInfoIntel()
{
    if (!mFirstLoad) {
        delete ui;
        delete autoSettings;
    }
}

QString UserInfoIntel::plugini18nName(){
    return pluginName;
}

int UserInfoIntel::pluginTypes(){
    return pluginType;
}

QWidget *UserInfoIntel::pluginUi(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::UserInfoIntel;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
        //构建System dbus调度对象
        sysdispatcher = new SystemDbusDispatcher;

        ui->changeGroupBtn->hide();
    //    ui->changeValidBtn->hide();

        //获取系统全部用户信息，用户Uid大于等于1000的
        _acquireAllUsersInfo();

        initSearchText();
        readCurrentPwdConf();
        initComponent();
        initAllUserStatus();
        //设置界面用户信息
        _refreshUserInfoUI();

    //    pwdSignalMapper = new QSignalMapper(this);
    //    faceSignalMapper = new QSignalMapper(this);
    //    typeSignalMapper = new QSignalMapper(this);
    //    delSignalMapper = new QSignalMapper(this);

    //    faceSize = QSize(64, 64);
    //    itemSize = QSize(230, 106); //?需要比btnsize大多少？否则显示不全
    //    btnSize = QSize(222, 92);
    //      showCreateUserDialog();

    //    get_all_users();
    //    ui_component_init();
    //    ui_status_init();
    }
    return pluginWidget;
}

const QString UserInfoIntel::name() const {

    return QStringLiteral("UserinfoIntel");
}

bool UserInfoIntel::isShowOnHomePage() const
{
    return isIntel();
}

QIcon UserInfoIntel::icon() const
{
    return QIcon();
}

bool UserInfoIntel::isEnable() const
{
    return isIntel();
}

void UserInfoIntel::initSearchText() {
    ui->changePhoneBtn->setIcon(QIcon(":/img/plugins/userinfo_intel/phone.svg"));
    //~ contents_path /Userinfo/Current User
    ui->titleLabel->setText(tr("Current User"));

    //~ contents_path /Userinfo/Other Users
    ui->title2Label->setText(tr("Other Users"));

    //~ contents_path /Userinfo/Change Tel
    ui->changePhoneBtn->setText(tr("Change Tel"));

    ui->changePwdBtn->setIcon(QIcon(":/img/plugins/userinfo_intel/password.svg"));

    //~ contents_path /Userinfo/Change pwd
    ui->changePwdBtn->setText(tr("Change pwd"));


    ui->delUserBtn->setIcon(QIcon(":/img/plugins/userinfo_intel/delete.svg"));
    m_pPictureToWhite = new PictureToWhite();
    ui->editBtn->setIcon(QIcon(m_pPictureToWhite->drawSymbolicColoredPixmap(QPixmap(":/img/plugins/userinfo_intel/settingedit.svg"))));
    const QByteArray id(ORG_UKUI_STYLE);
    if (QGSettings::isSchemaInstalled(id)) {
        m_pgsettings = new QGSettings(id);
        connect(m_pgsettings, &QGSettings::changed, this, [=] (const QString &key) {
            if (key==STYLE_NAME) {
                ui->editBtn->setIcon(QIcon(m_pPictureToWhite->drawSymbolicColoredPixmap(QPixmap(":/img/plugins/userinfo_intel/settingedit.svg"))));
                if (m_pgsettings->get(STYLE_NAME).toString() == "ukui-light") {
                    ui->userNameLabel->setStyleSheet("color: black");
                } else {
                    ui->userNameLabel->setStyleSheet("color: white");
                }
            }
        });
    }


    //~ contents_path /Userinfo/Delete user
    ui->delUserBtn->setText(tr("Delete user"));

    ui->frame->hide();
    ui->frame_2->hide();
    ui->line_2->hide();
    ui->line_4->hide();
    ui->line->hide();
}

QString UserInfoIntel::_accountTypeIntToString(int type){
//    qDebug()<<"allUserInfoMap.count()-------------"<<allUserInfoMap.count();
    QString atype;
    if (type == STANDARDUSER) {
        atype = tr("standard user"); 
    } else if (type == ADMINISTRATOR) {
        atype = tr("administrator");
        if (allUserInfoMap.count() != 1) {
            ui->line_3->hide();
            ui->delUserBtn->hide();
        } else {
            ui->title2Label->hide();
        }
    }
    else if (type == ROOT) {
        atype = tr("root");
    }

    return atype;
}

void UserInfoIntel::_acquireAllUsersInfo(){
    QStringList objectpaths = sysdispatcher->list_cached_users();

    //初始化用户信息QMap
    allUserInfoMap.clear();
    //初始化管理员数目为0
    adminnum = 0;

    for (QString objectpath : objectpaths){
        UserInfomation user;
        user = _acquireUserInfo(objectpath);
        allUserInfoMap.insert(user.username, user);
    }

    if (allUserInfoMap.isEmpty()) {
        ui->currentUserFrame->setVisible(false);
    } else {
        ui->currentUserFrame->setVisible(true);
    }
}

UserInfomation UserInfoIntel::_acquireUserInfo(QString objpath){
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
        if (user.username == QString(g_get_user_name())){
            user.current = true;
            user.logined = true;

            //获取当前用户免密登录属性
            QDBusInterface *tmpSysinterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                             "/",
                                             "com.control.center.interface",
                                             QDBusConnection::systemBus());
            //获取免密登录状态
            QDBusReply<QString> noPwdres;
            noPwdres = tmpSysinterface ->call("getNoPwdLoginStatus");
            //const QString &tmp=noPwdres;
            if(!noPwdres.isValid()){
                qDebug()<<"获取tmpSysinterface状态不合法---->"<< noPwdres.error();
            }
            delete tmpSysinterface;

            user.noPwdLogin = noPwdres.value().contains(user.username) ? true : false;
        }
        user.accounttype = propertyMap.find("AccountType").value().toInt();
        if (user.accounttype == ADMINISTRATOR)
            adminnum++;
        user.iconfile = propertyMap.find("IconFile").value().toString();
        user.passwdtype = propertyMap.find("PasswordMode").value().toInt();
        user.uid = propertyMap.find("Uid").value().toInt();
//        user.autologin = propertyMap.find("AutomaticLogin").value().toBool();
        user.autologin = this->getAutomaticLogin(user.username);
        user.objpath = objpath;
    }
    else
        qDebug() << "reply failed";

    delete iproperty;

    return user;
}

void UserInfoIntel::readCurrentPwdConf(){
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

void UserInfoIntel::initComponent(){
    //样式表
//    pluginWidget->setStyleSheet("background: #ffffff;");

//    ui->currentUserWidget->setStyleSheet("QWidget{background: #F4F4F4; border-top-left-radius: 6px; border-top-right-radius: 6px;}");
//    ui->autoLoginWidget->setStyleSheet("QWidget{background: #F4F4F4; border-bottom-left-radius: 6px; border-bottom-right-radius: 6px;}");

//    QString btnQss = QString("QPushButton{background: #FFFFFF; border-radius: 4px;}");
//    ui->changePwdBtn->setStyleSheet(btnQss);
//    ui->changeTypeBtn->setStyleSheet(btnQss);

//    ui->addUserWidget->setStyleSheet("QWidget{background: #F4F4F4; border-radius: 6px;}");

//    QString filename = "/etc/lightdm/lightdm.conf";
//    autoSettings = new QSettings(filename, QSettings::IniFormat);

    ui->listWidget->setStyleSheet("QListWidget::Item{background:palette(base);}");

    addWgt = new HoverWidget("");
    addWgt->setObjectName("addwgt");
    addWgt->setMinimumSize(QSize(0, 64));
    addWgt->setMaximumSize(QSize(16777215, 64));
    addWgt->setStyleSheet("HoverWidget#addwgt{background: palette(base); border-radius: 4px;}HoverWidget:hover:!pressed#addwgt{background: #2FB3E8; border-radius: 4px;}");

    QHBoxLayout *addLyt = new QHBoxLayout;

    QLabel * iconLabel = new QLabel();
    QLabel * textLabel = new QLabel(tr("Add new user"));
    QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
    iconLabel->setPixmap(pixgray);
    addLyt->addWidget(iconLabel);
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    addWgt->setLayout(addLyt);

    // 悬浮改变Widget状态
    connect(addWgt, &HoverWidget::enterWidget, this, [=](QString mname){
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "white", 12);
        iconLabel->setPixmap(pixgray);
        textLabel->setStyleSheet("color: palette(base);");

    });
    // 还原状态
    connect(addWgt, &HoverWidget::leaveWidget, this, [=](QString mname){
        QPixmap pixgray = ImageUtil::loadSvg(":/img/titlebar/add.svg", "black", 12);
        iconLabel->setPixmap(pixgray);
        textLabel->setStyleSheet("color: palette(windowText);");
    });

    ui->listWidget->setSpacing(0);

    ElipseMaskWidget * mainElipseMaskWidget = new ElipseMaskWidget(ui->currentUserFaceLabel);
    mainElipseMaskWidget->setGeometry(0, 0, ui->currentUserFaceLabel->width(), ui->currentUserFaceLabel->height());

    //设置添加用户的图标
//    ui->addBtn->setIcon(QIcon("://img/plugins/userinfo_intel/add.png"));
//    ui->addBtn->setIconSize(ui->addBtn->size());
//    ui->addBtn->setStyleSheet("QPushButton{background-color:transparent;}");

    ui->currentUserFaceLabel->installEventFilter(this);
//    ui->addUserFrame->installEventFilter(this);

    //修改当前用户绑定手机号的回调
    connect(ui->changePhoneBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        UserInfomation user = allUserInfoMap.value(g_get_user_name());

        showChangePhoDialog(user.username);
    });

    //修改当前用户密码的回调
    connect(ui->changePwdBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        UserInfomation user = allUserInfoMap.value(g_get_user_name());
        showChangePwdDialog(user.username);
    });
    //删除当前用户
    connect(ui->delUserBtn, &QPushButton::clicked, this, [=](bool checked){

        UserInfomation user = allUserInfoMap.value(g_get_user_name());
        DelUserIntelDialog * dialog = new DelUserIntelDialog;
        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->setFace(user.iconfile);
        dialog->setUsername(user.username,user.realname);

        connect(dialog, &DelUserIntelDialog::removefile_send, this, [=](bool removeFile, QString userName){
            qDebug()<<userName;
            QDBusInterface * m_interface = new QDBusInterface("cn.kylinos.SSOBackend",
                                                              "/cn/kylinos/SSOBackend",
                                                              "cn.kylinos.SSOBackend.accounts",
                                                              QDBusConnection::systemBus());
            QDBusMessage result = m_interface->call("DeleteAccount",user.username);
            isDelCurrentUser = true;
            QList<QVariant> outArgs = result.arguments();
            int status = outArgs.at(0).value<int>();
            delete m_interface;
            if (status == 0) {
                QProcess p(0);
                p.startDetached("ukui-session-tools --logout");
                p.waitForStarted();
            }
        });
        dialog->exec();
        return;
    });

//    //修改当前用户类型的回调
//    connect(ui->changeTypeBtn, &QPushButton::clicked, this, [=](bool checked){
//        Q_UNUSED(checked)
//        UserInfomation user = allUserInfoMap.value(g_get_user_name());

//        showChangeTypeDialog(user.username);
//    });

//    connect(ui->changeValidBtn, &QPushButton::clicked, this, [=](bool checked){
//        Q_UNUSED(checked)
//        UserInfomation user = allUserInfoMap.value(g_get_user_name());

//        showChangeValidDialog(user.username);

//    });

    connect(ui->changeGroupBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        showChangeGroupDialog();
    });


    //修改当前用户自动登录
//    connect(autoLoginSwitchBtn, &SwitchButton::checkedChanged, [=](bool checked){
//        UserInfomation user = allUserInfoMap.value(g_get_user_name());


//        UserDispatcher * userdispatcher  = new UserDispatcher(user.objpath);


//        bool status = userdispatcher->get_autoLogin_status();

//        bool status = this->getAutomaticLogin(user.username);


//        if ((checked != status)) {
//            if (checked) {
//                userdispatcher->change_user_autologin(user.username);
//            } else {
//                userdispatcher->change_user_autologin("");
//            }
//        }

//        bool lstStatus = userdispatcher->get_autoLogin_status();

//        bool lstStatus = this->getAutomaticLogin(user.username);
//        autoLoginSwitchBtn->setChecked(lstStatus);
//    });

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

void UserInfoIntel::_resetListWidgetHeigh(){
    //设置其他用户控件的总高度
    if (!isDelCurrentUser) {
        ui->listWidget->setFixedHeight((allUserInfoMap.count() - 1) * (ITEMHEIGH + 2));
    }
  //  ui->listWidget->setFixedHeight((20) * (ITEMHEIGH + 2));
}

void UserInfoIntel::initAllUserStatus(){
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

QPixmap UserInfoIntel::PixmapToRound(const QString &src, int radius)
{
    if (src == "") {
        return QPixmap();
    }
    QPixmap pixmapa(src);
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

void UserInfoIntel::_refreshUserInfoUI(){
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
            QPixmap iconcop = QPixmap(user.iconfile);
            if (iconcop.width() > iconcop.height()) {
                QPixmap iconPixmap = iconcop.copy((iconcop.width() - iconcop.height())/2, 0, iconcop.height(), iconcop.height());
                // 根据label高度等比例缩放图片
                ui->currentUserFaceLabel->setPixmap(iconPixmap.scaledToHeight(ui->currentUserFaceLabel->height()));

                setChangeFaceShadow();
            } else {
                QPixmap iconPixmap = iconcop.copy(0, (iconcop.height() - iconcop.width())/2, iconcop.width(), iconcop.width());
                // 根据label宽度等比例缩放图片
                ui->currentUserFaceLabel->setPixmap(iconPixmap.scaledToWidth(ui->currentUserFaceLabel->width()));

                setChangeFaceShadow();
            }

            current_user = user;

            QDir historyDir;
            historyDir.setPath(QString("/home/%1").arg(user.username));
            if(!historyDir.exists(QString("/home/%1/.historyfaces").arg(user.username))) {
                historyDir.mkpath(QString("/home/%1/.historyfaces").arg(user.username));
            } else {
//                qDebug()<<QString("/home/%1/.historyfaces/").arg(user.username)<<" Exist!";
            }

            //设置用户名

            ui->userNameLabel->setStyleSheet("QLineEdit{color: palette(windowText);}");
            QFontMetrics fontMetrics(ui->userNameLabel->font());
            int fontSize = fontMetrics.width(user.realname);
            m_userName = user.realname;
            if (fontSize > 180) {
                QString str = fontMetrics.elidedText(user.realname, Qt::ElideRight, 180);
                ui->userNameLabel->setFixedWidth(180);
                ui->userNameLabel->blockSignals(true);
                ui->userNameLabel->setText(str);
                ui->userNameLabel->blockSignals(false);
                ui->userNameLabel->setToolTip(user.realname);
            } else {
                ui->userNameLabel->setFixedWidth(fontSize+5);
                ui->userNameLabel->blockSignals(true);
                ui->userNameLabel->setText(user.realname);
                ui->userNameLabel->blockSignals(false);
            }
            ui->userNameLabel->setCursorPosition(0);
            ui->userNameLabel->setReadOnly(true);
            ui->userNameLabel->installEventFilter(this);

            oldName = ui->userNameLabel->text();
            ui->userNameLabel->setMaxLength(32);
            connect(ui->userNameLabel,&QLineEdit::textChanged, [=](QString text){
                QFontMetrics fontMetrics(ui->userNameLabel->font());
                int fontSize = fontMetrics.width(text);
                if (fontSize > 180) {
                    ui->userNameLabel->setFixedWidth(180);
                } else {
                    ui->userNameLabel->setFixedWidth(fontSize+5);
                }
                m_userName = text;
                qDebug()<<"*********************SET NAME = "<<text;
            });
            connect(ui->editBtn,&QPushButton::clicked,[=](){
                UserInfomation curruser = allUserInfoMap.value(g_get_user_name());
                ui->userNameLabel->blockSignals(true);
                ui->userNameLabel->setText(curruser.realname);
                ui->userNameLabel->blockSignals(false);
                ui->userNameLabel->setReadOnly(false);
                ui->userNameLabel->selectAll();
                ui->userNameLabel->setFocus();
                ui->editBtn->hide();
            });
            my_name = user.username;
            //设置用户类型
            ui->userTypeLabel->setText(_accountTypeIntToString(user.accounttype));
            //设置登录状态
           // autoLoginSwitchBtn->setChecked(user.autologin);
            //设置免密登录状态
           // nopwdSwitchBtn->setChecked(user.noPwdLogin);

        } else { //其他用户
            QListWidgetItem * item = otherUserItemMap.value(user.objpath); //是否需要判断？？

            QWidget * widget = ui->listWidget->itemWidget(item);

            QLabel * faceLabel = widget->findChild<QLabel *>("faceLabel");
//            faceLabel->setIcon(QIcon(user.iconfile));
//            _buildWidgetForItem(user);
            if (faceLabel->layout() != NULL) {
                QLayoutItem* item;
                while ((item = faceLabel->layout()->takeAt( 0 )) != NULL )
                {
                    delete item->widget();
                    delete item;
                }
//                delete faceLabel->layout();
            }
            faceLabel->setScaledContents(true);
            faceLabel->setPixmap(PixmapToRound(user.iconfile, faceLabel->width()/2));
        }
    }
    QDBusInterface *m_interface1 = new QDBusInterface("cn.kylinos.SSOBackend",
                                                      "/cn/kylinos/SSOBackend",
                                                      "cn.kylinos.SSOBackend.eduplatform",
                                                      QDBusConnection::systemBus());
    if (m_interface1->isValid()) {
        QDBusMessage result = m_interface1->call("CheckPincodeSet",QString(g_get_user_name()));
        QList<QVariant> outArgs = result.arguments();
        delete m_interface1;
        int pinstatus = 0;
        pinstatus = outArgs.at(0).value<int>();
        if (pinstatus == 0) {
            ui->changePwdBtn->setText(tr("set pwd"));
        }
    }

}

void UserInfoIntel::setChangeFaceShadow()
{
    //在头像上添加更换字样及阴影
    QLabel *changeLabel = new QLabel(ui->currentUserFaceLabel);
    int changeLabelHeight = 26;
    changeLabel->setGeometry(0, ui->currentUserFaceLabel->height() - changeLabelHeight, ui->currentUserFaceLabel->width(), changeLabelHeight);

    changeLabel->setStyleSheet("QLabel{color:white;font-size:12px;background-color:rgb(0,0,0,70);}");
    changeLabel->setText(tr("Change"));
    changeLabel->setAlignment(Qt::AlignCenter);

    QLabel *eraseOutLabel = new QLabel(ui->currentUserFaceLabel);
    eraseOutLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    eraseOutLabel->setGeometry(0, 0, ui->currentUserFaceLabel->width(), ui->currentUserFaceLabel->height());

    ElipseMaskWidget *eraseOutElipseMaskWidget = new ElipseMaskWidget(eraseOutLabel);
    eraseOutElipseMaskWidget->setGeometry(0, 0, eraseOutLabel->width(), eraseOutLabel->height());
}

void UserInfoIntel::_buildWidgetForItem(UserInfomation user){
    HoverWidget * baseWidget = new HoverWidget(user.username);
    baseWidget->setMinimumSize(0,64);
    baseWidget->setMaximumSize(16777215,64);
    baseWidget->setAttribute(Qt::WA_DeleteOnClose);

    QHBoxLayout * baseVerLayout = new QHBoxLayout(baseWidget);
    baseVerLayout->setSpacing(0);
    baseVerLayout->setMargin(0);

    QHBoxLayout * baseHorLayout = new QHBoxLayout();
    baseHorLayout->setSpacing(16);
    baseHorLayout->setMargin(0);

    QFrame * widget = new QFrame(baseWidget);
    widget->setFrameShape(QFrame::Shape::Box);
    widget->setFixedHeight(64);

    QHBoxLayout * mainHorLayout = new QHBoxLayout(widget);
    mainHorLayout->setSpacing(16);
    mainHorLayout->setContentsMargins(16, 0, 16, 0);

    QLabel * faceLabel = new QLabel(widget);
    faceLabel->setObjectName("faceLabel");
    faceLabel->setFixedSize(40, 40);

    QHBoxLayout * faceLabelHorLayout = new QHBoxLayout(faceLabel);
    faceLabelHorLayout->setSpacing(0);
    faceLabelHorLayout->setMargin(0);
    faceLabel->setLayout(faceLabelHorLayout);

    ElipseMaskWidget * faceElipseMaskWidget = new ElipseMaskWidget(faceLabel);
    faceElipseMaskWidget->setGeometry(0, 0, faceLabel->width(), faceLabel->height());

    QLabel * nameLabel = new QLabel(widget);
    QSizePolicy nameSizePolicy = nameLabel->sizePolicy();
    nameSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    nameSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    nameLabel->setSizePolicy(nameSizePolicy);
    nameLabel->setText(user.realname);

    QString btnQss = QString("QPushButton{background: #ffffff; border-radius: 4px;}");

    QPushButton * typeBtn = new QPushButton(widget);
    typeBtn->setFixedSize(64, 64);
//    typeBtn->setText(tr("Change type"));
    typeBtn->setIcon(QIcon(":/img/plugins/userinfo_intel/type.svg"));
//    typeBtn->setStyleSheet(btnQss);
    connect(typeBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        showChangeTypeDialog(user.username);
    });
    typeBtn->hide();

    QPushButton * pwdBtn = new QPushButton(widget);
    pwdBtn->setFixedSize(64, 64);
//    pwdBtn->setText(tr("Change pwd"));
    pwdBtn->setIcon(QIcon(":/img/plugins/userinfo_intel/password.svg"));
//    pwdBtn->setStyleSheet(btnQss);
    connect(pwdBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)        
        showChangePwdDialog(user.username);

    });
    pwdBtn->hide();

    QFrame * line = new QFrame;
    line->setFrameShape(QFrame::VLine);
    line->setFixedSize(4, 12);

    mainHorLayout->addWidget(faceLabel);
    mainHorLayout->addWidget(nameLabel);
    mainHorLayout->addStretch();
//    mainHorLayout->addWidget(pwdBtn);
//    mainHorLayout->addWidget(typeBtn);
    mainHorLayout->addWidget(line);
    widget->setLayout(mainHorLayout);

    QPushButton * delBtn = new QPushButton(baseWidget);
    delBtn->setFixedSize(64, 64);
//    delBtn->setText(tr("Delete"));
    delBtn->setIcon(QIcon(":/img/plugins/userinfo_intel/deluser.svg"));
//    delBtn->setStyleSheet("QPushButton{background: #FA6056; border-radius: 4px}");
    delBtn->hide();
    connect(delBtn, &QPushButton::clicked, this, [=](bool checked){
        Q_UNUSED(checked)
        showDeleteUserDialog(user.username);
    });

    connect(baseWidget, &HoverWidget::enterWidget, this, [=](QString name){
        Q_UNUSED(name)
        //对普通用户，隐藏对其他用户进行操作的按钮
        if (current_user.accounttype != STANDARDUSER){
            line->hide();
            //typeBtn->show();
            //pwdBtn->show();
            delBtn->show();
        }
//        delBtn->show();
    });
    connect(baseWidget, &HoverWidget::leaveWidget, this, [=](QString name){
        Q_UNUSED(name)
        line->show();
        typeBtn->hide();
        pwdBtn->hide();
        delBtn->hide();
    });

    baseHorLayout->addWidget(widget);
    baseHorLayout->addWidget(pwdBtn, Qt::AlignVCenter);
    baseHorLayout->addWidget(typeBtn, Qt::AlignVCenter);
    baseHorLayout->addWidget(delBtn, Qt::AlignVCenter);
//    baseHorLayout->addSpacing(4);
    baseHorLayout->setSpacing(8);


    baseVerLayout->addLayout(baseHorLayout);
//    baseVerLayout->addStretch();

    baseWidget->setLayout(baseVerLayout);

    QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
//    item->setSizeHint(QSize(ui->listWidget->width() - 4, ITEMHEIGH));
    item->setSizeHint(QSize(QSizePolicy::Expanding, ITEMHEIGH));
    item->setData(Qt::UserRole, QVariant(user.objpath));
    ui->listWidget->setItemWidget(item, baseWidget);

    otherUserItemMap.insert(user.objpath, item);

}

void UserInfoIntel::showCreateUserDialog(){
    //获取系统所有用户名列表，创建时判断重名
    QStringList usersStringList;
    for (QVariant tmp : allUserInfoMap.keys()){
        usersStringList << tmp.toString();
    }

    CreateUserIntelDialog * dialog = new CreateUserIntelDialog(usersStringList);
    dialog->setRequireLabel(pwdMsg);
    connect(dialog, &CreateUserIntelDialog::newUserWillCreate, this, [=](QString uName, QString pwd, QString pin, int aType){
        createUser(uName, pwd, pin, aType);
    });
    dialog->exec();
}

QStringList UserInfoIntel::getUsersList()
{
    QStringList usersStringList;
    for (QVariant tmp : allUserInfoMap.keys()){
        usersStringList << tmp.toString();
    }
    return usersStringList;
}

void UserInfoIntel::createUser(QString username, QString pwd, QString pin, int atype){
    Q_UNUSED(pin);
    sysdispatcher->create_user(username, "", atype);

    //使用全局变量传递新建用户密码
    _newUserPwd = pwd;
}

void UserInfoIntel::createUserDone(QString objpath){
    UserDispatcher * userdispatcher  = new UserDispatcher(objpath);
    //设置默认头像
    userdispatcher->change_user_face(DEFAULTFACE);
    //设置默认密码
    userdispatcher->change_user_pwd(_newUserPwd, "");

    //刷新全部用户信息
    _acquireAllUsersInfo();
    //重建其他用户ListWidget高度
    _resetListWidgetHeigh();

    //获取新建用户信息
    UserInfomation user;
    user = _acquireUserInfo(objpath);

    //构建Item
    _buildWidgetForItem(user);
    _refreshUserInfoUI();
}

void UserInfoIntel::showDeleteUserDialog(QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());
//    QDBusInterface * m_interface = new QDBusInterface("cn.kylinos.SSOBackend",
//                                                      "/cn/kylinos/SSOBackend",
//                                                      "cn.kylinos.SSOBackend.accounts",
//                                                      QDBusConnection::systemBus());
//    m_interface->call("DeleteAccount",user.username);
//    delete m_interface;
    DelUserIntelDialog * dialog = new DelUserIntelDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setFace(user.iconfile);
    dialog->setUsername(user.username,user.realname);
    connect(dialog, &DelUserIntelDialog::removefile_send, this, [=](bool removeFile, QString userName){
        qDebug()<<userName;
        deleteUser(removeFile, userName);
    });
    qDebug()<<"delete";
    dialog->exec();
}

void UserInfoIntel::deleteUser(bool removefile, QString username){
    qDebug() << allUserInfoMap.keys() << username;

    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    // hidden the item when click delete user button
    QListWidgetItem *item =  otherUserItemMap.find(user.objpath).value();
    ui->listWidget->setItemHidden(item, true);

    //sysdispatcher->delete_user(user.uid, removefile);
    QDBusInterface * m_interface = new QDBusInterface("cn.kylinos.SSOBackend",
                                                      "/cn/kylinos/SSOBackend",
                                                      "cn.kylinos.SSOBackend.accounts",
                                                      QDBusConnection::systemBus());
    m_interface->call("DeleteAccount",user.username);
    delete m_interface;

}

void UserInfoIntel::delete_user_slot(bool removefile, QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    sysdispatcher->delete_user(user.uid, removefile);
}

void UserInfoIntel::deleteUserDone(QString objpath){
    QListWidgetItem * item = otherUserItemMap.value(objpath);

    //删除Item
    ui->listWidget->takeItem(ui->listWidget->row(item));

    //更新其他用户QMap
    otherUserItemMap.remove(objpath);

    //更新所有用户信息
    _acquireAllUsersInfo();

    //重置其他用户ListWidget高度
    _resetListWidgetHeigh();
    if (allUserInfoMap.count() == 1 && !isDelCurrentUser) {
        ui->line_3->show();
        ui->delUserBtn->show();
        ui->title2Label->hide();
    }
}

void UserInfoIntel::showChangeGroupDialog(){
    ChangeGroupIntelDialog * dialog = new ChangeGroupIntelDialog();
    dialog->exec();
}

void UserInfoIntel::showChangeValidDialog(QString username){
    if (allUserInfoMap.keys().contains(username)){
        UserInfomation user = allUserInfoMap.value(username);

        ChangeValidIntelDialog * dialog = new ChangeValidIntelDialog(user.username);
        dialog->setUserName();
        dialog->setUserLogo(user.iconfile);
        dialog->setUserType(_accountTypeIntToString(user.accounttype));
        dialog->exec();

    } else {
        qDebug() << "User Data Error When Change User type";
    }
}


void UserInfoIntel::showChangeTypeDialog(QString username){
    if (allUserInfoMap.keys().contains(username)){
        UserInfomation user = allUserInfoMap.value(username);

        ChangeTypeIntelDialog * dialog = new ChangeTypeIntelDialog;
        dialog->setFace(user.iconfile);
        dialog->setUsername(user.username);
        dialog->setCurrentAccountTypeLabel(_accountTypeIntToString(user.accounttype));
        dialog->setCurrentAccountTypeBtn(user.accounttype);
        dialog->forbidenChange(adminnum);
//        connect(dialog, SIGNAL(type_send(int,QString,bool)), this, SLOT(change_accounttype_slot(int,QString,bool)));
        connect(dialog, &ChangeTypeIntelDialog::type_send, this, [=](int atype, QString userName){
            changeUserType(atype, userName);
        });
        dialog->exec();

    } else {
        qDebug() << "User Data Error When Change User type";
    }
}

void UserInfoIntel::changeUserType(int atype, QString username){
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


void UserInfoIntel::showChangeFaceDialog(QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    ChangeFaceIntelDialog * dialog = new ChangeFaceIntelDialog;
    dialog->setHistoryFacesPath(QString("/home/%1/.historyfaces").arg(user.username));
    dialog->setFace(user.iconfile);
    dialog->setUsername(user.username);
    dialog->setRealname(user.realname);
    dialog->setAccountType(_accountTypeIntToString(user.accounttype));
//    dialog->set_face_list_status(user.iconfile);
    connect(dialog, &ChangeFaceIntelDialog::face_file_send, [=](QString faceFile, QString userName){
        changeUserFace(faceFile, userName);

    });
    dialog->exec();
    //ui->autoLoginFrame->hide();
}

void UserInfoIntel::changeUserFace(QString facefile, QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    UserDispatcher * userdispatcher  = new UserDispatcher(user.objpath);
    userdispatcher->change_user_face(facefile);
//    userdispatcher->change_user_face(QString("/home/%1/.face").arg(user.username));

    //拷贝设置的头像文件到~/.face
    sysinterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                     "/",
                                     "com.control.center.interface",
                                     QDBusConnection::systemBus());

    if (!sysinterface->isValid()) {
        qCritical() << "Create Client Interface Failed When Copy Face File: " << QDBusConnection::systemBus().lastError();
        return;
    }

    QString cmd = QString("cp %1 /home/%2/.face").arg(facefile).arg(user.username);

    QDBusReply<QString> reply =  sysinterface->call("systemRun", QVariant(cmd));

    //重新获取全部用户QMap
    _acquireAllUsersInfo();

    //更新界面显示
    _refreshUserInfoUI();

    Q_UNUSED(reply)
}

void UserInfoIntel::showChangePwdDialog(QString username){
    if (allUserInfoMap.keys().contains(username)){
        UserInfomation user = allUserInfoMap.value(username);

        ChangePinIntelDialog * dialog = new ChangePinIntelDialog(user.username);
        connect(dialog, &ChangePinIntelDialog::changepwd, [=](){
            ui->changePwdBtn->setText(tr("Change pwd"));
        });
//        dialog->setFace(user.iconfile);
//        dialog->setUsername(user.username);
//        dialog->setAccountType(_accountTypeIntToString(user.accounttype));
//        connect(dialog, &ChangePwdIntelDialog::passwd_send, this, [=](QString pwd, QString userName){
//            changeUserPwd(pwd, userName);
//        });
        //修改PIN码弹窗（已实现PIN码验证，等待设置PIN码接口）
       //ChangePinIntelDialog * dialog = new ChangePinIntelDialog;
       //ChangePwdIntelDialog * dialog = new ChangePwdIntelDialog();
       dialog->exec();

    } else {
        qDebug() << "User Info Data Error When Change User type";
    }
}
void UserInfoIntel::showChangePhoDialog(QString username){
    if (allUserInfoMap.keys().contains(username)){
        UserInfomation user = allUserInfoMap.value(username);

        ChangePhoneIntelDialog * dialog = new ChangePhoneIntelDialog(user.username);
//        dialog->setFace(user.iconfile);
//        dialog->setUsername(user.username);
//        dialog->setAccountType(_accountTypeIntToString(user.accounttype));
//        connect(dialog, &ChangePwdIntelDialog::passwd_send, this, [=](QString pwd, QString userName){
//            changeUserPwd(pwd, userName);
//        });
        //修改PIN码弹窗（已实现PIN码验证，等待设置PIN码接口）
//        ChangePinIntelDialog * dialog = new ChangePinIntelDialog;
        dialog->exec();

    } else {
        qDebug() << "User Info Data Error When Change User type";
    }
}


void UserInfoIntel::changeUserPwd(QString pwd, QString username){
    //上层已做判断，这里不去判断而直接获取
    UserInfomation user = allUserInfoMap.value(username);

    UserDispatcher * userdispatcher  = new UserDispatcher(user.objpath); //继承QObject不再删除
    QString result = userdispatcher->change_user_pwd(pwd, "");

    Q_UNUSED(result)
}


bool UserInfoIntel::eventFilter(QObject *watched, QEvent *event){
    if (watched == ui->currentUserFaceLabel){
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton ){
                if(watched == ui->currentUserFaceLabel){
                    showChangeFaceDialog(my_name);
                }
                return true;
            } else {
                return false;
            }
        }
    }
    if (watched == ui->userNameLabel)
        {
            if (event->type() == QEvent::KeyPress)
            {
                QKeyEvent* keyevt = static_cast<QKeyEvent*>(event);
                if ((keyevt->key() == Qt::Key_Return) ||
                    (keyevt->key() == Qt::Key_Escape) ||
                    (keyevt->key() == Qt::Key_Enter))   // Qt::Key_Return是大键盘的回车键 Qt::Key_Enter是小键盘的回车键
                {
                    QString str_name =ui->userNameLabel->text().remove(QRegExp("\\s"));
                    qlonglong uid = getuid();
                    QDBusInterface user("org.freedesktop.Accounts",
                                        "/org/freedesktop/Accounts",
                                        "org.freedesktop.Accounts",
                                        QDBusConnection::systemBus());
                    QDBusMessage result = user.call("FindUserById", uid);
                    QString userpath = result.arguments().value(0).value<QDBusObjectPath>().path();

                    QDBusInterface userreal("org.freedesktop.Accounts",
                                            userpath,
                                            "org.freedesktop.Accounts.User",
                                            QDBusConnection::systemBus());

                    ui->userNameLabel->setCursorPosition(0);
                    if (str_name != NULL) {
                        userreal.call("SetRealName",m_userName);
                        allUserInfoMap.find(my_name).value().realname = m_userName;
                        QFontMetrics fontMetrics(ui->userNameLabel->font());
                        int fontSize = fontMetrics.width(ui->userNameLabel->text());
                        if (fontSize > 180) {
                            QString str = fontMetrics.elidedText(ui->userNameLabel->text(), Qt::ElideRight, 180);
                            ui->userNameLabel->setToolTip(m_userName);
                            ui->userNameLabel->blockSignals(true);
                            ui->userNameLabel->setText(str);
                            ui->userNameLabel->blockSignals(false);
                        } else {
                            ui->userNameLabel->blockSignals(true);
                            ui->userNameLabel->setText(ui->userNameLabel->text());
                            ui->userNameLabel->blockSignals(false);
                            ui->userNameLabel->setToolTip("");
                        }
                        ui->editBtn->show();
                        ui->userNameLabel->setReadOnly(true);
                        ui->userNameLabel->deselect();
                    } else {
                        qDebug()<<"回车";
                        enter = true;
                        MessageBoxPowerIntel *messageBoxpower = new MessageBoxPowerIntel();
                        messageBoxpower->exec();
                        QFontMetrics fontMetrics(ui->userNameLabel->font());
                        int fontSize = fontMetrics.width(userreal.property("RealName").toString());
                        if (fontSize > 180) {
                            QString str = fontMetrics.elidedText(userreal.property("RealName").toString(), Qt::ElideRight, 180);
                            ui->userNameLabel->setToolTip(userreal.property("RealName").toString());
                            ui->userNameLabel->blockSignals(true);
                            ui->userNameLabel->setText(str);
                            ui->userNameLabel->blockSignals(false);
                        } else {
                            ui->userNameLabel->blockSignals(true);
                            ui->userNameLabel->setText(userreal.property("RealName").toString());
                            ui->userNameLabel->blockSignals(false);
                            ui->userNameLabel->setToolTip("");
                        }
                        ui->editBtn->show();
                        ui->userNameLabel->setReadOnly(true);
                        ui->userNameLabel->deselect();
                    }
                }
            }
            else if (event->type() == QEvent::FocusOut)
            {

                QString str_name =ui->userNameLabel->text().remove(QRegExp("\\s"));
                qlonglong uid = getuid();
                QDBusInterface user("org.freedesktop.Accounts",
                                    "/org/freedesktop/Accounts",
                                    "org.freedesktop.Accounts",
                                    QDBusConnection::systemBus());
                QDBusMessage result = user.call("FindUserById", uid);
                QString userpath = result.arguments().value(0).value<QDBusObjectPath>().path();

                QDBusInterface userreal("org.freedesktop.Accounts",
                                        userpath,
                                        "org.freedesktop.Accounts.User",
                                        QDBusConnection::systemBus());
                if (str_name != NULL) {
                    ui->userNameLabel->setCursorPosition(0);
                    userreal.call("SetRealName",m_userName);
                    allUserInfoMap.find(my_name).value().realname = m_userName;
                    QFontMetrics fontMetrics(ui->userNameLabel->font());
                    int fontSize = fontMetrics.width(ui->userNameLabel->text());
                    if (fontSize > 180) {
                        QString str = fontMetrics.elidedText(ui->userNameLabel->text(), Qt::ElideRight, 180);
                        ui->userNameLabel->setToolTip(m_userName);
                        ui->userNameLabel->blockSignals(true);
                        ui->userNameLabel->setText(str);
                        ui->userNameLabel->blockSignals(false);
                    } else {
                        ui->userNameLabel->blockSignals(true);
                        ui->userNameLabel->setText(ui->userNameLabel->text());
                        ui->userNameLabel->blockSignals(false);
                        ui->userNameLabel->setToolTip("");
                    }
                    ui->editBtn->show();
                    ui->userNameLabel->setReadOnly(true);
                    ui->userNameLabel->deselect();
                } else {
                    qDebug()<<"失去焦点";
                    ui->userNameLabel->setCursorPosition(0);
                    if (enter) {
                        enter = false;
                    } else {
                        MessageBoxPowerIntel *messageBoxpower = new MessageBoxPowerIntel;
                        messageBoxpower->exec();
                        QFontMetrics fontMetrics(ui->userNameLabel->font());
                        int fontSize = fontMetrics.width(userreal.property("RealName").toString());
                        if (fontSize > 180) {
                            QString str = fontMetrics.elidedText(userreal.property("RealName").toString(), Qt::ElideRight, 180);
                            ui->userNameLabel->blockSignals(true);
                            ui->userNameLabel->setText(str);
                            ui->userNameLabel->blockSignals(false);
                            ui->userNameLabel->setToolTip(userreal.property("RealName").toString());
                        } else {
                            ui->userNameLabel->blockSignals(true);
                            ui->userNameLabel->setText(userreal.property("RealName").toString());
                            ui->userNameLabel->blockSignals(false);
                            ui->userNameLabel->setToolTip("");
                        }
                        ui->editBtn->show();
                        ui->userNameLabel->setReadOnly(true);
                        ui->userNameLabel->deselect();
                    }
                }

            }
        }
    return QObject::eventFilter(watched, event);
}

bool UserInfoIntel::getAutomaticLogin(QString username) {

    QString filename = "/etc/lightdm/lightdm.conf";
    autoSettings = new QSettings(filename, QSettings::IniFormat);
    autoSettings->beginGroup("SeatDefaults");

    QString autoUser = autoSettings->value("autologin-user", "").toString();

    autoSettings->endGroup();

    return autoUser == username ? true : false;
}
