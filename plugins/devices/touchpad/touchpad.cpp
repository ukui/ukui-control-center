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
#include "touchpad.h"
#include "ui_touchpad.h"

#include <QFile>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <X11/extensions/XInput.h>
#include <X11/Xatom.h>
}

#define TOUCHPAD_SCHEMA "org.ukui.peripherals-touchpad"
#define ACTIVE_TOUCHPAD_KEY "touchpad-enabled"
#define DISABLE_WHILE_TYPING_KEY "disable-while-typing"
#define TOUCHPAD_CLICK_KEY "tap-to-click"
#define V_EDGE_KEY "vertical-edge-scrolling"
#define H_EDGE_KEY "horizontal-edge-scrolling"
#define V_FINGER_KEY "vertical-two-finger-scrolling"
#define H_FINGER_KEY "horizontal-two-finger-scrolling"
#define N_SCROLLING "none"
#define TOUCHPAD_MOTION_ACCEL   "motion-acceleration"


bool findSynaptics();
bool _supportsXinputDevices();
XDevice* _deviceIsTouchpad (XDeviceInfo * deviceinfo);
bool _deviceHasProperty (XDevice * device, const char * property_name);

Touchpad::Touchpad()
{
    mFirstLoad = true;
    pluginName = tr("Trackpad");
    pluginType = DEVICES;
}

Touchpad::~Touchpad()
{
    if (!mFirstLoad) {
        delete ui;
        if (QGSettings::isSchemaInstalled(TOUCHPAD_SCHEMA)){
            delete tpsettings;
        }
    }
}

bool Touchpad::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==ui->touchpadSpeedSlider)
    {
        int dur = ui->touchpadSpeedSlider->maximum() - ui->touchpadSpeedSlider->minimum();
        if (event->type()==QEvent::MouseButtonPress)           //判断类型
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton)	//判断左键
            {
               int pos = ui->touchpadSpeedSlider->minimum() + dur * ((double)mouseEvent->x() / ui->touchpadSpeedSlider->width());
               if(pos != ui->touchpadSpeedSlider->sliderPosition())
                {
                  ui->touchpadSpeedSlider->setValue(pos);
                  tpsettings->set(TOUCHPAD_MOTION_ACCEL, (ui->touchpadSpeedSlider->maximum() - pos+3)/10.0);
                }
            }
        }
        else if(event->type()==QEvent::Wheel)
        {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            int val = ui->touchpadSpeedSlider->value();
            if(wheelEvent->delta()>0){
                ui->touchpadSpeedSlider->setValue(val - dur/100);
                tpsettings->set(TOUCHPAD_MOTION_ACCEL, (ui->touchpadSpeedSlider->maximum() - (val - dur/100)+3)/10.0);
            }
            else{
                ui->touchpadSpeedSlider->setValue(val + dur/100);
                tpsettings->set(TOUCHPAD_MOTION_ACCEL, (ui->touchpadSpeedSlider->maximum() - (val + dur/100)+3)/10.0);
            }
        }
    }
    return QObject::eventFilter(obj,event);
}

void Touchpad::initSearch(){
    //~ contents_path /touchpad/Enabled touchpad
    ui->label->setText(tr("Enabled touchpad"));
    //~ contents_path /touchpad/Disable touchpad while typing
    ui->label_2->setText(tr("Disable touchpad while typing"));
    //~ contents_path /touchpad/Enable mouse clicks with touchpad
    ui->label_3->setText(tr("Enable mouse clicks with touchpad"));
    //~ contents_path /touchpad/Scrolling
    ui->label_4->setText(tr("Scrolling"));
    //~ contents_path /touchpad/Pointer speed
    ui->touchpadSpeedLabel->setText(tr("Pointer speed"));
}

QString Touchpad::get_plugin_name(){
    return pluginName;
}

int Touchpad::get_plugin_type(){
    return pluginType;
}

