#include "userinfo.h"
#include "ui_userinfo.h"

#include <QDebug>

#define DEFAULTFACE "/usr/share/pixmaps/faces/stock_person.png"

UserInfo::UserInfo()
{
    ui = new Ui::UserInfo;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("userinfo");
    pluginType = ACCOUNT;

    sysdispatcher = new SystemDbusDispatcher;

    pwdSignalMapper = new QSignalMapper(this);
    faceSignalMapper = new QSignalMapper(this);
    typeSignalMapper = new QSignalMapper(this);
    delSignalMapper = new QSignalMapper(this);

    faceSize = QSize(64, 64);
    itemSize = QSize(160, 80);

    get_all_users();
    ui_component_init();
    ui_status_init();

    connect(sysdispatcher, SIGNAL(createuserdone(QString)), this, SLOT(create_user_done_slot(QString)));
    connect(sysdispatcher, SIGNAL(deleteuserdone(QString)), this, SLOT(delete_user_done_slot(QString)));
}

UserInfo::~UserInfo()
{
    delete ui;
}

QString UserInfo::get_plugin_name(){
    return pluginName;
}

int UserInfo::get_plugin_type(){
    return pluginType;
}

QWidget * UserInfo::get_plugin_ui(){
    return pluginWidget;
}

QString UserInfo::accounttype_enum_to_string(int id){
    QString type;
    if (id == STANDARDUSER)
        type = tr("standard user");
    else if (id == ADMINISTRATOR)
        type = tr("administrator");
    else if (id == ROOT)
        type = tr("root");
    return type;
}

QString UserInfo::login_status_bool_to_string(bool status){
    QString logined;
    if (status)
        logined = tr("logined");
    else
        logined = tr("unlogined");
    return logined;
}

void UserInfo::get_all_users(){
    QStringList objectpaths = sysdispatcher->list_cached_users();

    //清空allUserInfoMap
    allUserInfoMap.clear();
    adminnum = 0; //reset

    for (QString objectpath : objectpaths){
        UserInfomation user;
        user = init_user_info(objectpath);
        allUserInfoMap.insert(user.username, user);
    }
    if (!getuid())
        init_root_info();
}

UserInfomation UserInfo::init_user_info(QString objpath){
    UserInfomation user;

    //default set
    user.current = false;
    user.logined = false;
    user.autologin = false;

//    QDBusInterface * iface = new QDBusInterface("org.freedesktop.Accounts",
//                                         objpath,
//                                         "org.freedesktop.Accounts.User",
//                                         QDBusConnection::systemBus());
    QDBusInterface * iproperty = new QDBusInterface("org.freedesktop.Accounts",
                                            objpath,
                                            "org.freedesktop.DBus.Properties",
                                            QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply = iproperty->call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()){
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        user.username = propertyMap.find("UserName").value().toString();
        if (user.username == QString(g_get_user_name())){
            user.current = true;
            user.logined = true;
        }
        user.accounttype = propertyMap.find("AccountType").value().toInt();
        if (user.accounttype == ADMINISTRATOR)
            adminnum++;
        user.iconfile = propertyMap.find("IconFile").value().toString();
        user.passwdtype = propertyMap.find("PasswordMode").value().toInt();
        user.uid = propertyMap.find("Uid").value().toInt();
        user.autologin = propertyMap.find("AutomaticLogin").value().toBool();
        user.objpath = objpath;
    }
    else
        qDebug() << "reply failed";

    delete iproperty;

    return user;

}

void UserInfo::init_root_info(){
//    currentUserInfo->accounttype = 2; //root
//    currentUserInfo->current = true;
//    currentUserInfo->autologin = false;
//    currentUserInfo->username = g_get_user_name();
//    currentUserInfo->iconfile = "";
//    if (currentUserInfo->username == "root")
//        currentUserInfo->logined = true;
//    else
//        currentUserInfo->logined = false;
//    currentUserInfo->uid = 0;
}

void UserInfo::ui_status_init(){

    QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
    for (; it != allUserInfoMap.end(); it++){
        UserInfomation user = (UserInfomation) it.value();

        if (user.username == QString(g_get_user_name())){ //当前用户
            ui->faceLabel->setPixmap(QPixmap(user.iconfile).scaled(QSize(64, 64)));
            ui->usernameLabel->setText(user.username);
            ui->accounttypeLabel->setText(accounttype_enum_to_string(user.accounttype));
            ui->loginLabel->setText(login_status_bool_to_string(user.logined));
        }
        else{ //其他用户
            QMap<QString, QListWidgetItem *>::iterator itemit = otherItemMap.find(user.username);

            if (itemit != otherItemMap.end()){
                QWidget * widget = ui->listWidget->itemWidget((QListWidgetItem *) itemit.value());
                QToolButton * button = widget->findChild<QToolButton *>(user.username);
                // 获取账户类型
                QString type = accounttype_enum_to_string(user.accounttype);
                // 获取登录状态
                QString logined = login_status_bool_to_string(user.logined);
                button->setIcon(QIcon(user.iconfile));
                button->setText(QString("%1\n%2\n%3").arg(user.username, type, logined));
            }
            else
                qDebug() << QString(it.key()) << "QToolBtn init failed!";
        }
    }
}

