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

#include "xinputmanager.h"
#include <QProcess>
#include <QQueue>


XinputManager::XinputManager(QObject *parent):
    QObject(parent)
{
    init();
}

void XinputManager::init()
{
    m_pMonitorInputTask = MonitorInputTask::instance();
    connect(this, &XinputManager::sigStartThread, m_pMonitorInputTask, &MonitorInputTask::StartManager);
    connect(m_pMonitorInputTask, &MonitorInputTask::slaveAdded, this, &XinputManager::onSlaveAdded);
    connect(m_pMonitorInputTask, &MonitorInputTask::slaveRemoved, this, &XinputManager::onSlaveRemoved);

    m_pManagerThread = new QThread(this);
    m_pMonitorInputTask->moveToThread(m_pManagerThread);
}

void XinputManager::start()
{
    qDebug() << "info: [XinputManager][start]: thread id = " << QThread::currentThreadId();
    m_runningMutex.lock();
    m_pMonitorInputTask->m_running = true;
    m_runningMutex.unlock();

    m_pManagerThread->start();
    Q_EMIT sigStartThread();
}

void XinputManager::stop()
{
    if(m_pManagerThread->isRunning())
    {
        m_runningMutex.lock();
        m_pMonitorInputTask->m_running = false;
        m_runningMutex.unlock();

        m_pManagerThread->quit();
    }
}

void XinputManager::onSlaveAdded(int device_id)
{
    qDebug() << "info: [XinputManager][onSlaveAdded]: Slave Device(id =" << device_id << ") Added!";
    Q_EMIT xinputSlaveAdded(device_id);
}

void XinputManager::onSlaveRemoved(int device_id)
{
    qDebug() << "info: [XinputManager][onslaveRemoved]: Slave Device(id =" << device_id << ") Removed!";
    Q_EMIT xinputSlaveRemoved(device_id);
}

