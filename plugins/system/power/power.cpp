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
#include "power.h"
#include "powermacrodata.h"

#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QSettings>
#include <QFormLayout>

#include "../../../shell/utils/utils.h"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

#include "libupower-glib/upower.h"

typedef enum {
    BALANCE,
    SAVING,
    CUSTDOM
}MODE;

typedef enum {
    PRESENT,
    ALWAYS
}ICONDISPLAY;

Power::Power() : mFirstLoad(true)
{
    pluginName = tr("Power");
    pluginType = SYSTEM;
}

Power::~Power() {
    if (!mFirstLoad) {

    }
}

QString Power::get_plugin_name() {
    return pluginName;
}

int Power::get_plugin_type() {
    return pluginType;
}

QWidget * Power::get_plugin_ui() {
    if (mFirstLoad) {
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        InitUI(pluginWidget);
        isLidPresent();
        isHibernateSupply();
        isExitBattery();
        initSearText();
        resetui();

        const QByteArray styleID(STYLE_FONT_SCHEMA);
        const QByteArray id(POWERMANAGER_SCHEMA);
        const QByteArray iid(SESSION_SCHEMA);
        const QByteArray iiid(SCREENSAVER_SCHEMA);


        if (QGSettings::isSchemaInstalled(id) && QGSettings::isSchemaInstalled(styleID) && QGSettings::isSchemaInstalled(iid) && QGSettings::isSchemaInstalled(iiid)) {
            settings = new QGSettings(id, QByteArray(), this);
            stylesettings = new QGSettings(styleID, QByteArray(), this);
            sessionsettings = new QGSettings(iid, QByteArray(), this);
            screensettings = new QGSettings(iiid, QByteArray(), this);
            setupComponent();
            initCustomPlanStatus();
            setupConnect();
            connect(stylesettings,&QGSettings::changed,[=](QString key)
            {
                if("systemFont" == key || "systemFontSize" == key)
                {
                    retranslateUi();

                }
            });
            connect(settings,&QGSettings::changed,[=](QString key){
                initCustomPlanStatus();
            });
        }



    }

    return pluginWidget;
}

void Power::plugin_delay_control() {

}

const QString Power::name() const {

    return QStringLiteral("power");
}

