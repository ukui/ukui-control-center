#include "createusernew.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QFrame>
#include <QPushButton>
#include <QDir>
#include <QButtonGroup>
#include <QDBusInterface>
#include <QDBusReply>
#include <QCoreApplication>
#include <QDebug>
#include <QMouseEvent>

#include <kylin-chkname.h>

#include "../../../shell/utils/utils.h"

#define USER_LENGTH 32
#define NICKNAME_LENGTH 32
#define DEFAULTFACECOMMUNITY "/usr/share/ukui/faces/01-default-community.png"
#define DEFAULTFACECOMMERCIAL "/usr/share/ukui/faces/01-default-commercial.png"
#define DEFAULTFACE (Utils::isCommunity())?DEFAULTFACECOMMUNITY:DEFAULTFACECOMMERCIAL

CreateUserNew::CreateUserNew(QStringList allUsers, QWidget *parent) :
    QDialog(parent),
    _allNames(allUsers)
{
    setFixedSize(QSize(520, 572));
    setWindowTitle(tr("CreateUserNew"));

    //确认密码检测是否开启
    makeSurePwdNeedCheck();

    initUI();
    setConnect();

    refreshConfirmBtnStatus();
}

CreateUserNew::~CreateUserNew()
{
}

void CreateUserNew::initUI(){

    typeBtnGroup = new QButtonGroup;

    usernameLabel = new QLabel;
    usernameLabel->setFixedSize(100, 24);
    usernameLabel->setText(tr("UserName"));
    usernameTipLabel = new QLabel();
    QFont ft;
    ft.setPixelSize(14);
    usernameTipLabel->setFont(ft);
    usernameTipLabel->setFixedSize(QSize(340, 24));
    usernameTipLabel->setStyleSheet("color:red;");

    nicknameLabel = new QLabel;
    nicknameLabel->setFixedSize(100, 24);
    nicknameLabel->setText(tr("NickName"));
    nicknameTipLabel = new QLabel();
    nicknameTipLabel->setFont(ft);
    nicknameTipLabel->setFixedSize(QSize(340, 24));
    nicknameTipLabel->setStyleSheet("color:red;");

    hostnameLabel = new QLabel(tr("HostName"));
    hostnameLabel->setFixedSize(100, 24);
    hostnameTipLabel = new QLabel();
    hostnameTipLabel->setFont(ft);
    hostnameTipLabel->setFixedSize(QSize(340, 24));
    hostnameTipLabel->setStyleSheet("color:red;");

    newPwdLabel = new QLabel;
    newPwdLabel->setFixedSize(100, 24);
    newPwdLabel->setText(tr("Pwd"));
    newpwdTipLabel = new QLabel();
    newpwdTipLabel->setFont(ft);
    newpwdTipLabel->setFixedSize(QSize(340, 24));
    newpwdTipLabel->setStyleSheet("color:red;");

    surePwdLabel = new QLabel;
    surePwdLabel->setFixedSize(100, 24);
    surePwdLabel->setText(tr("SurePwd"));
    tipLabel = new QLabel;
    tipLabel->setFont(ft);
    tipLabel->setFixedSize(340, 36);
    tipLabel->setStyleSheet("color:red;");

    usernameLineEdit = new QLineEdit;
    usernameLineEdit->setFixedSize(340, 36);
    nicknameLineEdit = new QLineEdit;
    nicknameLineEdit->setFixedSize(340, 36);
    hostnameLineEdit = new QLineEdit;
    hostnameLineEdit->setFixedSize(340, 36);
    newPwdLineEdit = new QLineEdit;
    newPwdLineEdit->setFixedSize(340, 36);
    newPwdLineEdit->setEchoMode(QLineEdit::Password);
    surePwdLineEdit = new QLineEdit;
    surePwdLineEdit->setFixedSize(340, 36);
    surePwdLineEdit->setEchoMode(QLineEdit::Password);

    //用户名
    usernameHorLayout = new QHBoxLayout;
    usernameHorLayout->setSpacing(8);
    usernameHorLayout->setContentsMargins(0, 0, 0, 0);
    usernameHorLayout->addWidget(usernameLabel);
    usernameHorLayout->addStretch();
    usernameHorLayout->addWidget(usernameLineEdit);
    usernameTipHorLayout = new QHBoxLayout;
    usernameTipHorLayout->setSpacing(0);
    usernameTipHorLayout->setMargin(0);
    usernameTipHorLayout->addStretch();
    usernameTipHorLayout->addWidget(usernameTipLabel);

    //用户昵称
    nicknameHorLayout = new QHBoxLayout;
    nicknameHorLayout->setSpacing(8);
    nicknameHorLayout->setMargin(0);
    nicknameHorLayout->addWidget(nicknameLabel);
    nicknameHorLayout->addStretch();
    nicknameHorLayout->addWidget(nicknameLineEdit);
    nicknameTipHorLayout = new QHBoxLayout;
    nicknameTipHorLayout->setSpacing(0);
    nicknameTipHorLayout->setMargin(0);
    nicknameTipHorLayout->addStretch();
    nicknameTipHorLayout->addWidget(nicknameTipLabel);

    // 计算机名
    hostnameHorLayout = new QHBoxLayout;
    hostnameHorLayout->setSpacing(8);
    hostnameHorLayout->setMargin(0);
    hostnameHorLayout->addWidget(hostnameLabel);
    hostnameHorLayout->addStretch();
    hostnameHorLayout->addWidget(hostnameLineEdit);
    hostnameTipHorLayout = new QHBoxLayout;
    hostnameTipHorLayout->setSpacing(0);
    hostnameTipHorLayout->setMargin(0);
    hostnameTipHorLayout->addStretch();
    hostnameTipHorLayout->addWidget(hostnameTipLabel);

    //密码
    newPwdHorLayout = new QHBoxLayout;
    newPwdHorLayout->setSpacing(8);
    newPwdHorLayout->setMargin(0);
    newPwdHorLayout->addWidget(newPwdLabel);
    newPwdHorLayout->addStretch();
    newPwdHorLayout->addWidget(newPwdLineEdit);
    newPwdTipHorLayout = new QHBoxLayout;
    newPwdTipHorLayout->setSpacing(0);
    newPwdTipHorLayout->setMargin(0);
    newPwdTipHorLayout->addStretch();
    newPwdTipHorLayout->addWidget(newpwdTipLabel);

    //确认密码
    surePwdHorLayout = new QHBoxLayout;
    surePwdHorLayout->setSpacing(8);
    surePwdHorLayout->setMargin(0);
    surePwdHorLayout->addWidget(surePwdLabel);
    surePwdHorLayout->addStretch();
    surePwdHorLayout->addWidget(surePwdLineEdit);

    //提示信息
    tipHorLayout = new QHBoxLayout;
    tipHorLayout->setSpacing(0);
    tipHorLayout->setMargin(0);
    tipHorLayout->addStretch();
    tipHorLayout->addWidget(tipLabel);

    //"选择账户类型"
    typeNoteLabel = new QLabel;
    typeNoteLabel->setFixedHeight(24);
    typeNoteLabel->setText(tr("Select Type"));

    typeNoteHorLayout = new QHBoxLayout;
    typeNoteHorLayout->setSpacing(0);
    typeNoteHorLayout->setMargin(0);
    typeNoteHorLayout->addWidget(typeNoteLabel);
    typeNoteHorLayout->addStretch();

    adminLabel = new QLabel;
    adminLabel->setText(tr("Administrator"));
    adminLabel->setFixedHeight(24);
    adminDetailLabel = new QLabel;
    adminDetailLabel->setText(tr("Users can make any changes they need"));
    standardLabel = new QLabel;
    standardLabel->setText(tr("Standard User"));
    standardLabel->setFixedHeight(24);
    standardDetailLabel = new QLabel;
    standardDetailLabel->setText(tr("Users cannot change system settings"));

    adminRadioBtn = new QRadioButton;
    adminRadioBtn->setFixedSize(QSize(16,16));
    standardRadioBtn = new QRadioButton;
    standardRadioBtn->setFixedSize(QSize(16,16));

    typeBtnGroup->addButton(adminRadioBtn, 1);
    typeBtnGroup->addButton(standardRadioBtn, 0);

    adminRadioBtn->setChecked(true);

    //管理员RadioButton布局
    admin1VerLayout = new QVBoxLayout;
    admin1VerLayout->setSpacing(0);
    admin1VerLayout->setContentsMargins(0, 20, 0, 0);
    admin1VerLayout->addWidget(adminRadioBtn);
    admin1VerLayout->addStretch();

    admin2VerLayout = new QVBoxLayout;
    admin2VerLayout->setSpacing(0);
    admin2VerLayout->setMargin(0);
    admin2VerLayout->addStretch();
    admin2VerLayout->addWidget(adminLabel);
    admin2VerLayout->addWidget(adminDetailLabel);
    admin2VerLayout->addStretch();

    adminHorLayout = new QHBoxLayout;
    adminHorLayout->setSpacing(8);
    adminHorLayout->setContentsMargins(16, 0, 0, 0);
    adminHorLayout->addLayout(admin1VerLayout);
    adminHorLayout->addLayout(admin2VerLayout);
    adminHorLayout->addStretch();

    //管理员区域
    adminFrame = new QFrame;
    adminFrame->setMinimumSize(QSize(473, 78));
    adminFrame->setMaximumSize(QSize(16777215, 78));
    adminFrame->setFrameShape(QFrame::Box);
    adminFrame->setFrameStyle(QFrame::Plain);
    adminFrame->setLayout(adminHorLayout);
    adminFrame->installEventFilter(this);

    //标准用户RadioButton布局
    standard1VerLayout = new QVBoxLayout;
    standard1VerLayout->setSpacing(0);
    standard1VerLayout->setContentsMargins(0, 20, 0, 0);
    standard1VerLayout->addWidget(standardRadioBtn);
    standard1VerLayout->addStretch();

    standard2VerLayout = new QVBoxLayout;
    standard2VerLayout->setSpacing(0);
    standard2VerLayout->setMargin(0);
    standard2VerLayout->addStretch();
    standard2VerLayout->addWidget(standardLabel);
    standard2VerLayout->addWidget(standardDetailLabel);
    standard2VerLayout->addStretch();

    standardHorLayout = new QHBoxLayout;
    standardHorLayout->setSpacing(8);
    standardHorLayout->setContentsMargins(16, 0, 0, 0);
    standardHorLayout->addLayout(standard1VerLayout);
    standardHorLayout->addLayout(standard2VerLayout);
    standardHorLayout->addStretch();

    //标准用户区域
    standardFrame = new QFrame;
    standardFrame->setMinimumSize(QSize(473, 80));
    standardFrame->setMaximumSize(QSize(16777215, 80));
    standardFrame->setFrameShape(QFrame::Box);
    standardFrame->setFrameStyle(QFrame::Plain);
    standardFrame->setLineWidth(1);
    standardFrame->setLayout(standardHorLayout);
    standardFrame->installEventFilter(this);

    cancelBtn = new QPushButton;
    cancelBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    cancelBtn->setText(tr("Cancel"));
    confirmBtn = new QPushButton;
    confirmBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    confirmBtn->setText(tr("Confirm"));

    bottomHorLayout = new QHBoxLayout;
    bottomHorLayout->setSpacing(16);
    bottomHorLayout->setMargin(0);
    bottomHorLayout->addStretch();
    bottomHorLayout->addWidget(cancelBtn);
    bottomHorLayout->addWidget(confirmBtn);

    mainVerLayout = new QVBoxLayout;
    mainVerLayout->setContentsMargins(24, 28, 24, 16);
    mainVerLayout->setSpacing(0);
    mainVerLayout->addLayout(usernameHorLayout);
    mainVerLayout->addLayout(usernameTipHorLayout);

    mainVerLayout->addLayout(nicknameHorLayout);
    mainVerLayout->addLayout(nicknameTipHorLayout);

//    mainVerLayout->addLayout(hostnameHorLayout);
//    mainVerLayout->addLayout(hostnameTipHorLayout);

    mainVerLayout->addLayout(newPwdHorLayout);
    mainVerLayout->addLayout(newPwdTipHorLayout);

    mainVerLayout->addLayout(surePwdHorLayout);
    mainVerLayout->addLayout(tipHorLayout);
    mainVerLayout->addLayout(typeNoteHorLayout);
    mainVerLayout->addWidget(adminFrame);
    mainVerLayout->addWidget(standardFrame);
    mainVerLayout->addSpacing(24);
    mainVerLayout->addLayout(bottomHorLayout);

    usernameLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    nicknameLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    newPwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    surePwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    usernameLineEdit->installEventFilter(this);
    nicknameLineEdit->installEventFilter(this);
    newPwdLineEdit->installEventFilter(this);
    surePwdLineEdit->installEventFilter(this);

    setLayout(mainVerLayout);
}

