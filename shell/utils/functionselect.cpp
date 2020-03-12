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

QList<QList<FuncInfo>> FunctionSelect::funcinfoList;
QStack<RecordFunc> FunctionSelect::recordFuncStack;

//FuncInfo FunctionSelect::displayStruct;


FunctionSelect::FunctionSelect()
{
}

FunctionSelect::~FunctionSelect()
{
}

void FunctionSelect::initValue(){

    //system
    QList<FuncInfo> systemList;
    for (int i = 0; i < TOTALSYSFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = SYSTEM;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        systemList.append(funcStruct);
    }
    systemList[DISPLAY].nameString = QString("display");
    systemList[DISPLAY].namei18nString =  QObject::tr("display");
    systemList[DEFAULTAPP].nameString = QString("defaultapp");
    systemList[DEFAULTAPP].namei18nString =  QObject::tr("defaultapp");
    systemList[POWER].nameString = QString("power");
    systemList[POWER].namei18nString = QObject::tr("power");
    systemList[AUTOBOOT].nameString = QString("autoboot");
    systemList[AUTOBOOT].namei18nString = QObject::tr("autoboot");

    funcinfoList.append(systemList);

    //devices
    QList<FuncInfo> devicesList;
    for (int i = 0; i < TOTALDEVICESFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = DEVICES;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        devicesList.append(funcStruct);
    }
    devicesList[PRINTER].nameString = QString("printer");
    devicesList[PRINTER].namei18nString = QObject::tr("printer");
    devicesList[MOUSE].nameString = QString("mouse");
    devicesList[MOUSE].namei18nString = QObject::tr("mouse");
    devicesList[TOUCHPAD].nameString = QString("touchpad");
    devicesList[TOUCHPAD].namei18nString = QObject::tr("touchpad");
    devicesList[TOUCHPAD].mainShow = false;
    devicesList[KEYBOARD].nameString = QString("keyboard");
    devicesList[KEYBOARD].namei18nString = QObject::tr("keyboard");
    devicesList[SHORTCUT].nameString = QString("shortcut");
    devicesList[SHORTCUT].namei18nString = QObject::tr("shortcut");
    devicesList[SHORTCUT].mainShow = false;
    devicesList[AUDIO].nameString = QString("audio");
    devicesList[AUDIO].namei18nString = QObject::tr("audio");

    funcinfoList.append(devicesList);

    //personalized
    QList<FuncInfo> personalizedList;
    for (int i = 0; i < TOTALPERSFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = PERSONALIZED;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        personalizedList.append(funcStruct);
    }
    personalizedList[BACKGROUND].nameString = QString("background");
    personalizedList[BACKGROUND].namei18nString = QObject::tr("background");
    personalizedList[THEME].nameString = QString("theme");
    personalizedList[THEME].namei18nString = QObject::tr("theme");
    personalizedList[SCREENLOCK].nameString = QString("screenlock");
    personalizedList[SCREENLOCK].namei18nString = QObject::tr("screenlock");
    personalizedList[FONTS].nameString = QString("fonts");
    personalizedList[FONTS].namei18nString = QObject::tr("fonts");
    personalizedList[SCREENSAVER].nameString = QString("screensaver");
    personalizedList[SCREENSAVER].namei18nString = QObject::tr("screensaver");
    personalizedList[DESKTOP].nameString = QString("desktop");
    personalizedList[DESKTOP].namei18nString = QObject::tr("desktop");
    personalizedList[DESKTOP].mainShow = false;
    funcinfoList.append(personalizedList);

    //network
    QList<FuncInfo> networkList;
    for (int i = 0; i < TOTALNETFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = NETWORK;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        networkList.append(funcStruct);
    }
    networkList[NETCONNECT].nameString = QString("netconnect");
    networkList[NETCONNECT].namei18nString = QObject::tr("netconnect");
    networkList[VPN].nameString = QString("vpn");
    networkList[VPN].namei18nString = QObject::tr("vpn");
    networkList[PROXY].nameString = QString("proxy");
    networkList[PROXY].namei18nString = QObject::tr("proxy");

    funcinfoList.append(networkList);

    //account
    QList<FuncInfo> accountList;
    for (int i = 0; i < TOTALACCOUNTFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = ACCOUNT;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        accountList.append(funcStruct);
    }

    accountList[USERINFO].nameString = QString("userinfo");
    accountList[USERINFO].namei18nString = QObject::tr("userinfo");

    funcinfoList.append(accountList);

    //datetime
    QList<FuncInfo> datetimeList;
    for (int i = 0; i < TOTALDTFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = DATETIME;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        datetimeList.append(funcStruct);
    }

    datetimeList[DAT].nameString = QString("datetime");
    datetimeList[DAT].namei18nString = QObject::tr("datetime");
    datetimeList[AREA].nameString = QString("area");
    datetimeList[AREA].namei18nString = QObject::tr("area");

    funcinfoList.append(datetimeList);

    //securityupdates
    QList<FuncInfo> seupdatesList;
    for (int i = 0; i < TOTALSUFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = UPDATE;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        seupdatesList.append(funcStruct);
    }

    seupdatesList[UPDATES].nameString = QString("update");
    seupdatesList[UPDATES].namei18nString = QObject::tr("update");
    seupdatesList[BACKUP].nameString = QString("backup");
    seupdatesList[BACKUP].namei18nString = QObject::tr("backup");

    funcinfoList.append(seupdatesList);

    //noticeandtasks
    QList<FuncInfo> natList;
    for (int i = 0; i < TOTALNATFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = NOTICEANDTASKS;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        natList.append(funcStruct);
    }

    natList[NOTICE].nameString = QString("notice");
    natList[NOTICE].namei18nString = QObject::tr("notice");
    natList[ABOUT].nameString = QString("about");
    natList[ABOUT].namei18nString = QObject::tr("about");
    natList[EXPERIENCEPLAN].nameString = QString("experienceplan");
    natList[EXPERIENCEPLAN].namei18nString = QObject::tr("experienceplan");

    funcinfoList.append(natList);
}

void FunctionSelect::pushRecordValue(int type, QString name){
    RecordFunc reFuncStruct;
    reFuncStruct.type = type;
    reFuncStruct.namei18nString = name;
    if (recordFuncStack.length() < 1)
        recordFuncStack.push(reFuncStruct);
    else if (QString::compare(recordFuncStack.last().namei18nString, name) != 0){
//        qDebug() << recordFuncStack.last().namei18nString << name;
        recordFuncStack.push(reFuncStruct);
    }
}

void FunctionSelect::popRecordValue(){
    recordFuncStack.pop();
}