void Power::InitUI(QWidget *widget)
{
    QVBoxLayout *mverticalLayout = new QVBoxLayout(widget);
    mverticalLayout->setSpacing(0);
    mverticalLayout->setContentsMargins(0, 0, 32, 40);

    QWidget *Powerwidget = new QWidget(widget);
    Powerwidget->setMinimumSize(QSize(550, 0));
    Powerwidget->setMaximumSize(QSize(960, 16777215));

    QVBoxLayout *PowerLayout = new QVBoxLayout(Powerwidget);
    PowerLayout->setContentsMargins(0, 0, 0, 0);
    PowerLayout->setSpacing(1);

    CustomTitleLabel = new TitleLabel(Powerwidget);

    PowerLayout->addWidget(CustomTitleLabel);
    PowerLayout->addSpacing(7);

    mSleepPwdFrame = new QFrame(Powerwidget);
    mSleepPwdFrame->setMinimumSize(QSize(550, 60));
    mSleepPwdFrame->setMaximumSize(QSize(960, 60));
    mSleepPwdFrame->setFrameShape(QFrame::Box);

    QHBoxLayout *mSleepPwdLayout = new QHBoxLayout(mSleepPwdFrame);
    mSleepPwdLayout->setContentsMargins(16, 0, 16, 0);

    mSleepPwdLabel = new QLabel(mSleepPwdFrame);
    mSleepPwdLabel->setMinimumSize(550,60);

    mSleepPwdBtn = new SwitchButton(mSleepPwdFrame);

    mSleepPwdLayout->addWidget(mSleepPwdLabel);
    mSleepPwdLayout->addStretch();
    mSleepPwdLayout->addWidget(mSleepPwdBtn);

    PowerLayout->addWidget(mSleepPwdFrame);


    mWakenPwdFrame = new QFrame(Powerwidget);
    mWakenPwdFrame->setMinimumSize(QSize(550, 49));
    mWakenPwdFrame->setMaximumSize(QSize(960, 49));
    mWakenPwdFrame->setFrameShape(QFrame::Box);

    QHBoxLayout *mWakenPwdLayout = new QHBoxLayout(mWakenPwdFrame);
    mWakenPwdLayout->setContentsMargins(16, 0, 16, 0);

    mWakenPwdLabel = new QLabel(mWakenPwdFrame);
    mWakenPwdLabel->setMinimumSize(550,49);

    mWakenPwdBtn = new SwitchButton(mWakenPwdFrame);

    mWakenPwdLayout->addWidget(mWakenPwdLabel);
    mWakenPwdLayout->addStretch();
    mWakenPwdLayout->addWidget(mWakenPwdBtn);

    PowerLayout->addWidget(mWakenPwdFrame);

    mPowerKeyFrame = new QFrame(Powerwidget);
    mPowerKeyFrame->setObjectName("mpowerkeyframe");
    mPowerKeyFrame->setMinimumSize(QSize(550, 69));
    mPowerKeyFrame->setMaximumSize(QSize(960, 69));
    mPowerKeyFrame->setFrameShape(QFrame::Box);


    QHBoxLayout *mPowerKeyLayout = new QHBoxLayout(mPowerKeyFrame);
    mPowerKeyLayout->setContentsMargins(16, 0, 16, 0);

    mPowerKeyLabel = new QLabel(mPowerKeyFrame);
    mPowerKeyLabel->setMinimumSize(550,69);

    mPowerKeyComboBox = new QComboBox(mPowerKeyFrame);
    mPowerKeyComboBox->setFixedHeight(40);
    mPowerKeyComboBox->setMinimumWidth(200);

    mPowerKeyLayout->addWidget(mPowerKeyLabel);
    mPowerKeyLayout->addWidget(mPowerKeyComboBox);

    PowerLayout->addWidget(mPowerKeyFrame);

    mCloseFrame = new QFrame(Powerwidget);
    mCloseFrame->setObjectName("mcloseframe");
    mCloseFrame->setMinimumSize(QSize(550, 60));
    mCloseFrame->setMaximumSize(QSize(960, 60));
    mCloseFrame->setFrameShape(QFrame::Box);

    QHBoxLayout *mCloseLayout = new QHBoxLayout(mCloseFrame);
    mCloseLayout->setContentsMargins(16, 0, 16, 0);

    mCloseLabel = new QLabel(mCloseFrame);
    mCloseLabel->setMinimumSize(550,60);

    mCloseComboBox = new QComboBox(mCloseFrame);
    mCloseComboBox->setFixedHeight(40);
    mCloseComboBox->setMinimumWidth(200);

    mCloseLayout->addWidget(mCloseLabel);
    mCloseLayout->addWidget(mCloseComboBox);

    PowerLayout->addWidget(mCloseFrame);
    PowerLayout->addSpacing(1);

    mSleepFrame = new QFrame(Powerwidget);
    mSleepFrame->setObjectName("msleepframe");
    mSleepFrame->setMinimumSize(QSize(550, 59));
    mSleepFrame->setMaximumSize(QSize(960, 59));
    mSleepFrame->setFrameShape(QFrame::Box);


    QHBoxLayout *mSleepLayout = new QHBoxLayout(mSleepFrame);
    mSleepLayout->setContentsMargins(16, 0, 16, 0);

    mSleepLabel = new QLabel(mSleepFrame);
    mSleepLabel->setMinimumSize(550,59);

    mSleepComboBox = new QComboBox(mSleepFrame);
    mSleepComboBox->setFixedHeight(40);
    mSleepComboBox->setMinimumWidth(200);

    mSleepLayout->addWidget(mSleepLabel);
    mSleepLayout->addWidget(mSleepComboBox);

    PowerLayout->addWidget(mSleepFrame);

    mCloseLidFrame = new QFrame(Powerwidget);
    mCloseLidFrame->setObjectName("mcloselidframe");
    mCloseLidFrame->setMinimumSize(QSize(550, 59));
    mCloseLidFrame->setMaximumSize(QSize(960, 59));
    mCloseLidFrame->setFrameShape(QFrame::Box);


    QHBoxLayout *mCloseLidLayout = new QHBoxLayout(mCloseLidFrame);
    mCloseLidLayout->setContentsMargins(16, 0, 16, 0);

    mCloseLidLabel = new QLabel(mCloseLidFrame);
    mCloseLidLabel->setMinimumSize(550,59);

    mCloseLidComboBox = new QComboBox(mCloseLidFrame);
    mCloseLidComboBox->setFixedHeight(40);
    mCloseLidComboBox->setMinimumWidth(200);

    mCloseLidLayout->addWidget(mCloseLidLabel);
    mCloseLidLayout->addWidget(mCloseLidComboBox);

    PowerLayout->addWidget(mCloseLidFrame);
    PowerLayout->addSpacing(39);

    PowerPlanTitleLabel = new TitleLabel(Powerwidget);

    PowerLayout->addWidget(PowerPlanTitleLabel);
    PowerLayout->addSpacing(7);

    mPowerFrame = new QFrame(Powerwidget);
    mPowerFrame->setObjectName("mpowerframe");
    mPowerFrame->setMinimumSize(QSize(550, 60));
    mPowerFrame->setMaximumSize(QSize(960, 60));
    mPowerFrame->setFrameShape(QFrame::Box);

    QHBoxLayout *mPowerLayout = new QHBoxLayout(mPowerFrame);
    mPowerLayout->setContentsMargins(16, 0, 16, 0);


    mPowerLabel = new QLabel(mPowerFrame);
    mPowerLabel->setMinimumSize(550,60);

    mPowerComboBox = new QComboBox(mPowerFrame);
    mPowerComboBox->setFixedHeight(40);
    mPowerComboBox->setMinimumWidth(200);

    mPowerLayout->addWidget(mPowerLabel);
    mPowerLayout->addWidget(mPowerComboBox);

    PowerLayout->addWidget(mPowerFrame);

    mBatteryFrame = new QFrame(Powerwidget);
    mBatteryFrame->setObjectName("mbatteryframe");
    mBatteryFrame->setMinimumSize(QSize(550, 59));
    mBatteryFrame->setMaximumSize(QSize(960, 59));
    mBatteryFrame->setFrameShape(QFrame::Box);

    QHBoxLayout *mBatteryLayout = new QHBoxLayout(mBatteryFrame);
    mBatteryLayout->setContentsMargins(16, 0, 16, 0);

    mBatteryLabel = new QLabel(mBatteryFrame);
    mBatteryLabel->setMinimumSize(550,59);

    mBatteryComboBox = new QComboBox(mBatteryFrame);
    mBatteryComboBox->setFixedHeight(40);
    mBatteryComboBox->setMinimumWidth(200);

    mBatteryLayout->addWidget(mBatteryLabel);
    mBatteryLayout->addWidget(mBatteryComboBox);

    PowerLayout->addWidget(mBatteryFrame);
    PowerLayout->addSpacing(40);

    BatteryPlanTitleLabel = new TitleLabel(Powerwidget);
    PowerLayout->addWidget(BatteryPlanTitleLabel);
    PowerLayout->addSpacing(7);

    mDarkenFrame = new QFrame(Powerwidget);
    mDarkenFrame->setObjectName("mdarkenframe");
    mDarkenFrame->setMinimumSize(QSize(550, 59));
    mDarkenFrame->setMaximumSize(QSize(960, 59));
    mDarkenFrame->setFrameShape(QFrame::Box);

    QHBoxLayout *mDarkenLayout = new QHBoxLayout(mDarkenFrame);
    mDarkenLayout->setContentsMargins(16, 0, 16, 0);

    mDarkenLabel = new QLabel(mDarkenFrame);
    mDarkenLabel->setMinimumSize(550,59);

    mDarkenComboBox = new QComboBox(mDarkenFrame);
    mDarkenComboBox->setFixedHeight(40);
    mDarkenComboBox->setMinimumWidth(200);

    mDarkenLayout->addWidget(mDarkenLabel);
    mDarkenLayout->addWidget(mDarkenComboBox);

    PowerLayout->addWidget(mDarkenFrame);

    mLowpowerFrame = new QFrame(Powerwidget);
    mLowpowerFrame->setObjectName("mlowpowerframe");
    mLowpowerFrame->setMinimumSize(QSize(550, 60));
    mLowpowerFrame->setMaximumSize(QSize(960, 60));
    mLowpowerFrame->setFrameShape(QFrame::Box);


    mLowpowerLabel1 = new QLabel(mLowpowerFrame);
    mLowpowerLabel1->setFixedSize(84,60);
    mLowpowerLabel2 = new QLabel(mLowpowerFrame);
    mLowpowerLabel2->setFixedSize(72,60);

    QHBoxLayout *mLowpowerLayout = new QHBoxLayout(mLowpowerFrame);
    mLowpowerLayout->setContentsMargins(16, 0, 16, 0);

    mLowpowerComboBox1 = new QComboBox(mLowpowerFrame);
    mLowpowerComboBox1->setFixedSize(70, 40);
    mLowpowerComboBox2 = new QComboBox(mLowpowerFrame);
    mLowpowerComboBox2->setFixedHeight(40);
    mLowpowerComboBox2->setMinimumWidth(200);

    mLowpowerLayout->setSpacing(16);
    mLowpowerLayout->addWidget(mLowpowerLabel1);
    mLowpowerLayout->addWidget(mLowpowerComboBox1);
    mLowpowerLayout->addWidget(mLowpowerLabel2);
    mLowpowerLayout->addSpacerItem(new QSpacerItem(284, 20, QSizePolicy::Maximum));
    mLowpowerLayout->addWidget(mLowpowerComboBox2);

    PowerLayout->addWidget(mLowpowerFrame);

    mNoticeLFrame = new QFrame(Powerwidget);
    mNoticeLFrame->setObjectName("mnoticeframe");
    mNoticeLFrame->setMinimumSize(QSize(550, 60));
    mNoticeLFrame->setMaximumSize(QSize(960, 60));
    mNoticeLFrame->setFrameShape(QFrame::Box);


    QHBoxLayout *mNoticeLayout = new QHBoxLayout(mNoticeLFrame);
    mNoticeLayout->setContentsMargins(16, 0, 16, 0);

    mNoticeLabel = new QLabel(mNoticeLFrame);
    mNoticeLabel->setMinimumSize(550,59);

    mNoticeComboBox = new QComboBox(mNoticeLFrame);
    mNoticeComboBox->setFixedHeight(40);
    mNoticeComboBox->setMinimumWidth(200);

    mNoticeLayout->addWidget(mNoticeLabel);
    mNoticeLayout->addWidget(mNoticeComboBox);

    PowerLayout->addWidget(mNoticeLFrame);

    mLowSaveFrame = new QFrame(Powerwidget);
    mLowSaveFrame->setObjectName("mlowsaveframe");
    mLowSaveFrame->setMinimumSize(QSize(550, 60));
    mLowSaveFrame->setMaximumSize(QSize(960, 60));
    mLowSaveFrame->setFrameShape(QFrame::Box);


    QHBoxLayout *mLowSaveLayout = new QHBoxLayout(mLowSaveFrame);
    mLowSaveLayout->setContentsMargins(16, 0, 16, 0);

    mLowSaveLabel = new QLabel(mLowSaveFrame);
    mLowSaveLabel->setMinimumSize(550,59);

    mLowSaveBtn = new SwitchButton(mLowSaveFrame);

    mLowSaveLayout->addWidget(mLowSaveLabel);
    mLowSaveLayout->addStretch();
    mLowSaveLayout->addWidget(mLowSaveBtn);

    PowerLayout->addWidget(mLowSaveFrame);

    mBatterySaveFrame = new QFrame(Powerwidget);
    mBatterySaveFrame->setObjectName("mbatterysaveframe");
    mBatterySaveFrame->setMinimumSize(QSize(550, 60));
    mBatterySaveFrame->setMaximumSize(QSize(960, 60));
    mBatterySaveFrame->setFrameShape(QFrame::Box);

    QHBoxLayout *mBatterySaveLayout = new QHBoxLayout(mBatterySaveFrame);
    mBatterySaveLayout->setContentsMargins(16, 0, 16, 0);

    mBatterySaveLabel = new QLabel(mBatterySaveFrame);
    mBatterySaveLabel->setMinimumSize(550,59);

    mBatterySaveBtn = new SwitchButton(mBatterySaveFrame);

    mBatterySaveLayout->addWidget(mBatterySaveLabel);
    mBatterySaveLayout->addStretch();
    mBatterySaveLayout->addWidget(mBatterySaveBtn);

    PowerLayout->addWidget(mBatterySaveFrame);

    mDisplayTimeFrame = new QFrame(Powerwidget);
    mDisplayTimeFrame->setObjectName("mdisplaytimeframe");
    mDisplayTimeFrame->setMinimumSize(QSize(550, 60));
    mDisplayTimeFrame->setMaximumSize(QSize(960, 60));
    mDisplayTimeFrame->setFrameShape(QFrame::Box);


    QHBoxLayout *mDisplayTimeLayout = new QHBoxLayout(mDisplayTimeFrame);
    mDisplayTimeLayout->setContentsMargins(16, 0, 16, 0);

    mDisplayTimeLabel = new QLabel(mDisplayTimeFrame);
    mDisplayTimeLabel->setMinimumSize(550,59);

    mDisplayTimeBtn = new SwitchButton(mDisplayTimeFrame);

    mDisplayTimeLayout->addWidget(mDisplayTimeLabel);
    mDisplayTimeLayout->addStretch();
    mDisplayTimeLayout->addWidget(mDisplayTimeBtn);

    PowerLayout->addWidget(mDisplayTimeFrame);

    mverticalLayout->addWidget(Powerwidget);
    mverticalLayout->addStretch();

    retranslateUi();
}

