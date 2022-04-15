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
#include "mousecontrol.h"
#include "ui_mousecontrol.h"

#include <QDir>
#include <QFile>
#include <QTimer>
#include <QDebug>
#include <QSettings>
#include <QSizePolicy>
#include <QtDBus>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <X11/Xlib.h>
#include <glib.h>
#include <gio/gio.h>
}
#define CONTROL_CENTER_PERSONALISE "org.ukui.control-center.personalise"
#define CURSOR_SPEED            "cursor-speed"
#define MOUSE_SCHEMA            "org.ukui.peripherals-mouse"
#define HAND_KEY                "left-handed"
#define DOUBLE_CLICK_KEY        "double-click"
#define LOCATE_KEY              "locate-pointer"
#define CURSOR_SIZE_KEY         "cursor-size"
#define ACCELERATION_KEY        "motion-acceleration"
#define ACCEL_KEY               "mouse-accel"

#define SESSION_SCHEMA          "org.ukui.session"
#define SESSION_MOUSE_KEY       "mouse-size-changed"

#define DESKTOP_SCHEMA          "org.mate.interface"
#define DESKTOP_SCHEMA_QT       "org.ukui.style"
#define CURSOR_BLINK_KEY        "cursor-blink"
#define CURSOR_BLINK_TIME_KEY   "cursor-blink-time"

#define MOUSE_MID_GET_CMD       "/usr/bin/mouse-midbtn-speed-get"
#define MOUSE_MID_SET_CMD       "/usr/bin/mouse-midbtn-speed-set"

MyLabel::MyLabel(){
//    setAttribute(Qt::WA_StyledBackground,true);
    setAttribute(Qt::WA_DeleteOnClose);

    QSizePolicy pSizePolicy = this->sizePolicy();
    pSizePolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    pSizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
    this->setSizePolicy(pSizePolicy);

    setFixedSize(QSize(54, 28));
    setScaledContents(true);

    setPixmap(QPixmap(":/img/plugins/mouse/double-click-off.png"));

    const QByteArray id(MOUSE_SCHEMA);
    if (QGSettings::isSchemaInstalled(id)){
        mSettings = new QGSettings(id);
    }

}

MyLabel::~MyLabel(){
    if (QGSettings::isSchemaInstalled(MOUSE_SCHEMA))
        delete mSettings;
}

void MyLabel::mouseDoubleClickEvent(QMouseEvent *event){
    int delay = mSettings->get(DOUBLE_CLICK_KEY).toInt();
    setPixmap(QPixmap(":/img/plugins/mouse/double-click-on.png"));
    QTimer::singleShot(delay, this, [=]{
        setPixmap(QPixmap(":/img/plugins/mouse/double-click-off.png"));
    });
}


MouseControl::MouseControl() : mFirstLoad(true)
{
    pluginName = tr("Mouse");
    pluginType = DEVICES;
}

MouseControl::~MouseControl()
{
    if (!mFirstLoad) {
        delete ui;
        if (QGSettings::isSchemaInstalled(SESSION_SCHEMA)
         && QGSettings::isSchemaInstalled(MOUSE_SCHEMA)
         && QGSettings::isSchemaInstalled(DESKTOP_SCHEMA)
         && QGSettings::isSchemaInstalled(DESKTOP_SCHEMA_QT)
         && QGSettings::isSchemaInstalled(CONTROL_CENTER_PERSONALISE)) {
            delete settings;
            delete sesstionSetttings;
            delete desktopSettings;
            delete gsettings;
        }
        if(styleSettings)
            delete styleSettings;
    }
}

QString MouseControl::get_plugin_name(){
    return pluginName;
}

int MouseControl::get_plugin_type(){
    return pluginType;
}

QWidget *MouseControl::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::MouseControl;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);

        Slow1=new QLabel;
        Fast1=new QLabel;
        Slow2=new QLabel;
        Fast2=new QLabel;


        Low=new QLabel;
        High=new QLabel;

        pointerSpeedSlider = new Uslider;
        pointerSpeedSlider->setOrientation(Qt::Horizontal);
        pointerSpeedSlider->setRange(10,100);

        cursorSpeedSlider = new Uslider;
        cursorSpeedSlider->setOrientation(Qt::Horizontal);
        cursorSpeedSlider->setRange(0,98);
        initSearchText();
        ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
        ui->title2Label->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
        ui->title3Label->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");
        //初始化鼠标设置GSettings
        const QByteArray id(MOUSE_SCHEMA);
        const QByteArray sessionId(SESSION_SCHEMA);
        const QByteArray idd(DESKTOP_SCHEMA);
        const QByteArray iddd(DESKTOP_SCHEMA_QT);
        if (QGSettings::isSchemaInstalled(sessionId)
         && QGSettings::isSchemaInstalled(id)
         && QGSettings::isSchemaInstalled(idd)
         && QGSettings::isSchemaInstalled(iddd)
         && QGSettings::isSchemaInstalled(CONTROL_CENTER_PERSONALISE)) {
            sesstionSetttings = new QGSettings(sessionId);
            settings = new QGSettings(id);
            desktopSettings = new QGSettings(idd);
            styleSettings = new QGSettings(iddd);
            gsettings = new QGSettings(CONTROL_CENTER_PERSONALISE);

        setupComponent();
        initHandHabitStatus();
        initPointerStatus();
        initCursorStatus();
        initWheelStatus();
        }
    }
    return pluginWidget;
}

