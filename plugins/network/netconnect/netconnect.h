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
#ifndef NETCONNECT_H
#define NETCONNECT_H


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
#include <QMap>
#include <QPropertyAnimation>
#include <QPoint>

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>

#include "shell/interface.h"
#include "AddBtn/addbtn.h"
#include "Label/fixlabel.h"
#include "SwitchButton/switchbutton.h"
#include "commonComponent/HoverBtn/hoverbtn.h"
#include "lanitem.h"
#include "deviceframe.h"
#include "itemframe.h"

enum {
    DISCONNECTED,
    NOINTERNET,
    CONNECTED
};

namespace Ui {
class NetConnect;
}

typedef struct DeviceLanlistInfo_s
{
    QMap<QString,ItemFrame*> deviceLayoutMap;
    QMap<QString, LanItem*> lanItemMap;
}DeviceLanlistInfo;

class NetConnect : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kycc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    NetConnect();
    ~NetConnect();

    QString get_plugin_name() Q_DECL_OVERRIDE;
    int get_plugin_type() Q_DECL_OVERRIDE;
    QWidget * get_plugin_ui() Q_DECL_OVERRIDE;
    void plugin_delay_control() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
private:
    void initSearchText();
    void initComponent();
    void rebuildDeviceComponent(ItemFrame *frame, QString deviceName, int count);
    void rebuildAddComponent(ItemFrame *frame, QString deviceName);
    void rebuildAvailComponent(ItemFrame *frame, QString iconpath, QString deviceName, QString name, QString ssid, bool status, int type);
    void runExternalApp();
    void setSwitchStatus();
    void clearLayout(QVBoxLayout *layout);
    void runKylinmApp(QString netName, QString deviceName, int type);
    void getDeviceList();

protected:
    bool eventFilter(QObject *w,QEvent *e);

private:
    Ui::NetConnect     *ui;

    QString            pluginName;
    int                pluginType;
    QWidget            *pluginWidget;

    QDBusInterface     *m_interface = nullptr;
    QDBusInterface     *kdsDbus = nullptr;
    SwitchButton       *wiredSwitch;

//    QMap<QString, LanItem*> lanItemMap;

    bool               mFirstLoad;

    QGSettings         *m_switchGsettings;
    DeviceLanlistInfo   deviceLanlistInfo;
    QMap<QString, bool> deviceListMap;
    QMap<QString, bool> dropDownMap;
private slots:
    void wifiSwitchSlot(bool status);
    void getNetListFromDevice(QString deviceName, bool deviceStatus, QVBoxLayout *layout, int count);
    void rebuildOneFrame(QString deviceName, ItemFrame *frame);
    void dropDownAnimation(DeviceFrame * deviceFrame, QString deviceName, QMap<QString, bool> deviceListMap);
    void setItemLoading(QString devName, QString ssid);
    void setItemStopLoading(QString devName);
    void netPropertiesChangeSlot(QMap<QString, QVariant> property);
};

Q_DECLARE_METATYPE(QList<QDBusObjectPath>);

#endif // NETCONNECT_H