void Power::retranslateUi()
{
    if (QLabelSetText(mSleepPwdLabel, tr("Require password when sleep/hibernation"))) {
        mSleepPwdLabel->setToolTip(tr("Require password when sleep/hibernation"));
    }

    if (QLabelSetText(mWakenPwdLabel, tr("Password required when waking up the screen"))) {
        mSleepPwdLabel->setToolTip(tr("Password required when waking up the screen"));
    }

    if (QLabelSetText(mPowerKeyLabel, tr("Press the power button"))) {
        mPowerKeyLabel->setToolTip("Press the power button");
    }

    if (QLabelSetText(mCloseLabel, tr("Time to close display"))) {
        mCloseLabel->setToolTip(tr("Time to close display"));
    }

    if (QLabelSetText(mSleepLabel, tr("Time to sleep"))) {
        mSleepLabel->setToolTip(tr("Time to sleep"));
    }

    if (QLabelSetText(mCloseLidLabel, tr("Notebook cover"))) {
        mCloseLidLabel->setToolTip(tr("Notebook cover"));
    }

    if (QLabelSetText(mPowerLabel, tr("Using power"))) {
        mPowerLabel->setToolTip(tr("Using power"));
    }

    if (QLabelSetText(mBatteryLabel, tr("Using battery"))) {
        mBatteryLabel->setToolTip(tr("Using power"));
    }

    if (QLabelSetText(mDarkenLabel, tr(" Time to darken"))) {
        mDarkenLabel->setToolTip(tr(" Time to darken"));
    }

    if (QLabelSetText(mLowpowerLabel1, tr("Battery level is lower than"))) {
        mLowpowerLabel1->setToolTip(tr("Battery level is lower than"));
    }

    mLowpowerLabel2->setText(tr("Run"));

    if (QLabelSetText(mNoticeLabel, tr("Low battery notification"))) {
        mNoticeLabel->setToolTip(tr("Low battery notification"));
    }

    if (QLabelSetText(mLowSaveLabel, tr("Automatically run saving mode when low battery"))) {
        mLowSaveLabel->setToolTip(tr("Automatically run saving mode when the low battery"));
    }

    if (QLabelSetText(mBatterySaveLabel, tr("Automatically run saving mode when using battery"))) {
        mBatterySaveLabel->setToolTip(tr("Automatically run saving mode when using battery"));
    }

    if (QLabelSetText(mDisplayTimeLabel, tr("Display remaining charging time and usage time"))) {
        mDisplayTimeLabel->setToolTip(tr("Display remaining charging time and usage time"));
    }
}