QWidget *Touchpad::get_plugin_ui(){
    if (mFirstLoad) {
        mFirstLoad = false;
        ui = new Ui::Touchpad;
        double touchpadMotionAccel = 0.0;
        pluginWidget = new QWidget;
        pluginWidget->setAttribute(Qt::WA_StyledBackground,true);
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(pluginWidget);
        initSearch();
        //~ contents_path /touchpad/Touchpad Settings
        ui->titleLabel->setText(tr("Touchpad Settings"));
        ui->titleLabel->setStyleSheet("QLabel{font-size: 14px; color: palette(windowText);}");

        ui->touchpadSpeedSlider->setStyleSheet( "QSlider::groove:horizontal {"
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

        ui->touchpadSpeedSlider->setFixedHeight(60);
        ui->touchpadSpeedSlider->setMinimum(0);
        ui->touchpadSpeedSlider->setMaximum(35);
        ui->touchpadSpeedSlider->setPageStep(1);

        const QByteArray id(TOUCHPAD_SCHEMA);

        if (QGSettings::isSchemaInstalled(TOUCHPAD_SCHEMA)){
            tpsettings = new QGSettings(id);
            setupComponent();

            if (findSynaptics()){
                qDebug() << "Touch Devices Available";
                ui->tipLabel->hide();
                initTouchpadStatus();
            } else {
                ui->clickFrame->hide();
                ui->enableFrame->hide();
                ui->scrollingFrame->hide();
                ui->typingFrame->hide();
                ui->motionAccelFrame->hide();
            }
        }

        touchpadMotionAccel = tpsettings->get(TOUCHPAD_MOTION_ACCEL).toDouble();
        touchpadMotionAccel = touchpadMotionAccel == -1 ? 20 : (ui->touchpadSpeedSlider->maximum() - (touchpadMotionAccel*10 -3));
        ui->touchpadSpeedSlider->setValue(touchpadMotionAccel*1);

        connect(ui->touchpadSpeedSlider,&QSlider::sliderMoved,this,[=](int value){
               //printf("%s %d %d\n",__FUNCTION__,__LINE__,ui->touchpadSpeedSlider->maximum() - value+3);
               tpsettings->set(TOUCHPAD_MOTION_ACCEL, (ui->touchpadSpeedSlider->maximum() - value+3)/10.0);
        });
    }
    return pluginWidget;
}

void Touchpad::plugin_delay_control(){

}

const QString Touchpad::name() const {

    return QStringLiteral("touchpad");
}

void Touchpad::setupComponent(){
    //
    enableBtn = new SwitchButton(pluginWidget);
    ui->enableHorLayout->addWidget(enableBtn);

    //
    typingBtn = new SwitchButton(pluginWidget);
    ui->typingHorLayout->addWidget(typingBtn);

    //
    clickBtn = new SwitchButton(pluginWidget);
    ui->clickHorLayout->addWidget(clickBtn);

    //
    ui->scrollingTypeComBox->addItem(tr("Vertical two-finger scrolling"), V_FINGER_KEY);
    ui->scrollingTypeComBox->addItem(tr("Vertical edge scrolling"), V_EDGE_KEY);
    ui->scrollingTypeComBox->addItem(tr("Disable rolling"), N_SCROLLING);

    connect(enableBtn, &SwitchButton::checkedChanged, [=](bool checked){
        tpsettings->set(ACTIVE_TOUCHPAD_KEY, checked);
    });

    connect(typingBtn, &SwitchButton::checkedChanged, [=](bool checked){
        tpsettings->set(DISABLE_WHILE_TYPING_KEY, checked);
    });

    connect(clickBtn, &SwitchButton::checkedChanged, [=](bool checked){
        tpsettings->set(TOUCHPAD_CLICK_KEY, checked);
    });

#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 8)
    connect(ui->scrollingTypeComBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index){
#else
    connect(ui->scrollingTypeComBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index){
#endif
        Q_UNUSED(index)

        QString data = ui->scrollingTypeComBox->currentData().toString();
        if (data.compare(V_EDGE_KEY) == 0) {
            tpsettings->set(V_EDGE_KEY, true);
            tpsettings->set(H_EDGE_KEY, true);
            tpsettings->set(V_FINGER_KEY, false);
            tpsettings->set(H_FINGER_KEY, false);
        } else if (data.compare(V_FINGER_KEY) == 0) {
            tpsettings->set(V_EDGE_KEY, false);
            tpsettings->set(H_EDGE_KEY, false);
            tpsettings->set(V_FINGER_KEY, true);
            tpsettings->set(H_FINGER_KEY, true);
        } else if (data.compare(N_SCROLLING) == 0) {
            tpsettings->set(V_EDGE_KEY, false);
            tpsettings->set(H_EDGE_KEY, false);
            tpsettings->set(V_FINGER_KEY, false);
            tpsettings->set(H_FINGER_KEY, false);
        }
    });
}

void Touchpad::initTouchpadStatus(){
    //初始化启用按钮
    enableBtn->blockSignals(true);
    enableBtn->setChecked(tpsettings->get(ACTIVE_TOUCHPAD_KEY).toBool());
    enableBtn->blockSignals(false);

    // 初始化打字禁用
    typingBtn->blockSignals(true);
    typingBtn->setChecked(tpsettings->get(DISABLE_WHILE_TYPING_KEY).toBool());
    typingBtn->blockSignals(false);

    // 初始化触摸板点击
    clickBtn->blockSignals(true);
    clickBtn->setChecked(tpsettings->get(TOUCHPAD_CLICK_KEY).toBool());
    clickBtn->blockSignals(false);

    //初始化滚动
    ui->scrollingTypeComBox->blockSignals(true);
    ui->scrollingTypeComBox->setCurrentIndex(ui->scrollingTypeComBox->findData(_findKeyScrollingType()));
    ui->scrollingTypeComBox->blockSignals(false);
}

QString Touchpad::_findKeyScrollingType()
{
    if (tpsettings->get(V_EDGE_KEY).toBool()) {
        //水平滚动默认有效
        tpsettings->set(H_EDGE_KEY, true);

        tpsettings->set(H_FINGER_KEY,false);
        tpsettings->set(V_FINGER_KEY,false);
        return V_EDGE_KEY;
    }
    if (tpsettings->get(V_FINGER_KEY).toBool()) {
        //水平滚动默认有效
        tpsettings->set(H_FINGER_KEY, true);

        tpsettings->set(H_EDGE_KEY,false);
        tpsettings->set(V_EDGE_KEY,false);
        return V_FINGER_KEY;
    }

    tpsettings->set(H_FINGER_KEY,false);
    tpsettings->set(H_EDGE_KEY,false);
    return N_SCROLLING;
}

bool findSynaptics(){
    XDeviceInfo *device_info;
    int n_devices;
    bool retval;

    if (_supportsXinputDevices() == false)
        return true;

    device_info = XListInputDevices (QX11Info::display(), &n_devices);
    if (device_info == nullptr)
        return false;

    retval = false;
    for (int i = 0; i < n_devices; i++) {
        XDevice *device;

        device = _deviceIsTouchpad (&device_info[i]);
        if (device != nullptr) {
            retval = true;
            break;
        }
    }
    if (device_info != nullptr)
        XFreeDeviceList (device_info);

    return retval;
}

bool _supportsXinputDevices(){
    int op_code, event, error;

    return XQueryExtension (QX11Info::display(),
                            "XInputExtension",
                            &op_code,
                            &event,
                            &error);
}

XDevice* _deviceIsTouchpad (XDeviceInfo *deviceinfo){
    XDevice *device;
    if (deviceinfo->type != XInternAtom (QX11Info::display(), XI_TOUCHPAD, true))
        return nullptr;
    device = XOpenDevice (QX11Info::display(), deviceinfo->id);
    if(device == nullptr)
    {
        qDebug()<<"device== null";
        return nullptr;
    }

    if (_deviceHasProperty(device, "libinput Tapping Enabled") ||
            _deviceHasProperty(device, "Synaptics Off")) {
        return device;
    }
    XCloseDevice (QX11Info::display(), device);
    return nullptr;
}

bool _deviceHasProperty(XDevice *device, const char *property_name){
    Atom realtype, prop;
    int realformat;
    unsigned long nitems, bytes_after;
    unsigned char *data;

    prop = XInternAtom (QX11Info::display(), property_name, True);
    if (!prop)
        return false;
    if ((XGetDeviceProperty (QX11Info::display(), device, prop, 0, 1, False,
                             XA_INTEGER, &realtype, &realformat, &nitems,
                             &bytes_after, &data) == Success) && (realtype != None))
    {
        XFree (data);
        return true;
    }
    return false;
}