void MouseControl::plugin_delay_control(){

}

const QString MouseControl::name() const {

    return QStringLiteral("mouse");
}

void MouseControl::initSearchText() {
    //~ contents_path /mouse/Mouse Key Settings
    ui->titleLabel->setText(tr("Mouse Key Settings"));
    //~ contents_path /mouse/Hand habit
    ui->handLabel->setText(tr("Hand habit"));
    //~ contents_path /mouse/Pointer Settings
    ui->title2Label->setText(tr("Pointer Settings"));
    //~ contents_path /mouse/Speed
    ui->speedLabel->setText(tr("Speed"));
    //~ contents_path /mouse/Visibility
    ui->visLabel->setText(tr("Visibility"));
    ui->watch_label->setText(tr("Pointer position"));
    //~ contents_path /mouse/Pointer size
    ui->sizeLabel->setText(tr("Pointer size"));
    //~ contents_path /mouse/Cursor Settings
    ui->title3Label->setText(tr("Cursor Settings"));
    //~ contents_path /mouse/Enable flashing on text area
    ui->flashLabel->setText(tr("Enable flashing on text area"));
    //~ contents_path /mouse/Cursor speed
    ui->cursorspdLabel->setText(tr("Cursor speed"));

    Slow1->setText(tr("Slow"));
    Fast1->setText(tr("Fast"));
    Slow2->setText(tr("Slow"));
    Fast2->setText(tr("Fast"));
    Low->setText(tr("Low"));
    High->setText(tr("High"));

}

