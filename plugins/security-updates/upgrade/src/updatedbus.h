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

#ifndef UPDATEDBUS_H
#define UPDATEDBUS_H

#define KYLIN_UPDATE_MANAGER_PATH "/cn/kylinos/KylinUpdateManager"

#define KYLIN_UPDATE_MANAGER_SERVICE "cn.kylinos.KylinUpdateManager"

#define KYLIN_UPDATE_MANAGER_INTERFACE "cn.kylinos.KylinUpdateManager"

//#define SERVICE_NAME_SIZE 30

#include <QObject>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QtDBus>
#include <unistd.h>
#include <sys/types.h>
#include <QFile>
#include <QSettings>
#include <QProcess>
#include <QTime>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlError>
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/file.h>
#include <pwd.h>
#include "traybusthread.h"

//#include <Python.h>
//#include "appupdate.h"
#include "metatypes.h"
#include "utils.h"
#include <QMetaType>
#include <fcntl.h>


//Q_DECLARE_METATYPE(AppAllMsg) //注册AppMsg结构用于信号槽传输

class UpdateDbus : public QObject
{
    Q_OBJECT
public:
    static UpdateDbus *getInstance(QObject *parenet  = nullptr);
    ~UpdateDbus();
    QDBusInterface *interface;  //dbus接口
    QDBusReply<QString> replyStr;   //string类型的返回值
    QDBusReply<bool> replyBool;     //bool类型的返回值
    QDBusReply<QStringList> replyStrList;
    QDBusReply<int> replyInt; //int类型的返回值
    QDBusInterface *interface1;

    //拷贝软件包到安装目录
    void copyFinsh(QStringList srcPath,QString appName);
    //创建root目录
    bool makeDirs(QString path);
    // setImportantStatus
    void setImportantStatus(bool status);
    //检查列表中的包是否可升级
    QStringList checkInstallOrUpgrade(QStringList list);

    void getAppMessage(QStringList list);
    //安装和升级
    bool installAndUpgrade(QString pkgName);
    //修改配置文件
    void modifyConf(QString path,QString group,QString key,QString value);
    //kill pid
    void cleanUpdateList();
    //初始化cache
    void init_cache();
//    bool makeDirs(QString path);
//dbus接口函数定义完毕
    //
    void SetDownloadLimit(QString,bool);
    int GetDownloadLimit(void);
    void onRequestSendDesktopNotify(QString message);
    QStringList inameList;  //重要更新列表
    QStringList importantList;
    QStringList failedList;
    int importantSize = 0;
    bool isPointOutNotBackup = true; //是否在单包更新时弹出提示未备份

    QString notifyMsg = "";


    bool fileLock();
    void fileUnLock();
    //调用接口插入数据库
    void insertInstallStates(QString item, QString info);
    void disconnectDbusSignal();
signals:
    void copyFinish(QString appName);
    void transferAptProgress(QString status,QString appName,float aptPercent,QString errormsg);
    void sendImportant();
    void sendAppMessageSignal(AppAllMsg msg);
//    void emitInameList(QStringList list);
    void sendFinishGetMsgSignal(int size);
    void sendUpdateSize(long size);

public slots:
    void getAptSignal(QString arg, QMap<QString, QVariant> map);
    void slotCopyFinished(QString appName);
    void getInameAndCnameList(QString arg);    //获取重要更新列表和紧急更新列表
//    void initD_bus(bool status);
    void getAppMessageSignal(QMap<QString, QVariant> map, QStringList urlList, QStringList nameList,QStringList fullnameList,QStringList sizeList, QString allSize, bool dependState);
    void slotFinishGetMessage(QString num);

private:
    explicit UpdateDbus(QObject *parent = nullptr);
    static UpdateDbus *updateMutual;  //UpdateDbus类静态对象
    QString lockPath = "/tmp/lock/kylin-update.lock";
};


#endif // UPDATEDBUS_H
