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

#if 0
const QByteArray kVinoSchemas = "org.gnome.Vino";
const QString kVinoViewOnlyKey = "view-only";
const QString kVinoPromptKey = "prompt-enabled";
const QString kAuthenticationKey = "authentication-methods";
const QString kVncPwdKey = "vnc-password";

const QByteArray kUkccVnoSchmas = "org.ukui.control-center.vino";
const QString kUkccPromptKey = "remote";

enum RequestPwd {
    NOPWD,
    NEEDPWD
};
#endif

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

    QLineEdit *mPwdLineEdit;

    QVBoxLayout *mVlayout;
    QHBoxLayout *mOutputHLayout;

    ComKylinRemoteDesktopInterface *krd;
    QList<QRadioButton*> output_list;

    QSettings* mSettingsIni;       //配置文件
    int mIsOpen;
    int mNeedPwd;

    //QRadioButton *mNoticeWBtn;
    //QRadioButton *mNoticeOBtn;
    //QRadioButton *mNoticeNBtn;
    //QLabel *mAccessLabel;
    //SwitchButton *mAccessBtn;  // 为本机确认每次访问
    //QGSettings *mVinoGsetting;
    //QSystemTrayIcon *mSysTrayIcon;
    //static bool mIsOpening = false;
private:

    void initUI();
    void initTitleLabel();
    void initEnableUI();
    void initPwdUI();
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

    void savePwdEnableState();
    void checkPwdEnableState();

    //void initShareStatus(bool isConnnect, bool isPwd);
    //void initEnableStatus();
    //void setVinoService(bool status);
    //void initSysTrayIcon();
    //void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
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

    //void viewBoxSlot();
    //void accessSlot(bool status);
    //void closeAllClient();
    //void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason);//托盘槽函数声明
};

#endif // SHAREMAIN_H
