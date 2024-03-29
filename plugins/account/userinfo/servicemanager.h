/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 * 
**/

#ifndef SERVICEMANAGER_H
#define SERVICEMANAGER_H

#include <QObject>
#include <QDBusInterface>

class ServiceManager : public QObject
{
    Q_OBJECT
public:
    static ServiceManager *instance();
    bool serviceExists();

private:
    explicit ServiceManager(QObject *parent = nullptr);
    void init();
    bool connectToService();

signals:
    void serviceStatusChanged(bool activate);

public slots:
    void onDBusNameOwnerChanged(const QString &name,
                                const QString &oldOwner,
                                const QString &newOwner);

private:
    static ServiceManager   *instance_;
    QDBusInterface          *dbusService;
    QDBusInterface          *bioService;
    bool                    serviceStatus;
};

#endif // SERVICEMANAGER_H
