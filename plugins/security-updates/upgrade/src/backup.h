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

#ifndef BACKUP_H
#define BACKUP_H

#include <QObject>
#include <QFile>
#include <QDBusInterface>
#include <QDBusServiceWatcher>
//#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>

class BackUp : public QObject
{
    Q_OBJECT
public:
    explicit BackUp(QObject *parent = nullptr);

    bool setProgress = false;

signals:
    void backupStartRestult(int);
    bool calCapacity();
    void bakeupFinish(int);
    void backupProgress(int);
public slots:
    int needBacdUp();
    void startBackUp(int);
    void creatInterface();

private slots:
    void sendRate(int,int);
    void receiveStartBackupResult(int result);

private:
    QString timeStamp ="";
    int bakeupState = 0;
    bool m_isActive = false;
    bool haveBackTool();
    bool readSourceManagerInfo();
    bool readBackToolInfo();
    QDBusInterface *interface = nullptr;
    QDBusServiceWatcher *watcher = nullptr;
    void onDBusNameOwnerChanged(const QString &name,const QString &oldOwner,const QString &newOwner);

};

#endif // BACKUP_H
