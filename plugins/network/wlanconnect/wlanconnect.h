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

typedef struct ActiveConInfo_s {
    QString strConName;
    QString strConUUID;
    QString strConType;
}ActiveConInfo;

namespace Ui {
class WlanConnect;
}

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
public:
    bool getwifiisEnable();
    void initComponent();
    void runExternalApp();
    void initSearchText();
    void rebuildAvailComponent(QString iconpath, QString netName, QString type);
    void rebuildWifiActComponent(QString iconPath, QStringList netNameList);
    void runKylinmApp(QString netName, QString type);
private:
    bool getInitStatus();
    void clearContent();

    bool getWifiStatus();
    bool getHasWirelessCard();

    int                setSignal(QString lv);
    QString            wifiIcon(bool isLock, int strength);
    void               getWifiListDone(QVector<QStringList> wifislist);
    void               getActiveConInfo(QList<ActiveConInfo>& qlActiveConInfo);
private:
    Ui::WlanConnect *ui;

    QString            pluginName;
    int                pluginType;
    QWidget            *pluginWidget;

    QStringList        TwifiList;
    QStringList        lanList;
    QStringList        wifilist;

    QDBusInterface     *m_interface = nullptr;
    QDBusInterface     *kdsDbus = nullptr;
    QList<ActiveConInfo> mActiveInfo;

    QMap<QString, int> connectedWifi;
    QMap<QString,int>  wifiList;

    QStringList        actWifiNames;

    QString            connectWifi;
    QTimer             *refreshTimer;
private:
    SwitchButton       *wifiBtn;
    bool               mFirstLoad;

private slots:
    void refreshNetInfoTimerSlot();
    void wifiSwitchSlot(bool status);
    void getNetList();
};
#endif // WLANCONNECT_H