void UserInfo::build_item_with_widget(UserInfomation user){

    QWidget * otherWidget = new QWidget();
    otherWidget->setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout * otherVerLayout = new QVBoxLayout(otherWidget);
    QToolButton * otherToolBtn = new QToolButton(otherWidget);
    otherToolBtn->setObjectName(user.username);
    otherToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
//            otherToolBtn->setIcon(QIcon(user.iconfile));
    otherToolBtn->setIconSize(faceSize);
//            otherToolBtn->setText(QString("%1\n%2\n%3").arg(user.username, type, logined));
    otherbtnMap.insert(user.username, otherToolBtn);

    QMenu * menu = new QMenu(otherToolBtn);
    QAction * chpwd = menu->addAction(tr("change pwd"));
    QAction * chface = menu->addAction(tr("change face"));
    QAction * chtype = menu->addAction(tr("change accounttype"));
    QAction * deluer = menu->addAction(tr("delete user"));
    connect(chpwd, SIGNAL(triggered()), pwdSignalMapper, SLOT(map()));
    connect(chface, SIGNAL(triggered()), faceSignalMapper, SLOT(map()));
    connect(chtype, SIGNAL(triggered()), typeSignalMapper, SLOT(map()));
    connect(deluer, SIGNAL(triggered()), delSignalMapper, SLOT(map()));

    pwdSignalMapper->setMapping(chpwd, user.username);
    faceSignalMapper->setMapping(chface, user.username);
    typeSignalMapper->setMapping(chtype, user.username);
    delSignalMapper->setMapping(deluer, user.username);

    otherToolBtn->setMenu(menu);
    otherToolBtn->setPopupMode(QToolButton::InstantPopup);
//            otherToolBtn->setStyleSheet("QToolButton::menu-indicator{image:none;}");
    otherVerLayout->addWidget(otherToolBtn);
    otherWidget->setLayout(otherVerLayout);

    QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(itemSize);
    item->setData(Qt::UserRole, QVariant(user.objpath));
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, otherWidget);

    otherItemMap.insert(user.username, item);
}

void UserInfo::ui_component_init(){

    ui->listWidget->setViewMode(QListView::IconMode);
    ui->listWidget->setSpacing(5);

    //设置创建用户按钮
    QWidget * newuserWidget = new QWidget();
    newuserWidget->setAttribute(Qt::WA_DeleteOnClose);
    QVBoxLayout * newVLayout = new QVBoxLayout(newuserWidget);
    QToolButton * newToolBtn = new QToolButton(newuserWidget);
    newToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    newToolBtn->setIcon(QIcon(":/more.png"));
    newToolBtn->setIconSize(faceSize);
    newToolBtn->setText(tr("add new user"));
    newVLayout->addWidget(newToolBtn);
    newuserWidget->setLayout(newVLayout);
    connect(newToolBtn, SIGNAL(clicked()), this, SLOT(show_create_user_dialog_slot()));

    QListWidgetItem * newitem = new QListWidgetItem(ui->listWidget);
    newitem->setSizeHint(itemSize);
    newitem->setData(Qt::UserRole, QVariant(""));
    ui->listWidget->addItem(newitem);
    ui->listWidget->setItemWidget(newitem, newuserWidget);

    //设置其他用户按钮
    if (allUserInfoMap.count() > 1){

        QMap<QString, UserInfomation>::iterator it = allUserInfoMap.begin();
        for (; it != allUserInfoMap.end(); it++){
            UserInfomation user = (UserInfomation)it.value();

            //当前用户
            if (user.current){
                connect(ui->chpwdPushBtn, SIGNAL(clicked(bool)), pwdSignalMapper, SLOT(map()));
                connect(ui->chfacePushBtn, SIGNAL(clicked(bool)), faceSignalMapper, SLOT(map()));
                connect(ui->chtypePushBtn, SIGNAL(clicked(bool)), typeSignalMapper, SLOT(map()));

                pwdSignalMapper->setMapping(ui->chpwdPushBtn, user.username);
                faceSignalMapper->setMapping(ui->chfacePushBtn, user.username);
                typeSignalMapper->setMapping(ui->chtypePushBtn, user.username);
                continue;
            }

            build_item_with_widget(user);
        }
        connect(pwdSignalMapper, SIGNAL(mapped(QString)), this, SLOT(show_change_pwd_dialog_slot(QString)));
        connect(faceSignalMapper, SIGNAL(mapped(QString)), this, SLOT(show_change_face_dialog_slot(QString)));
        connect(typeSignalMapper, SIGNAL(mapped(QString)), this, SLOT(show_change_accounttype_dialog_slot(QString)));
        connect(delSignalMapper, SIGNAL(mapped(QString)), this, SLOT(show_del_user_dialog_slot(QString)));
    }
}

