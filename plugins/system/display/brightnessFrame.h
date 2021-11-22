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

#ifndef BRIGHTNESSFRAME_H
#define BRIGHTNESSFRAME_H

#include <QFrame>
#include <QLabel>
#include <QWidget>
#include <QLabel>
#include "Uslider/uslider.h"
#include <QMutex>
#include <QFuture>
#include <Label/fixlabel.h>

class BrightnessFrame : public QFrame
{
    Q_OBJECT
public:
    BrightnessFrame(const QString &name, const bool &isBattery, const QString &edidHash = "", QWidget *parent = nullptr);
    ~BrightnessFrame();
    void setTextLabelName(QString text);
    void setTextLabelValue(QString text);
    void setOutputEnable(const bool &enable);
    bool getOutputEnable();
    void runConnectThread(const bool &openFlag);
    int  getDDCBrighthess();
    bool getSliderEnable();
    void setSliderEnable(const bool &enable);
    void setDDCBrightness(const int &value);
    void updateEdidHash(const QString &edid);
    QString getEdidHash();
    QString getOutputName();
private:
    FixLabel *labelName = nullptr;
    QLabel *labelValue = nullptr;
    Uslider *slider = nullptr;
    QString outputName;          //屏幕名
    bool    outputEnable;     //该屏幕是否打开
    bool    connectFlag;  //该屏幕是否连接
    QString edidHash;
    QMutex      mLock;
    bool exitFlag;
    bool isBattery;
    QFuture<void> threadRun;
    volatile bool threadRunFlag;
};

#endif // BRIGHTNESSFRAME_H
