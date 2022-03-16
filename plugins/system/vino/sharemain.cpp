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
#include "sharemain.h"

#include <QProcess>
#include <QHBoxLayout>
#include <QAbstractButton>
#include <QPushButton>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>

#include <QDBusInterface>
#include <QDBusConnection>

ShareMain::ShareMain(QWidget *parent) :
    QWidget(parent)
{
    mVlayout = new QVBoxLayout(this);
    mVlayout->setContentsMargins(0, 0, 0, 0);
    initUI();
    initConnection();
}

ShareMain::~ShareMain()
{
}

void ShareMain::initUI()
{
    QFrame *mVinoFrame = new QFrame(this);
    mVinoFrame->setMinimumSize(550, 0);
    mVinoFrame->setMaximumSize(16777215, 16777215);
    mVinoFrame->setFrameShape(QFrame::Box);

    QVBoxLayout *mVinoLyt = new QVBoxLayout(mVinoFrame);
    mVinoLyt->setContentsMargins(0, 0, 0, 0);

    mVinoTitleLabel = new TitleLabel(this);
    mVinoTitleLabel->setText(tr("Remote Desktop"));
    mEnableFrame = new QFrame(this);
    setFrame_Noframe(mEnableFrame);

    QHBoxLayout *enableHLayout = new QHBoxLayout();
    enableHLayout->setContentsMargins(12, 0, 16, 0);

    mEnableBtn = new SwitchButton(this);
     //~ contents_path /Vino/Allow others to view your desktop
    mEnableLabel = new QLabel(tr("Allow others to view your desktop"), this);
    enableHLayout->addWidget(mEnableLabel);
    enableHLayout->addStretch();
    enableHLayout->addWidget(mEnableBtn);

    mEnableFrame->setLayout(enableHLayout);

    mViewFrame = new QFrame(this);
    setFrame_Noframe(mViewFrame);

    QHBoxLayout *viewHLayout = new QHBoxLayout();
    viewHLayout->setContentsMargins(12, 0, 16, 0);

    mViewBtn = new SwitchButton(this);
    //~ contents_path /Vino/Allow connection to control screen
    mViewLabel = new QLabel(tr("Allow connection to control screen"), this);
    viewHLayout->addWidget(mViewLabel);
    viewHLayout->addStretch();
    viewHLayout->addWidget(mViewBtn);

    mViewFrame->setLayout(viewHLayout);

    mSecurityFrame = new QFrame(this);
    setFrame_Noframe(mSecurityFrame);

    QHBoxLayout *secHLayout = new QHBoxLayout();
    secHLayout->setContentsMargins(12, 0, 16, 0);

    mAccessBtn = new SwitchButton(this);
     //~ contents_path /Vino/You must confirm every visit for this machine
    mAccessLabel = new QLabel(tr("You must confirm every visit for this machine"), this);
    secHLayout->addWidget(mAccessLabel);
    secHLayout->addStretch();
    secHLayout->addWidget(mAccessBtn);

    mSecurityFrame->setLayout(secHLayout);

    mSecurityPwdFrame = new QFrame(this);
    setFrame_Noframe(mSecurityPwdFrame);

    QHBoxLayout *pwdHLayout = new QHBoxLayout();
    pwdHLayout->setContentsMargins(12, 0, 16, 0);

    mPwdBtn = new SwitchButton(this);
     //~ contents_path /Vino/Require user to enter this password:
    mPwdsLabel = new QLabel(tr("Require user to enter this password: "), this);

    mPwdinputBtn = new QPushButton(this);

    pwdHLayout->addWidget(mPwdsLabel);
    pwdHLayout->addWidget(mPwdinputBtn);
    pwdHLayout->addStretch();
    pwdHLayout->addWidget(mPwdBtn);

    mSecurityPwdFrame->setLayout(pwdHLayout);

    line_1 = setLine(mVinoFrame);
    line_2 = setLine(mVinoFrame);
    line_3 = setLine(mVinoFrame);

    mVinoLyt->addWidget(mEnableFrame);
    mVinoLyt->addWidget(line_1);
    mVinoLyt->addWidget(mViewFrame);
    mVinoLyt->addWidget(line_2);
    mVinoLyt->setSpacing(0);

    mVinoLyt->addWidget(mSecurityFrame);
    mVinoLyt->addWidget(line_3);
    mVinoLyt->addWidget(mSecurityPwdFrame);

    mVlayout->addWidget(mVinoTitleLabel);
    mVlayout->setSpacing(8);
    mVlayout->addWidget(mVinoFrame);
    mVlayout->addStretch();
}