void UserInfo::show_create_user_dialog_slot(){
    allUserInfoMap.keys();
    QStringList usersStringList;

    for (QVariant tmp : allUserInfoMap.keys()){
        usersStringList << tmp.toString();
    }

    CreateUserDialog * dialog = new CreateUserDialog(usersStringList);
    dialog->set_face_label(DEFAULTFACE);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, SIGNAL(user_info_send(QString,QString,QString,int)), this, SLOT(create_user_slot(QString,QString,QString,int)));
    dialog->exec();
}

void UserInfo::create_user_slot(QString username, QString pwd, QString pin, int atype){
    sysdispatcher->create_user(username, "", atype);
}

void UserInfo::create_user_done_slot(QString objpath){
    //设置默认头像
    UserDispatcher * userdispatcher  = new UserDispatcher(objpath);
    userdispatcher->change_user_face(DEFAULTFACE);

    UserInfomation user;
    user = init_user_info(objpath);

    build_item_with_widget(user);

    //刷新界面
    get_all_users();
    ui_status_init();

}

void UserInfo::show_del_user_dialog_slot(QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    DelUserDialog * dialog = new DelUserDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->set_face_label(user.iconfile);
    dialog->set_username_label(user.username);
    connect(dialog, SIGNAL(removefile_send(bool, QString)), this, SLOT(delete_user_slot(bool, QString)));
    dialog->exec();
}

void UserInfo::delete_user_slot(bool removefile, QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    sysdispatcher->delete_user(user.uid, removefile);
}

void UserInfo::delete_user_done_slot(QString objpath){
    QMap<QString, QListWidgetItem *>::iterator it = otherItemMap.begin();
    QString del;

    for (; it != otherItemMap.end(); it++){
        QListWidgetItem * item = (QListWidgetItem *) it.value();
        if (item->data(Qt::UserRole).toString() == objpath){
            del = QString(it.key());
            ui->listWidget->takeItem(ui->listWidget->row(item));
            break;
        }
    }

    if (otherItemMap.contains(del))
        otherItemMap.remove(del);
}

void UserInfo::show_change_accounttype_dialog_slot(QString username = g_get_user_name()){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    ChangeTypeDialog * dialog = new ChangeTypeDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->set_face_label(user.iconfile);
    dialog->set_username_label(user.username);
    dialog->set_account_type_label(accounttype_enum_to_string(user.accounttype));
    dialog->set_current_account_type(user.accounttype);
    connect(dialog, SIGNAL(type_send(int,QString)), this, SLOT(change_accounttype_slot(int,QString)));
    dialog->exec();
}

void UserInfo::change_accounttype_slot(int atype, QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    UserDispatcher * userdispatcher  = new UserDispatcher(user.objpath);
    userdispatcher->change_user_type(atype);

    //刷新界面
    get_all_users();
    ui_status_init();
}

void UserInfo::change_accounttype_done_slot(){
    qDebug() << "**********";
}

void UserInfo::show_change_face_dialog_slot(QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    ChangeFaceDialog * dialog = new ChangeFaceDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->set_face_label(user.iconfile);
    dialog->set_username_label(user.username);
    dialog->set_account_type_label(accounttype_enum_to_string(user.accounttype));
    dialog->set_face_list_status(user.iconfile);
    connect(dialog, SIGNAL(face_file_send(QString,QString)), this, SLOT(change_face_slot(QString,QString)));
    dialog->exec();
}

void UserInfo::change_face_slot(QString facefile, QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    UserDispatcher * userdispatcher  = new UserDispatcher(user.objpath);
    userdispatcher->change_user_face(facefile);

    get_all_users();
    ui_status_init();
}

void UserInfo::show_change_pwd_dialog_slot(QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    ChangePwdDialog * dialog = new ChangePwdDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->set_faceLabel(user.iconfile);
    dialog->set_usernameLabel(user.username);
    connect(dialog, SIGNAL(passwd_send(QString, QString)), this, SLOT(change_pwd_slot(QString, QString)));
    dialog->exec();
}

void UserInfo::change_pwd_slot(QString pwd, QString username){
    UserInfomation user = (UserInfomation)(allUserInfoMap.find(username).value());

    UserDispatcher * userdispatcher  = new UserDispatcher(user.objpath);
    QString result = userdispatcher->change_user_pwd(pwd, "");
}
