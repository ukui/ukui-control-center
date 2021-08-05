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
#include <HoverWidget/hoverwidget.h>
#include <QMap>

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>

#include "shell/interface.h"
#include "SwitchButton/switchbutton.h"
#include "commonComponent/HoverBtn/hoverbtn.h"
#include "commonComponent/HoverWidget/hoverwidget.h"

enum {
    DISCONNECTED,
    NOINTERNET,
    CONNECTED
};

namespace Ui {
class NetConnect;
}

typedef struct ActiveConInfo_s {
    QString strConName;
    QString strConUUID;
    QString strConType;
}ActiveConInfo;

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

public:
    void initSearchText();
    void initComponent();
    void rebuildNetStatusComponent(QString iconPath, QMap<QString, bool> netNameMap);
    void rebuildAvailComponent(QString iconpath, QString netName, QString type);

    void runExternalApp();
    void runKylinmApp(QString netName, QString type);

    bool getwifiisEnable();

    void getActiveConInfo(QList<ActiveConInfo>& qlActiveConInfo);

protected:
    bool eventFilter(QObject *w,QEvent *e);

private:
    Ui::NetConnect     *ui;

    QString            pluginName;
    int                pluginType;
    QWidget            *pluginWidget;
    HoverWidget        *addLanWidget;

    QDBusInterface     *m_interface = nullptr;
    QDBusInterface     *kdsDbus = nullptr;
    SwitchButton       *wiredSwitch;

    QMap<QString, bool> actLanNames;

    QMap<QString, bool> noneAct;

    QStringList        TlanList;
    QStringList        lanList;

    bool               mFirstLoad;

    QList<ActiveConInfo> mActiveInfo;

    QGSettings         *qtSettings;
private:
    QStringList execGetLanList();
    void         getWifiListDone(QStringList lanList);

    bool        getInitStatus();
    void        clearContent();
    void        initHoverWidget();
private slots:
    void wifiSwitchSlot(bool status);
    void getNetList();
    void netPropertiesChangeSlot(QMap<QString, QVariant> property);
};

Q_DECLARE_METATYPE(QList<QDBusObjectPath>);

#endif // NETCONNECT_H
