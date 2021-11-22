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

#ifndef UKCCSESSIONSERVER_H
#define UKCCSESSIONSERVER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <QVariantMap>
#include <QCoreApplication>
#include <QDBusContext>
#include <QScopedPointer>
#include <QDBusVariant>

#include "json.h"
#include "screenStruct.h"

using QtJson::JsonObject;
using QtJson::JsonArray;

class ukccSessionServer : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.interface")
public:
    explicit ukccSessionServer();

private:
    QMap<QString, QVariant> getJsonInfo(const  QString &confFile);

private:
    QString mScreenMode;
    QVariantList mPreScreenCfg;
    QString mFilePath;

Q_SIGNALS:
    void configChanged();
    void screenChanged(QString status);

public Q_SLOTS:
    void exitService();
    void        ReloadSecurityConfig();
    QVariantMap getModuleHideStatus();
    QString     GetSecurityConfigPath();
    QString getScreenMode();
    void setScreenMode(QString screenMode);
    void setPreScreenCfg(QVariantList preScreenCfg);
    QVariantList getPreScreenCfg();
};

#endif // UKCCSESSIONSERVER_H