void Power::resetui()
{
    //9X0隐藏这些设置项
    if (Utils::isWayland()) {
        mNoticeLFrame->hide();
        mLowSaveFrame->hide();
        mBatterySaveFrame->hide();
        mDisplayTimeFrame->hide();
        mWakenPwdFrame->hide();
        mCloseFrame->hide();
    }

    //不存在盖子隐藏该项
    if (!isExitsLid) {
        mCloseLidFrame->hide();
    }

    //不存在电池隐藏这些设置项
    if (!hasBat) {
        mBatteryFrame->hide();
        BatteryPlanTitleLabel->hide();
        mDarkenFrame->hide();
        mLowpowerFrame->hide();
        mNoticeLFrame->hide();
        mLowSaveFrame->hide();
        mBatterySaveFrame->hide();
        mDisplayTimeFrame->hide();
    }
}

void Power::initSearText()
{
    //~ contents_path /power/General
    CustomTitleLabel->setText(tr("General"));
    //~ contents_path /power/Select Powerplan
    PowerPlanTitleLabel->setText(tr("Select Powerplan"));
    //~ contents_path /power/Battery saving plan
    BatteryPlanTitleLabel->setText((tr("Battery saving plan")));
}

void Power::setupComponent()
{
    // 合盖
    closeLidStringList << tr("nothing") << tr("blank") << tr("suspend") << tr("shutdown");
    mCloseLidComboBox->insertItem(0, closeLidStringList.at(0), "nothing");
    mCloseLidComboBox->insertItem(1, closeLidStringList.at(1), "blank");
    mCloseLidComboBox->insertItem(2, closeLidStringList.at(2), "suspend");
    mCloseLidComboBox->insertItem(3, closeLidStringList.at(3), "shutdown");
    if (!Utils::isWayland() && isExitHibernate){
        closeLidStringList << tr("hibernate");
        mCloseLidComboBox->insertItem(4, closeLidStringList.at(4), "hibernate");
     }

    //按下电源键时
    buttonStringList << tr("interactive") << tr("suspend") << tr("shutdown") << tr("hibernate");
    mPowerKeyComboBox->insertItem(0, buttonStringList.at(0), "interactive");
    mPowerKeyComboBox->insertItem(1, buttonStringList.at(1), "suspend");
    mPowerKeyComboBox->insertItem(2, buttonStringList.at(2), "shutdown");
    if (isExitHibernate) {
        mPowerKeyComboBox->insertItem(3, buttonStringList.at(3), "hibernate");
    }

    //关闭显示器
    closeStringList << tr("5min") << tr("10minn") << tr("15min") << tr("30min") << tr("1h") << tr("2h") << tr("never");
    mCloseComboBox->insertItem(0, closeStringList.at(0), QVariant::fromValue(5));
    mCloseComboBox->insertItem(1, closeStringList.at(1), QVariant::fromValue(10));
    mCloseComboBox->insertItem(2, closeStringList.at(2), QVariant::fromValue(15));
    mCloseComboBox->insertItem(3, closeStringList.at(3), QVariant::fromValue(30));
    mCloseComboBox->insertItem(4, closeStringList.at(4), QVariant::fromValue(60));
    mCloseComboBox->insertItem(5, closeStringList.at(5), QVariant::fromValue(120));
    mCloseComboBox->insertItem(6, closeStringList.at(6), QVariant::fromValue(0));

    //睡眠
    sleepStringList << tr("10min") << tr("15min") << tr("30min") << tr("1h") << tr("2h") << tr("3h") << tr("never");
    mSleepComboBox->insertItem(0, sleepStringList.at(0), QVariant::fromValue(10));
    mSleepComboBox->insertItem(1, sleepStringList.at(1), QVariant::fromValue(15));
    mSleepComboBox->insertItem(2, sleepStringList.at(2), QVariant::fromValue(30));
    mSleepComboBox->insertItem(3, sleepStringList.at(3), QVariant::fromValue(60));
    mSleepComboBox->insertItem(4, sleepStringList.at(4), QVariant::fromValue(120));
    mSleepComboBox->insertItem(5, sleepStringList.at(5), QVariant::fromValue(180));
    mSleepComboBox->insertItem(6, sleepStringList.at(6), QVariant::fromValue(0));

    //电源计划
    PowerplanStringList << tr("Balance Model") << tr("Save Model")<<tr("Performance Model");
    mPowerComboBox->insertItem(0, PowerplanStringList.at(0), "Balance Model");
    mPowerComboBox->insertItem(1, PowerplanStringList.at(1), "Save Model");
    mPowerComboBox->insertItem(2, PowerplanStringList.at(2), "Performance Model");

    BatteryplanStringList << tr("Balance Model") << tr("Save Model")<<tr("Performance Model");
    mBatteryComboBox->insertItem(0, BatteryplanStringList.at(0), "Balance Model");
    mBatteryComboBox->insertItem(1, BatteryplanStringList.at(1), "Save Model");
     mBatteryComboBox->insertItem(2, BatteryplanStringList.at(2), "Performance Model");

    //变暗
    DarkenStringList << tr("1min") << tr("5min") << tr("10min") << tr("20min") << tr("never");
    mDarkenComboBox->insertItem(0, DarkenStringList.at(0), QVariant::fromValue(1));
    mDarkenComboBox->insertItem(1, DarkenStringList.at(1), QVariant::fromValue(5));
    mDarkenComboBox->insertItem(2, DarkenStringList.at(2), QVariant::fromValue(10));
    mDarkenComboBox->insertItem(3, DarkenStringList.at(3), QVariant::fromValue(20));
    mDarkenComboBox->insertItem(4, DarkenStringList.at(4), QVariant::fromValue(0));


    //低电量时执行
    LowpowerStringList << tr("nothing") << tr("blank") << tr("suspend") << tr("shutdown");
    mLowpowerComboBox2->insertItem(0, LowpowerStringList.at(0), "nothing");
    mLowpowerComboBox2->insertItem(1, LowpowerStringList.at(1), "blank");
    mLowpowerComboBox2->insertItem(2, LowpowerStringList.at(2), "suspend");
    mLowpowerComboBox2->insertItem(3, LowpowerStringList.at(3), "shutdown");
    if (isExitHibernate){
        LowpowerStringList << tr("hibernate");
        mLowpowerComboBox2->insertItem(4, LowpowerStringList.at(4), "hibernate");
     }

    //低电量通知
    for (int i = 1; i < 5; i++) {
        mNoticeComboBox->insertItem(i-1, QString("%1%").arg(i*10));
    }

    //电池低电量范围
    int batteryRemain = settings->get(PER_ACTION_CRI).toInt();
    for(int i = 5; i < batteryRemain; i++) {
        mLowpowerComboBox1->insertItem(i - 5, QString("%1%").arg(i));
    }
}

