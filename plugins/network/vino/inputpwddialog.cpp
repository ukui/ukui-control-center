/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "inputpwddialog.h"

#include <QDebug>
#include <QLineEdit>
#include <QSpacerItem>
#include <QLabel>
#include <QPushButton>
#include <QRegExpValidator>
#include "sharemain.h"


InputPwdDialog::InputPwdDialog(QGSettings *Keygsettiings,QWidget *parent) :
    QDialog(parent),
    mgsettings(Keygsettiings)
{

    setupInit();
    initConnect();
}

InputPwdDialog::~InputPwdDialog()
{

}

void InputPwdDialog::setupInit()
{
    setWindowTitle(tr("Set Password"));
    this->setFixedSize(380, 161);
    this->setMinimumSize(QSize(380, 161));
    this->setMaximumSize(QSize(380, 161));
    mpwd = new QLineEdit(this);
    mpwd->setAttribute(Qt::WA_InputMethodEnabled, false);  //限制中文输入法
    mpwd->setGeometry(32, 25, 316,42);
//    mpwd->setFocus();
//    mpwd->clearFocus();
    mpwd->installEventFilter(this);
    this->installEventFilter(this);

    mfirstload = true;
    mstatus = false;

    mHintLabel = new QLabel(this);
    mHintLabel->setGeometry(32,67,316,28);
    mHintLabel->setContentsMargins(8,2,8,2);
    mHintLabel->setStyleSheet("color:red;");

    mCancelBtn = new QPushButton(this);
    mCancelBtn->setContentsMargins(36,6,36,6);
    mCancelBtn->setGeometry(112,99,110,33);
    mCancelBtn->setText(tr("Cancel"));

    mConfirmBtn = new QPushButton(this);
    mConfirmBtn->setContentsMargins(36,6,36,6);
    mConfirmBtn->setGeometry(238,99,110,33);
    mConfirmBtn->setText(tr("Confirm"));

    if(QByteArray::fromBase64(mgsettings->get(kVncPwdKey).toString().toLatin1()).length() <= 8) {
        if (mgsettings->get(kVncPwdKey).toString() == "keyring") {
            mpwd->setText("");
            mConfirmBtn->setEnabled(false);
            mHintLabel->setText(tr("Password can not be blank"));
            mHintLabel->setVisible(true);
        } else {
            mpwd->setText(QByteArray::fromBase64(mgsettings->get(kVncPwdKey).toString().toLatin1()));
        }
    }
}

void InputPwdDialog::mpwdInputSlot(const QString &pwd)
{
    Q_UNUSED(pwd);
    mstatus = true;
    mConfirmBtn->setEnabled(true);
    if (pwd.length() <= 8 && !pwd.isEmpty()) {
        QByteArray text = pwd.toLocal8Bit();
        secPwd = text.toBase64();
        mHintLabel->setVisible(false);
    } else if (pwd.isEmpty()) {
        mConfirmBtn->setEnabled(false);
        mHintLabel->setText(tr("Password can not be blank"));
        mHintLabel->setStyleSheet("color:red;");
        mHintLabel->setVisible(true);
        secPwd = NULL;
    } else {
        mHintLabel->setText(tr("less than or equal to 8"));
        mHintLabel->setStyleSheet("color:red;");
        mHintLabel->setVisible(true);
        mpwd->setText(pwd.mid(0, 8));
        QByteArray text = pwd.mid(0, 8).toLocal8Bit();
        secPwd = text.toBase64();
    }
}

void InputPwdDialog::initConnect() {

    connect(mCancelBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        this->close();
    });

    connect(mConfirmBtn, &QPushButton::clicked, [=](bool checked){
        Q_UNUSED(checked)
        if (mstatus && secPwd.length() == 0) {
            return;
        } else if (!mstatus){
            mgsettings->set(kAuthenticationKey, "vnc");
            this->close();
        } else {
            mgsettings->set(kVncPwdKey, secPwd);
            mgsettings->set(kAuthenticationKey, "vnc");
            this->close();
        }
    });
    //使用textEdited信号是为了防止密码框setText时触发信号
    connect(mpwd, &QLineEdit::textEdited, this, &InputPwdDialog::mpwdInputSlot);
}

bool InputPwdDialog::eventFilter(QObject *wcg, QEvent *event)
{
    //过滤
       if(wcg==mpwd){
           if(event->type() == QEvent::MouseButtonPress){
               if(mpwd->hasFocus()){
                   if (mfirstload) {
                       mpwd->setText("");
                       mfirstload = false;
                   }
               }
           }
       }
       return QWidget::eventFilter(wcg,event);
}
