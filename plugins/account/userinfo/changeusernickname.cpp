#include "changeusernickname.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>

#include <QDBusInterface>
#include <QDBusReply>

#include <QPainter>
#include <QPainterPath>

#include <glib.h>

#define NICKNAME_LENGTH 32

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

ChangeUserNickname::ChangeUserNickname(QString nn, QStringList ns, QString op, QWidget *parent) :
    QDialog(parent),
    namesIsExists(ns),
    realname(nn)
{
    setFixedSize(QSize(480, 216));
    this->setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("Set Nickname"));

    cniface = new QDBusInterface("org.freedesktop.Accounts",
                                  op,
                                  "org.freedesktop.Accounts.User",
                                  QDBusConnection::systemBus());


    initUI();
    setConnect();
    setupStatus();
}

ChangeUserNickname::~ChangeUserNickname()
{
    delete cniface;
}

void ChangeUserNickname::initUI(){

    //用户名
    userNameLabel = new QLabel();
    userNameLabel->setFixedWidth(100);
    setTextDynamicInNick(userNameLabel, tr("UserName"));

    userNameLineEdit = new QLineEdit();
    userNameLineEdit->setFixedSize(QSize(300, 36));
    userNameLineEdit->setPlaceholderText(QString(g_get_user_name()));
    userNameLineEdit->setReadOnly(true);

    userNameHorLayout = new QHBoxLayout;
    userNameHorLayout->setSpacing(25);
    userNameHorLayout->setContentsMargins(0, 0, 0, 0);
    userNameHorLayout->addWidget(userNameLabel);
    userNameHorLayout->addWidget(userNameLineEdit);

    //用户昵称
    nickNameLabel = new QLabel();
    nickNameLabel->setFixedWidth(100);
    setTextDynamicInNick(nickNameLabel, tr("NickName"));

    tipLabel = new QLabel();
    tipLabel->setFixedSize(QSize(300, 36));
    QString tipinfo = tr("nickName already in use.");
    setTextDynamicInNick(tipLabel, tipinfo);

    nickNameLineEdit = new QLineEdit();
    nickNameLineEdit->setFixedSize(QSize(300, 36));
    nickNameLineEdit->setText(realname);

    nickNameHorLayout = new QHBoxLayout;
    nickNameHorLayout->setSpacing(25);
    nickNameHorLayout->setContentsMargins(0, 0, 0, 0);
    nickNameHorLayout->addWidget(nickNameLabel);
    nickNameHorLayout->addWidget(nickNameLineEdit);

    tipHorLayout = new QHBoxLayout;
    tipHorLayout->setSpacing(0);
    tipHorLayout->setContentsMargins(0, 0, 0, 0);
    tipHorLayout->addStretch();
    tipHorLayout->addWidget(tipLabel);

    nickNameWithTipVerLayout = new QVBoxLayout;
    nickNameWithTipVerLayout->setSpacing(4);
    nickNameWithTipVerLayout->setContentsMargins(0, 0, 0, 0);
    nickNameWithTipVerLayout->addLayout(nickNameHorLayout);
    nickNameWithTipVerLayout->addLayout(tipHorLayout);

    //中部输入区域
    contentVerLayout = new QVBoxLayout;
    contentVerLayout->setSpacing(24);
    contentVerLayout->setContentsMargins(0, 0, 0, 0);
    contentVerLayout->addLayout(userNameHorLayout);
    contentVerLayout->addLayout(nickNameWithTipVerLayout);
    contentVerLayout->addStretch();

    //底部“取消”、“确定”按钮
    cancelBtn = new QPushButton();
    cancelBtn->setMinimumWidth(96);
    cancelBtn->setText(tr("Cancel"));
    confirmBtn = new QPushButton();
    confirmBtn->setMinimumWidth(96);
    confirmBtn->setText(tr("Confirm"));

    bottomBtnsHorLayout = new QHBoxLayout;
    bottomBtnsHorLayout->setSpacing(16);
    bottomBtnsHorLayout->setContentsMargins(0, 0, 0, 0);
    bottomBtnsHorLayout->addStretch();
    bottomBtnsHorLayout->addWidget(cancelBtn);
    bottomBtnsHorLayout->addWidget(confirmBtn);

    //主布局
    mainVerLayout = new QVBoxLayout;
    mainVerLayout->setContentsMargins(24, 24, 24, 24);
    mainVerLayout->addLayout(contentVerLayout);
    mainVerLayout->addStretch();
    mainVerLayout->addLayout(bottomBtnsHorLayout);

    setLayout(mainVerLayout);

}

void ChangeUserNickname::setConnect(){
    nickNameLineEdit->installEventFilter(this);

    connect(nickNameLineEdit, &QLineEdit::editingFinished, this, [=]{
        if (nickNameLineEdit->text().isEmpty()){
            nickNameLineEdit->setText(QString(g_get_real_name()));
        }
    });

    connect(nickNameLineEdit, &QLineEdit::textEdited, this, [=](QString txt){
        if (namesIsExists.contains(txt)){
            tipLabel->show();
            confirmBtn->setEnabled(false);
        } else if(nickNameLineEdit->text().length() >= NICKNAME_LENGTH) {
            QString tipinfo = tr("nickName length must less than %1 letters!").arg(NICKNAME_LENGTH);
            setTextDynamicInNick(tipLabel, tipinfo);
            tipLabel->show();
            confirmBtn->setEnabled(false);
        } else {
            tipLabel->hide();
            confirmBtn->setEnabled(true);
        }
    });

    connect(confirmBtn, &QPushButton::clicked, this, [=]{
        cniface->call("SetRealName", nickNameLineEdit->text());
        close();
    });

    connect(cancelBtn, &QPushButton::clicked, this, [=]{
        close();
    });
}

void ChangeUserNickname::setupStatus(){
    tipLabel->hide();
}

bool ChangeUserNickname::setTextDynamicInNick(QLabel *label, QString string){

    bool isOverLength = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);

    QString str = string;
    int pSize = label->width();
    if (fontSize > pSize) {
        str = fontMetrics.elidedText(string, Qt::ElideRight, pSize);
        label->setToolTip(string);
        isOverLength = true;
    } else {
        label->setToolTip("");
    }
    label->setText(str);
    return isOverLength;

}

void ChangeUserNickname::keyPressEvent(QKeyEvent * event){
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


bool ChangeUserNickname::eventFilter(QObject *watched, QEvent *event){
    if (event->type() == QEvent::MouseButtonPress){
        QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton ){
            if (watched == nickNameLineEdit){
                if (QString::compare(nickNameLineEdit->text(), g_get_real_name()) == 0){
                    nickNameLineEdit->setText("");
                }
            }
        }
    }
    return QObject::eventFilter(watched, event);
}