void Power::setupConnect()
{
    connect(mSleepPwdBtn,&SwitchButton::checkedChanged, [=](bool checked){
       screensettings->set(SLEEP_ACTIVATION_ENABLED,checked);
    });

    connect(mWakenPwdBtn,&SwitchButton::checkedChanged, [=](bool checked){
       settings->set(LOCK_BLANK_SCREEN,checked);
    });

    connect(mPowerKeyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        settings->set(BUTTON_POWER_KEY, mPowerKeyComboBox->itemData(index));
    });

    connect(mCloseComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        settings->set(SLEEP_DISPLAY_AC_KEY, QVariant(mCloseComboBox->currentData(Qt::UserRole).toInt() * 60));
        settings->set(SLEEP_DISPLAY_BATT_KEY, QVariant(mCloseComboBox->currentData(Qt::UserRole).toInt() * 60));
    });

    connect(mSleepComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        settings->set(SLEEP_COMPUTER_AC_KEY, QVariant(mSleepComboBox->currentData(Qt::UserRole).toInt() * 60));
        settings->set(SLEEP_COMPUTER_BATT_KEY, QVariant(mSleepComboBox->currentData(Qt::UserRole).toInt() * 60));
    });

    connect(mCloseLidComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        settings->set(BUTTON_LID_AC_KEY, mCloseLidComboBox->itemData(index));
        settings->set(BUTTON_LID_BATT_KET, mCloseLidComboBox->itemData(index));
    });

    if (settings->keys().contains("powerPolicyAc") && settings->keys().contains("powerPolicyBattery")) {
        connect(mPowerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            if (index == 2) {
                 settings->set(POWER_POLICY_AC, 0);
            } else {
                 settings->set(POWER_POLICY_AC, index + 1);
            }
        });

        connect(mBatteryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            //当开启了 低电量自动开启节能模式 时，在此低电量范围内调整电池计划，则自动关闭 低电量自动开启节能模式
            if (!Utils::isWayland() && settings->keys().contains("lowBatteryAutoSave")) {
                if (mLowSaveBtn->isChecked() &&  getBattery() <= settings->get(PERCENTAGE_LOW).toDouble()) {
                    mLowSaveBtn->setChecked(false);
                }
            }
            if (index == 2) {
                 settings->set(POWER_POLICY_BATTARY, 0);
            } else {
                 settings->set(POWER_POLICY_BATTARY, index + 1);
            }
        });
    }

    connect(mDarkenComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        Q_UNUSED(index)
        settings->set(IDLE_DIM_TIME_KEY, QVariant(mDarkenComboBox->currentData(Qt::UserRole).toInt() * 60));
    });

    connect(mLowpowerComboBox1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        settings->set(PER_ACTION_KEY, index + 5);
    });

    connect(mLowpowerComboBox2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        settings->set(ACTION_CRI_BTY, mLowpowerComboBox2->itemData(index));
    });

    connect(mNoticeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        settings->set(PERCENTAGE_LOW, (index + 1)*10);
    });

    connect(mLowSaveBtn,&SwitchButton::checkedChanged, [=](bool checked){
       settings->set(LOW_BATTERY_AUTO_SAVE,checked);
    });

    connect(mBatterySaveBtn,&SwitchButton::checkedChanged, [=](bool checked){
       settings->set(ON_BATTERY_AUTO_SAVE,checked);
    });

    connect(mDisplayTimeBtn,&SwitchButton::checkedChanged, [=](bool checked){
       settings->set(DISPLAY_LEFT_TIME_OF_CHARGE_AND_DISCHARGE,checked);
    });

}