void CreateUserNew::setConnect(){

    connect(usernameLineEdit, &QLineEdit::textEdited, this, [=](QString txt){
        if (usernameLineEdit->text().length() > USER_LENGTH) {
            usernameLineEdit->setText(oldName);
        } else {
            oldName = txt;
        }
        nameLegalityCheck(txt);
    });

    connect(nicknameLineEdit, &QLineEdit::textEdited, this, [=](QString txt){

        nameLegalityCheck2(txt);

    });

    connect(newPwdLineEdit, &QLineEdit::textEdited, this, [=](QString txt){
        pwdLegalityCheck(txt);
    });

    connect(surePwdLineEdit, &QLineEdit::textEdited, this, [=](QString txt){
        if (!txt.isEmpty() && txt != newPwdLineEdit->text()){
            surePwdTip = tr("Inconsistency with pwd");
        } else {
            surePwdTip = "";
        }

        setCunTextDynamic(tipLabel, surePwdTip);

        refreshConfirmBtnStatus();
    });

    connect(cancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });

    connect(confirmBtn, &QPushButton::clicked, this, [=]{
        QDBusInterface tmpSysinterface("com.control.center.qt.systemdbus",
                                       "/",
                                       "com.control.center.interface",
                                       QDBusConnection::systemBus());

        //底层创建用户存在延时，先隐藏掉创建用户界面
        this->hide();

        if (tmpSysinterface.isValid()){

            tmpSysinterface.call("setPid", QCoreApplication::applicationPid());
            tmpSysinterface.call("createUser", usernameLineEdit->text(), nicknameLineEdit->text(), typeBtnGroup->checkedId(), DEFAULTFACE, newPwdLineEdit->text());

        } else {
            qCritical() << "Create Client Interface Failed When : " << QDBusConnection::systemBus().lastError();
        }

        close();

    });
}

