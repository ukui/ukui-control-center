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
#ifndef ABOUT_H
#define ABOUT_H

#include <QObject>
#include <QtPlugin>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
#include <QSysInfo>
#include <QLabel>
#include <QStringList>
#include <QSharedPointer>
#include <QGSettings/QGSettings>
#include <QPushButton>
#include <QHBoxLayout>
#include <QThread>
#include <QtConcurrent>
#include <QFuture>

#include "HpQRCodeInterface.h"

#include "shell/interface.h"
#include "Label/titlelabel.h"

namespace Ui {
class About;
}

class About : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    About();
    ~About();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget *get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;
    const QString name() const Q_DECL_OVERRIDE;

private:
    void initUI(QWidget *widget);
    void retranslateUi();
    void initSearchText();
    void initActiveDbus();
    void setupSerialComponent();
    void setupVersionCompenent();
    void setupDesktopComponent();
    void setupKernelCompenent();
    void setupDiskCompenet();
    void setupSystemVersion();
    void showExtend(QString dateres);
    char *ntpdate();


    int getMonth(QString month);

    void setLabelText(QLabel *label,QString text);
    bool eventFilter(QObject *obj, QEvent *event);

    QStringList getUserDefaultLanguage();
    QStringList  readFile(QString filepath);
    QString getTotalMemory();
private:
    QWidget *pluginWidget;
    QFrame *mHoldWidget;
    QWidget *mQrCodeWidget;
    QString pluginName;
    int pluginType;

    TitleLabel *mTitleLabel;
    TitleLabel *mHoldTitleLabel;

    QFrame *mInformationFrame;
    QFrame *mActivationFrame;
    QFrame *mVersionFrame;
    QFrame *mVersionNumFrame;
    QFrame *mKernelFrame;
    QFrame *mCpuFrame;
    QFrame *mMemoryFrame;
    QFrame *mDiskFrame;
    QFrame *mDesktopFrame;
    QFrame *mUsernameFrame;

    QHBoxLayout *mDiskLayout;

    QLabel *mLogoLabel;
    QLabel *mVersionLabel_1;
    QLabel *mVersionNumLabel_1;
    QLabel *mKernelLabel_1;
    QLabel *mCpuLabel_1;
    QLabel *mMemoryLabel_1;
    QLabel *mDiskLabel_1;
    QLabel *mDesktopLabel_1;
    QLabel *mUsernameLabel_1;
    QLabel *mVersionLabel_2;
    QLabel *mVersionNumLabel_2;
    QLabel *mKernelLabel_2;
    QLabel *mCpuLabel_2;
    QLabel *mMemoryLabel_2;
    QLabel *mDiskLabel_2;
    QLabel *mDesktopLabel_2;
    QLabel *mUsernameLabel_2;
    QLabel *mStatusLabel_1;
    QLabel *mStatusLabel_2;
    QLabel *mTimeLabel_1;
    QLabel *mTimeLabel_2;
    QLabel *mSequenceLabel_1;
    QLabel *mSequenceLabel_2;
    QLabel *mTrialLabel;
    QLabel *mHpLabel;
    QLabel *mEducateIconLabel;
    QLabel *mEducateLabel;


    QPushButton *mActivationBtn;
    QPushButton *mTrialBtn;
    QPushButton *mHpBtn;
    QPushButton *mEducateBtn;

    QStringList diskInfo;
    QMap<QString, QStringList> disk2; //disk2的结构 QMap<块设备名，总大小和可用大小>

    bool mFirstLoad;

    QSharedPointer<QDBusInterface> activeInterface;

    QGSettings *themeStyleQgsettings;
    hp::QRCodeInterface *app;//hp插件

Q_SIGNALS:
    void resize();

private slots:
    void activeSlot(int activeSignal);
    void runActiveWindow();
    void ChangedSlot();

};

#endif // ABOUT_H