void Power::initCustomPlanStatus()
{
    // 信号阻塞
    mPowerKeyComboBox->blockSignals(true);
    mCloseComboBox->blockSignals(true);
    mSleepComboBox->blockSignals(true);
    mCloseLidComboBox->blockSignals(true);
    mPowerComboBox->blockSignals(true);
    mBatteryComboBox->blockSignals(true);
    mDarkenComboBox->blockSignals(true);
    mLowpowerComboBox1->blockSignals(true);
    mLowpowerComboBox2->blockSignals(true);
    mNoticeComboBox->blockSignals(true);
    mSleepPwdBtn->blockSignals(true);
    mWakenPwdBtn->blockSignals(true);
    mLowSaveBtn->blockSignals(true);
    mBatterySaveBtn->blockSignals(true);
    mDisplayTimeBtn->blockSignals(true);

    mPowerKeyComboBox->setCurrentIndex(mPowerKeyComboBox->findData(settings->get(BUTTON_POWER_KEY).toString()));
    mSleepComboBox->setCurrentIndex(mSleepComboBox->findData(settings->get(SLEEP_COMPUTER_AC_KEY).toInt() / FIXES));
    mCloseComboBox->setCurrentIndex(mCloseComboBox->findData(settings->get(SLEEP_DISPLAY_AC_KEY).toInt() / FIXES));
    mCloseLidComboBox->setCurrentIndex(mCloseLidComboBox->findData(settings->get(BUTTON_LID_AC_KEY).toString()));

    //避免不存在该键值，出现闪退情况
    if (settings->keys().contains("powerPolicyAc") && settings->keys().contains("powerPolicyBattery")) {
        if (1 == settings->get(POWER_POLICY_AC).toInt()) {
            mPowerComboBox->setCurrentIndex(mPowerComboBox->findData("Balance Model"));
        } else if (2 == settings->get(POWER_POLICY_AC).toInt()) {
            mPowerComboBox->setCurrentIndex(mPowerComboBox->findData("Save Model"));
        } else {
            mPowerComboBox->setCurrentIndex(mPowerComboBox->findData("Performance Model"));
        }
        if (1 == settings->get(POWER_POLICY_BATTARY).toInt()) {
            mBatteryComboBox->setCurrentIndex(mBatteryComboBox->findData("Balance Model"));
        } else if (2 == settings->get(POWER_POLICY_BATTARY).toInt()){
            mBatteryComboBox->setCurrentIndex(mBatteryComboBox->findData("Save Model"));
        } else {
            mBatteryComboBox->setCurrentIndex(mBatteryComboBox->findData("Performance Model"));
        }
    } else {
        mPowerComboBox->setEnabled(false);
        mBatteryComboBox->setEnabled(false);
    }

    mDarkenComboBox->setCurrentIndex(mDarkenComboBox->findData(settings->get(IDLE_DIM_TIME_KEY).toInt() / FIXES));
    mLowpowerComboBox1->setCurrentIndex(settings->get(PER_ACTION_KEY).toInt() - 5);
    mLowpowerComboBox2->setCurrentIndex(mLowpowerComboBox2->findData(settings->get(ACTION_CRI_BTY).toString()));
    mNoticeComboBox->setCurrentIndex(settings->get(PERCENTAGE_LOW).toInt()/10 - 1);


    mSleepPwdBtn->setChecked(screensettings->get(SLEEP_ACTIVATION_ENABLED).toBool());
    mWakenPwdBtn->setChecked(settings->get(LOCK_BLANK_SCREEN).toBool());

    if (settings->keys().contains("lowBatteryAutoSave") && settings->keys().contains("onBatteryAutoSave") && settings->keys().contains("dispalyLeftTimeOfChargeAndDischarge")) {
        mLowSaveBtn->setChecked(settings->get(LOW_BATTERY_AUTO_SAVE).toBool());
        mBatterySaveBtn->setChecked(settings->get(ON_BATTERY_AUTO_SAVE).toBool());
        mDisplayTimeBtn->setChecked(settings->get(DISPLAY_LEFT_TIME_OF_CHARGE_AND_DISCHARGE).toBool());
    } else {
        mLowSaveFrame->hide();
        mBatterySaveFrame->hide();
        mDisplayTimeFrame->hide();
    }

    // 信号阻塞解除
    mPowerKeyComboBox->blockSignals(false);
    mCloseComboBox->blockSignals(false);
    mSleepComboBox->blockSignals(false);
    mCloseLidComboBox->blockSignals(false);
    mPowerComboBox->blockSignals(false);
    mBatteryComboBox->blockSignals(false);
    mDarkenComboBox->blockSignals(false);
    mLowpowerComboBox1->blockSignals(false);
    mLowpowerComboBox2->blockSignals(false);
    mNoticeComboBox->blockSignals(false);
    mSleepPwdBtn->blockSignals(false);
    mWakenPwdBtn->blockSignals(false);
    mLowSaveBtn->blockSignals(false);
    mBatterySaveBtn->blockSignals(false);
    mDisplayTimeBtn->blockSignals(false);

}

