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
#ifndef BLUETOOTH_H
#define BLUETOOTH_H


#include <QObject>
#include <QtPlugin>
#include <QTimer>
#include <QtDBus>
#include <QGSettings/QGSettings>
#include "shell/interface.h"
#include "HoverWidget/hoverwidget.h"
#include "ImageUtil/imageutil.h"
#include "SwitchButton/switchbutton.h"
#include "bluetooththread.h"
#define KYLIN_BLUETOOTH_SERVER_NAME      "org.blueman.Applet"
#define KYLIN_BLUETOOTH_SERVER_PATH      "/org/blueman/applet"
#define KYLIN_BLUETOOTH_SERVER_INTERFACE "org.blueman.Applet"

namespace Ui {
class Bluetooth;
}

class Bluetooth : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Bluetooth();
    ~Bluetooth();
    bool initBluetoothDbus();
    void initBluetoothStatus();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;

public:
    void initComponent();
    void runExternalApp();

private:

private:
    Ui::Bluetooth *ui;
    QString pluginName;
    int pluginType;
    QWidget * pluginWidget;
    HoverWidget * addWgt;
    SwitchButton *blueToothBtn;

private:
    QTimer * pTimer;
    QDBusInterface *m_pServiceInterface;
    bool m_bbluetoothStatus;
    bluetooththread *a;
    QGSettings * qtSettings;
public slots:
//    void refreshBluetoothDev();
    void bluetoothButtonClickSlots(bool status);
    void BluetoothStatusChangedSlots(bool status);
    void bluetoothservice(bool status);
};

#endif // BLUETOOTH_H
