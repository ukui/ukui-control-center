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
#include <QMouseEvent>

#include "HpQRCodeInterface.h"
#include "shell/utils/utils.h"
#include "hostnamedialog.h"
#include "privacydialog.h"
#include "statusdialog.h"

#include "shell/interface.h"
#include <ukcc/widgets/titlelabel.h>
#include <ukcc/widgets/fixlabel.h>
#include <ukcc/widgets/lightlabel.h>
#include <ukcc/widgets/switchbutton.h>

namespace Ui {
class About;
}

class About : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    About();
    ~About();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    const QString name() const Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    void initUI(QWidget *widget);
    void retranslateUi();
    void initSearchText();
    void initActiveDbus();
    void setupSerialComponent();
    void setVersionNumCompenent();
    void setupVersionCompenent();
    void setupDesktopComponent();
    void setupKernelCompenent();
    void setupDiskCompenet();
    void setupSystemVersion();
    void setHostNameCompenet();
    void setPrivacyCompent();
    void showExtend(QString dateres);
    int ntp_gethostbyname(char *dname, int family, QStringList & host);
    char *ntpdate(char *hostname);
    char *getntpdate();
    char *ntpdate();
    int getMonth(QString month);
    void reboot();
    void setLabelText(QLabel *label,QString text);
    bool eventFilter(QObject *obj, QEvent *event);

    QStringList getUserDefaultLanguage();
    QStringList  readFile(QString filepath);
    void getTotalMemory();
    void compareTime(QString date);

    QPixmap loadSvg(const QString &path, int width, int height);

    void setFrame_NoFrame(QFrame *frame);
    void setFrame_Box(QFrame *frame);
private:
    QWidget *pluginWidget;
    QFrame *mHoldWidget;
    QWidget *mQrCodeWidget;
    QString pluginName;
    int pluginType;

    TitleLabel *mTitleLabel;
    TitleLabel *mPriTitleLabel;
    TitleLabel *mHoldTitleLabel;

    QFrame *mInformationFrame;
    QFrame *mActivationFrame;
    QFrame *mVersionFrame;
    QFrame *mVersionNumFrame;
    QFrame *mVersionNumberFrame;
    QFrame *mInterVersionFrame;
    QFrame *mHostNameFrame;
    QFrame *mKernelFrame;
    QFrame *mCpuFrame;
    QFrame *mMemoryFrame;
    QFrame *mDiskFrame;
    QFrame *mDesktopFrame;
    QFrame *mUsernameFrame;
    QFrame *mPrivacyFrame;
    QFrame *mBtnFrame;

    QHBoxLayout *mDiskLayout;

    FixLabel *mLogoLabel;
    FixLabel *mVersionLabel_1;
    FixLabel *mVersionNumLabel_1;
    FixLabel *mVersionNumberLabel_1;
    FixLabel *mInterVersionLabel_1;
    FixLabel *mHostNameLabel_1;
    FixLabel *mKernelLabel_1;
    FixLabel *mCpuLabel_1;
    FixLabel *mMemoryLabel_1;
    FixLabel *mDiskLabel_1;
    FixLabel *mDesktopLabel_1;
    FixLabel *mUsernameLabel_1;
    LightLabel *mVersionLabel_2;
    LightLabel *mVersionNumLabel_2;
    LightLabel *mVersionNumberLabel_2;
    LightLabel *mInterVersionLabel_2;
    LightLabel *mHostNameLabel_2;
    LightLabel *mHostNameLabel_3;
    LightLabel *mKernelLabel_2;
    LightLabel *mCpuLabel_2;
    LightLabel *mMemoryLabel_2;
    LightLabel *mDiskLabel_2;
    LightLabel *mDesktopLabel_2;
    LightLabel *mUsernameLabel_2;
    FixLabel *mStatusLabel_1;
    LightLabel *mStatusLabel_2;
    FixLabel *mTimeLabel_1;
    FixLabel *mTimeLabel_2;
    FixLabel *mSequenceLabel_1;
    FixLabel *mSequenceLabel_2;
    FixLabel *mTrialLabel;
    FixLabel *mAndLabel;
    FixLabel *mAgreeLabel;
    QLabel *mHpLabel;
    FixLabel *mEducateIconLabel;
    QLabel *mEducateLabel;
    FixLabel *mTipLabel;



    QPushButton *mActivationBtn;
    QPushButton *mTrialBtn;
    QPushButton *mHpBtn;
    QPushButton *mEducateBtn;
    SwitchButton *mPriBtn;

    QStringList diskInfo;
    QMap<QString, QStringList> disk2; //disk2的结构 QMap<块设备名，总大小和可用大小>

    bool mFirstLoad;

    QSharedPointer<QDBusInterface> activeInterface;

    QGSettings *themeStyleQgsettings;
    hp::QRCodeInterface *app;//hp插件

    QSpacerItem *item;

    QString mMemtotal;
    QString mMemAvaliable;

    QString dateRes = "";

    QStringList mNtphostName;

    QPixmap mThemePixmap;
    bool activestatus = true;

Q_SIGNALS:
    void resize();
    void changeTheme();

private slots:
    void activeSlot(int activeSignal);
    void runActiveWindow();
    void ChangedSlot();

};

#endif // ABOUT_H