void Power::isLidPresent()
{
    QDBusInterface *LidInterface = new QDBusInterface("org.freedesktop.UPower",
                       "/org/freedesktop/UPower",
                       "org.freedesktop.DBus.Properties",
                        QDBusConnection::systemBus(),this);


    if (!LidInterface->isValid()) {
        qDebug() << "Create UPower Lid Interface Failed : " <<
            QDBusConnection::systemBus().lastError();
        return;
    }
    QDBusReply<QVariant> LidInfo;
    LidInfo = LidInterface->call("Get", "org.freedesktop.UPower", "LidIsPresent");
    isExitsLid = LidInfo.value().toBool();
}

void Power::isHibernateSupply()
{
    QDBusInterface *HibernateInterface = new QDBusInterface("org.freedesktop.login1",
                       "/org/freedesktop/login1",
                       "org.freedesktop.login1.Manager",
                        QDBusConnection::systemBus(),this);
    if (!HibernateInterface->isValid()) {
        qDebug() << "Create login1 Hibernate Interface Failed : " <<
            QDBusConnection::systemBus().lastError();
        return;
    }
    QDBusReply<QString> HibernateInfo;
    HibernateInfo = HibernateInterface->call("CanHibernate");
    isExitHibernate = HibernateInfo == "yes"?true:false;
}

