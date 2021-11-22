/*
 * Copyright 2021 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "ukscconn.h"
UKSCConn::UKSCConn()
{
    ukscDb = QSqlDatabase::addDatabase("QSQLITE");

    QString UKSC_CACHE_DIR = QString(getenv("HOME")) + "/.cache/uksc/";
    QString UKSC_DATA_DIR = "/usr/share/kylin-software-center/data/";
    QString KUM_DIR = "/usr/share/kylin-update-manager/";
    QString dbFileName = "";
    dbFileName = UKSC_CACHE_DIR + "uksc.db";
    QFile file(dbFileName);
    if (!file.exists()) {
        dbFileName = UKSC_DATA_DIR + "uksc.db";
        file.setFileName(dbFileName);

        if (!file.exists()) {
            dbFileName = KUM_DIR + "uksc.db";
            file.setFileName(dbFileName);
            if (file.exists()) {
                ukscDb.setDatabaseName(dbFileName);
            }
        }
    }
    ukscDb.setDatabaseName(dbFileName);
    if (!ukscDb.open())
    {
        qWarning() << "软件商店数据库打开错误！";
        isConnectUskc = false;
    }
    else
    {
        query = QSqlQuery(ukscDb);//连接数据库
    }
}

// 根据应用名获取应用信息
QStringList UKSCConn::getInfoByName(QString appName)
{
    QStringList appInfo;
    if(isConnectUskc == true)
    {
        QString str = QString("select * from application where app_name = '%1'").arg(appName);
        query.exec(str);
        QString iconPath;       // 应用图标
        QString displayNameCN;  // 应用中文名
        QString displayName;  // 应用英文名
        QString description;    // 应用描述

        while (query.next())
        {
            iconPath = query.value(15).toString();
            displayNameCN = query.value(4).toString();
            displayName = query.value(3).toString();
            description =  query.value(7).toString();
        }

        iconPath = QString(getenv("HOME")) + "/.cache/uksc/icons/" + appName + ".png";
        QFile file(iconPath);
        if (!file.exists())
        {
            appInfo.append("");
        }
        else
        {
            appInfo.append(iconPath);

        }
        appInfo.append(displayNameCN);
        appInfo.append(displayName);
        appInfo.append(description);
    }
    return appInfo;

}
