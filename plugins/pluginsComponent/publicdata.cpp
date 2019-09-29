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
#include "publicdata.h"

PublicData::PublicData()
{
    // system
    QStringList systemPlugins;
    systemPlugins << QObject::tr("display") << QObject::tr("defaultapp") << QObject::tr("power") << QObject::tr("autoboot");
    subfuncList.append(systemPlugins);
    QStringList en_systemPlugins;
    en_systemPlugins << "display" << "defaultapp" << "power" << "autoboot";
    en_subfuncList.append(en_systemPlugins);

    //devices
    QStringList devicesPlugins;
    devicesPlugins << QObject::tr("printer") << QObject::tr("mousecontrol") << QObject::tr("keyboardcontrol") << QObject::tr("audio");
    subfuncList.append(devicesPlugins);
    QStringList en_devicesPlugins;
    en_devicesPlugins << "printer" << "mousecontrol" << "keyboardcontrol" << "audio";
    en_subfuncList.append(en_devicesPlugins);

    // personalized
    QStringList personalizedPlugins;
    personalizedPlugins<< QObject::tr("background") << QObject::tr("theme") << QObject::tr("screenlock") << QObject::tr("fonts") << QObject::tr("screensaver") /*<< "start" << "panel"*/;
    subfuncList.append(personalizedPlugins);
    QStringList en_personalizedPlugins;
    en_personalizedPlugins<< "background" << "theme" << "screenlock" << "fonts" << "screensaver" /*<< "start" << "panel"*/;
    en_subfuncList.append(en_personalizedPlugins);

    //network
    QStringList networkPlugins;
    networkPlugins << QObject::tr("netconnect") << QObject::tr("vpn") << QObject::tr("proxy");
    subfuncList.append(networkPlugins);
    QStringList en_networkPlugins;
    en_networkPlugins << "netconnect" << "vpn" << "proxy";
    en_subfuncList.append(en_networkPlugins);

    //account
    QStringList accountPLugins;
    accountPLugins << QObject::tr("userinfo") /*<< QObject::tr("loginoptions")*/;
    subfuncList.append(accountPLugins);
    QStringList en_accountPLugins;
    en_accountPLugins << "userinfo" /*<< QObject::tr("loginoptions")*/;
    en_subfuncList.append(en_accountPLugins);

    //time language
    QStringList timelanguagePlugins;
    timelanguagePlugins/*<< "language"*/ << QObject::tr("datetime") << QObject::tr("area");
    subfuncList.append(timelanguagePlugins);
    QStringList en_timelanguagePlugins;
    en_timelanguagePlugins/*<< "language"*/ << "datetime" << "area";
    en_subfuncList.append(en_timelanguagePlugins);

    //security update
    QStringList secupdatePlugins;
    secupdatePlugins << QObject::tr("update") /*<< QObject::tr("recovery")*/ << QObject::tr("backup");
    subfuncList.append(secupdatePlugins);
    QStringList en_secupdatePlugins;
    en_secupdatePlugins << "update" /*<< "recovery"*/ << "backup";
    en_subfuncList.append(en_secupdatePlugins);

    //messages task
    QStringList mestaskPlugins;
    mestaskPlugins /*<< QObject::tr("notice-operation") << QObject::tr("multitask") */<< QObject::tr("about");
    subfuncList.append(mestaskPlugins);
    QStringList en_mestaskPlugins;
    en_mestaskPlugins /*<< "notice-operation" << "multitask" */<< "about";
    en_subfuncList.append(en_mestaskPlugins);
}

PublicData::~PublicData()
{

}