void CreateUserNew::makeSurePwdNeedCheck(){

#ifdef ENABLEPQ
    int ret;
    void *auxerror;
    char buf[255];

    settings = pwquality_default_settings();
    if (settings == NULL) {
        enablePwdQuality = false;
        qDebug() << "init pwquality settings failed";
    } else {
        enablePwdQuality = true;
    }

    ret = pwquality_read_config(settings, PWCONF, &auxerror);
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

#else
    enablePwdQuality = false;
#endif

}

void CreateUserNew::refreshConfirmBtnStatus(){
    if (usernameLineEdit->text().isEmpty() ||
            nicknameLineEdit->text().isEmpty() ||
            newPwdLineEdit->text().isEmpty() ||
            surePwdLineEdit->text().isEmpty() ||
            !userNameTip.isEmpty() || !nickNameTip.isEmpty() || !newPwdTip.isEmpty() || !surePwdTip.isEmpty())
        confirmBtn->setEnabled(false);
    else
        confirmBtn->setEnabled(true);

    if (((usernameLineEdit->text().length() >= USER_LENGTH) || (nicknameLineEdit->text().length() >= NICKNAME_LENGTH))
            && newPwdTip.isEmpty() && surePwdTip.isEmpty()
            && !usernameLineEdit->text().isEmpty()
            && !nicknameLineEdit->text().isEmpty()
            && !newPwdLineEdit->text().isEmpty()
            && !surePwdLineEdit->text().isEmpty()) {
        confirmBtn->setEnabled(true);
    }
}

