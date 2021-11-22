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

#ifndef TRAYBUSTHREAD_H
#define TRAYBUSTHREAD_H

#include <QThread>
#include <QtDBus>

class traybusthread : public QThread
{
    Q_OBJECT
signals:
    void result(QStringList updateList);

private:
    void run();
    QStringList inameList;  //重要更新列表
    QDBusInterface *m_pServiceInterface;



public slots:
    void getInameAndCnameList(QString arg);

};


#endif // TRAYBUSTHREAD_H