void ShareMain::initConnection()
{
    QByteArray id(kVinoSchemas);
    if (QGSettings::isSchemaInstalled(id)) {
        mVinoGsetting = new QGSettings(kVinoSchemas, QByteArray(), this);

        initEnableStatus();

        connect(mEnableBtn, &SwitchButton::checkedChanged, this, &ShareMain::enableSlot);
        connect(mViewBtn, &SwitchButton::checkedChanged, this, &ShareMain::viewBoxSlot);
        connect(mAccessBtn, &SwitchButton::checkedChanged, this, &ShareMain::accessSlot);
        connect(mPwdBtn, &SwitchButton::checkedChanged, this, &ShareMain::pwdEnableSlot);
        connect(mPwdinputBtn, &QPushButton::clicked, this, &ShareMain::pwdInputSlot);
    }
}

void ShareMain::initEnableStatus()
{
    bool isShared = mVinoGsetting->get(kVinoViewOnlyKey).toBool();
    bool secPwd = mVinoGsetting->get(kVinoPromptKey).toBool();
    QString pwd = mVinoGsetting->get(kAuthenticationKey).toString();
    secpwd = mVinoGsetting->get(kVncPwdKey).toString();

    mAccessBtn->setChecked(secPwd);
    mViewBtn->setChecked(!isShared);
    if (pwd == "vnc") {
        if (secpwd == "keyring") {
            mPwdBtn->setChecked(false);
            mPwdinputBtn->hide();
            mVinoGsetting->set(kAuthenticationKey, "none");
        } else {
            mPwdBtn->setChecked(true);
            mPwdinputBtn->setText(QByteArray::fromBase64(secpwd.toLatin1()));
        }
    } else {
        mPwdBtn->setChecked(false);
        mPwdinputBtn->setVisible(false);
    }

    QProcess *process = new QProcess;

    process->start("systemctl", QStringList() << "--user" << "is-active" << "vino-server.service");
    process->waitForFinished();

    setFrameVisible((process->readAllStandardOutput().replace("\n", "") == "active"));
    delete process;
}

void ShareMain::setFrameVisible(bool visible)
{
    mEnableBtn->setChecked(visible);
    mViewFrame->setVisible(visible);
    mSecurityFrame->setVisible(visible);
    mSecurityPwdFrame->setVisible(visible);
    line_1->setVisible(visible);
    line_2->setVisible(visible);
    line_3->setVisible(visible);
}

void ShareMain::setVinoService(bool status)
{
    QDBusInterface vinoIfc("org.ukui.SettingsDaemon",
                           "/org/ukui/SettingsDaemon/Sharing",
                           "org.ukui.SettingsDaemon.Sharing",
                           QDBusConnection::sessionBus());
    if (vinoIfc.isValid()) {
        if (status) {
            vinoIfc.call("EnableService", "vino-server");
        } else {
            vinoIfc.call("DisableService", "vino-server");
        }
    }
}

void ShareMain::setFrame_Noframe(QFrame *frame)
{
    frame->setMinimumSize(QSize(550, 60));
    frame->setMaximumSize(QSize(16777215, 60));
    frame->setFrameShape(QFrame::NoFrame);
}

QFrame *ShareMain::setLine(QFrame *frame)
{
    QFrame *line = new QFrame(frame);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

void ShareMain::enableSlot(bool status)
{
    setFrameVisible(status);
    setVinoService(status);
}

void ShareMain::viewBoxSlot(bool status)
{
    mVinoGsetting->set(kVinoViewOnlyKey, !status);
}

void ShareMain::accessSlot(bool status)
{
    if (status) {
        mVinoGsetting->set(kVinoPromptKey, true);
    } else {
        mVinoGsetting->set(kVinoPromptKey, false);
    }
}

void ShareMain::pwdEnableSlot(bool status)
{
    if (status) {
        mPwdinputBtn->setVisible(secpwd == "keyring" ? false:true);
        mPwdinputBtn->setText(QByteArray::fromBase64(mVinoGsetting->get(kVncPwdKey).toString().toLatin1()));
        pwdInputSlot();
        mPwdinputBtn->setVisible(status);
        if (mVinoGsetting->get(kAuthenticationKey).toString() == "none") {
            mPwdBtn->setChecked(false);
        }

    } else {
        mPwdinputBtn->setVisible(false);
        mVinoGsetting->set(kAuthenticationKey, "none");
    }
}

void ShareMain::pwdInputSlot()
{
    InputPwdDialog *mwindow = new InputPwdDialog(mVinoGsetting,this);
    mwindow->exec();
    secpwd = mVinoGsetting->get(kVncPwdKey).toString();
    mPwdinputBtn->setText(QByteArray::fromBase64(secpwd.toLatin1()));
}
