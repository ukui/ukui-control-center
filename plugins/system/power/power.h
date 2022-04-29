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
#ifndef POWER_H
#define POWER_H

#include <QObject>
#include <QtPlugin>
#include <QStyledItemDelegate>
#include <QGSettings>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRadioButton>
#include <QButtonGroup>

#include "shell/interface.h"
#include <ukcc/widgets/titlelabel.h>

#include <ukcc/widgets/comboxframe.h>
#include <ukcc/widgets/switchbutton.h>

namespace Ui {
class Power;
}

class Power : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    explicit Power();
    ~Power();

public:
    QString plugini18nName()   Q_DECL_OVERRIDE;
    int pluginTypes()       Q_DECL_OVERRIDE;
    QWidget *pluginUi()   Q_DECL_OVERRIDE;
    const QString name() const Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

public:
    void InitUI(QWidget *widget);
    void retranslateUi();
    void resetui();

    void initSearText();
    void setupComponent();
    void setupConnect();
    void initCustomPlanStatus();
    void isLidPresent();
    void isHibernateSupply();
    bool isExistBattery();
    double getBattery();
    bool QLabelSetText(QLabel *label, QString string);
    void clearAutoItem(QVBoxLayout *mLyt);
    void setFrame_Noframe(QFrame *frame);
    QFrame *setLine(QFrame *frame);

private:

    QWidget *pluginWidget;

    QGSettings *settings;
    QGSettings *sessionSetting;
    QGSettings *stylesettings;
    QGSettings *sessionsettings;
    QGSettings *screensettings;
    QGSettings *m_centerSettings;
    QGSettings *m_qsettings;

    QString pluginName;
    int pluginType;

    TitleLabel *CustomTitleLabel;
    TitleLabel *PowerPlanTitleLabel;
    TitleLabel *BatteryPlanTitleLabel;
    QLabel *mSleepPwdLabel;
    QLabel *mWakenPwdLabel;
    QLabel *mPowerKeyLabel;
    QLabel *mCloseLabel;
    QLabel *mSleepLabel;
    QLabel *mCloseLidLabel;
    QLabel *mPowerLabel;
    QLabel *mBatteryLabel;
    QLabel *mDarkenLabel;
    QLabel *mLowpowerLabel1;
    QLabel *mLowpowerLabel2;
    QLabel *mNoticeLabel;
    QLabel *mLowSaveLabel;
    QLabel *mBatterySaveLabel;
    QLabel *mDisplayTimeLabel;

    QLabel *mSaveLabel_1;
    QLabel *mSaveLabel_2;
    QLabel *mBalanceLabel_1;
    QLabel *mBalanceLabel_2;

    QFrame *Powerwidget;
    QFrame *PowerPlanwidget;
    QFrame *Batterywidget;
    QFrame *mSleepPwdFrame;
    QFrame *mWakenPwdFrame;
    QFrame *mPowerKeyFrame;
    QFrame *mCloseFrame;
    QFrame *mSleepFrame;
    QFrame *mCloseLidFrame;
    QFrame *mPowerFrame;
    QFrame *mBatteryFrame;
    QFrame *mDarkenFrame;
    QFrame *mLowpowerFrame;
    QFrame *mNoticeLFrame;
    QFrame *mLowSaveFrame;
    QFrame *mBatterySaveFrame;
    QFrame *mDisplayTimeFrame;
    QFrame *mSaveFrame;
    QFrame *mBalanceFrame;

    QFrame *line_1;
    QFrame *line_2;
    QFrame *line_3;
    QFrame *line_4;
    QFrame *line_5;
    QFrame *line_6;
    QFrame *line_7;
    QFrame *line_8;
    QFrame *line_9;
    QFrame *line_10;
    QFrame *line_11;
    QFrame *line_12;

    QSpacerItem *mItem;

    QComboBox *mPowerKeyComboBox;
    QComboBox *mCloseComboBox;
    QComboBox *mSleepComboBox;
    QComboBox *mCloseLidComboBox;
    QComboBox *mPowerComboBox;
    QComboBox *mBatteryComboBox;
    QComboBox *mDarkenComboBox;
    QComboBox *mLowpowerComboBox1;
    QComboBox *mLowpowerComboBox2;
    QComboBox *mNoticeComboBox;

    SwitchButton *mSleepPwdBtn;
    SwitchButton *mWakenPwdBtn;
    SwitchButton *mLowSaveBtn;
    SwitchButton *mBatterySaveBtn;
    SwitchButton *mDisplayTimeBtn;

    QRadioButton *mSaveBtn;
    QRadioButton *mBalanceBtn;

    QButtonGroup *mPowerBtnGroup;

    QSpacerItem *verticalSpacer;
    QSpacerItem *verticalSpacer_1;
    QSpacerItem *verticalSpacer_2;

    QStringList buttonStringList;
    QStringList sleepStringList;
    QStringList closeStringList;
    QStringList closeLidStringList;
    QStringList PowerplanStringList;
    QStringList BatteryplanStringList;
    QStringList DarkenStringList;
    QStringList LowpowerStringList;

    QStringList mKeys;

    QVBoxLayout *BatteryLayout;
    QVBoxLayout *PowerLayout;

    bool mFirstLoad;
    bool isExistsLid;
    bool isExistHibernate;

    bool hasBat;
};

#endif // POWER_H
