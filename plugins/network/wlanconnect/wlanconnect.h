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

#ifndef WLANCONNECT_H
#define WLANCONNECT_H

#include <QObject>
#include <QtPlugin>

#include <QNetworkInterface>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QtNetwork/QtNetwork>

#include <QTimer>
#include <QStringList>
#include <QString>
#include <QGSettings>
#include <QListWidget>
#include <QListWidgetItem>
#include <HoverWidget/hoverwidget.h>
#include <QMap>

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>

#include "shell/interface.h"
#include "SwitchButton/switchbutton.h"
#include "commonComponent/HoverBtn/hoverbtn.h"
#include "itemframe.h"
#include "wlanitem.h"
namespace Ui {
class WlanConnect;
}


typedef struct DeviceWlanlistInfo_s
{
    QMap<QString,ItemFrame *> deviceLayoutMap;
    QMap<QString, WlanItem *> wlanItemMap;
}DeviceWlanlistInfo;

class WlanConnect : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    WlanConnect();
    ~WlanConnect();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;

private:
    void initComponent();
    void runExternalApp();
    void initSearchText();
    void rebuildDeviceComponent(ItemFrame *frame, QString deviceName, int count);
    void rebuildAvailComponent(ItemFrame *frame, QString deviceName, QString name, QString signal, bool isLock, bool status, int type);
    void activeConnect(QString netName, QString deviceName, int type);
    void deActiveConnect(QString netName, QString deviceName, int type);
    void clearLayout(QVBoxLayout * layout);
    void setSwitchStatus();
    void initNet();
    void getDeviceList();
    int                setSignal(QString lv);
    QString            wifiIcon(bool isLock, int strength);

protected:
    bool eventFilter(QObject *w,QEvent *e);

private:
    Ui::WlanConnect *ui;

    QString            pluginName;
    int                pluginType;
    QWidget            *pluginWidget;

    QDBusInterface     *m_interface = nullptr;
    QDBusInterface     *kdsDbus = nullptr;

    QGSettings         *m_switchGsettings = nullptr;

    QMap<QString, bool> deviceListMap;
    QMap<QString, bool> dropDownMap;

    DeviceWlanlistInfo   deviceWlanlistInfo;

private:
    SwitchButton       *wifiSwtch;
    bool               mFirstLoad;

private slots:
    void setItemStartLoading(QString devName, QString ssid);
    void setItemStopLoading(QString devName);
    void dropDownAnimation(DeviceFrame * deviceFrame, QString deviceName);
    void getNetListFromDevice(QString deviceName, bool deviceStatus, QVBoxLayout *layout, int count);
    void rebuildOneFrame(QString deviceName, ItemFrame *frame);
};
#endif // WLANCONNECT_H
