/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "brightnessFrame.h"
#include <QHBoxLayout>
#include <QtConcurrent>
#include <QGSettings>
#include <QDBusConnection>
#include <QDBusMessage>
#include <unistd.h>
#include <QDBusReply>
#include <QDBusInterface>

#define POWER_SCHMES                     "org.ukui.power-manager"
#define POWER_KEY                        "brightness-ac"
#define POWER_KEY_C                      "brightnessAc"
 
BrightnessFrame::BrightnessFrame(const QString &name, const bool &isBattery, const QString &edidHash, QWidget *parent) :
    QFrame(parent)
{
    this->setFixedHeight(50);
    this->setMinimumWidth(550);
    this->setMaximumWidth(960);
    this->setFrameShape(QFrame::Shape::Box);
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(6);
    layout->setMargin(9);

    labelName = new FixLabel(this);
    labelName->setFixedWidth(118);

    slider = new Uslider(Qt::Horizontal, this);
    slider->setRange(10, 100);

    labelValue = new QLabel(this);
    labelValue->setFixedWidth(35);
    labelValue->setAlignment(Qt::AlignRight);

    layout->addWidget(labelName);
    layout->addWidget(slider);
    layout->addWidget(labelValue);

    this->outputEnable = true;
    this->connectFlag = true;
    this->exitFlag = false;
    this->isBattery = isBattery;
    this->outputName = name;
    this->edidHash = edidHash;
    this->threadRunFlag = false;

    labelValue->setText("0"); //最低亮度10,获取前显示为0
    slider->setEnabled(false); //成功连接了再改为true，否则表示无法修改亮度
}

BrightnessFrame::~BrightnessFrame()
{
    exitFlag = true;
    threadRun.waitForFinished();
}

void BrightnessFrame::setTextLabelName(QString text)
{
    this->labelName->setText(text);
}

void BrightnessFrame::setTextLabelValue(QString text)
{
    this->labelValue->setText(text);
}

void BrightnessFrame::runConnectThread(const bool &openFlag)
{
    outputEnable = openFlag;
    if (false == isBattery) {
        if (true == threadRunFlag)
            return;

        threadRun = QtConcurrent::run([=]{
            threadRunFlag = true;
            if ("" == this->edidHash) {
                threadRunFlag = false;
                return;
            }

            int brightnessValue = getDDCBrighthess();
            if (brightnessValue < 0 || !slider || exitFlag) {
                threadRunFlag = false;
                return;
            }
            slider->setValue(brightnessValue);
            setTextLabelValue(QString::number(brightnessValue));
            slider->setEnabled(true);
            disconnect(slider,&QSlider::valueChanged,this,0);
            connect(slider, &QSlider::valueChanged, this, [=](){
                 qDebug()<<outputName<<"brightness"<<" is changed, value = "<<slider->value();
                 setTextLabelValue(QString::number(slider->value()));
                 setDDCBrightness(slider->value());
            });
            threadRunFlag = false;
        });
    } else {
        QByteArray powerId(POWER_SCHMES);
        if (QGSettings::isSchemaInstalled(powerId)) {
            QGSettings *mPowerGSettings = new QGSettings(powerId, QByteArray(), this);
            if (!mPowerGSettings->keys().contains(POWER_KEY_C)) {
                setTextLabelValue("-1");
            } else {
                int brightnessValue = mPowerGSettings->get(POWER_KEY).toInt();
                setTextLabelValue(QString::number(brightnessValue));
                slider->setValue(brightnessValue);
                slider->setEnabled(true);
                disconnect(slider,&QSlider::valueChanged,this,0);
                connect(slider, &QSlider::valueChanged, this, [=](){
                    qDebug()<<outputName<<"brightness"<<" is changed, value = "<<slider->value();
                    mPowerGSettings->blockSignals(true);
                    mPowerGSettings->set(POWER_KEY, slider->value());
                    mPowerGSettings->blockSignals(false);
                    setTextLabelValue(QString::number(mPowerGSettings->get(POWER_KEY).toInt()));
                });
                disconnect(mPowerGSettings,&QGSettings::changed,this,0);
                connect(mPowerGSettings,&QGSettings::changed,this,[=](QString key){
                   if (key == POWER_KEY_C) {
                       int value = mPowerGSettings->get(POWER_KEY).toInt();
                       slider->setValue(value);
                       setTextLabelValue(QString::number(value));
                   }
                });
            }
        }
    }
}

void BrightnessFrame::setOutputEnable(const bool &enable)
{
    outputEnable = enable;
}

bool BrightnessFrame::getSliderEnable()
{
    return slider->isEnabled();
}

void BrightnessFrame::setSliderEnable(const bool &enable)
{
    this->slider->setEnabled(enable);
    if (false == enable) {
        slider->setValue(0);
        setTextLabelValue("0");
    }
    return;
}

bool BrightnessFrame::getOutputEnable()
{
    return outputEnable;
}

QString BrightnessFrame::getOutputName()
{
    return outputName;
}

int BrightnessFrame::getDDCBrighthess()
{
    int times = 10;
    QDBusInterface ukccIfc("com.control.center.qt.systemdbus",
                           "/",
                           "com.control.center.interface",
                           QDBusConnection::systemBus());
    QDBusReply<int> reply;
    while (--times) {
        if (this->edidHash == "" || exitFlag)
            return -1;
        reply = ukccIfc.call("getDisplayBrightness", this->edidHash);
        if (reply.isValid() && reply.value() >= 0 && reply.value() <= 100) {
            return reply.value();
        }
        sleep(2);
    }
    return -1;
}

void BrightnessFrame::setDDCBrightness(const int &value)
{
    if (this->edidHash == "")
        return;

    QDBusInterface ukccIfc("com.control.center.qt.systemdbus",
                           "/",
                           "com.control.center.interface",
                           QDBusConnection::systemBus());


    if (mLock.tryLock()) {
        ukccIfc.call("setDisplayBrightness", QString::number(value), this->edidHash);
        mLock.unlock();
    }
}

void BrightnessFrame::updateEdidHash(const QString &edid)
{
    this->edidHash = edid;
}

QString BrightnessFrame::getEdidHash()
{
    return this->edidHash;
}
