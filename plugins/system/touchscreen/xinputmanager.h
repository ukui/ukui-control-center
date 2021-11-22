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

#ifndef XINPUTMANAGER_H
#define XINPUTMANAGER_H
#include "monitorinputtask.h"
#include <QObject>
#include <QThread>




class XinputManager : public QObject
{
    Q_OBJECT
public:
    XinputManager(QObject *parent = nullptr);

    void start();
    void stop();

Q_SIGNALS:
    void sigStartThread();
    void xinputSlaveAdded(int device_id);
    void xinputSlaveRemoved(int device_id);

private:
    void init();

private:
    QThread *m_pManagerThread;
    QMutex m_runningMutex;
    MonitorInputTask *m_pMonitorInputTask;

private Q_SLOTS:
    void onSlaveAdded(int device_id);
    void onSlaveRemoved(int device_id);

private:
    void SetPenRotation(int device_id);

};


#endif // XINPUTMANAGER_H

