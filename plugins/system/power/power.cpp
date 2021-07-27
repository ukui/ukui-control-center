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
#include "ui_power.h"
#include "powermacrodata.h"

#include <QDebug>
#include <QFile>
#include <QSettings>

typedef enum {
    BALANCE,
    SAVING,
    CUSTDOM
}MODE;

typedef enum {
    PRESENT,
    ALWAYS
}ICONDISPLAY;

/**
 * 平衡：关闭显示器10(ba)/30(ac)分钟；不进入睡眠；屏幕亮度、100
 * 节能：关闭显示器1分钟；计算机进入睡眠1分半；屏幕亮度10
 * 自定义: 合盖操作从设置读取
 */

#define DISPLAY_BALANCE_BA 5 * 60
#define DISPLAY_BALANCE_AC 10 * 60
#define COMPUTER_BALANCE_BA 5 * 60
#define COMPUTER_BALANCE_AC 10 * 60
#define BRIGHTNESS_BALANCE 100
#define DISPLAY_SAVING 60
#define COMPUTER_SAVING 2 * 60
#define BRIGHTNESS_SAVING 20



Power::Power() : mFirstLoad(true)
{
    pluginName = tr("Battery");
    pluginType = SYSTEM;
}

Power::~Power()
{
    if (!mFirstLoad) {
        delete ui;
        if (settingsCreate)
            delete settings;
        if (m_qsettings)
            delete m_qsettings;
    }
}

QString Power::get_plugin_name(){
    return pluginName;
}

int Power::get_plugin_type(){
    return pluginType;
}

QWidget * Power::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Power;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        initStyleSheet();
        settingsCreate = false;

        ui->darkenAllowFrame->hide();
        allowdarkenBtn = new SwitchButton(ui->darkenAllowFrame);
        ui->darkenAllowFrame->layout()->addWidget(allowdarkenBtn);

        setupComponent();
        isPowerSupply();
        initConnection();
        ui->frame_2->hide();
        if (ui->custdomRadioBtn->isChecked()) {
            checkedStatus = "custdom";
        } else if (ui->balanceRadioBtn->isChecked()) {
            checkedStatus = "balance";
        }
    }
    return pluginWidget;
}

void Power::plugin_delay_control(){

}

const QString Power::name() const {

    return QStringLiteral("power");
}

