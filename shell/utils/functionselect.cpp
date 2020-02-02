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
#include "functionselect.h"
#include "../interface.h"

#include <QDebug>


QList<QStringList> FunctionSelect::funcsList;
QList<QList<FuncInfo>> FunctionSelect::funcinfoList;

//FuncInfo FunctionSelect::displayStruct;


FunctionSelect::FunctionSelect()
{
}

FunctionSelect::~FunctionSelect()
{
}

void FunctionSelect::initValue(){
    //列表增加元素
    for (int i = 0; i < FUNCTOTALNUM; i++){
        funcsList.append(QStringList());
    }

    //system
    QList<FuncInfo> systemList;
    FuncInfo displayStruct;
    displayStruct.type = SYSTEM; displayStruct.index = 0; displayStruct.nameString = QString("display"); displayStruct.namei18nString = QObject::tr("display");
    systemList.append(displayStruct);

    FuncInfo defaultappStruct;
    defaultappStruct.type = SYSTEM; defaultappStruct.index = 1; defaultappStruct.nameString = QString("defaultapp"); defaultappStruct.namei18nString = QObject::tr("defaultapp");
    systemList.append(defaultappStruct);

    FuncInfo powerStruct;
    powerStruct.type = SYSTEM; powerStruct.index = 2; powerStruct.nameString = QString("power"); powerStruct.namei18nString = QObject::tr("power");
    systemList.append(powerStruct);

    FuncInfo autobootStruct;
    autobootStruct.type = SYSTEM; autobootStruct.index = 3; autobootStruct.nameString = QString("autoboot"); autobootStruct.namei18nString = QObject::tr("autoboot");
    systemList.append(autobootStruct);

    funcinfoList.append(systemList);

    //devices
    QList<FuncInfo> devicesList;
    FuncInfo printerStruct;
    printerStruct.type = DEVICES; printerStruct.index = 0; printerStruct.nameString = QString("printer"); printerStruct.namei18nString = QObject::tr("printer");
    devicesList.append(printerStruct);

    FuncInfo mousecontrolStruct;
    mousecontrolStruct.type = DEVICES;
    mousecontrolStruct.index = 1;
    mousecontrolStruct.nameString = QString("mousecontrol");
    mousecontrolStruct.namei18nString = QObject::tr("mousecontrol");
    devicesList.append(mousecontrolStruct);

    FuncInfo keyboardcontrolStruct;
    keyboardcontrolStruct.type = DEVICES;
    keyboardcontrolStruct.index = 2;
    keyboardcontrolStruct.nameString = QString("keyboard");
    keyboardcontrolStruct.namei18nString = QObject::tr("keyboard");
    devicesList.append(keyboardcontrolStruct);

    FuncInfo audioStruct;
    audioStruct.type = DEVICES;
    audioStruct.index = 3;
    audioStruct.nameString = QString("audio");
    audioStruct.namei18nString = QObject::tr("audio");
    devicesList.append(audioStruct);

    funcinfoList.append(devicesList);

    //personalized
    QList<FuncInfo> personalizedList;
    FuncInfo backgroundStruct;
    backgroundStruct.type = PERSONALIZED; backgroundStruct.index = 0; backgroundStruct.nameString = QString("background"); backgroundStruct.namei18nString = QObject::tr("background");
    personalizedList.append(backgroundStruct);

    FuncInfo themeStruct;
    themeStruct.type = PERSONALIZED; themeStruct.index = 1; themeStruct.nameString = QString("theme"); themeStruct.namei18nString = QObject::tr("theme");
    personalizedList.append(themeStruct);

    FuncInfo screenlockStruct;
    screenlockStruct.type = PERSONALIZED; screenlockStruct.index = 2; screenlockStruct.nameString = QString("screenlock"); screenlockStruct.namei18nString = QObject::tr("screenlock");
    personalizedList.append(screenlockStruct);

    FuncInfo fontsStruct;
    fontsStruct.type = PERSONALIZED; fontsStruct.index = 3; fontsStruct.nameString = QString("fonts"); fontsStruct.namei18nString = QObject::tr("fonts");
    personalizedList.append(fontsStruct);

    FuncInfo screensaverStruct;
    screensaverStruct.type = PERSONALIZED; screensaverStruct.index = 4; screensaverStruct.nameString = QString("screensaver"); screensaverStruct.namei18nString = QObject::tr("screensaver");
    personalizedList.append(screensaverStruct);

    FuncInfo desktopStruct;
    desktopStruct.type = PERSONALIZED; desktopStruct.index = 4; desktopStruct.nameString = QString("desktop"); desktopStruct.namei18nString = QObject::tr("desktop");
    personalizedList.append(desktopStruct);

    funcinfoList.append(personalizedList);

    //network
    QList<FuncInfo> networkList;
    FuncInfo netconnectStruct;
    netconnectStruct.type = NETWORK; netconnectStruct.index = 0; netconnectStruct.nameString = QString("netconnect"); netconnectStruct.namei18nString = QObject::tr("netconnect");
    networkList.append(netconnectStruct);

    FuncInfo vpnStruct;
    vpnStruct.type = NETWORK; vpnStruct.index = 1; vpnStruct.nameString = QString("vpn"); vpnStruct.namei18nString = QObject::tr("vpn");
    networkList.append(vpnStruct);

    FuncInfo proxyStruct;
    proxyStruct.type = NETWORK; proxyStruct.index = 2; proxyStruct.nameString = QString("proxy"); proxyStruct.namei18nString = QObject::tr("proxy");
    networkList.append(proxyStruct);

    funcinfoList.append(networkList);

    //account
    QList<FuncInfo> accountList;
    FuncInfo userinfoStruct;
    userinfoStruct.type = ACCOUNT; userinfoStruct.index = 0; userinfoStruct.nameString = QString("userinfo"); userinfoStruct.namei18nString = QObject::tr("userinfo");
    accountList.append(userinfoStruct);

    funcinfoList.append(accountList);

    //datetime
    QList<FuncInfo> datetimeList;
    FuncInfo datetimeStruct;
    datetimeStruct.type = DATETIME; datetimeStruct.index = 0; datetimeStruct.nameString = QString("datetime"); datetimeStruct.namei18nString = QObject::tr("datetime");
    datetimeList.append(datetimeStruct);

    FuncInfo areaStruct;
    areaStruct.type = DATETIME; areaStruct.index = 1; areaStruct.nameString = QString("area"); areaStruct.namei18nString = QObject::tr("area");
    datetimeList.append(areaStruct);

    funcinfoList.append(datetimeList);

    //update
    QList<FuncInfo> updateList;
    FuncInfo updateStruct;
    updateStruct.type = UPDATE; updateStruct.index = 0; updateStruct.nameString = QString("update"); updateStruct.namei18nString = QObject::tr("update");
    updateList.append(updateStruct);

    FuncInfo backupStruct;
    backupStruct.type = UPDATE; backupStruct.index = 1; backupStruct.nameString = QString("backup"); backupStruct.namei18nString = QObject::tr("backup");
    updateList.append(backupStruct);

    funcinfoList.append(updateList);

    //messages
    QList<FuncInfo> messagesList;
    FuncInfo aboutStruct;
    aboutStruct.type = MESSAGES; aboutStruct.index = 0; aboutStruct.nameString = QString("about"); aboutStruct.namei18nString = QObject::tr("about");
    messagesList.append(aboutStruct);

    funcinfoList.append(messagesList);

    //-------------------------------------------------------------//

    //system
    QStringList systemPlugins;
    systemPlugins << QString("display") << QString("defaultapp") << QString("power") << QString("autoboot");
    funcsList[SYSTEM] = systemPlugins;

    //devices
    QStringList devicesPlugins;
    devicesPlugins << QString("printer") << QString("mousecontrol") << QString("keyboard") << QString("audio");
//    devicesPlugins << QObject::tr("printer") << QObject::tr("mousecontrol") << QObject::tr("keyboardcontrol") << QObject::tr("audio");
    funcsList[DEVICES] = devicesPlugins;

    //personalized
    QStringList personalizedPlugins;
    personalizedPlugins << QString("background") << QString("theme") << QString("screenlock") << QString("fonts") << QString("screensaver");
    funcsList[PERSONALIZED] = personalizedPlugins;

    //network
    QStringList networkPlugins;
    networkPlugins << QString("netconnect") << QString("vpn") << QString("proxy");
    funcsList[NETWORK] = networkPlugins;

    //account
    QStringList accountPlugins;
    accountPlugins << QString("userinfo") /*<< QString("loginoptions")*/;
    funcsList[ACCOUNT] = accountPlugins;

    //datetime
    QStringList datetimePlugings;
    datetimePlugings << QString("datetime") << QString("area");
    funcsList[DATETIME] = datetimePlugings;

    //update
    QStringList updatePlugins;
    updatePlugins << QString("update") << QString("backup");
    funcsList[UPDATE] = updatePlugins;

    //messages
    QStringList messagesPlugins;
    messagesPlugins << QString("about");
    funcsList[MESSAGES] = messagesPlugins;
}
