#include "publicdata.h"

PublicData::PublicData()
{
    // system
    QStringList systemPlugins;
    systemPlugins << QObject::tr("display") << QObject::tr("defaultapp") << QObject::tr("power") << QObject::tr("autoboot");
    subfuncList.append(systemPlugins);

    //devices
    QStringList devicesPlugins;
    devicesPlugins << QObject::tr("printer") << QObject::tr("mousecontrol") << QObject::tr("keyboardcontrol") << QObject::tr("audio");
    subfuncList.append(devicesPlugins);

    // personalized
    QStringList personalizedPlugins;
    personalizedPlugins<< QObject::tr("background") << QObject::tr("theme") << QObject::tr("screenlock") << QObject::tr("fonts") << QObject::tr("screensaver") /*<< "start" << "panel"*/;
    subfuncList.append(personalizedPlugins);

    //network
    QStringList networkPlugins;
    networkPlugins << QObject::tr("netconnect") << QObject::tr("vpn") << QObject::tr("proxy");
    subfuncList.append(networkPlugins);

    //account
    QStringList accountPLugins;
    accountPLugins << QObject::tr("userinfo") /*<< QObject::tr("loginoptions")*/;
    subfuncList.append(accountPLugins);

    //time language
    QStringList timelanguagePlugins;
    timelanguagePlugins/*<< "language"*/ << QObject::tr("datetime") << QObject::tr("area");
    subfuncList.append(timelanguagePlugins);

    //security update
    QStringList secupdatePlugins;
    secupdatePlugins << QObject::tr("update") /*<< QObject::tr("recovery")*/ << QObject::tr("backup");
    subfuncList.append(secupdatePlugins);

    //messages task
    QStringList mestaskPlugins;
    mestaskPlugins /*<< QObject::tr("notice-operation") << QObject::tr("multitask") */<< QObject::tr("about");
    subfuncList.append(mestaskPlugins);
}

PublicData::~PublicData()
{

}