void Power::initConnection() {
    if (QGSettings::isSchemaInstalled(POWERMANAGER_SCHEMA)
     && QGSettings::isSchemaInstalled(UKUI_QUICK_OPERATION_PANEL)
     && QGSettings::isSchemaInstalled(UKUI_CONTROL_CENTER_PERSONALISE)){
        settingsCreate = true;
        settings = new QGSettings(POWERMANAGER_SCHEMA);
        m_qsettings = new QGSettings(UKUI_QUICK_OPERATION_PANEL);
        m_centerSettings = new QGSettings(UKUI_CONTROL_CENTER_PERSONALISE);
        setupConnect();
        initModeStatus();
        initPowerOtherStatus();
    } else {
        qCritical() << POWERMANAGER_SCHEMA << "not installed!\n";
        return;
    }

    connect(allowdarkenBtn, &SwitchButton::checkedChanged, this, [ = ](bool checked) {
        settings->set(IDLE_DIM_AC, checked);
        settings->set(IDLE_DIM_BA, checked);
    });

    if (m_qsettings) {
        connect(m_qsettings,&QGSettings::changed,this,[=](const QString &key){
            bool whichChecked = m_centerSettings->get(ISWHICHCHECKED).toBool();
            if (key == "energysavingmode") {
                bool savingMode = m_qsettings->get(ENERGYSAVINGMODE).toBool();
                if (savingMode) {
                    ui->frame_2->setFixedHeight(64);
                    ui->darkenAllowFrame->hide();
                    ui->custom1Frame->hide();
                    ui->custom2Frame->hide();
                    ui->suspendFrame->hide();
                    ui->closeLidFrame->hide();
                    ui->savingRadioBtn->setChecked(true);
                } else {
                    if (!whichChecked) {
                        if (checkedStatus == "custdom") {
                            ui->frame_2->setFixedHeight(381);
                            ui->darkenAllowFrame->show();
                            ui->custom1Frame->show();
                            ui->custom2Frame->show();
                            ui->suspendFrame->show();
                            ui->closeLidFrame->show();
                            ui->custdomRadioBtn->setChecked(true);
                        }
                        if (checkedStatus == "balance") {
                            ui->frame_2->setFixedHeight(64);
                            ui->darkenAllowFrame->hide();
                            ui->custom1Frame->hide();
                            ui->custom2Frame->hide();
                            ui->suspendFrame->hide();
                            ui->closeLidFrame->hide();
                            ui->balanceRadioBtn->setChecked(true);
                        }

                    }
                }
            }
        });
    }
}
void Power::initStyleSheet() {
    ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    ui->title2Label->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
    ui->balanceRadioBtn->setStyleSheet("QRadioButton::indicator:unchecked {image: url(:/img/plugins/theme/notselected_default.svg);}"
                                       "QRadioButton::indicator:unchecked:hover {image: url(:/img/plugins/theme/selected_hover.svg);}"
                                       "QRadioButton::indicator:unchecked:pressed {image: url(:/img/plugins/theme/selected_default.svg);}"
                                       "QRadioButton::indicator:checked {image: url(:/img/plugins/theme/selected_click.svg);}");
    ui->savingRadioBtn->setStyleSheet("QRadioButton::indicator:unchecked {image: url(:/img/plugins/theme/notselected_default.svg);}"
                                      "QRadioButton::indicator:unchecked:hover {image: url(:/img/plugins/theme/selected_hover.svg);}"
                                      "QRadioButton::indicator:unchecked:pressed {image: url(:/img/plugins/theme/selected_default.svg);}"
                                      "QRadioButton::indicator:checked {image: url(:/img/plugins/theme/selected_click.svg);}");
    ui->custdomRadioBtn->setStyleSheet("QRadioButton::indicator:unchecked {image: url(:/img/plugins/theme/notselected_default.svg);}"
                                      "QRadioButton::indicator:unchecked:hover {image: url(:/img/plugins/theme/selected_hover.svg);}"
                                      "QRadioButton::indicator:unchecked:pressed {image: url(:/img/plugins/theme/selected_default.svg);}"
                                      "QRadioButton::indicator:checked {image: url(:/img/plugins/theme/selected_click.svg);}");
    ui->batteryBtn->setAttribute(Qt::WA_StyledBackground,true);
    ui->batteryBtn->setStyleSheet("QPushButton {background-color: palette(Button);border-top-right-radius: 4px;border-bottom-right-radius: 4px;}\
                                       QPushButton:checked {background-color: palette(Highlight);border-top-right-radius: 4px;border-bottom-right-radius: 4px;}");
    ui->acBtn->setStyleSheet("QPushButton {background-color: palette(Button);border-top-left-radius: 4px;border-bottom-left-radius: 4px;}\
                              QPushButton:checked {background-color: palette(Highlight);border-top-left-radius: 4px;border-bottom-left-radius: 4px;}");

    ui->balanceFrame->setAutoFillBackground(false);
    ui->savingFrame->setAutoFillBackground(false);
    ui->customFrame->setAutoFillBackground(false);
    ui->custom1Frame->setAutoFillBackground(false);
    ui->custom2Frame->setAutoFillBackground(false);
    ui->darkenFrame->setAutoFillBackground(false);
    ui->closeLidFrame->setAutoFillBackground(false);
    ui->title2Label->setVisible(false);
    ui->iconFrame->setVisible(false);
}
void Power::initSearText() {
    //~ contents_path /power/select power plan
    ui->titleLabel->setText(tr("select power plan"));
    //~ contents_path /power/Balance (suggest)
    ui->balanceLabel->setText(tr("Balance (suggest)"));
    //~ contents_path /power/Saving
    ui->saveLabel->setText(tr("Saving"));
    ui->customLabel->setText(tr("Custom"));
}

void Power::isPowerSupply(){
    //ubuntukylin youker DBus interface
    QDBusInterface *brightnessInterface = new QDBusInterface("org.freedesktop.UPower",
                                     "/org/freedesktop/UPower/devices/DisplayDevice",
                                     "org.freedesktop.DBus.Properties",
                                     QDBusConnection::systemBus());
    if (!brightnessInterface->isValid()) {
        qDebug() << "Create UPower Interface Failed : " << QDBusConnection::systemBus().lastError();
        return;
    }

    QDBusReply<QVariant> briginfo;
    briginfo  = brightnessInterface ->call("Get", "org.freedesktop.UPower.Device", "PowerSupply");
    if (!briginfo.isValid()) {
        qDebug()<<"brightness info is invalid"<<endl;
        ui->batteryBtn->setVisible(false);
    } else {
        qDebug() << "brightness info is valid";
        bool status = briginfo.value().toBool();
        ui->batteryBtn->setVisible(status);
    }
}