void MouseControl::setupComponent(){

    ui->cursorWeightFrame->hide();

    // 设置Slider样式
    pointerSpeedSlider->setFixedHeight(60);
    pointerSpeedSlider->setStyleSheet( "QSlider::groove:horizontal {"
                                       "border: 0px none;"
                                       "background: palette(button);"
                                       "height: 8px;"
                                       "border-radius: 5px;"
                                       "}"

                                       "QSlider::handle:horizontal {"

                                       "height: 40px;"
                                       "width: 36px;"
                                       "margin: 30px;"
                                       "border-image: url(://img/plugins/mouse/slider.svg);"
                                       "margin: -20 -4px;"
                                       "}"

                                       "QSlider::add-page:horizontal {"
                                       "background: palette(button);"
                                       "border-radius: 20px;"
                                       "}"

                                       "QSlider::sub-page:horizontal {"
                                       "background: #2FB3E8;"
                                       "border-radius: 5px;"
                                       "}");


    ui->horizontalLayout_3->addWidget(Slow1);
    ui->horizontalLayout_3->addItem(new QSpacerItem(16, 4));
    ui->horizontalLayout_3->addWidget(pointerSpeedSlider);
    ui->horizontalLayout_3->addItem(new QSpacerItem(16, 4));
    ui->horizontalLayout_3->addWidget(Fast1);

    cursorSpeedSlider->setFixedHeight(60);
    cursorSpeedSlider->setStyleSheet( "QSlider::groove:horizontal {"
                                       "border: 0px none;"
                                       "background: #F5F5F5;"
                                       "height: 8px;"
                                       "border-radius: 5px;"
                                       "}"

                                       "QSlider::handle:horizontal {"

                                      "height: 40px;"
                                      "width: 36px;"
                                      "margin: 30px;"
                                       "border-image: url(://img/plugins/mouse/slider.svg);"
                                       "margin: -20 -4px;"
                                       "}"

                                       "QSlider::add-page:horizontal {"
                                       "background: #DDDDDD;"
                                       "border-radius: 20px;"
                                       "}"

                                       "QSlider::sub-page:horizontal {"
                                       "background: #2FB3E8;"
                                       "border-radius: 5px;"
                                       "}");

    ui->horizontalLayout_13->addWidget(Slow2);
    ui->horizontalLayout_13->addItem(new QSpacerItem(16, 4));
    ui->horizontalLayout_13->addWidget(cursorSpeedSlider);
    ui->horizontalLayout_13->addItem(new QSpacerItem(16, 4));
    ui->horizontalLayout_13->addWidget(Fast2);

    // 设置左手右手鼠标控件
    ui->Rhand->setText(tr("Righthand"));
    ui->Lhand->setText(tr("Lefthand"));

    // 设置指针可见性控件
    visiblityBtn = new SwitchButton(pluginWidget);
    ui->visibilityHorLayout->addWidget(visiblityBtn);

    // 设置指针大小
    ui->pointerSizeComBox->setMaxVisibleItems(5);
    ui->pointerSizeComBox->addItem(tr("Default(Recommended)"), 24); //100%
    ui->pointerSizeComBox->addItem(tr("Medium"), 32); //125%
    ui->pointerSizeComBox->addItem(tr("Large"), 48); //150%

    // 设置鼠标滚轮是否显示
    if (!g_file_test(MOUSE_MID_GET_CMD, G_FILE_TEST_EXISTS) || !g_file_test(MOUSE_MID_SET_CMD, G_FILE_TEST_EXISTS)){
        ui->midSpeedFrame->hide();
    }

    //设置启用光标闪烁
    flashingBtn = new SwitchButton(pluginWidget);
    ui->enableFlashingHorLayout->addWidget(flashingBtn);
    bool checked = desktopSettings->get(CURSOR_BLINK_KEY).toBool();
    ui->cursorSpeedFrame->setVisible(checked);

    connect(ui->Lhand,SIGNAL(clicked(bool)),this,SLOT(Lhandchange()));

    connect(ui->Rhand,SIGNAL(clicked(bool)),this,SLOT(Rhandchange()));

    ui->Lhand->setStyleSheet("QRadioButton::indicator:unchecked {image: url(:/img/plugins/theme/notselected_default.svg);}"
                                "QRadioButton::indicator:unchecked:hover {image: url(:/img/plugins/theme/selected_hover.svg);}"
                                "QRadioButton::indicator:unchecked:pressed {image: url(:/img/plugins/theme/selected_default.svg);}"
                                "QRadioButton::indicator:checked {image: url(:/img/plugins/theme/selected_click.svg);}");
    ui->Rhand->setStyleSheet("QRadioButton::indicator:unchecked {image: url(:/img/plugins/theme/notselected_default.svg);}"
                                "QRadioButton::indicator:unchecked:hover {image: url(:/img/plugins/theme/selected_hover.svg);}"
                                "QRadioButton::indicator:unchecked:pressed {image: url(:/img/plugins/theme/selected_default.svg);}"
                                "QRadioButton::indicator:checked {image: url(:/img/plugins/theme/selected_click.svg);}");


    connect(pointerSpeedSlider, &QSlider::valueChanged, [=](int value){
        settings->set(ACCELERATION_KEY, static_cast<double>(value)/pointerSpeedSlider->maximum()*10);
        qDebug()<<static_cast<double>(value)/pointerSpeedSlider->maximum()*10;
    });

    connect(visiblityBtn, &SwitchButton::checkedChanged, [=](bool checked){
        settings->set(LOCATE_KEY, checked);
    });

    connect(ui->pointerSizeComBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MouseControl::mouseSizeChange);

    preValue = styleSettings->get(CURSOR_BLINK_TIME_KEY).toInt();
    if (preValue != 0) {
        if (gsettings->keys().contains("cursorSpeed")) {
            gsettings->set(CURSOR_SPEED,preValue);
        }
    }
    if (gsettings->keys().contains("cursorSpeed")) {
        value = gsettings->get(CURSOR_SPEED).toInt();
    }
    connect(gsettings,&QGSettings::changed,this,[=](const QString &key){
        if (key == "cursorSpeed" && gsettings->keys().contains("cursorSpeed")) {
            value = gsettings->get(CURSOR_SPEED).toInt();
        }
    });
    connect(flashingBtn, &SwitchButton::checkedChanged, [=](bool checked) {
        desktopSettings->set(CURSOR_BLINK_KEY, checked);
        if (!checked) {
            styleSettings->set(CURSOR_BLINK_TIME_KEY,0);
        } else {
            styleSettings->set(CURSOR_BLINK_TIME_KEY,value);
        }
        ui->cursorSpeedFrame->setVisible(checked);
    });

    connect(ui->midHorSlider, &QSlider::sliderReleased, [=] {
        _set_mouse_mid_speed(ui->midHorSlider->value());
    });

    connect(cursorSpeedSlider, &QSlider::sliderReleased, [=] {
        desktopSettings->set(CURSOR_BLINK_TIME_KEY, 990 - cursorSpeedSlider->value() * 10);
        styleSettings->set(CURSOR_BLINK_TIME_KEY, 990 - cursorSpeedSlider->value() * 10);
        if (gsettings->keys().contains("cursorSpeed")) {
            gsettings->set(CURSOR_SPEED,990 - cursorSpeedSlider->value() * 10);
        }
    });
}

