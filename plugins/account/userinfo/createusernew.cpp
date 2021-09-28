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

#define USER_LENGTH 32
#define DEFAULTFACE "/usr/share/ukui/faces/default.png"

CreateUserNew::CreateUserNew(QStringList allUsers, QWidget *parent) :
    QDialog(parent),
    _allNames(allUsers)
{
    setFixedSize(QSize(520, 608));
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
    nicknameLabel = new QLabel;
    nicknameLabel->setFixedSize(100, 24);
    nicknameLabel->setText(tr("NickName"));
    newPwdLabel = new QLabel;
    newPwdLabel->setFixedSize(100, 24);
    newPwdLabel->setText(tr("Pwd"));
    surePwdLabel = new QLabel;
    surePwdLabel->setFixedSize(100, 24);
    surePwdLabel->setText(tr("SurePwd"));
    tipLabel = new QLabel;
    tipLabel->setFixedSize(340, 24);

    usernameLineEdit = new QLineEdit;
    usernameLineEdit->setFixedWidth(340);
    nicknameLineEdit = new QLineEdit;
    nicknameLineEdit->setFixedWidth(340);
    newPwdLineEdit = new QLineEdit;
    newPwdLineEdit->setFixedWidth(340);
    newPwdLineEdit->setEchoMode(QLineEdit::Password);
    surePwdLineEdit = new QLineEdit;
    surePwdLineEdit->setFixedWidth(340);
    surePwdLineEdit->setEchoMode(QLineEdit::Password);

    //用户名
    usernameHorLayout = new QHBoxLayout;
    usernameHorLayout->setSpacing(8);
    usernameHorLayout->setContentsMargins(0, 0, 0, 0);
    usernameHorLayout->addWidget(usernameLabel);
    usernameHorLayout->addStretch();
    usernameHorLayout->addWidget(usernameLineEdit);

    //用户昵称
    nicknameHorLayout = new QHBoxLayout;
    nicknameHorLayout->setSpacing(8);
    nicknameHorLayout->setMargin(0);
    nicknameHorLayout->addWidget(nicknameLabel);
    nicknameHorLayout->addStretch();
    nicknameHorLayout->addWidget(nicknameLineEdit);

    //密码
    newPwdHorLayout = new QHBoxLayout;
    newPwdHorLayout->setSpacing(8);
    newPwdHorLayout->setMargin(0);
    newPwdHorLayout->addWidget(newPwdLabel);
    newPwdHorLayout->addStretch();
    newPwdHorLayout->addWidget(newPwdLineEdit);

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

    typeBtnGroup->addButton(adminRadioBtn, 0);
    typeBtnGroup->addButton(standardRadioBtn, 1);

    adminRadioBtn->setChecked(true);

    //管理员RadioButton布局
    admin1VerLayout = new QVBoxLayout;
    admin1VerLayout->setSpacing(0);
    admin1VerLayout->setContentsMargins(0, 4, 0, 0);
    admin1VerLayout->addWidget(adminRadioBtn);
    admin1VerLayout->addStretch();

    admin2VerLayout = new QVBoxLayout;
    admin2VerLayout->setSpacing(4);
    admin2VerLayout->setMargin(0);
    admin2VerLayout->addWidget(adminLabel);
    admin2VerLayout->addWidget(adminDetailLabel);

    adminHorLayout = new QHBoxLayout;
    adminHorLayout->setSpacing(8);
    adminHorLayout->setContentsMargins(16, 14, 0, 0);
    adminHorLayout->addLayout(admin1VerLayout);
    adminHorLayout->addLayout(admin2VerLayout);
    adminHorLayout->addStretch();

    //管理员区域
    adminFrame = new QFrame;
    adminFrame->setMinimumSize(QSize(473, 88));
    adminFrame->setMaximumSize(QSize(16777215, 88));
    adminFrame->setFrameShape(QFrame::StyledPanel);
    adminFrame->setFrameStyle(QFrame::Plain);
    adminFrame->setLayout(adminHorLayout);

    //标准用户RadioButton布局
    standard1VerLayout = new QVBoxLayout;
    standard1VerLayout->setSpacing(0);
    standard1VerLayout->setContentsMargins(0, 4, 0, 0);
    standard1VerLayout->addWidget(standardRadioBtn);
    standard1VerLayout->addStretch();

    standard2VerLayout = new QVBoxLayout;
    standard2VerLayout->setSpacing(4);
    standard2VerLayout->setMargin(0);
    standard2VerLayout->addWidget(standardLabel);
    standard2VerLayout->addWidget(standardDetailLabel);

    standardHorLayout = new QHBoxLayout;
    standardHorLayout->setSpacing(8);
    standardHorLayout->setContentsMargins(16, 14, 0, 0);
    standardHorLayout->addLayout(standard1VerLayout);
    standardHorLayout->addLayout(standard2VerLayout);
    standardHorLayout->addStretch();

    //标准用户区域
    standardFrame = new QFrame;
    standardFrame->setMinimumSize(QSize(473, 88));
    standardFrame->setMaximumSize(QSize(16777215, 88));
    standardFrame->setFrameShape(QFrame::StyledPanel);
    standardFrame->setFrameStyle(QFrame::Plain);
    standardFrame->setLayout(standardHorLayout);

    cancelBtn = new QPushButton;
    confirmBtn = new QPushButton;

    bottomHorLayout = new QHBoxLayout;
    bottomHorLayout->setSpacing(16);
    bottomHorLayout->setMargin(0);
    bottomHorLayout->addStretch();
    bottomHorLayout->addWidget(cancelBtn);
    bottomHorLayout->addWidget(confirmBtn);

    mainVerLayout = new QVBoxLayout;
    mainVerLayout->setSpacing(0);
    mainVerLayout->setContentsMargins(24, 35, 24, 24);
    mainVerLayout->addLayout(usernameHorLayout);
    mainVerLayout->addLayout(nicknameHorLayout);
    mainVerLayout->addLayout(newPwdHorLayout);
    mainVerLayout->addLayout(surePwdHorLayout);
    mainVerLayout->addLayout(tipHorLayout);
    mainVerLayout->addSpacing(24);
    mainVerLayout->addLayout(typeNoteHorLayout);
    mainVerLayout->addWidget(adminFrame);
    mainVerLayout->addWidget(standardFrame);
    mainVerLayout->addSpacing(32);
    mainVerLayout->addLayout(bottomHorLayout);

    setLayout(mainVerLayout);
}

