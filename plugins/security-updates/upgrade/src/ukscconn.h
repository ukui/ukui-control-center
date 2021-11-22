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

#ifndef UKSCCONN_H
#define UKSCCONN_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QApplication>
#include <QSqlError>
#include <QFile>
#include <QDebug>
#include <QFileInfo>

class UKSCConn : public QObject
{
public:
    explicit UKSCConn();
    QSqlDatabase ukscDb;
    QSqlQuery query;
    bool isConnectUskc = true;

    // 根据应用名获取图标、中文名、描述
    QStringList getInfoByName(QString appName);

};

#endif // UKSCCONN_H
