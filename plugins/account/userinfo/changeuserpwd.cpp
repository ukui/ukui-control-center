#include "changeuserpwd.h"

#include "passwdcheckutil.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>
#include <QDebug>

#include <QDBusInterface>
#include <QDBusReply>

#include <QCoreApplication>

#include <QPainter>
#include <QPainterPath>


#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <unistd.h>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

ChangeUserPwd::ChangeUserPwd(QString n, QWidget *parent) :
    QDialog(parent),
    name(n)
{

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

    setFixedSize(QSize(480, 266));
    setWindowTitle(tr("Change password"));

    //当前密码
    currentPwdLabel = new QLabel();
    currentPwdLabel->setFixedSize(100,24);
    setTextDynamicInPwd(currentPwdLabel, tr("Current Pwd"));
    curTipLabel = new QLabel();
    QFont ft;
    ft.setPointSize(12);
    curTipLabel->setFont(ft);
    curTipLabel->setFixedSize(QSize(322, 24));
    curTipLabel->setStyleSheet("color:red;");
    curTipHorLayout = new QHBoxLayout;
    curTipHorLayout->setContentsMargins(110, 0, 0, 0);
    curTipHorLayout->addStretch();
    curTipHorLayout->addWidget(curTipLabel);
    currentPwdLineEdit = new QLineEdit();
    currentPwdLineEdit->setFixedSize(QSize(322, 36));
    currentPwdLineEdit->setPlaceholderText(tr("Current Pwd"));
    currentPwdLineEdit->setEchoMode(QLineEdit::Password);
    currentPwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    currentPwdLineEdit->setTextMargins(0,0,30,0);
    currentPwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    currentPwdLineEdit->installEventFilter(this);
    currentPwdEyeBtn = new QPushButton;
    currentPwdEyeBtn->setFixedSize(QSize(24, 24));
    currentPwdEyeBtn->setIcon(QIcon::fromTheme("ukui-eye-hidden-symbolic"));
    currentPwdEyeBtn->setCursor(Qt::PointingHandCursor);
    currentPwdEyeBtn->setFlat(true);
    currentPwdEyeBtn->setStyleSheet("QPushButton::pressed{border:none;background-color:transparent}"
                                    "QPushButton::hover::!pressed{border:none;background-color:transparent}");
    currentPwdEyeBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    QHBoxLayout *currentPwdEyeBtnHLayout = new QHBoxLayout;
    currentPwdEyeBtnHLayout->addStretch();
    currentPwdEyeBtnHLayout->addWidget(currentPwdEyeBtn);
    currentPwdEyeBtnHLayout->setContentsMargins(0,0,8,0);
    currentPwdLineEdit->setLayout(currentPwdEyeBtnHLayout);

    currentPwdHorLayout = new QHBoxLayout;
    currentPwdHorLayout->setSpacing(8);
    currentPwdHorLayout->setContentsMargins(0, 0, 0, 0);
    currentPwdHorLayout->addWidget(currentPwdLabel);
    currentPwdHorLayout->addWidget(currentPwdLineEdit);

    //新密码
    newPwdLabel = new QLabel();
    newPwdLabel->setFixedSize(100,24);
    setTextDynamicInPwd(newPwdLabel, tr("New Pwd"));
    newTipLabel = new QLabel();
    newTipLabel->setFont(ft);
    newTipLabel->setFixedSize(QSize(322, 24));
    newTipLabel->setStyleSheet("color:red;");
    newTipHorLayout = new QHBoxLayout;
    newTipHorLayout->setContentsMargins(110, 0, 0, 0);
    newTipHorLayout->addStretch();
    newTipHorLayout->addWidget(newTipLabel);
    newPwdLineEdit = new QLineEdit();
    newPwdLineEdit->setFixedSize(QSize(322, 36));
    newPwdLineEdit->setPlaceholderText(tr("New Pwd"));
    newPwdLineEdit->setEchoMode(QLineEdit::Password);
    newPwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    newPwdLineEdit->setTextMargins(0,0,30,0);
    newPwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    newPwdLineEdit->installEventFilter(this);
    newPwdEyeBtn = new QPushButton;
    newPwdEyeBtn->setFixedSize(QSize(24, 24));
    newPwdEyeBtn->setIcon(QIcon::fromTheme("ukui-eye-hidden-symbolic"));
    newPwdEyeBtn->setCursor(Qt::PointingHandCursor);
    newPwdEyeBtn->setFlat(true);
    newPwdEyeBtn->setStyleSheet("QPushButton::pressed{border:none;background-color:transparent}"
                                "QPushButton::hover::!pressed{border:none;background-color:transparent}");
    newPwdEyeBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    QHBoxLayout *newPwdEyeBtnHLayout = new QHBoxLayout;
    newPwdEyeBtnHLayout->addStretch();
    newPwdEyeBtnHLayout->addWidget(newPwdEyeBtn);
    newPwdEyeBtnHLayout->setContentsMargins(0,0,8,0);
    newPwdLineEdit->setLayout(newPwdEyeBtnHLayout);

    newPwdHorLayout = new QHBoxLayout;
    newPwdHorLayout->setSpacing(8);
    newPwdHorLayout->setContentsMargins(0, 0, 0, 0);
    newPwdHorLayout->addWidget(newPwdLabel);
    newPwdHorLayout->addWidget(newPwdLineEdit);

    //确认密码
    surePwdLabel = new QLabel();
    surePwdLabel->setFixedSize(100,24);
    setTextDynamicInPwd(surePwdLabel, tr("Sure Pwd"));

    surePwdLineEdit = new QLineEdit();
    surePwdLineEdit->setFixedSize(QSize(322, 36));
    surePwdLineEdit->setPlaceholderText(tr("Sure Pwd"));
    surePwdLineEdit->setEchoMode(QLineEdit::Password);
    surePwdLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    surePwdLineEdit->setTextMargins(0,0,30,0);
    surePwdLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    surePwdLineEdit->installEventFilter(this);
    surePwdEyeBtn = new QPushButton;
    surePwdEyeBtn->setFixedSize(QSize(24, 24));
    surePwdEyeBtn->setIcon(QIcon::fromTheme("ukui-eye-hidden-symbolic"));
    surePwdEyeBtn->setCursor(Qt::PointingHandCursor);
    surePwdEyeBtn->setFlat(true);
    surePwdEyeBtn->setStyleSheet("QPushButton::pressed{border:none;background-color:transparent}"
                                 "QPushButton::hover::!pressed{border:none;background-color:transparent}");
    surePwdEyeBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    QHBoxLayout *surePwdEyeBtnHLayout = new QHBoxLayout;
    surePwdEyeBtnHLayout->addStretch();
    surePwdEyeBtnHLayout->addWidget(surePwdEyeBtn);
    surePwdEyeBtnHLayout->setContentsMargins(0,0,8,0);
    surePwdLineEdit->setLayout(surePwdEyeBtnHLayout);

    surePwdHorLayout = new QHBoxLayout;
    surePwdHorLayout->setSpacing(8);
    surePwdHorLayout->setContentsMargins(0, 0, 0, 0);
    surePwdHorLayout->addWidget(surePwdLabel);
    surePwdHorLayout->addWidget(surePwdLineEdit);

    tipLabel = new QLabel();
    tipLabel->setFont(ft);
    tipLabel->setFixedSize(QSize(322, 30));
    tipLabel->setStyleSheet("color:red;");

    tipHorLayout = new QHBoxLayout;
    tipHorLayout->setSpacing(0);
    tipHorLayout->setContentsMargins(110, 0, 0, 0);
    tipHorLayout->addStretch();
    tipHorLayout->addWidget(tipLabel);

    surePwdWithTipVerLayout = new QVBoxLayout;
    surePwdWithTipVerLayout->setSpacing(8);
    surePwdWithTipVerLayout->setContentsMargins(0, 0, 0, 0);
    surePwdWithTipVerLayout->addLayout(surePwdHorLayout);
    surePwdWithTipVerLayout->addLayout(tipHorLayout);

    //中部输入区域
    contentVerLayout = new QVBoxLayout;
    contentVerLayout->setSpacing(0);
    contentVerLayout->setContentsMargins(24, 0, 35, 0);
    if (isCurrentUser){
        contentVerLayout->addLayout(currentPwdHorLayout);
        contentVerLayout->addLayout(curTipHorLayout);
    }
    contentVerLayout->addLayout(newPwdHorLayout);
    contentVerLayout->addLayout(newTipHorLayout);
    contentVerLayout->addLayout(surePwdWithTipVerLayout);

    //底部“取消”、“确定”按钮
    cancelBtn = new QPushButton();
    cancelBtn->setMinimumWidth(96);
    cancelBtn->setText(tr("Cancel"));
    cancelBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    confirmBtn = new QPushButton();
    confirmBtn->setMinimumWidth(96);
    confirmBtn->setText(tr("Confirm"));
    confirmBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    bottomBtnsHorLayout = new QHBoxLayout;
    bottomBtnsHorLayout->setSpacing(16);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 25, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(cancelBtn);
    bottomBtnsHorLayout->addWidget(confirmBtn);

    mainVerLayout = new QVBoxLayout;
    mainVerLayout->setContentsMargins(0, 10, 0, 24);
    mainVerLayout->addLayout(contentVerLayout);
    mainVerLayout->addStretch();
    mainVerLayout->addLayout(bottomBtnsHorLayout);

    setLayout(mainVerLayout);
}

