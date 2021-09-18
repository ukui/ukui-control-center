#include "changeuserpwd.h"

#include "passwdcheckutil.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>
#include <QDebug>


#ifdef signals
#undef signals
#endif

extern "C" {

#include <glib.h>
#include <unistd.h>

}

ChangeUserPwd::ChangeUserPwd(QString n, QWidget *parent) :
    QDialog(parent),
    name(n)
{
    setFixedSize(QSize(480, 296));
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);

    //判断是否是当前用户
    if (QString::compare(name, QString(g_get_user_name())) == 0){
        isCurrentUser = true;
    } else {
        isCurrentUser = false;
    }

    //密码校验状态
    isChecking = false;

    //构造密码校验线程
    thread1ForCheckPwd = new PwdCheckThread();

    makeSurePwqualityEnabled();

    initUI();
    setupStatus(name);

    setupConnect();
}

ChangeUserPwd::~ChangeUserPwd()
{
}

void ChangeUserPwd::makeSurePwqualityEnabled(){
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

    if (PasswdCheckUtil::getCurrentPamState())
        enablePwdQuality = true;
    else
        enablePwdQuality = false;

#else
    enablePwdQuality = false;
#endif
}

void ChangeUserPwd::initUI(){

    closeBtn = new QPushButton();
    closeBtn->setFixedSize(QSize(14, 14));

    titleHorLayout = new QHBoxLayout;
    titleHorLayout->setSpacing(0);
    titleHorLayout->setContentsMargins(0, 0, 14, 0);
    titleHorLayout->addStretch();
    titleHorLayout->addWidget(closeBtn);

    //当前密码
    currentPwdLabel = new QLabel();
    currentPwdLabel->setMinimumWidth(60);
    currentPwdLabel->setMaximumWidth(120);
    currentPwdLabel->setText(tr("Current Pwd"));

    currentPwdLineEdit = new QLineEdit();
    currentPwdLineEdit->setFixedSize(QSize(300, 36));
    currentPwdLineEdit->setPlaceholderText(tr("Current Pwd"));
    currentPwdLineEdit->setEchoMode(QLineEdit::Password);

    currentPwdHorLayout = new QHBoxLayout;
    currentPwdHorLayout->setSpacing(25);
    currentPwdHorLayout->setContentsMargins(0, 0, 0, 0);
    currentPwdHorLayout->addWidget(currentPwdLabel);
    currentPwdHorLayout->addWidget(currentPwdLineEdit);

    //新密码
    newPwdLabel = new QLabel();
    newPwdLabel->setMinimumWidth(60);
    newPwdLabel->setMaximumWidth(120);
    newPwdLabel->setText(tr("New Pwd"));

    newPwdLineEdit = new QLineEdit();
    newPwdLineEdit->setFixedSize(QSize(300, 36));
    newPwdLineEdit->setPlaceholderText(tr("New Pwd"));
    newPwdLineEdit->setEchoMode(QLineEdit::Password);

    newPwdHorLayout = new QHBoxLayout;
    newPwdHorLayout->setSpacing(25);
    newPwdHorLayout->setContentsMargins(0, 0, 0, 0);
    newPwdHorLayout->addWidget(newPwdLabel);
    newPwdHorLayout->addWidget(newPwdLineEdit);

    //确认密码
    surePwdLabel = new QLabel();
    surePwdLabel->setMinimumWidth(60);
    surePwdLabel->setMaximumWidth(120);
    surePwdLabel->setText(tr("Sure Pwd"));

    surePwdLineEdit = new QLineEdit();
    surePwdLineEdit->setFixedSize(QSize(300, 36));
    surePwdLineEdit->setPlaceholderText(tr("Sure Pwd"));
    surePwdLineEdit->setEchoMode(QLineEdit::Password);

    surePwdHorLayout = new QHBoxLayout;
    surePwdHorLayout->setSpacing(25);
    surePwdHorLayout->setContentsMargins(0, 0, 0, 0);
    surePwdHorLayout->addWidget(surePwdLabel);
    surePwdHorLayout->addWidget(surePwdLineEdit);

    tipLabel = new QLabel();
    tipLabel->setFixedSize(QSize(300, 24));
    tipLabel->setStyleSheet("color:red;");

    tipHorLayout = new QHBoxLayout;
    tipHorLayout->setSpacing(0);
    tipHorLayout->setContentsMargins(0, 0, 0, 0);
    tipHorLayout->addStretch();
    tipHorLayout->addWidget(tipLabel);

    surePwdWithTipVerLayout = new QVBoxLayout;
    surePwdWithTipVerLayout->setSpacing(4);
    surePwdWithTipVerLayout->setContentsMargins(0, 0, 0, 0);
    surePwdWithTipVerLayout->addLayout(surePwdHorLayout);
    surePwdWithTipVerLayout->addLayout(tipHorLayout);

    //中部输入区域
    contentVerLayout = new QVBoxLayout;
    contentVerLayout->setSpacing(10);
    contentVerLayout->setContentsMargins(24, 0, 35, 24);
    if (isCurrentUser){
        contentVerLayout->addLayout(currentPwdHorLayout);
    }
    contentVerLayout->addLayout(newPwdHorLayout);
    contentVerLayout->addLayout(surePwdWithTipVerLayout);

    //底部“取消”、“确定”按钮
    cancelBtn = new QPushButton();
    cancelBtn->setMinimumWidth(96);
    confirmBtn = new QPushButton();
    confirmBtn->setMinimumWidth(96);

    bottomBtnsHorLayout = new QHBoxLayout;
    bottomBtnsHorLayout->setSpacing(16);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 25, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(cancelBtn);
    bottomBtnsHorLayout->addWidget(confirmBtn);

    mainVerLayout = new QVBoxLayout;
    mainVerLayout->setSpacing(20);
    mainVerLayout->setContentsMargins(0, 0, 0, 24);
    mainVerLayout->addLayout(titleHorLayout);
    mainVerLayout->addLayout(contentVerLayout);
    mainVerLayout->addLayout(bottomBtnsHorLayout);

    setLayout(mainVerLayout);
}