bool Power::isExitBattery()
{
    /* 默认机器没有电池 */
    hasBat = false;
    QDBusInterface *brightnessInterface = new QDBusInterface("org.freedesktop.UPower",
                                     "/org/freedesktop/UPower/devices/DisplayDevice",
                                     "org.freedesktop.DBus.Properties",
                                     QDBusConnection::systemBus(), this);
    if (!brightnessInterface->isValid()) {
        qDebug() << "Create UPower Interface Failed : " << QDBusConnection::systemBus().lastError();
        return false;
    }

    QDBusReply<QVariant> briginfo;
    briginfo  = brightnessInterface ->call("Get", "org.freedesktop.UPower.Device", "PowerSupply");

    if (briginfo.value().toBool()) {
        hasBat = true ;
    }

    return hasBat;
}

double Power::getBattery()
{
    QDBusInterface *BatteryInterface = new QDBusInterface("org.freedesktop.UPower",
                       "/org/freedesktop/UPower/devices/battery_BAT0",
                       "org.freedesktop.DBus.Properties",
                        QDBusConnection::systemBus(),this);


    if (!BatteryInterface->isValid()) {
        qDebug() << "Create UPower Battery Interface Failed : " <<
            QDBusConnection::systemBus().lastError();
        return 0;
    }
    QDBusReply<QVariant> BatteryInfo;
    BatteryInfo = BatteryInterface->call("Get", "org.freedesktop.UPower.Device", "Percentage");
    return BatteryInfo.value().toDouble();
}

bool Power::QLabelSetText(QLabel *label, QString string)
{
    bool is_over_length = false;
    QFontMetrics fontMetrics(label->font());
    int fontSize = fontMetrics.width(string);
    QString str = string;
    if (fontSize > (label->width()-5)) {
        str = fontMetrics.elidedText(string, Qt::ElideRight, label->width());
        is_over_length = true;
    }
    label->setText(str);
    return is_over_length;
}