void Power::setupComponent(){
    //
    ui->powerModeBtnGroup->setId(ui->balanceRadioBtn, BALANCE);
    ui->powerModeBtnGroup->setId(ui->savingRadioBtn, SAVING);
    ui->powerModeBtnGroup->setId(ui->custdomRadioBtn, CUSTDOM);

//显示器关闭延迟
    closeStringList  << tr("never") << tr("1 min") << tr("5 min") << tr("10 min") << tr("20 min") << tr("30 min") << tr("60 min") << tr("120 min");
    ui->closeComboBox->insertItem(0, closeStringList.at(0), QVariant::fromValue(0));
    ui->closeComboBox->insertItem(1, closeStringList.at(1), QVariant::fromValue(1));
    ui->closeComboBox->insertItem(2, closeStringList.at(2), QVariant::fromValue(5));
    ui->closeComboBox->insertItem(3, closeStringList.at(3), QVariant::fromValue(10));
    ui->closeComboBox->insertItem(4, closeStringList.at(4), QVariant::fromValue(20));
    ui->closeComboBox->insertItem(5, closeStringList.at(5), QVariant::fromValue(30));
    ui->closeComboBox->insertItem(6, closeStringList.at(6), QVariant::fromValue(60));
    ui->closeComboBox->insertItem(7, closeStringList.at(7), QVariant::fromValue(120));

    //合盖
    closeLidStringList << tr("nothing") << tr("blank") << tr("suspend") << tr("shutdown");
    ui->closeLidCombo->insertItem(0, closeLidStringList.at(0), "nothing");
    ui->closeLidCombo->insertItem(1, closeLidStringList.at(1), "blank");
    ui->closeLidCombo->insertItem(2, closeLidStringList.at(2), "suspend");
    ui->closeLidCombo->insertItem(3, closeLidStringList.at(3), "shutdown");

    //使用电池时屏幕变暗
    darkenStringList << tr("never") << tr("1 min") << tr("5 min") << tr("10 min") << tr("20 min") << tr("30 min");
    ui->darkenCombo->insertItem(0, darkenStringList.at(0), QVariant::fromValue(0));
    ui->darkenCombo->insertItem(1, darkenStringList.at(1), QVariant::fromValue(1));
    ui->darkenCombo->insertItem(2, darkenStringList.at(2), QVariant::fromValue(5));
    ui->darkenCombo->insertItem(3, darkenStringList.at(3), QVariant::fromValue(10));
    ui->darkenCombo->insertItem(4, darkenStringList.at(4), QVariant::fromValue(20));
    ui->darkenCombo->insertItem(5, darkenStringList.at(5), QVariant::fromValue(30));

    suspendList << tr("never") << tr("1 min") << tr("5 min") << tr("10 min") << tr("20 min") << tr("30 min");
    ui->suspendComboBox->insertItem(0, suspendList.at(0), QVariant::fromValue(0));
    ui->suspendComboBox->insertItem(1, suspendList.at(1), QVariant::fromValue(1));
    ui->suspendComboBox->insertItem(2, suspendList.at(2), QVariant::fromValue(5));
    ui->suspendComboBox->insertItem(3, suspendList.at(3), QVariant::fromValue(10));
    ui->suspendComboBox->insertItem(4, suspendList.at(4), QVariant::fromValue(20));
    ui->suspendComboBox->insertItem(5, suspendList.at(5), QVariant::fromValue(30));

    //默认电源
    ui->acBtn->setChecked(true);
    //电源不显示变暗功能
    ui->line_5->hide();
    ui->darkenFrame->hide();
    //s3tos4先隐藏
    ui->s3Tos4Frame->hide();

    //电源图标
    iconShowList << tr("always") << tr("present") << tr("charge");
    ui->iconComboBox->insertItem(0, iconShowList.at(0), "always");
    ui->iconComboBox->insertItem(1, iconShowList.at(1), "present");
    ui->iconComboBox->insertItem(2, iconShowList.at(2), "charge");
    QHBoxLayout * acLyt = new QHBoxLayout(ui->acBtn);
    acIconLabel = new QLabel;
    QLabel * acNameLabel = new QLabel;
    if(ui->acBtn->isChecked()){
        acIconLabel->setPixmap(QPixmap("://img/plugins/power/SupplyWhite.svg"));
    } else {
        acIconLabel->setPixmap(QPixmap("://img/plugins/power/Supply.svg"));
    }
    acNameLabel->setText(tr("Power supply"));
    acLyt->addStretch();
    acLyt->addWidget(acIconLabel);
    acLyt->addWidget(acNameLabel);
    acLyt->addStretch();
    ui->acBtn->setLayout(acLyt);
    QHBoxLayout * btLyt = new QHBoxLayout(ui->batteryBtn);
    btIconLabel = new QLabel;
    QLabel * btNameLabel = new QLabel;
    if(ui->batteryBtn->isChecked()){
        btIconLabel->setPixmap(QPixmap("://img/plugins/power/BatteryWhite.svg"));
    } else {
        btIconLabel->setPixmap(QPixmap("://img/plugins/power/Battery.svg"));
    }
    btNameLabel->setText(tr("Battery powered"));
    btLyt->addStretch();
    btLyt->addWidget(btIconLabel);
    btLyt->addWidget(btNameLabel);
    btLyt->addStretch();
    ui->batteryBtn->setLayout(btLyt);
    refreshUI();
}

