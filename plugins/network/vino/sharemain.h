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
#ifndef SHAREMAIN_H
#define SHAREMAIN_H

#include <QWidget>
#include <QFrame>
#include <QCheckBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QButtonGroup>
#include <QByteArray>
#include <QDebug>
#include <QGSettings/QGSettings>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QSpinBox>
#include <QSystemTrayIcon>
#include <QEvent>
#include <QGestureEvent>
#include "SwitchButton/switchbutton.h"
#include "krd.h"


enum Protocol{
    VNC,
    RDP,
};

class ShareMain : public QWidget
{
    Q_OBJECT
public:
    ShareMain(QWidget *parent = nullptr);
    ~ShareMain();

private:
    QFrame *mEnableFrame;
    QFrame *mControlFrame;
    QFrame *mSecurityFrame;
    QFrame *mSecurityPwdFrame;
    QFrame *mNoticeWFrame;
    QFrame *mNoticeOFrame;
    QFrame *mNoticeNFrame;
    QFrame *mOutputFrame;
    QFrame *mInputFrame;
    QFrame *mClientFrame;
    QFrame *mClientNumFrame;
    QFrame *mVncProtocolFrame;
    QFrame *mRdpProtocalFrame;

    SwitchButton *mEnableBtn;  // 允许其他人查看桌面
    SwitchButton *mViewBtn;    // 允许连接控制屏幕
    SwitchButton *mPwdBtn;    // 要求用户输入密码
    SwitchButton *mOutputBtn;    // 选择output按钮
    SwitchButton *mPointBtn;    // 选择output按钮
    SwitchButton *mKeyboardBtn;    // 选择output按钮
    SwitchButton *mClipboardBtn;    // 选择output按钮
    SwitchButton *mMaxClientBtn;    // 选择output按钮

    QTableWidget  *mTbClients;
    QSpinBox *mMaxClientSpinBox;
    QPushButton  *mViewOnlyNBtn;
    QPushButton  *mCloseBtn;

    QLabel *mShareTitleLabel;
    QLabel *mEnableLabel;
    QLabel *mViewLabel;
    QLabel *mSecurityTitleLabel;
    QLabel *mPwdsLabel;
    QLabel *mNoticeTitleLabel;
    QLabel *mNoticeWLabel;
    QLabel *mNoticeOLabel;
    QLabel *mNoticeNLabel;
    QLabel *mHintLabel;
    QLabel *mOutputLabel;
    QLabel *mPointLabel;
    QLabel *mKeyboardLabel;
    QLabel *mClipboardLabel;
    QLabel *mMaxClientLabel;
    QLabel *mClientLabel;
    QLabel *mClientInfoLabel;
    QLabel *mOutputTitleLabel;
    QLabel *mInputTitleLabel;
    QLabel *mClientTitleLabel;
    QLabel *mProtocolTitleLabel;

    QLineEdit *mPwdLineEdit;

    QVBoxLayout *mVlayout;
    QHBoxLayout *mOutputHLayout;

    ComKylinRemoteDesktopInterface *krd;
    QList<QRadioButton*> output_list;

    QRadioButton *mVncRadioBtn;
    QRadioButton *mRdpRadioBtn;
    QButtonGroup *mBtnGroup;

    QSettings* mSettingsIni;       //配置文件
    int mIsOpen;
    int mNeedPwd;
    QString mProtocol;

private:

    void initUI();
    void initTitleLabel();
    void initEnableUI();
    void initPwdUI();
    void initProtocolUI();      // 协议选择
    void initOutputUI();
    void initInputUI();
    void initClientUI();
    void setFrame();

    void initData();
    void initConnection();

    void setFrameVisible(bool visible);
    void startKrbService();

    void update_outputs();
    void update_inputs();
    void update_auth();
    void update_clients();

    void initComponentStatus();

    void savePwdEnableState();
    void checkPwdEnableState();
private slots:
    void enableSlot(bool status);
    void pwdEnableSlot(bool status);
    void pwdInputSlot(const QString &pwd);
    void onChanged(int type);
    void onPointerClickedSlot(bool checked);
    void onKeyboardClickedSlot(bool checked);
    void onClipboardClickedSlot(bool checked);
    void on_wl_speed_valueChanged(int arg1);
    void on_pb_start_clicked();
    void maxClientValueChangedSlot(int cNum);
    void on_pb_viewonly_clicked();
    void on_pb_close_clicked();
    void exitAllClient();
    void on_pb_passwd_clicked();
    void onProtocolSelectSlot(int protocol);
};

#endif // SHAREMAIN_H