void ChangeUserPwd::test(){

}

void ChangeUserPwd::setupConnect(){

    //通用的connect
    connect(closeBtn, &QPushButton::clicked, this, [=]{
        close();
    });

    connect(cancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });

    connect(newPwdLineEdit, &QLineEdit::textEdited, [=]{
        checkPwdLegality();

        refreshConfirmBtnStatus();
    });

    connect(surePwdLineEdit, &QLineEdit::textEdited, [=](QString txt){
        if (!txt.isEmpty() && txt != newPwdLineEdit->text()){
            surePwdTip = tr("Inconsistency with pwd");
        } else {
            surePwdTip = "";
        }

        updateTipLableInfo(surePwdTip);

        if (surePwdTip.isEmpty()){
            if (!newPwdTip.isEmpty()){
                updateTipLableInfo(newPwdTip);
            } else if (!curPwdTip.isEmpty()){
                updateTipLableInfo(curPwdTip);
            }
        }
    });

    //需要区分的connect
    if (isCurrentUser){

        connect(thread1ForCheckPwd, &PwdCheckThread::complete, [=](QString re){

            curPwdTip = re;

            //返回值为空，密码校验
            if (re.isEmpty()){

            } else {
                if (re.contains("Failed")){
                    curPwdTip = tr("Authentication failed, input authtok again!");
                }

                updateTipLableInfo(curPwdTip);

                //重置当前密码输入框
                currentPwdLineEdit->setText("");

                refreshConfirmBtnStatus();
            }

            //密码校验完成
            isChecking = true;

            refreshCloseBtnStatus();
        });

        connect(currentPwdLineEdit, &QLineEdit::textEdited, [=](QString txt){
            if (!txt.isEmpty()){
                curPwdTip = "";
                updateTipLableInfo(curPwdTip);

                //再次校验新密码，需要保证"与旧密码相同"等条件生效
                checkPwdLegality();

            }

            refreshConfirmBtnStatus();
        });

        connect(confirmBtn, &QPushButton::clicked, [=]{
            //密码校验失败有几秒延时，防止用户重复点击确定按钮
            if (isChecking)
                return;

            thread1ForCheckPwd->setArgs(name, currentPwdLineEdit->text());
            thread1ForCheckPwd->start();

            //开始当前密码校验
            isChecking = true;

            refreshCloseBtnStatus();

        });
    } else {


    }

}

void ChangeUserPwd::setupStatus(QString n){

    curPwdTip = QString();
    newPwdTip = QString();
    surePwdTip = QString();

    //初始化确定按钮状态
    refreshConfirmBtnStatus();
}