void Power::setupConnect(){
    connect(ui->balanceRadioBtn,&QRadioButton::clicked,this,[=](){
        ui->frame_2->setFixedHeight(64);
        ui->custom1Frame->hide();
        ui->custom2Frame->hide();
        ui->suspendFrame->hide();
        ui->closeLidFrame->hide();
        checkedStatus = "balance";
        m_centerSettings->set(ISWHICHCHECKED,true);
        m_centerSettings->set(POWER_MODE,"balance");
        if (m_qsettings) {
            m_qsettings->set(ENERGYSAVINGMODE,false);
        }
        //设置屏幕亮度
        settings->set(BRIGHTNESS_AC, BRIGHTNESS_BALANCE);
        //设置合盖挂起
        QString value = "suspend";
        settings->set(BUTTON_LID_AC_KEY, value);
        settings->set(BUTTON_LID_BATT_KET, value);
        ui->darkenAllowFrame->hide();
    });
    connect(ui->savingRadioBtn,&QRadioButton::clicked,this,[=](){
        ui->frame_2->setFixedHeight(64);
        ui->custom1Frame->hide();
        ui->custom2Frame->hide();
        ui->suspendFrame->hide();
        ui->closeLidFrame->hide();
        m_centerSettings->set(ISWHICHCHECKED,true);
        m_centerSettings->set(POWER_MODE,"saving");
        //同步侧边栏节能模式按钮状态
        if (m_qsettings) {
            m_qsettings->set(ENERGYSAVINGMODE,true);
        }
        //设置屏幕亮度
        settings->set(BRIGHTNESS_AC, BRIGHTNESS_SAVING);
        //设置合盖黑屏
        QString value = "suspend";
        settings->set(BUTTON_LID_AC_KEY, value);
        settings->set(BUTTON_LID_BATT_KET, value);
        ui->darkenAllowFrame->hide();
    });
    connect(ui->custdomRadioBtn,&QRadioButton::clicked,this,[=](){
        ui->custom1Frame->show();
        ui->custom2Frame->show();
        ui->suspendFrame->show();
        ui->closeLidFrame->show();
        ui->darkenFrame->show();
        m_centerSettings->set(ISWHICHCHECKED,true);
        checkedStatus = "custdom";
        //同步侧边栏节能模式按钮状态
        if (m_qsettings)
            m_qsettings->set(ENERGYSAVINGMODE,false);
        resetCustomPlanStatus();
    });
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(ui->powerTypeBtnGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, [=]{
#else
    connect(ui->powerTypeBtnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, [=]{
#endif
        initCustomPlanStatus();
    });

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(ui->closeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index){
#else
    connect(ui->closeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
#endif
        Q_UNUSED(index)
        int value = ui->closeComboBox->currentData(Qt::UserRole).toInt() * 60;
        if (ui->acBtn->isChecked()){
            settings->set(SLEEP_DISPLAY_AC_KEY, QVariant(value));
        }
        if (ui->batteryBtn->isChecked()){
            settings->set(SLEEP_DISPLAY_BATT_KEY, QVariant(value));
        }
    });

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(ui->suspendComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index){
#else
    connect(ui->suspendComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
#endif
        Q_UNUSED(index)
        int value = ui->suspendComboBox->currentData(Qt::UserRole).toInt() * 60;
        if (ui->acBtn->isChecked()){
            settings->set(SLEEP_COMPUTER_AC_KEY, QVariant(value));
        }
        if (ui->batteryBtn->isChecked()){
            settings->set(SLEEP_COMPUTER_BATT_KEY, QVariant(value));
        }
    });

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(ui->iconComboBox,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index){
#else
    connect(ui->iconComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
#endif

        Q_UNUSED(index)
        QString value = ui->iconComboBox->currentData(Qt::UserRole).toString();
        settings->set(ICONPOLICY, value);
    });

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(ui->closeLidCombo,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index){
#else
    connect(ui->closeLidCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
#endif

        Q_UNUSED(index)
        QString value = ui->closeLidCombo->currentData(Qt::UserRole).toString();
        if (ui->acBtn->isChecked()){
            settings->set(BUTTON_LID_AC_KEY, value);
        }
        if (ui->batteryBtn->isChecked()){
            settings->set(BUTTON_LID_BATT_KET, value);
        }
        qDebug()<<value;
    });

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(ui->darkenCombo,static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=](int index){
#else
    connect(ui->darkenCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index){
#endif

        Q_UNUSED(index)
        int idleDarken = ui->darkenCombo->currentData(Qt::UserRole).toInt() * 60;
        settings->set(IDLE_DIM_TIME_KEY, idleDarken);
        if (idleDarken == 0) {
            settings->set(IDLE_DIM_AC, false);
            settings->set(IDLE_DIM_BA, false);
        } else {
            settings->set(IDLE_DIM_AC, true);
            settings->set(IDLE_DIM_BA, true);
        }
    });
}

void Power::initModeStatus(){
    QString powerMode = m_centerSettings->get(POWER_MODE).toString();
    if (powerMode == "balance") {
        ui->balanceRadioBtn->setChecked(true);
        ui->frame_2->setFixedHeight(64);
    } else if (powerMode == "saving"){
        ui->savingRadioBtn->setChecked(true);
        ui->frame_2->setFixedHeight(64);
    }
    refreshUI();
}

void Power::writeConf(int is_saving) {
    QDBusInterface * tmpSysinterface = new QDBusInterface("com.control.center.qt.systemdbus",
                                                          "/",
                                                          "com.control.center.interface",
                                                          QDBusConnection::systemBus());

    if (!tmpSysinterface->isValid()){
        qCritical() << "Create Client Interface Failed When : " << QDBusConnection::systemBus().lastError();
        return;
    }
    tmpSysinterface->call("setPowerStatus", is_saving);

    delete tmpSysinterface;
}

void Power::initPowerOtherStatus(){
    QString value = settings->get(ICONPOLICY).toString();
    ui->iconComboBox->blockSignals(true);
    ui->iconComboBox->setCurrentIndex(ui->iconComboBox->findData(value));
    ui->iconComboBox->blockSignals(false);
    allowdarkenBtn->blockSignals(true);
    allowdarkenBtn->setChecked(settings->get(IDLE_DIM_BA).toBool());
    allowdarkenBtn->blockSignals(false);
}

void Power::resetCustomPlanStatus(){
    //当其他电源计划切换至自定义时，默认状态为从不
    //设置显示器关闭
    settings->set(SLEEP_DISPLAY_AC_KEY, 0);
    settings->set(SLEEP_DISPLAY_BATT_KEY, 0);
    //设置计算机睡眠
    settings->set(SLEEP_COMPUTER_AC_KEY, 1200);
    settings->set(SLEEP_COMPUTER_BATT_KEY, 1200);

    ui->acBtn->setChecked(true);
    initCustomPlanStatus();
    ui->darkenAllowFrame->show();
}

void Power::initCustomPlanStatus(){
    //信号阻塞
    ui->closeComboBox->blockSignals(true);
    ui->darkenCombo->blockSignals(true);

    if (ui->acBtn->isChecked()){
        //挂起时间
        int acsleep = settings->get(SLEEP_COMPUTER_AC_KEY).toInt() / FIXES;
        if (ui->suspendComboBox->findData(acsleep) >= 0)
            ui->suspendComboBox->setCurrentIndex(ui->suspendComboBox->findData(acsleep));
        else {
            ui->suspendComboBox->blockSignals(true);
            ui->suspendComboBox->setCurrentIndex(4);
            ui->suspendComboBox->blockSignals(false);
        }

        //显示器关闭延迟
        int acclose = settings->get(SLEEP_DISPLAY_AC_KEY).toInt() / FIXES;
        if (ui->closeComboBox->findData(acclose) >= 0)
            ui->closeComboBox->setCurrentIndex(ui->closeComboBox->findData(acclose));
        else {
            ui->closeComboBox->blockSignals(true);
            ui->closeComboBox->setCurrentIndex(0);
            ui->closeComboBox->blockSignals(false);
        }
        //合盖
        QString aclid = settings->get(BUTTON_LID_AC_KEY).toString();
        ui->closeLidCombo->setCurrentIndex(ui->closeLidCombo->findData(aclid));

        //变暗
        ui->line_5->hide();
        ui->darkenFrame->hide();
        ui->frame_2->setFixedHeight(325);

        btIconLabel->setPixmap(QPixmap("://img/plugins/power/Battery.svg"));
        acIconLabel->setPixmap(QPixmap("://img/plugins/power/SupplyWhite.svg"));

    }

    if (ui->batteryBtn->isChecked()){
        //挂起时间
        int acsleep = settings->get(SLEEP_COMPUTER_BATT_KEY).toInt() / FIXES;
        if (ui->suspendComboBox->findData(acsleep) >= 0)
            ui->suspendComboBox->setCurrentIndex(ui->suspendComboBox->findData(acsleep));
        else {
            ui->suspendComboBox->blockSignals(true);
            ui->suspendComboBox->setCurrentIndex(4);
            ui->suspendComboBox->blockSignals(false);
        }

        //显示器关闭延迟
        int batclose = settings->get(SLEEP_DISPLAY_BATT_KEY).toInt() / FIXES;
        if (ui->closeComboBox->findData(batclose) >= 0)
            ui->closeComboBox->setCurrentIndex(ui->closeComboBox->findData(batclose));
        else {
            ui->closeComboBox->blockSignals(true);
            ui->closeComboBox->setCurrentIndex(0);
            ui->closeComboBox->blockSignals(false);
        }

        //合盖
        QString batlid = settings->get(BUTTON_LID_BATT_KET).toString();
        ui->closeLidCombo->setCurrentIndex(ui->closeLidCombo->findData(batlid));

        //变暗
        int darkentime = settings->get(IDLE_DIM_TIME_KEY).toInt() / 60;
        if (ui->darkenCombo->findData(darkentime) >= 0)
            ui->darkenCombo->setCurrentIndex(ui->darkenCombo->findData(darkentime));
        else {
            ui->darkenCombo->blockSignals(true);
            ui->darkenCombo->setCurrentIndex(0);
            ui->darkenCombo->blockSignals(false);
        }
        if (settings->get(IDLE_DIM_TIME_KEY).toInt() == 30) {
            settings->set(IDLE_DIM_TIME_KEY, 0);
            ui->darkenCombo->setCurrentIndex(ui->darkenCombo->findData(0));
        }
        //当前版本先隐藏自动降低亮度时间设置，由电源管理决定时间
        ui->line_5->hide();
        ui->darkenFrame->hide();
        ui->frame_2->setFixedHeight(325);

        btIconLabel->setPixmap(QPixmap("://img/plugins/power/BatteryWhite.svg"));
        acIconLabel->setPixmap(QPixmap("://img/plugins/power/Supply.svg"));
    }

    //信号阻塞解除
    ui->closeComboBox->blockSignals(false);
    ui->darkenCombo->blockSignals(false);
}

void Power::refreshUI(){
    if (ui->powerModeBtnGroup->checkedId() != CUSTDOM){
        ui->custom1Frame->hide();
        ui->custom2Frame->hide();
        ui->suspendFrame->hide();
        ui->closeLidFrame->hide();
        if (ui->batteryBtn->isChecked()){
            ui->line_5->hide();
            ui->darkenFrame->hide();
        }
        ui->line_2->hide();
        ui->line_3->hide();
        ui->line_4->hide();
        ui->frame_2->setFixedHeight(64);
    } else {
        ui->custom1Frame->show();
        ui->custom2Frame->show();
        ui->suspendFrame->show();
        ui->closeLidFrame->show();
        ui->line_2->show();
        ui->line_3->show();
        ui->line_4->show();
        ui->frame_2->setFixedHeight(325);
    }
}
