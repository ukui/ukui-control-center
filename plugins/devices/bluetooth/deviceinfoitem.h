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

#ifndef DEVICEINFOITEM_H
#define DEVICEINFOITEM_H

#include "config.h"


#include <KF5/BluezQt/bluezqt/device.h>

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QPixmap>
#include <QString>
#include <QIcon>
#include <QDebug>
#include <QResizeEvent>
#include <QPropertyAnimation>
#include <QTimer>
#include <QDateTime>
#include <QString>
#include <QFont>
#include <QGSettings/QGSettings>


#define ITEM_WIDTH 220
#define ITEM_WIDTH1 130
#define BTN_1_X 215
#define BTN_2_X 90
#define BTN_1_WIDTH 120
#define BTN_2_WIDTH 85

class DeviceInfoItem : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceInfoItem(QWidget *parent = nullptr);
    ~DeviceInfoItem();
    void initInfoPage(QString d_name = "",DEVICE_STATUS status = DEVICE_STATUS::NOT,BluezQt::DevicePtr device = nullptr);
    QString get_dev_name();
    void changeDevStatus(bool);
    void setDevConnectedIcon(bool);
    void AnimationInit();

    void refresh_device_icon(BluezQt::Device::Type changeType);
protected:
    void resizeEvent(QResizeEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
signals:
    void sendConnectDevice(QString);
    void sendDisconnectDeviceAddress(QString);
    void sendDeleteDeviceAddress(QString);
    void send_this_item_is_pair();
    void sendPairedAddress(QString);

    void connectComplete();

private slots:
    void onClick_Connect_Btn(bool);
    void onClick_Disconnect_Btn(bool);
    void onClick_Delete_Btn(bool);
    void updateDeviceStatus(DEVICE_STATUS status = DEVICE_STATUS::NOT);
    void GSettingsChanges(const QString &key);
private:
    QGSettings *item_gsettings = nullptr;

    QWidget *parent_widget = nullptr;
    QLabel *device_icon = nullptr;
    QLabel *device_name = nullptr;
    QLabel *device_status = nullptr;

    BluezQt::DevicePtr device_item = nullptr;

    QPushButton *connect_btn = nullptr;
    QPushButton *disconnect_btn = nullptr;
    QPushButton *del_btn = nullptr;

    DEVICE_STATUS d_status;

    QFrame *info_page = nullptr;
    QTimer *icon_timer = nullptr;
    QTimer *connect_timer = nullptr;
    int i = 7;

    QPropertyAnimation *enter_action = nullptr;
    QPropertyAnimation *leave_action = nullptr;

    bool AnimationFlag = false;
    QTimer *mouse_timer = nullptr;
};

#endif // DEVICEINFOITEM_H