void MouseControl::Lhandchange()
{
    qDebug()<<"Lclicked";
    settings->set(HAND_KEY, true);
    ui->Lhand->setChecked(true);
}

void MouseControl::Rhandchange()
{
    qDebug()<<"Rclicked";
    settings->set(HAND_KEY, false);
    ui->Rhand->setChecked(true);

}
void MouseControl::initHandHabitStatus(){

    bool state=settings->get(HAND_KEY).toBool();
    if (state) {
        ui->Lhand->setChecked(true);
    } else {
        ui->Rhand->setChecked(true);
    }
}

void MouseControl::initPointerStatus(){

    //当前系统指针加速值，-1为系统默认
    double mouse_acceleration = settings->get(ACCELERATION_KEY).toDouble();


    //当从接口获取的是-1,则代表系统默认值，真实值需要从底层获取
    if (static_cast<int>(mouse_acceleration) == -1) {
        // speed sensitivity
        int accel_numerator, accel_denominator, threshold;  //当加速值和灵敏度为系统默认的-1时，从底层获取到默认的具体值

        XGetPointerControl(QX11Info::display(), &accel_numerator, &accel_denominator, &threshold);

        settings->set(ACCELERATION_KEY, static_cast<double>(accel_numerator/accel_denominator));
    }

    //初始化指针速度控件
    pointerSpeedSlider->blockSignals(true);
    pointerSpeedSlider->setValue(static_cast<int>(settings->get(ACCELERATION_KEY).toDouble()*10));
    pointerSpeedSlider->blockSignals(false);

    //初始化可见性控件
    visiblityBtn->blockSignals(true);
    visiblityBtn->setChecked(settings->get(LOCATE_KEY).toBool());
    visiblityBtn->blockSignals(false);

    //初始化指针大小
    int sizeIndex = ui->pointerSizeComBox->findData(settings->get(CURSOR_SIZE_KEY).toInt());
    ui->pointerSizeComBox->blockSignals(true);
    ui->pointerSizeComBox->setCurrentIndex(sizeIndex);
    ui->pointerSizeComBox->blockSignals(false);
}

void MouseControl::initCursorStatus(){
    flashingBtn->blockSignals(true);
    flashingBtn->setChecked(desktopSettings->get(CURSOR_BLINK_KEY).toBool());
    flashingBtn->blockSignals(false);

    cursorSpeedSlider->blockSignals(true);
//    cursorSpeedSlider->setValue(desktopSettings->get(CURSOR_BLINK_TIME_KEY).toInt());
    cursorSpeedSlider->setValue(99 - desktopSettings->get(CURSOR_BLINK_TIME_KEY).toInt() / 10);
    cursorSpeedSlider->blockSignals(false);
}

void MouseControl::initWheelStatus(){
    int value = _get_mouse_mid_speed();
    ui->midHorSlider->blockSignals(true);
    ui->midHorSlider->setValue(value);
    ui->midHorSlider->blockSignals(false);
}

int MouseControl::_get_mouse_mid_speed(){

    int value = 0;

    if (g_file_test(MOUSE_MID_GET_CMD, G_FILE_TEST_EXISTS)){
        QProcess * getProcess = new QProcess();
        getProcess->start(MOUSE_MID_GET_CMD);
        getProcess->waitForFinished();

        QByteArray ba = getProcess->readAllStandardOutput();
        QString speedStr = QString(ba.data()).simplified();
        value = speedStr.toInt();
    }

    return value;
}

void MouseControl::_set_mouse_mid_speed(int value){
    QString cmd;

    cmd = MOUSE_MID_SET_CMD + QString(" ") + QString::number(value);

    QProcess * setProcess = new QProcess();
    setProcess->start(cmd);
    setProcess->waitForFinished();
}

void MouseControl::mouseSizeChange() {

    settings->set(CURSOR_SIZE_KEY, ui->pointerSizeComBox->currentData().toInt());

    QStringList keys = sesstionSetttings->keys();
    if (keys.contains("mouseSizeChanged")) {
        sesstionSetttings->set(SESSION_MOUSE_KEY, true);
    }

    QString filename = QDir::homePath() + "/.config/kcminputrc";
    QSettings *mouseSettings = new QSettings(filename, QSettings::IniFormat);

    mouseSettings->beginGroup("Mouse");
    mouseSettings->setValue("cursorSize", ui->pointerSizeComBox->currentData().toInt());
    mouseSettings->endGroup();

    delete mouseSettings;

#if QT_VERSION <= QT_VERSION_CHECK(5,12,0)

#else
    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(5);
    args.append(0);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
#endif

}