bool CreateUserNew::isHomeUserExists(QString username)
{
    if (username.isEmpty())
        return false;

    QStringList homeList;
    QDir dir("/home");
    if (dir.exists())
    {
        homeList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

        if (homeList.contains(username))
            return true;
    }

    return false;
}

bool CreateUserNew::nameTraverse(QString username){
    QString::const_iterator cit = NULL;
    for (cit = username.cbegin(); cit < username.cend(); cit++){
        QString str = *cit;
        if (str.contains(QRegExp("[a-z]"))){
        } else if (str.contains(QRegExp("[0-9]"))){
        } else if (str.contains("_")){
        } else{
            return false;
        }
    }
    return true;
}

void CreateUserNew::nameLegalityCheck2(QString nickname){
    if (nickname.isEmpty()){
        nickNameTip = tr("The nick name cannot be empty");
    } else if (_allNames.contains(nickname)){
        nickNameTip = tr("nickName already in use.");
    } else if(nickname.length() > NICKNAME_LENGTH) {
        nicknameLineEdit->setText(oldNickName);
        nickNameTip = tr("nickName length must less than %1 letters!").arg(NICKNAME_LENGTH);
    } else {
        oldNickName = nickname;
        nickNameTip = tr("");
    }

    setCunTextDynamic(nicknameTipLabel, nickNameTip);

    refreshConfirmBtnStatus();

}

void CreateUserNew::nameLegalityCheck(QString username){

    int isValued = kylin_username_check(username.toLatin1().data(), 1);

    if (isValued != 0) {
        qDebug() <<"err_num:" <<  isValued << ";" << kylin_username_strerror(isValued);
        userNameTip = kylin_username_strerror(isValued);
    } else {
        userNameTip = "";
    }

    if (isHomeUserExists(username) && userNameTip.isEmpty()) {
        userNameTip = tr("Username's folder exists, change another one");
    }

    if (!newPwdLineEdit->text().isEmpty()) {
        pwdLegalityCheck(newPwdLineEdit->text());
    }

    setCunTextDynamic(usernameTipLabel, userNameTip);

    refreshConfirmBtnStatus();
}

