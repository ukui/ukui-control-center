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
    //account
    QList<FuncInfo> accountList;
    for (int i = 0; i < TOTALACCOUNTFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = ACCOUNT;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        accountList.append(funcStruct);
    }
    accountList[USERINFO].nameString = QString("Userinfo");
    accountList[USERINFO].namei18nString = QObject::tr("Userinfo");
    funcinfoList.append(accountList);

    //network
    QList<FuncInfo> networkList;
    for (int i = 0; i < TOTALNETFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = NETWORK;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        networkList.append(funcStruct);
    }
    networkList[NETCONNECT].nameString = QString("Network");
    networkList[NETCONNECT].namei18nString = QObject::tr("Network");
    networkList[BLUETOOTH].nameString = QString("Bluetooth");
    networkList[BLUETOOTH].namei18nString = QObject::tr("Bluetooth");
//    networkList[VPN].nameString = QString("VPN");
//    networkList[VPN].namei18nString = QObject::tr("VPN");
    networkList[PROXY].nameString = QString("Proxy");
    networkList[PROXY].namei18nString = QObject::tr("Proxy");

    funcinfoList.append(networkList);

    //personalized
    QList<FuncInfo> personalizedList;
    for (int i = 0; i < TOTALPERSFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = PERSONALIZED;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        personalizedList.append(funcStruct);
    }
    personalizedList[WALLPAPER].nameString = QString("Wallpaper");
    personalizedList[WALLPAPER].namei18nString = QObject::tr("Wallpaper");
    personalizedList[WALLPAPER].mainShow = false;
    personalizedList[THEME].nameString = QString("Theme");
    personalizedList[THEME].namei18nString = QObject::tr("Theme");
    personalizedList[LOCKSCREEN].nameString = QString("Lockscreen");
    personalizedList[LOCKSCREEN].namei18nString = QObject::tr("Lockscreen");
    personalizedList[FONTS].nameString = QString("Fonts");
    personalizedList[FONTS].namei18nString = QObject::tr("Fonts");
    funcinfoList.append(personalizedList);

    //system
    QList<FuncInfo> systemList;
    for (int i = 0; i < TOTALSYSFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = SYSTEM;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        systemList.append(funcStruct);
    }
    systemList[BATTERY].nameString = QString("Battery");
    systemList[BATTERY].namei18nString = QObject::tr("Battery");
    systemList[STARTUP].nameString = QString("Startup");
    systemList[STARTUP].namei18nString = QObject::tr("Startup");
    systemList[OPERATIONMODE].nameString = QString("Operation Mode");
    systemList[OPERATIONMODE].namei18nString = QObject::tr("Operation Mode");
    systemList[SOUNDS].nameString = QString("Sounds ");
    systemList[SOUNDS].namei18nString = QObject::tr("Sounds ");
    systemList[DATTIME].nameString = QString("Date & Time");
    systemList[DATTIME].namei18nString = QObject::tr("Date & Time");
    systemList[LANGUAGEREAGION].nameString = QString("Language & Region");
    systemList[LANGUAGEREAGION].namei18nString = QObject::tr("Language & Region");
    systemList[SYSTEMRECOVERY].nameString = QString("System Recovery");
    systemList[SYSTEMRECOVERY].namei18nString = QObject::tr("System Recovery");
    funcinfoList.append(systemList);

    //devices
    QList<FuncInfo> devicesList;
    for (int i = 0; i < TOTALDEVICESFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = DEVICES;
        funcStruct.index = i;
//        funcStruct.mainShow = true;
        devicesList.append(funcStruct);
    }
    devicesList[DISPLAYS].nameString = QString("Displays");
    devicesList[DISPLAYS].namei18nString =  QObject::tr("Displays");
    devicesList[PRINTERS].nameString = QString("Printers");
    devicesList[PRINTERS].namei18nString = QObject::tr("Printers");
    devicesList[MOUSE].nameString = QString("Mouse");
    devicesList[MOUSE].namei18nString = QObject::tr("Mouse");
    devicesList[TRACKPAD].nameString = QString("Trackpad");
    devicesList[TRACKPAD].namei18nString = QObject::tr("Trackpad");
    devicesList[PEN].nameString = QString("Pen");
    devicesList[PEN].namei18nString = QObject::tr("Pen");
    devicesList[TRACKPAD].mainShow = false;
    devicesList[SOUNDS].mainShow = false;
    funcinfoList.append(devicesList);

    QList<FuncInfo> otherList;
    for (int i = 0; i < TOTALNETFUNC; i++){
        FuncInfo funcStruct;
        funcStruct.type = OTHER;
        funcStruct.index = i;
        funcStruct.mainShow = true;
        otherList.append(funcStruct);
    }
    otherList[GESTURE].nameString = QString("Gesture Guidance");
    otherList[GESTURE].namei18nString = QObject::tr("Gesture Guidance");
    otherList[ABOUT].nameString = QString("About & Support");
    otherList[ABOUT].namei18nString = QObject::tr("About & Support");
    funcinfoList.append(otherList);
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