void CreateUserNew::setConnect(){

    connect(usernameLineEdit, &QLineEdit::textEdited, this, [=](QString txt){
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

        if (surePwdTip.isEmpty()){
            if (!newPwdTip.isEmpty()){
                setCunTextDynamic(tipLabel, newPwdTip);
            } else if (!userNameTip.isEmpty()){
                setCunTextDynamic(tipLabel, userNameTip);
            } else if (!nickNameTip.isEmpty()){
                setCunTextDynamic(tipLabel, nickNameTip);
            }
        }

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
    }

    if (QString::compare(nickname, usernameLineEdit->text()) == 0){
        nickNameTip = tr("Nickname cannot same with username");
    } else {
        nickNameTip = tr("");
    }

    setCunTextDynamic(tipLabel, nickNameTip);

    if (nickNameTip.isEmpty()){
        if (!userNameTip.isEmpty()){
            setCunTextDynamic(tipLabel, userNameTip);
        } else if (!newPwdTip.isEmpty()){
            setCunTextDynamic(tipLabel, newPwdTip);
        } else if (!surePwdTip.isEmpty()){
            setCunTextDynamic(tipLabel, surePwdTip);
        }
    }

}

void CreateUserNew::nameLegalityCheck(QString username){

    if (username.isEmpty())
        userNameTip = tr("The user name cannot be empty");
    else if (username.startsWith("_") || username.left(1).contains((QRegExp("[0-9]")))){
        userNameTip = tr("Must be begin with lower letters!");
    }
    else if (username.contains(QRegExp("[A-Z]"))){
        userNameTip = tr("Can not contain capital letters!");
    }
    else if (nameTraverse(username))
        if (username.length() > 0 && username.length() < USER_LENGTH){

            QString cmd = QString("getent group %1").arg(username);
            QString output;

            FILE   *stream;
            char buf[256];

            if ((stream = popen(cmd.toLatin1().data(), "r" )) == NULL){
                return;
            }

            while(fgets(buf, 256, stream) != NULL){
                output = QString(buf).simplified();
            }

            pclose(stream);

            if (_allNames.contains(username)){
                userNameTip = tr("Name already in use, change another one.");
            } else if (!output.isEmpty()) {
                userNameTip = tr("Name corresponds to group already exists.");
            }else {
                userNameTip = "";
            }
        } else {
            userNameTip = tr("Name length must less than %1 letters!").arg(USER_LENGTH);
    } else {
        userNameTip = tr("Can only contain letters,digits,underline!");
    }

    if (isHomeUserExists(username) && userNameTip.isEmpty()) {
        userNameTip = tr("Username's folder exists, change another one");
    }

    if (QString::compare(username, nicknameLineEdit->text()) == 0){
        userNameTip = tr("Nickname cannot same with username");
    }

    setCunTextDynamic(tipLabel, userNameTip);

    if (userNameTip.isEmpty()){

        if (!newPwdTip.isEmpty()){
            setCunTextDynamic(tipLabel, newPwdTip);
        } else if (!surePwdTip.isEmpty()){
            setCunTextDynamic(tipLabel, surePwdTip);
        } else if (!nickNameTip.isEmpty()){
            setCunTextDynamic(tipLabel, nickNameTip);
        }
    }

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

    setCunTextDynamic(tipLabel, newPwdTip);
    if (newPwdTip.isEmpty()){
        if (!surePwdTip.isEmpty()){
            setCunTextDynamic(tipLabel, surePwdTip);
        } else if (!userNameTip.isEmpty()){
            setCunTextDynamic(tipLabel, userNameTip);
        } else if (!nickNameTip.isEmpty()){
            setCunTextDynamic(tipLabel, nickNameTip);
        }
    }

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
