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

#ifndef UTILS_H
#define UTILS_H
#include <QDir>
#include <QMetaType>

#define DOWN_CACHE_PATH QString("%1/.kylin-update-manager/").arg(QDir::homePath())
#define DOWN_TEMP_PATH QString("%1/.kylin-update-manager/kylin-update-manager-deb/deb/").arg(QDir::homePath())
#define SOURCESLIST "/etc/apt/sources.list"

enum type {http, ftp, file};
struct UrlMsg //记录单包信息：包名、全名、链接、大小
{
    QString name = "";
    QString fullname = "";
    QString url = "";
    long size = 0;
};

struct pkgProgress
{
    QString name;
    mutable int downloadProgress = 0;
    mutable int installProgress = 0;
};

struct AppMsg //记录当前包信息：包名、依赖列表、总大小、获取依赖状态
{
    QVector<UrlMsg> depList;
    long allSize = 0;
    bool getDepends = false;
};

struct AppAllMsg
{
    QString name;
    QString section;
    QString origin;
    long installedSize;
    QString maintainer;
    QString source;
    QString version;
    long packageSize;
    QString shortDescription;
    QString longDescription;

    QString changedPackages;
    QString packageCount;

    QString changelogUrl;
    QString screenshotUrl;
    QString availableVersion;

    bool isInstalled;
    bool upgradeable;
    AppMsg msg;
};
Q_DECLARE_METATYPE(AppMsg) //注册AppMsg结构用于信号槽传输
Q_DECLARE_METATYPE(AppAllMsg) //注册AppMsg结构用于信号槽传输
#endif // UTILS_H