void ChangeUserPwd::updateTipLableInfo(QString info){

    if (setTextDynamicInPwd(tipLabel, info)){
        tipLabel->setToolTip(info);
    } else {
        tipLabel->setToolTip("");
    }
}

void ChangeUserPwd::refreshCloseBtnStatus(){
    if (isChecking){
        closeBtn->setEnabled(false);
        cancelBtn->setEnabled(false);
    } else {
        closeBtn->setEnabled(true);
        cancelBtn->setEnabled(true);
    }
}

void ChangeUserPwd::refreshConfirmBtnStatus(){

    if (getuid() && isCurrentUser){
        if (!tipLabel->text().isEmpty() || \
                currentPwdLineEdit->text().isEmpty() || currentPwdLineEdit->text() == tr("Current Pwd") || \
                newPwdLineEdit->text().isEmpty() || newPwdLineEdit->text() == tr("New Pwd") || \
                surePwdLineEdit->text().isEmpty() || surePwdLineEdit->text() == tr("Sure Pwd") ||
                !curPwdTip.isEmpty() || !newPwdTip.isEmpty() || !surePwdTip.isEmpty())
            confirmBtn->setEnabled(false);
        else
            confirmBtn->setEnabled(true);
    } else {
        if (!tipLabel->text().isEmpty() || \
                newPwdLineEdit->text().isEmpty() || newPwdLineEdit->text() == tr("New Pwd") || \
                surePwdLineEdit->text().isEmpty() || surePwdLineEdit->text() == tr("Sure Pwd") ||
                !newPwdTip.isEmpty() || !surePwdTip.isEmpty())
            confirmBtn->setEnabled(false);
        else
            confirmBtn->setEnabled(true);
    }
}

bool ChangeUserPwd::isContainLegitimacyChar(QString word){
    //需要用'在shell解释中做强引用
    if (word.contains("'"))
        return false;

    foreach (QChar ch, word){
        if (int(ch.toLatin1() <= 0 || int(ch.toLatin1()) > 127)){
            return false;
        }
    }
    return true;
}

void ChangeUserPwd::checkPwdLegality(){

    //判断用户是否输入了非法字符
    if (!isContainLegitimacyChar(newPwdLineEdit->text())){
        newPwdTip = tr("Contains illegal characters!");
    } else {
        if (enablePwdQuality){

#ifdef ENABLEPQ
            void * auxerror;
            int ret;
            const char * msg;
            char buf[256];

            QByteArray ba = newPwdLineEdit->text().toLatin1();
            QByteArray ba1 = currentPwdLineEdit->text().toLatin1();

            if (isCurrentUser){
               ret = pwquality_check(settings, ba.data(), ba1.data(), name.toLatin1().data(), &auxerror);
            } else {
               ret = pwquality_check(settings, ba.data(), NULL, name.toLatin1().data(), &auxerror);
            }

            if (ret < 0 && newPwdLineEdit->text().length() > 0){
                msg = pwquality_strerror(buf, sizeof(buf), ret, auxerror);
                newPwdTip = QString(msg);
            } else {
                newPwdTip = "";
            }
#endif

        } else {
            //系统未开启pwdquality模块
            newPwdTip = "";
        }
    }


    //防止先输入确认密码，再输入密码后tipLabel无法刷新
    if (!surePwdTip.isEmpty()){
        if (QString::compare(newPwdLineEdit->text(), surePwdLineEdit->text()) == 0){
            surePwdTip = "";
        } else {
            surePwdTip = tr("Inconsistency with pwd");
        }
    }


    //设置新密码的提示
    updateTipLableInfo(newPwdTip);

    if (newPwdTip.isEmpty()){
        if (!surePwdTip.isEmpty()){
            updateTipLableInfo(surePwdTip);
        } else if (!curPwdTip.isEmpty()){
            updateTipLableInfo(curPwdTip);
        }
    }
}

bool ChangeUserPwd::setTextDynamicInPwd(QLabel *label, QString string){

    bool isOverLength = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);

    QString str = string;
    int pSize = label->width();
    if (fontSize > pSize) {
        str = fontMetrics.elidedText(string, Qt::ElideRight, pSize);
        isOverLength = true;
    } else {

    }
    label->setText(str);
    return isOverLength;

}