void ChangeUserPwd::test(){

}

void ChangeUserPwd::setupConnect(){

    //通用的connect

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

        updateTipLableInfo(tipLabel,surePwdTip);

        refreshConfirmBtnStatus();
    });

    connect(currentPwdEyeBtn, &QPushButton::clicked, this, [=](){
        if (currentPwdLineEdit->echoMode() == QLineEdit::Password) {
            currentPwdLineEdit->setEchoMode(QLineEdit::Normal);
            currentPwdEyeBtn->setIcon(QIcon::fromTheme("ukui-eye-display-symbolic"));
        } else {
            currentPwdLineEdit->setEchoMode(QLineEdit::Password);
            currentPwdEyeBtn->setIcon(QIcon::fromTheme("ukui-eye-hidden-symbolic"));
        }
    });

    connect(newPwdEyeBtn, &QPushButton::clicked, this, [=](){
        if (newPwdLineEdit->echoMode() == QLineEdit::Password) {
            newPwdLineEdit->setEchoMode(QLineEdit::Normal);
            newPwdEyeBtn->setIcon(QIcon::fromTheme("ukui-eye-display-symbolic"));
        } else {
            newPwdLineEdit->setEchoMode(QLineEdit::Password);
            newPwdEyeBtn->setIcon(QIcon::fromTheme("ukui-eye-hidden-symbolic"));
        }

    });

    connect(surePwdEyeBtn, &QPushButton::clicked, this, [=](){
        if (surePwdLineEdit->echoMode() == QLineEdit::Password) {
            surePwdLineEdit->setEchoMode(QLineEdit::Normal);
            surePwdEyeBtn->setIcon(QIcon::fromTheme("ukui-eye-display-symbolic"));
        } else {
            surePwdLineEdit->setEchoMode(QLineEdit::Password);
            surePwdEyeBtn->setIcon(QIcon::fromTheme("ukui-eye-hidden-symbolic"));
        }

    });


    //需要区分的connect
    if (isCurrentUser){

        connect(thread1ForCheckPwd, &PwdCheckThread::complete, [=](QString re){

            curPwdTip = re;

            //返回值为空，密码校验成功
            if (re.isEmpty()){

                //修改密码
                QString output;

                char * cmd = g_strdup_printf("/usr/bin/changeuserpwd '%s' '%s'", currentPwdLineEdit->text().toLatin1().data(), newPwdLineEdit->text().toLatin1().data());

                FILE   *stream;
                char buf[256];

                if ((stream = popen(cmd, "r" )) == NULL){
                    return -1;
                }

                while(fgets(buf, 256, stream) != NULL){
                    output = QString(buf).simplified();
                }

                pclose(stream);

                this->accept();

            } else {
                if (re.contains("Failed")){
                    curPwdTip = tr("Authentication failed, input authtok again!");
                }

                updateTipLableInfo(tipLabel,curPwdTip);

                //重置当前密码输入框
                currentPwdLineEdit->setText("");

                refreshConfirmBtnStatus();

                //密码校验完成
                isChecking = false;

                refreshCloseBtnStatus();
            }


        });

        connect(currentPwdLineEdit, &QLineEdit::textEdited, [=](QString txt){
            if (!txt.isEmpty()){
                curPwdTip = "";
                updateTipLableInfo(curTipLabel,curPwdTip);

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
        connect(confirmBtn, &QPushButton::clicked, this, [=]{

            //修改其他用户密码
            QDBusInterface tmpiface("com.control.center.qt.systemdbus",
                                    "/",
                                    "com.control.center.interface",
                                    QDBusConnection::systemBus());

            if (!tmpiface.isValid()){
                qWarning() << "Create Client Interface Failed When : " << QDBusConnection::systemBus().lastError();
                return;
            }

            QDBusReply<int> reply = tmpiface.call("setPid", QCoreApplication::applicationPid());
            if (reply.isValid()){
                tmpiface.call("changeOtherUserPasswd", name, newPwdLineEdit->text());
            }

            this->accept();
        });


    }

}

void ChangeUserPwd::setupStatus(QString n){

    curPwdTip = QString();
    newPwdTip = QString();
    surePwdTip = QString();

    //初始化确定按钮状态
    refreshConfirmBtnStatus();
}

void ChangeUserPwd::updateTipLableInfo(QLabel *Label,QString info){

    if (setTextDynamicInPwd(Label, info)){
        Label->setToolTip(info);
    } else {
        Label->setToolTip("");
    }
}

void ChangeUserPwd::refreshCloseBtnStatus(){
    if (isChecking){
//        closeBtn->setEnabled(false);
        cancelBtn->setEnabled(false);
    } else {
//        closeBtn->setEnabled(true);
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
    if (!surePwdLineEdit->text().isEmpty()){
        if (QString::compare(newPwdLineEdit->text(), surePwdLineEdit->text()) == 0){
            surePwdTip = "";
        } else {
            surePwdTip = tr("Inconsistency with pwd");
        }
    }

    //设置新密码的提示
    updateTipLableInfo(newTipLabel,newPwdTip);

    updateTipLableInfo(tipLabel,surePwdTip);

    updateTipLableInfo(curTipLabel,curPwdTip);

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
        label->setToolTip(string);
    } else {
        label->setToolTip("");
    }
    label->setText(str);
    return isOverLength;

}


void ChangeUserPwd::keyPressEvent(QKeyEvent * event){
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

//事件过滤如下
//当接收到这些事件时，需要被过滤掉，所以返回true
bool ChangeUserPwd::eventFilter(QObject *target, QEvent *event)
{
    if (target == currentPwdLineEdit || target == newPwdLineEdit || target == surePwdLineEdit) {
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
    return QWidget::eventFilter(target, event);
    //继续传递该事件到被观察者，由其本身调用相应的事件。
}