bool CreateUserNew::checkCharLegitimacy(QString password){
    //密码不能包含非标准字符
    foreach (QChar ch, password){
        if (int(ch.toLatin1() <= 0 || int(ch.toLatin1()) > 127)){
            return false;
        }
    }
    return true;
}

void CreateUserNew::pwdLegalityCheck(QString pwd){

    if (!checkCharLegitimacy(pwd)){
        newPwdTip = tr("Contains illegal characters!");
    } else {
        if (enablePwdQuality){
    #ifdef ENABLEPQ
            void * auxerror;
            int ret;
            const char * msg;
            char buf[256];

            QByteArray ba = pwd.toLatin1();
            QByteArray ba1 = usernameLineEdit->text().toLatin1();

            ret = pwquality_check(settings, ba.data(), NULL, ba1.data(), &auxerror);
            if (ret < 0 && pwd.length() > 0){
                msg = pwquality_strerror(buf, sizeof(buf), ret, auxerror);
                newPwdTip = QString(msg);
            } else {
                newPwdTip = "";
            }
    #endif
        } else {
            newPwdTip = "";
        }
    }

    //防止先输入确认密码，再输入密码后surePwdTip无法刷新
    if (!surePwdLineEdit->text().isEmpty()){
        if (newPwdLineEdit->text() == surePwdLineEdit->text()) {
            surePwdTip = "";
        } else {
            surePwdTip = tr("Inconsistency with pwd");
        }
    }

    setCunTextDynamic(newpwdTipLabel, newPwdTip);

    setCunTextDynamic(tipLabel, surePwdTip);

    refreshConfirmBtnStatus();
}

bool CreateUserNew::setCunTextDynamic(QLabel *label, QString string){

    bool isOverLength = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);

    QString str = string;
    if (fontSize > label->width()) {
        str = fontMetrics.elidedText(string, Qt::ElideRight, label->width());
        label->setToolTip(string);
        isOverLength = true;
    } else {
//        label->setFixedWidth(fontSize);
    }
    label->setText(str);
    return isOverLength;

}

void CreateUserNew::keyPressEvent(QKeyEvent * event){
    switch (event->key())
    {
    case Qt::Key_Escape:
        break;
    case Qt::Key_Enter:
        if (confirmBtn->isEnabled())
            confirmBtn->clicked();
        break;
    case Qt::Key_Return:
        if (confirmBtn->isEnabled())
            confirmBtn->clicked();
        break;
    default:
        QDialog::keyPressEvent(event);
    }
}

bool CreateUserNew::eventFilter(QObject *watched, QEvent *event){
    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton ){
            if (watched == adminFrame){
                adminRadioBtn->setChecked(true);
            } else if (watched == standardFrame){
                standardRadioBtn->setChecked(true);
            }
        }
    }

    if (event->type() == QEvent::FocusOut) {
        if (watched == usernameLineEdit) {
            if (usernameLineEdit->text().isEmpty()) {
                userNameTip = tr("Username's length must be between 1 and %1 characters!").arg(USER_LENGTH);
                setCunTextDynamic(usernameTipLabel, userNameTip);
            }
        } else if (watched == nicknameLineEdit) {
            if (nicknameLineEdit->text().isEmpty()) {
                nickNameTip = tr("The nick name cannot be empty");
                setCunTextDynamic(nicknameTipLabel, nickNameTip);
            }
        } else if (watched == newPwdLineEdit) {
            if (newPwdLineEdit->text().isEmpty()) {
                newPwdTip = tr("new pwd cannot be empty!");
                setCunTextDynamic(newpwdTipLabel, newPwdTip);
            }
        } else if (watched == surePwdLineEdit) {
            if (surePwdLineEdit->text().isEmpty()) {
                surePwdTip = tr("sure pwd cannot be empty!");
                setCunTextDynamic(tipLabel, surePwdTip);
            }
        }
    }

    //事件过滤如下
    //当接收到这些事件时，需要被过滤掉，所以返回true
    if (watched == usernameLineEdit  || watched == nicknameLineEdit || watched == newPwdLineEdit || watched == surePwdLineEdit) {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            if(keyEvent->matches(QKeySequence::Copy) || keyEvent->matches(QKeySequence::Cut))
            {
                qDebug() <<"Copy || Cut";
                return true;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}
