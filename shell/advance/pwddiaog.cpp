#include "pwddiaog.h"

#include <QVBoxLayout>
#include <QKeyEvent>
#include <QDebug>
#include <QDBusReply>
#include <QDBusConnection>

#define AUTH_PWD "admin"

PwdDiaog::PwdDiaog(QWidget *parent) :
    QDialog(parent)
{
    initUi();
    setConnect();
}

void PwdDiaog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        if (mUnLockBtn->isEnabled())
            emit mUnLockBtn->click();
        return;
    }
    return QDialog::keyPressEvent(event);

}

void PwdDiaog::initUi()
{
    setWindowTitle(tr("Set"));
    this->setFixedSize(420, 200);

    QVBoxLayout *mInputPwdLyt = new QVBoxLayout(this);
    mInputPwdLyt->setContentsMargins(24, 24, 24, 24);
    mInputPwdLyt->setSpacing(0);

    QFrame *mInputPwdFrame = new QFrame(this);
    mInputPwdFrame->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mLyt_1= new QHBoxLayout(mInputPwdFrame);
    mLyt_1->setContentsMargins(0, 0, 0, 0);
    mLyt_1->setSpacing(8);

    FixLabel *mSetPwdLabel = new FixLabel(mInputPwdFrame);
    mSetPwdLabel->setText(tr("Input pwd to unlock \"advanced setting\"."));

    mpwd = new QLineEdit(mInputPwdFrame);
    mpwd->setEchoMode(QLineEdit::Password);
    mpwd->setAttribute(Qt::WA_InputMethodEnabled, false);  //限制中文输入法

    mLyt_1->addWidget(mpwd);

    mHintLabel = new QLabel(this);
    mHintLabel->setText("");
    mHintLabel->setStyleSheet("QLabel{color:red; font-size : 14px}");

    QFrame *mInputPwdFrame_1 = new QFrame(this);
    mInputPwdFrame_1->setFrameShape(QFrame::NoFrame);

    QHBoxLayout *mLyt_2= new QHBoxLayout(mInputPwdFrame_1);
    mLyt_2->setContentsMargins(0, 0, 0, 0);
    mLyt_2->setSpacing(16);


    mLyt_1->addWidget(mSetPwdLabel);
    mLyt_1->addWidget(mpwd);

    mCancelBtn = new QPushButton(mInputPwdFrame_1);
    mCancelBtn->setFixedWidth(96);
    mCancelBtn->setText(tr("Cancel"));

    mUnLockBtn = new QPushButton(mInputPwdFrame_1);
    mUnLockBtn->setFixedWidth(96);
    mUnLockBtn->setText(tr("UnLock"));
    mUnLockBtn->setEnabled(false);

    mLyt_2->addStretch();
    mLyt_2->addWidget(mCancelBtn);
    mLyt_2->addWidget(mUnLockBtn);

    mInputPwdLyt->addWidget(mSetPwdLabel);
    mInputPwdLyt->addSpacing(8);
    mInputPwdLyt->addWidget(mInputPwdFrame);
    mInputPwdLyt->addWidget(mHintLabel);
    mInputPwdLyt->addStretch();
    mInputPwdLyt->addWidget(mInputPwdFrame_1);

    mHintLabel->setVisible(true);

}

void PwdDiaog::setConnect()
{
    connect(mpwd, &QLineEdit::textEdited, this, &PwdDiaog::pwdInputSlot);

    connect(mCancelBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        this->close();
    });

    connect(mUnLockBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
       if (getPwd() == mpwd->text()) {
           this->close();
           emit open();
       } else {
            mHintLabel->setText(tr("Password verification failed"));
            mHintLabel->setVisible(true);
            mpwd->clear();
            mpwd->setFocus();
            mUnLockBtn->setEnabled(false);
       }
    });
}

QString PwdDiaog::getPwd()
{
    mUkccInterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                                        "/",
                                                        "com.control.center.interface",
                                                        QDBusConnection::systemBus(), this);
    if (mUkccInterface->isValid()) {
        QDBusReply<QString> result = mUkccInterface->call("getAdvancePwd");
        return result;
    }
    return "admin";
}

void PwdDiaog::pwdInputSlot(const QString &pwd)
{
    mUnLockBtn->setEnabled(true);
    mHintLabel->setVisible(false);
    if (pwd.length() <= 16 && !pwd.isEmpty()) {

    } else if (pwd.isEmpty()) {
        mUnLockBtn->setEnabled(false);
    } else {
        mpwd->setText(pwd.mid(0, 16));
    }
}

