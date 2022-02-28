#include "tabwidget.h"
#include <QProcess>
#include <kybackup/backuptools-define.h>

#define JSON_FILE_PATH "/var/lib/kylin-system-updater/json/"

TabWid::TabWid(QWidget *parent):QWidget(parent)
{
    allComponents();
}

void TabWid::initDbus()
{
    /* 更新管理器dbus接口 */
    updateMutual = UpdateDbus::getInstance();

    /* 源管理器dbus接口 */
    updateSource = new UpdateSource();

    QThread *tmp_thread = new QThread;
    connect(tmp_thread,&QThread::started,updateSource,&UpdateSource::startDbus);
    connect(updateSource,&UpdateSource::startDbusFinished,this,&TabWid::dbusFinished);
    updateSource->moveToThread(tmp_thread);
    tmp_thread->start();

    ukscConnect = new UKSCConn();

    /*备份还原初始化*/
    backup = new BackUp;
    backupThread = new QThread;
    backup->moveToThread(backupThread);
    backupThread->start();

    connect(updateMutual,&UpdateDbus::sendAppMessageSignal,this,&TabWid::loadingOneUpdateMsgSlot);
    connect(updateMutual,&UpdateDbus::sendFinishGetMsgSignal,this,&TabWid::loadingFinishedSlot);
    connect(checkUpdateBtn,&QPushButton::clicked,this,&TabWid::checkUpdateBtnClicked);
    connect(this, &TabWid::updateAllSignal, this, &TabWid::updateAllApp);
    connect(historyUpdateLog,&QPushButton::clicked,this,&TabWid::showHistoryWidget);
    connect(updateSource,&UpdateSource::getReplyFalseSignal,this,&TabWid::getReplyFalseSlot);

    connect(DownloadHBtn,&SwitchButton::checkedChanged,this,&TabWid::DownloadLimitChanged);
    connect(DownloadHValue,&QComboBox::currentTextChanged,this,&TabWid::DownloadLimitValueChanged);
    /*实时更新信号*/
//    connect(updateMutual,&UpdateDbus::DownloadSpeedChanged,this->DownloadHValue,&QComboBox::setCurrentText);

    //    bacupInit();//初始化备份
    checkUpdateBtn->stop();
    //    checkUpdateBtn->setText(tr("检查更新"));
    checkUpdateBtn->setText(tr("Check Update"));
//    checkUpdateBtn->setText(tr("initializing"));
//    checkUpdateBtn->setEnabled(false);

}


void TabWid::dbusFinished()
{
    /*获取当前自动更新的状态*/
    checkUpdateBtnClicked();
}

void TabWid::getAutoUpgradeStatus()
{
//    fileLock();
    QStringList list;
    list << "CONTROL_CENTER/autoupdate_run_status";
    QString ret =  updateSource->getOrSetConf("get", list);

    if (!ret.compare("backup")) {
        isAutoUpgrade = true;
        /*如果自动更新在备份中，那就直接绑定备份还原信号即可*/
        isAllUpgrade = true;
        bool ret = autoUpdateLoadUpgradeList(true);
        if (ret) {
            bacupInit(true);
            backup->creatInterface();
            backup->setProgress = true;
            backupProgress(0);
        } else {
            bacupInit(false);
        }
    } else if (!ret.compare("download")) {
        /*如果自动更新在下载中，调用dbus去kill掉下载程序，继续原流程，不进行多余操作*/
        QFile file("/var/run/apt-download.pid");
        QString pid;
        if (file.open(QIODevice::ReadOnly)) {
            pid = file.readAll();
        }
        file.close();
        updateSource->killProcessSignal(pid.toInt(), 10);
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->setText(tr("Check Update"));
        checkUpdateBtnClicked();
    } else if (!ret.compare("install")){
        isAutoUpgrade = true;
        /*如果自动更新在安装中，绑定更新管理器dbus接收信号即可*/
        isAllUpgrade = true;
        checkUpdateBtn->hide();
        checkUpdateBtn->setText(tr("UpdateAll"));
        checkUpdateBtn->adjustSize();
        bool ret = autoUpdateLoadUpgradeList(false);
        if (!ret)
            updateMutual->disconnectDbusSignal();
    } else if (!ret.compare("idle")) {
        /*如果没有进行自动更新，那就不需要操作 */
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->setText(tr("Check Update"));
        checkUpdateBtnClicked();
    } else {
        /*如果读不到，默认也不进行操作*/
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->setText(tr("Check Update"));
        checkUpdateBtnClicked();
    }
}

bool TabWid::autoUpdateLoadUpgradeList(bool isBackUp)
{
    QSettings get("/var/lib/kylin-auto-upgrade/kylin-autoupgrade-pkglist.conf", QSettings::IniFormat);
    QString str;
    if (isBackUp)
        str = get.value("DOWNLOAD/pkgname").toString();
    else
        str = get.value("DOWNLOAD/uninstpkg").toString();
    qDebug() << "----------pkgname---->" << str;

    if (str.isNull()) {
        versionInformationLab->setText(tr("Your system is the latest!"));
//        versionInformationLab->setToolTip("");
        QString updatetime = tr("No Information!");
        QSqlQuery queryInstall(QSqlDatabase::database("A"));
        queryInstall.exec("select * from updateinfos order by id desc");
        while (queryInstall.next()) {
            QString statusType = queryInstall.value("keyword").toString();
            if (statusType == "" || statusType =="1") {
                updatetime = queryInstall.value("date").toString();
                break;
            }
        }
        lastRefreshTime->setText(tr("Last refresh:")+ updatetime);
        lastRefreshTime->show();
        checkUpdateBtn->setText(tr("Check Update"));
        return false;
    } else {
        QStringList list;
        if(str.contains(" ")) {
            list = str.split(" ");
        } else {
            list << str;
        }
        versionInformationLab->setText(tr("Downloading and installing updates..."));
//        versionInformationLab->setToolTip("");
        lastRefreshTime->hide();
        allProgressBar->show();
        allProgressBar->setValue(10);
        updateMutual->getAppMessage(list);
        return true;
    }
}

TabWid::~TabWid()
{
    qDebug() << "~TabWid" ;
    delete updateMutual;
    updateMutual = nullptr;
    backupDelete();//回收资源

//    fileUnLock();
}

void TabWid::backupMessageBox(QString str)
{
    QMessageBox msgBox;
    msgBox.setText(str);
    msgBox.addButton(tr("Update now"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("Cancel update"), QMessageBox::RejectRole);
    int ret = msgBox.exec();
    if(ret == 0)
    {
        qDebug() << "立即更新!";
        foreach (AppUpdateWid *wid, widgetList) {
            if(wid->updateAPPBtn->text() == tr("Update"))
                wid->updateAPPBtn->setEnabled(false);
        }
        checkUpdateBtn->start();
        checkUpdateBtn->setEnabled(false);
        versionInformationLab->setText(tr("Being updated..."));
//        versionInformationLab->setToolTip("");
        //         versionInformationLab->setText("正在更新...");
        updateMutual->isPointOutNotBackup = false;   //全部更新时不再弹出单个更新未备份提示
        emit updateAllSignal(false);
    }
    else if(ret == 1)
    {
        bacupInit(false);
        qDebug() << "不进行全部更新。";
        checkUpdateBtn->stop();
        checkUpdateBtn->setEnabled(true);
        //       checkUpdateBtn->setText(tr("全部更新"));
        versionInformationLab->setText(tr("Updatable app detected on your system!"));
//        versionInformationLab->setToolTip("");
        checkUpdateBtn->setText(tr("UpdateAll"));
        checkUpdateBtn->adjustSize();
        foreach (AppUpdateWid *wid, widgetList) {
//            wid->updateAPPBtn->show();
            wid->updateAPPBtn->setEnabled(true);
        }
        isAllUpgrade=false;
    }
}

void TabWid::backupCore()
{
    int initresult = emit needBackUp();
    switch (initresult) {
    case -1:
        backupMessageBox(tr("The backup restore partition could not be found. The system will not be backed up in this update!"));
        //如果是则立即更新,否的话取消全部更新
        return;
    case -2:
        versionInformationLab->setText(tr("Kylin backup restore tool is doing other operations, please update later."));
//        versionInformationLab->setToolTip("");
        bacupInit(false);
        isAllUpgrade=false;
        return;
    case -3:
        versionInformationLab->setText(tr("The source manager configuration file is abnormal, the system temporarily unable to update!"));
//        versionInformationLab->setToolTip("");
        bacupInit(false);
        isAllUpgrade=false;
        return;
    case -4:
        versionInformationLab->setText(tr("Backup already, no need to backup again."));
//        versionInformationLab->setToolTip("");
        checkUpdateBtn->start();
        checkUpdateBtn->setEnabled(false);
        updateMutual->isPointOutNotBackup = false;   //全部更新时不再弹出单个更新未备份提示
        emit updateAllSignal(false);
        bacupInit(false);
        return;
    case 1://正在备份
        emit startBackUp(0);
        versionInformationLab->setText(tr("Start backup,getting progress")+"...");
//        versionInformationLab->setToolTip("");
        foreach (AppUpdateWid *wid, widgetList) {
//            if(wid->updateAPPBtn->text() == tr("Update"))
                wid->updateAPPBtn->setEnabled(false);
        }
        break;
    case needBack://需要备份
        emit startBackUp(1);
        foreach (AppUpdateWid *wid, widgetList) {
//            if(wid->updateAPPBtn->text() == tr("Update"))
                wid->updateAPPBtn->setEnabled(false);
        }
        break;
    case -9://备份还原工具不存在
        backupMessageBox(tr("Kylin backup restore tool does not exist, this update will not backup the system!"));
        return;
    default:
        qDebug()<<"备份还原工具状态码"<<initresult;
        isAllUpgrade=false;
        bacupInit(false);
        return;
    }
    qDebug()<<"符合备份工具运行条件";
}

void TabWid::isCancelabled(bool status)
{
    if(status)
    {
        if(isAllUpgrade){
        checkUpdateBtn->setText(tr("Cancel"));
        checkUpdateBtn->setEnabled(true);
        }
//        foreach (AppUpdateWid *wid, widgetList) {
//            if(wid->updateAPPBtn->text() == tr("Cancel"))
//                wid->updateAPPBtn->setEnabled(true);
//        }
    }
    else{
        if(isAllUpgrade){
        checkUpdateBtn->setText(tr("Cancel"));
        checkUpdateBtn->setEnabled(false);
        }
//        foreach (AppUpdateWid *wid, widgetList) {
//            if(wid->updateAPPBtn->text() == tr("Cancel"))
//                wid->updateAPPBtn->setEnabled(false);
//        }
    }

}

void TabWid::DependResloveResult(bool ResloveStatus,bool ResloveResult,QStringList DeletePkgList, QStringList DeletePkgDeslist,QStringList DeletePkgReasonlist,QString Error,QString Reason)
{
    checkUpdateBtn->stop();
    checkUpdateBtn->setText(tr("Calculated"));

    if(!ResloveStatus)
    {
        qDebug()<<Error<<Reason;

        QMessageBox msgBox(qApp->activeModalWidget());
        msgBox.setText(tr("There are unresolved dependency conflicts in this update，Please select Dist-upgrade"));
        msgBox.setWindowTitle(tr("Prompt information"));
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.addButton(tr("Dist-upgrade"), QMessageBox::YesRole);
        msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);

        int ret = msgBox.exec();
        switch (ret) {
        case 0:
            qDebug() << "全盘修复。。。。。。";
//            distUpgradeAllApp(false);
            connect(updateMutual->interface,SIGNAL(DistupgradeDependResloveStatus(bool,bool,QStringList,QStringList,QString,QString)),this,SLOT(DistupgradeDependResloveResult(bool,bool,QStringList,QStringList,QString,QString)));
            updateMutual->DistUpgradeSystem(false);
            break;
        case 1:
            qDebug() << "取消更新";
            updatecancel();
            break;
        default:
            break;
        }

    }else{
        if (!ResloveResult)
        {
            emit updateAllSignal(true);
        }else{
            showDependSlovePtompt(UpdateAll,DeletePkgList,DeletePkgDeslist,DeletePkgReasonlist);
        }
    }
    disconnect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
}

void TabWid::DistupgradeDependResloveResult(bool ResloveStatus,bool ResloveResult,QStringList DeletePkgList, QStringList DeletePkgDeslist,QString Error,QString Reason)
{

    if(!ResloveStatus)
    {
        qDebug()<<Error<<Reason;
        //遇到无法修复的依赖
        QMessageBox msgBox(qApp->activeModalWidget());
        msgBox.setText(tr("There are unresolved dependency conflicts in this update，Please contact the administrator!"));
        msgBox.setWindowTitle(tr("Prompt information"));
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.addButton(tr("Sure"), QMessageBox::YesRole);

        int ret = msgBox.exec();
        switch (ret) {
        case 0:
            qDebug() << "取消更新";
            updatecancel();
            break;
        default:
            break;
        }
    }else{
        if (!ResloveResult)
        {
            foreach (AppUpdateWid *wid, widgetList) {
                wid->hide();
            }
            connect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
            connect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));

            updateMutual->DistUpgradeSystem(true);
//            distUpgradeAllApp(true);
        }else{
           QStringList DeletePkgReasonlist={""};
           showDependSlovePtompt(UpdateSystem,DeletePkgList,DeletePkgDeslist,DeletePkgReasonlist);
        }
    }
}

void TabWid::getAllProgress (QStringList pkgName,int Progress,QString status,QString detailinfo)
{
    if (!isAllUpgrade)
        return ;
    if(isContinueUpgrade)
    {
        checkUpdateBtn->show();
        isContinueUpgrade=false;
    }

    qDebug() << pkgName << Progress << status;
    versionInformationLab->setText(tr("Downloading and installing updates..."));
//    versionInformationLab->setToolTip("");
    updatedetaileInfo->show();


    if (Progress < 100)
    detailLabel->setText(detailinfo);
    allProgressBar->setValue(Progress);
    allProgressBar->show();
    lastRefreshTime->hide();
    if (Progress == 100) {

        disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));
    }
}

//当前下载速度单位换算
QString TabWid::modifySpeedUnit(long size, float time)
{
    if(size < 0)
        size = 0;
    size = size/time;
    if((size/1024) >= 1)
    {
        if((size/1024/1024) >= 1)
        {
            if((size/1024/1024/1024) >= 1)
                return QString("%1%2").arg((int)(size/1024/1024/1024)).arg("GB/S");
            else
                return QString("%1%2").arg((int)(size/1024/1024)).arg("MB/S");
        }
        else
            return QString("%1%2").arg((int)(size/1024)).arg("KB/S");
    }
    else
        return QString("%1%2").arg(size).arg("B/S");
}

//转换包大小的单位
QString TabWid::modifySizeUnit(long size)
{
    double tmpSize = size/1024.0;
    if(tmpSize >= 1)
    {
        if((tmpSize/1024.0) >= 1)
        {
            if((tmpSize/1024.0/1024.0) >= 1)
                return QString("%1%2").arg((int)(tmpSize/1024.0/1024.0*100 + 0.5)/100.0).arg("GB");//保留小数点后两位，并四舍五入
            else
                return QString("%1%2").arg((int)(tmpSize/1024.0*100 + 0.5)/100.0).arg("MB");//保留小数点后两位，并四舍五入
        }
        else
            return QString("%1%2").arg((int)(tmpSize*100 + 0.5)/100.0).arg("KB");//保留小数点后两位，并四舍五入
    }
    else
        return QString("%1%2").arg(size).arg("B");
}

void TabWid::showDownloadInfo(QStringList pkgname,int currentserial,int total,uint downloadedsize,uint totalsize,int speed)
{
    Q_UNUSED(pkgname);
    if(speed > 0)
        alldownloadstart=true;
    QString downloadspeed = modifySpeedUnit(speed, 1);
    if(currentserial==total)
    {
        progressLabel->setText(tr("In the install..."));
        disconnect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
        return;
    }
    if(speed == 0 && !alldownloadstart)
    {
        progressLabel->setText(tr("In the download")+":"+modifySizeUnit(downloadedsize)+"/"+modifySizeUnit(totalsize)+"("+tr("calculating")+")");
        return;
    }
    progressLabel->setText(tr("In the download")+":"+modifySizeUnit(downloadedsize)+"/"+modifySizeUnit(totalsize)+"("+downloadspeed+")");


}
void TabWid::backupProgress(int progress)
{
    qDebug() <<"备份进度：---->" << progress;
    if(progress==100)
    {
        bacupInit(false);
        //备份完成，开始安装
        qDebug()<<"备份完成，开始安装";
        versionInformationLab->setText(tr("Backup complete."));
//        versionInformationLab->setToolTip("");
        checkUpdateBtn->show();
        allProgressBar->hide();
        lastRefreshTime->show();
        return;
    }
    versionInformationLab->setText(tr("System is backing up..."));
//    versionInformationLab->setToolTip("");
    allProgressBar->setValue(progress);
    checkUpdateBtn->hide();
    allProgressBar->show();
    lastRefreshTime->hide();
    foreach (AppUpdateWid *wid, widgetList) {
//            if(wid->updateAPPBtn->text() == tr("Update"))
            wid->updateAPPBtn->setEnabled(false);
    }
}

void TabWid::bakeupFinish(int result)
{
    switch (result) {
    case -20:
        //        versionInformationLab->setText(tr("备份过程被中断，停止更新！"));
        versionInformationLab->setText(tr("Backup interrupted, stop updating!"));
//        versionInformationLab->setToolTip("");
        foreach (AppUpdateWid *wid, widgetList) {
//            if(wid->updateAPPBtn->text() == tr("Update"))
                wid->updateAPPBtn->setEnabled(true);
        }
        allProgressBar->hide();
        checkUpdateBtn->show();
        lastRefreshTime->show();
        break;
    case 99:
        //        versionInformationLab->setText(tr("备份完成！"));
        versionInformationLab->setText(tr("Backup finished!"));
//        versionInformationLab->setToolTip("");
        allProgressBar->hide();
        checkUpdateBtn->show();
        break;
    default:
        //        backupMessageBox(tr("麒麟备份还原工具异常：")+QString::number(result)+","+tr("本次更新不会备份系统！"));
        backupMessageBox(tr("Kylin backup restore tool exception:")+QString::number(result)+","+tr("There will be no backup in this update!"));
        break;
    }
}

void TabWid::backupHideUpdateBtn(int result)
{
    if(result == 99)
    {
        checkUpdateBtn->start();
        //        versionInformationLab->setText("正在更新...");
        versionInformationLab->setText(tr("Being updated..."));
//        versionInformationLab->setToolTip("");
        updateMutual->isPointOutNotBackup = false;   //全部更新时不再弹出单个更新未备份提示
        emit updateAllSignal(false);
        foreach (AppUpdateWid *wid, widgetList) {
            if(wid->updateAPPBtn->text() == tr("Cancel"))
                wid->updateAPPBtn->hide();
        }
    }
    else if(result == -20)
    {
        checkUpdateBtn->stop();
        checkUpdateBtn->setEnabled(true);
        //        checkUpdateBtn->setText(tr("全部更新"));
        checkUpdateBtn->setText(tr("UpdateAll"));
        checkUpdateBtn->adjustSize();

    }else{
        versionInformationLab->setText(tr("The status of backup completion is abnormal"));
//        versionInformationLab->setToolTip("");
    }


}

void TabWid::bacupInit(bool isConnect)
{
    qDebug() << "======>tabwid info: " <<backupThread ;
    connect(this,&TabWid::needBackUp,backup,&BackUp::needBacdUp,Qt::BlockingQueuedConnection);//同步信号，阻塞，取返回值
    if (isConnect) {
        qDebug() << "is connect ;;";
        connect(this,&TabWid::startBackUp,backup,&BackUp::startBackUp);
        connect(backup, &BackUp::calCapacity, this, &TabWid::whenStateIsDuing);
        connect(backup, &BackUp::backupStartRestult, this, &TabWid::receiveBackupStartResult);
        connect(backup,&BackUp::bakeupFinish,this,&TabWid::bakeupFinish);
        connect(backup,&BackUp::backupProgress,this,&TabWid::backupProgress);
        connect(backup,&BackUp::bakeupFinish,this,&TabWid::backupHideUpdateBtn);
    } else {
        qDebug() << "is disconnect;;";
        disconnect(this,&TabWid::startBackUp,backup,&BackUp::startBackUp);
        disconnect(backup, &BackUp::calCapacity, this, &TabWid::whenStateIsDuing);
        disconnect(backup, &BackUp::backupStartRestult, this, &TabWid::receiveBackupStartResult);
        disconnect(backup,&BackUp::bakeupFinish,this,&TabWid::bakeupFinish);
        disconnect(backup,&BackUp::backupProgress,this,&TabWid::backupProgress);
        disconnect(backup,&BackUp::bakeupFinish,this,&TabWid::backupHideUpdateBtn);
    }

}

void TabWid::backupDelete()
{
    backup->deleteLater();
    backupThread->deleteLater();
}

void TabWid::slotUpdateTemplate(QString status)
{
    qDebug() <<  "源管理器：" <<"update template status :" << status;

}

void TabWid::getpkginfoFromJson(AppAllMsg *msg,QString pkgname)
{
    /*判断json文件是否存在*/
    QString filename = QString(JSON_FILE_PATH) +pkgname +".json";
    qDebug() << filename;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)){
        qDebug() << "JSON file open failed! ";
        return ;
    }
    QByteArray jsonData = file.readAll();

    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(jsonData, &err_rpt); // 字符串格式化为JSON

    if (!root_Doc.isNull() && (err_rpt.error == QJsonParseError::NoError)) {  // 解析未发生错误
        if (root_Doc.isObject()) { // JSON 文档为对象
            QJsonObject object = root_Doc.object();  // 转化为对象
            if (QLocale::system().name() == "zh_CN"){
                QString name  = object.value("name").toObject().value("zh_CN").toString();
                QString description  = object.value("description").toObject().value("zh_CN").toString();
                if (!name.isNull()) {
                    msg->name=name;
                }
                if (!description.isNull()) {
                    msg->shortDescription=description;
                }
            }else {
                QString name  = object.value("name").toObject().value("en_US").toString();
                QString description  = object.value("description").toObject().value("en_US").toString();
                if (!name.isNull()) {
                    msg->name=name;
                }
                if (!description.isNull()) {
                    msg->shortDescription=description;
                }
            }
            QString upgrade_download_size_str  = object.value("upgrade_list").toObject().value("total_download_size").toString();
            long upgrade_download_size  = upgrade_download_size_str.toLong();
            QString install_download_size_str  = object.value("install_list").toObject().value("total_download_size").toString();
            long install_download_size  = install_download_size_str.toLong();
            msg->msg.allSize = upgrade_download_size + install_download_size;
            QString upgrade_install_size_str  = object.value("upgrade_list").toObject().value("total_install_size").toString();
            long upgrade_install_size  = upgrade_install_size_str.toLong();
            QString install_install_size_str  = object.value("install_list").toObject().value("total_install_size").toString();
            long install_install_size = install_install_size_str.toLong();
            msg->msg.installSize = upgrade_install_size + install_install_size;

            QString new_version = object.value("new_version").toString();
            if (!new_version.isNull())
                msg->availableVersion=new_version;
            QString cur_version = object.value("cur_version").toString();
            if (!cur_version.isNull())
                msg->version=cur_version;
            QString changelog = object.value("changelog").toString();
            if (!changelog.isNull())
                msg->longDescription=changelog;
        }
    }else{
        qDebug() << "JSON文件格式错误！";
        return ;
    }
    file.close();
}

void TabWid::slotUpdateCache(bool result,QStringList pkgname, QString error ,QString reason)
{
        AppAllMsg msg;
        isConnectSourceSignal = true;
        int size=0;
        qDebug() <<"源管理器：" << "update cache status :" << result;
        QDir dir("/tmp/upgrade");
        if(!dir.exists("/tmp/upgrade/"))
        {
            dir.mkdir("/tmp/upgrade/");
        }
        if (result) {
//            int progress = 100;
//            versionInformationLab->setText(tr("Update software source :") + QString::number(progress)+"%");
            versionInformationLab->setText(tr("Getting update list")+"...");
//            versionInformationLab->setToolTip("");
            QList<QString>::Iterator it = pkgname.begin(),itend = pkgname.end();
            int i = 0;
            for (;it != itend; it++,i++){
                qDebug() << "slotUpdateCache函数：获取到的包列表：" << pkgname[i];
                QFile file("/tmp/upgrade/upgrade.list");
                file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
                QString package=pkgname[i]+" ";
                file.write(package.toUtf8());
                file.close();
                getpkginfoFromJson(&msg,pkgname[i]);
                msg.name=pkgname[i];
                loadingOneUpdateMsgSlot(msg);
                size++;
            }
            loadingFinishedSlot(size);
        }
        else
        {
            if(pkgname[0]=="kylin-system-updater")
            {
                checkUpdateBtn->hide();
                versionInformationLab->setText(tr("Software source update successed: ")+error);
            }
            else
            {
                qDebug() << "更新失败 ";
                checkUpdateBtn->setEnabled(true);
                checkUpdateBtn->stop();
                //                checkUpdateBtn->setText(tr("检查更新"));
                checkUpdateBtn->setText(tr("Check Update"));
                //                versionInformationLab->setText(tr("软件源更新失败：")+failedInfo );
                versionInformationLab->setText(tr("Software source update failed: ")+error);
/*自己写的悬浮窗*/
//                QString failinfo = "";
//                failinfo+= tr("Software source update failed: ")+error;

//                QFontMetrics failinfoFontMetrics(versionInformationLab->font());
//                int failinfoSize = failinfoFontMetrics.width(failinfo);
//                int failinfoWidth = versionInformationLab->width();
//                QString nameFormatBody = failinfo;
//                if (failinfoSize > (failinfoWidth )) {//- 10
//                    nameFormatBody = failinfoFontMetrics.elidedText(nameFormatBody , Qt::ElideRight , failinfoWidth );//- 70
//                    versionInformationLab->setText(nameFormatBody);
////                    this->nameTipStatus = true;
//                    versionInformationLab->setToolTip(failinfo);
//                } else {
////                    this->nameTipStatus = false;
//                    versionInformationLab->setText(nameFormatBody);
//                }
            }
            disconnect(updateSource->serviceInterface,SIGNAL(updateTemplateStatus(QString)),this,SLOT(slotUpdateTemplate(QString)));
            disconnect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
            disconnect(updateMutual->interface,SIGNAL(UpdateDetectStatusChanged(int,QString)),this,SLOT(slotUpdateCacheProgress(int,QString)));

        }  
}
void TabWid::slotUpdateCacheProgress(int progress, QString status)
{
    isConnectSourceSignal = true;

    if (progress > 100) {
//        progress = 100;
        return;
    }

    if  (lastprogress > progress)
        return;

    lastprogress = progress;

    if(lastprogress==92)
        lastprogress=0;

    qDebug() << "update cache progress :" << progress;

    versionInformationLab->setText(tr("Update software source :") + QString::number(progress)+"%");
//    versionInformationLab->setToolTip("");
    //versionInformationLab->setText(tr("更新软件源进度：") + QString::number(progress)+"%");

}

void TabWid::allComponents()
{
    /*工行项目新增*/
    mainTabLayout = new QVBoxLayout();  //整个页面的主布局
    scrollArea = new QScrollArea(this);
    updateTab = new QWidget(this);  //更新页面

    AppMessage = new QVBoxLayout();
    AppMessage->setAlignment(Qt::AlignTop);
    updateTab->setLayout(AppMessage);
    systemWidget = new QFrame(updateTab);
    systemWidget->setFrameShape(QFrame::Box);
    systemWidget->setMinimumSize(QSize(550,0));
    systemWidget->setMaximumSize(QSize(16777215, 16777215));
    systemPortraitLab = new QLabel(systemWidget);

    allUpdateWid = new QWidget(this);
    allUpdateLayout = new QVBoxLayout();
    allUpdateLayout->setAlignment(Qt::AlignTop);
    allUpdateWid->setLayout(allUpdateLayout);
    allUpdateLayout->setSpacing(2);
    allUpdateLayout->setMargin(0);

    /*update*/
    labUpdate = new TitleLabel(this);
    //    labUpdate->setText(tr("更新"));
    labUpdate->setText(tr("Update"));
    labUpdate->adjustSize();
    labUpdate->setFixedHeight(27);

    scrollArea->setWidget(updateTab);
    scrollArea->setFrameStyle(0);
    scrollArea->setWidgetResizable(true);

    systemWidget->resize(560,140);
    systemWidget->setFixedHeight(200);

    systemPortraitLab = new QLabel();
    tab1HLayout = new QHBoxLayout();
    systemWidget->setLayout(tab1HLayout);
    systemPortraitLab->setFixedSize(96,96);
    systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/normal.png").scaled(QSize(96,96)));

    QWidget *historyUpdateLogWid = new QWidget(this);
    QHBoxLayout *historyUpdateLogLayout = new QHBoxLayout();
    historyUpdateLog = new QPushButton(this); // 历史日志弹出窗口控制按钮
    //    historyUpdateLog->setText(tr("查看更新历史"));
    historyUpdateLog->setText(tr("View history"));
    historyUpdateLog->setFixedHeight(36);
    historyUpdateLogWid->setLayout(historyUpdateLogLayout);
    historyUpdateLogLayout->setAlignment(Qt::AlignLeft);
    historyUpdateLogLayout->addWidget(historyUpdateLog);
    historyUpdateLogLayout->setSpacing(0);
    historyUpdateLogLayout->setMargin(0);


    inforLayout = new QVBoxLayout();
    lastRefreshTime = new QLabel(systemWidget);
    allProgressBar = new QProgressBar(systemWidget);
    allProgressBar->setFixedHeight(4);
    allProgressBar->hide();
    allProgressBar->setValue(0);
    allProgressBar->setTextVisible(false);

    progressLabel = new QLabel(systemWidget);
    //    progressLabel->setText(tr("45M/300M"));
    //progressLabel->setMinimumWidth(250);
    progressLabel->adjustSize();
    progressLabel->hide();

    detailLabel = new QLabel(systemWidget);
    detailLabel->adjustSize();
    detailLabel->hide();

    versionInformationLab = new FixLabel();
    inforLayout->setAlignment(Qt::AlignVCenter);
    inforLayout->addWidget(versionInformationLab);
    inforLayout->addSpacing(4);
    inforLayout->addWidget(lastRefreshTime);
    inforLayout->addSpacing(4);
    inforLayout->addWidget(allProgressBar);
    inforLayout->addSpacing(4);
    inforLayout->addWidget(progressLabel);
    inforLayout->addSpacing(4);
    inforLayout->addWidget(detailLabel);
    inforLayout->addSpacing(4);
    inforLayout->setContentsMargins(8,20,0,0);
    QWidget *inforWidget = new QWidget();
    inforWidget->setLayout(inforLayout);

    controlLayout = new QVBoxLayout();

    checkUpdateBtn = new m_button(systemWidget);
    checkUpdateBtn->setDefault(true);
    checkUpdateBtn->start();
    checkUpdateBtn->adjustSize();

    updatedetaileInfo = new QPushButton(this);
    updatedetaileInfo->setText(tr("details"));
    updatedetaileInfo->hide();
    updatedetaileInfo->setFixedSize(120,36);

    controlLayout->addWidget(checkUpdateBtn);
    controlLayout->addSpacing(8);
    controlLayout->addWidget(updatedetaileInfo);

    QWidget *controlWidget = new QWidget(systemWidget);
    controlWidget->setLayout(controlLayout);

    tab1HLayout->addWidget(systemPortraitLab,0,Qt::AlignLeft);
    tab1HLayout->addWidget(inforWidget,Qt::AlignLeft|Qt::AlignTop);
    tab1HLayout->addSpacerItem(new QSpacerItem(50, 2, QSizePolicy::Fixed));
    tab1HLayout->addWidget(controlWidget);
    tab1HLayout->setContentsMargins(8,25,20,20);
    tab1HLayout->setSpacing(0);



    /*******Update Settings*******/
    updateSettingLab = new TitleLabel();
    //~ contents_path /upgrade/Update Settings
    updateSettingLab->setText(tr("Update Settings"));
    updateSettingLab->setFixedHeight(29);
    /*the whole update setting layout*/
    updateSettingWidget = new QFrame(this);
    updateSettingWidget->setFrameShape(QFrame::Box);
    updatesettingLayout = new QVBoxLayout();
    updateSettingWidget->setLayout(updatesettingLayout);

    /*switchbutton：下载限速*/
    DownloadVWidget = new QFrame();
    DownloadVWidget->setFrameShape(QFrame::Box);
    DownloadVLayout = new QVBoxLayout();
    DownloadHLayout = new QHBoxLayout();
    DownloadVLabLayout = new QHBoxLayout();
    DownloadHLab = new QLabel();
    DownloadHLab->setText(tr("Download Limit(Kb/s)"));
    DownloadHBtn = new SwitchButton();
    DownloadHValue = new QComboBox();
    QStringList strList;
    strList<<"50"<<"100"<<"200"<<"300"<<"600";
    DownloadHValue->addItems(strList);
    DownloadVLab = new FixLabel();
    DownloadVLab->setText(tr("It will be avaliable in the next download."));
    DownloadVLab->setWordWrap(true);
    DownloadVLab->setAlignment(Qt::AlignTop);
    DownloadHLayout->addSpacing(6);
    DownloadHLayout->addWidget(DownloadHLab);
    DownloadHLayout->addWidget(DownloadHValue);
    DownloadHLayout->addWidget(DownloadHBtn);
    DownloadVLayout->addLayout(DownloadHLayout);
    DownloadVLayout->addLayout(DownloadVLabLayout);
    DownloadVLabLayout->addSpacing(6);
    DownloadVLabLayout->addWidget(DownloadVLab);
    DownloadVWidget->setLayout(DownloadVLayout);

    /*第二个titlelabel的布局*/
    updatesettingLayout->setAlignment(Qt::AlignTop);
    updatesettingLayout->addWidget(updateSettingLab);
//    updatesettingLayout->addSpacing(10);

//    updatesettingLayout->addWidget(isAutoUpgradeWidget);

    updatesettingLayout->addWidget(DownloadVWidget);
    updatesettingLayout->setSpacing(2);
    updatesettingLayout->setMargin(0);

    AppMessage->addWidget(labUpdate);
    AppMessage->addWidget(systemWidget);
    AppMessage->addWidget(allUpdateWid);
    AppMessage->addSpacing(30);
    AppMessage->addWidget(updateSettingLab);
    AppMessage->addWidget(updateSettingWidget);
    AppMessage->addSpacing(2);
    AppMessage->addWidget(historyUpdateLogWid);
    AppMessage->addStretch();
    AppMessage->setContentsMargins(0,0,32,10);

    mainTabLayout->setAlignment(Qt::AlignTop);
    mainTabLayout->addWidget(scrollArea);
    mainTabLayout->setMargin(0);
    this->setLayout(mainTabLayout);
    getAllDisplayInformation();

}

void TabWid::loadingOneUpdateMsgSlot(AppAllMsg msg)
{
    //    checkUpdateBtn->setText();
    if(updateMutual->importantList.indexOf(msg.name) == -1)
    {
        struct pkgProgress pkg;
        pkg.name = msg.name;
        pkgList << pkg;
        updateMutual->importantList.append(msg.name);   //重要更新列表中添加appname
        AppUpdateWid *appWidget = new AppUpdateWid(msg, this);
        widgetList << appWidget;
        connect(appWidget, &AppUpdateWid::allappupdatefinishsignal, this, &TabWid::allappupdatefinishSlot);
        connect(appWidget, &AppUpdateWid::changeupdateall, this, &TabWid::checkUpdateBtnClicked);
        connect(appWidget, &AppUpdateWid::appupdateiscancel, this, &TabWid::slotCancelDownload);
        connect(appWidget,&AppUpdateWid::oneappUpdateResultSignal,this,&TabWid::oneappUpdateresultSlot);
        connect(appWidget,&AppUpdateWid::changeUpdateAllSignal,this,&TabWid::changeUpdateAllSlot);
        connect(updateMutual,&UpdateDbus::sendFinishGetMsgSignal,appWidget,&AppUpdateWid::showUpdateBtn);
        connect(appWidget,&AppUpdateWid::filelockedSignal,this,&TabWid::waitCrucialInstalled);
//        connect(backup,&BackUp::bakeupFinish,appWidget,&AppUpdateWid::hideOrShowUpdateBtnSlot);
        connect(appWidget,&AppUpdateWid::startoneappupdate,this,&TabWid::startoneappupdateslot);
        /*判断是否是后台自动更新*/
        if (isAutoUpgrade) {
           // connect(appWidget, &AppUpdateWid::sendProgress, this, &TabWid::getAllProgress);
            appWidget->isUpdateAll = true;
            appWidget->isAutoUpgrade = true;
            appWidget->updateAPPBtn->hide();
            appWidget->appVersion->setText(tr("Ready to install"));
        }
        if(ukscConnect->isConnectUskc == true)
        {
            QStringList list = ukscConnect->getInfoByName(msg.name);
            if(list[2] != "")
            {
                appWidget->appNameLab->setText(list[2]);
                appWidget->dispalyName = list[2];

            }
            if(list[1] != "" && QLocale::system().name()=="zh_CN")
            {
                appWidget->appNameLab->setText(list[1]);
                appWidget->dispalyName = list[1];
            }
            if(list[0] != "" && !appWidget->haveThemeIcon)
            {
                appWidget->appIcon->setPixmap(QPixmap(list[0]).scaled(32, 32));
            }
        }
        allUpdateLayout->addWidget(appWidget);
        qDebug() << "更新管理器：" << "loadingOneUpdateMsgSlot:" << appWidget->dispalyName;
    }

}

void TabWid::loadingFinishedSlot(int size)
{
    if (isAutoUpgrade)
        bacupInit(true);
    else
        bacupInit(false);
    disconnect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
    disconnect(updateMutual->interface,SIGNAL(UpdateDetectStatusChanged(int,QString)),this,SLOT(slotUpdateCacheProgress(int,QString)));
    if(updateMutual->importantList.size() == 0) {
//        updateMutual->fileUnLock();
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->stop();
        //        checkUpdateBtn->setText(tr("检查更新"));
        checkUpdateBtn->setText(tr("Check Update"));
        //        versionInformationLab->setText(tr("您的系统已是最新！"));
        versionInformationLab->setText(tr("Your system is the latest!"));
        disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
        allProgressBar->hide();
        QString updatetime = tr("No Information!");
        QSqlQuery queryInstall(QSqlDatabase::database("A"));
        queryInstall.exec("select * from updateinfos order by id desc");
        while (queryInstall.next()) {
            QString statusType = queryInstall.value("keyword").toString();
            if (statusType == "" || statusType =="1") {
                updatetime = queryInstall.value("date").toString();
                break;
            }
        }
        lastRefreshTime->setText(tr("Last refresh:")+ updatetime);
        lastRefreshTime->show();
        allProgressBar->hide();
    }
    else {
        updateMutual->importantSize = updateMutual->importantList.size();   //此次检测结果的更新数量
        checkUpdateBtn->stop();
        checkUpdateBtn->setEnabled(true);
        //        checkUpdateBtn->setText(tr("全部更新"));
        checkUpdateBtn->setText(tr("UpdateAll"));
        checkUpdateBtn->adjustSize();
        //        versionInformationLab->setText(tr("检测到你的系统有可更新的应用！"));
        if (!isAutoUpgrade) {
            versionInformationLab->setText(tr("Updatable app detected on your system!"));
        }

        systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/update.png").scaled(96,96));
    }

}

void TabWid::allappupdatefinishSlot()
{
//    updateMutual->fileUnLock();
    checkUpdateBtn->setEnabled(true);
    checkUpdateBtn->stop();
    //        checkUpdateBtn->setText(tr("检查更新"));
    checkUpdateBtn->setText(tr("Check Update"));
    //        versionInformationLab->setText(tr("您的系统已是最新！"));
    versionInformationLab->setText(tr("Your system is the latest!"));
    disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
    allProgressBar->hide();
    QString updatetime = tr("No Information!");
    QSqlQuery queryInstall(QSqlDatabase::database("A"));
    queryInstall.exec("select * from updateinfos order by id desc");
    while (queryInstall.next()) {
        QString statusType = queryInstall.value("keyword").toString();
        if (statusType == "" || statusType =="1") {
            updatetime = queryInstall.value("date").toString();
            break;
        }
    }
    lastRefreshTime->setText(tr("Last refresh:")+ updatetime);
    lastRefreshTime->show();
    allProgressBar->hide();
}

void TabWid::getAllDisplayInformation()
{
    QSqlQuery query(QSqlDatabase::database("A"));
    QString updatetime;
    QString checkedtime;
    QString checkedstatues;
    QString backupStatus;
    QString downloadlimitstatus;
    QString downloadlimitvalue;
    QString allowshutdownupgradestatus;
    QString time;
    query.exec("select * from display");
    while(query.next())
    {
        checkedtime = query.value("check_time").toString();
        checkedstatues = query.value("auto_check").toString();
        backupStatus = query.value("auto_backup").toString();
        downloadlimitstatus = query.value("download_limit").toString();
        downloadlimitvalue = query.value("download_limit_value").toString();
//        allowshutdownupgradestatus = query.value("allow_unattended_upgrades_shutdown").toString();
//        time = query.value("update_period").toString();
    }
    qDebug()<<"downloadlimitstatus:"<<downloadlimitstatus;
    QSqlQuery queryInstall(QSqlDatabase::database("A"));
    updatetime = tr("No Information!");
    queryInstall.exec("select * from updateinfos order by id desc");
    while(queryInstall.next())
    {
        QString statusType = queryInstall.value("keyword").toString();
        if(statusType == "" || statusType =="1") {
            updatetime = queryInstall.value("date").toString();
            break;
        }
    }
    lastRefreshTime->setText(tr("Last refresh:") + updatetime);
    versionInformationLab->setText(tr("Last Checked:")+checkedtime);

    if(downloadlimitstatus == "false")
    {
        DownloadHBtn->setChecked(false);
        DownloadHValue->setCurrentText(downloadlimitvalue);
        DownloadHValue->setEnabled(false);
    }
    else
    {
        DownloadHBtn->setChecked(true);
        DownloadHValue->setEnabled(true);
        DownloadHValue->setCurrentText(downloadlimitvalue);
    }
//    if(allowshutdownupgradestatus == "false")
//    {
//        ontimeUpgradeBtn->setChecked(true);
//        ontimeUpgradeValue->setEnabled(true);
//        ontimeUpgradeValue->setCurrentText(time);
//    }
//    else
//    {
//        poweroffUpgradeBtn->setChecked(true);
//        ontimeUpgradeValue->setEnabled(false);
//    }
}
void TabWid::showHistoryWidget()
{

    historyLog = m_updatelog::GetInstance(this);
    //在屏幕中央显示
    //QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    //historyLog->move((availableGeometry.width()-historyLog->width())/2,(availableGeometry.height()- historyLog->height())/2);
    historyLog->show();
}

void TabWid::showDependSlovePtompt(int updatemode,QStringList pkgname,QStringList description,QStringList deletereason)
{
    dependsloveptompt = updatedeleteprompt::GetInstance(this);
    //在屏幕中央显示
    //QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    //historyLog->move((availableGeometry.width()-historyLog->width())/2,(availableGeometry.height()- historyLog->height())/2);
    connect(dependsloveptompt,&updatedeleteprompt::updatedependsolvecancelsignal,this,&TabWid::updatecancel);
    connect(dependsloveptompt,&updatedeleteprompt::updatealldependsolveacceptsignal,this,&TabWid::updateAllApp);
    connect(dependsloveptompt,&updatedeleteprompt::disupdatedependsolveacceptsignal,this,&TabWid::disupdateallaccept);
    int pkgnum = dependsloveptompt->updatedeletepkglist(pkgname,description,deletereason);//更新列表
    QString pkgnamestr = QString::number(pkgnum);
    if(updatemode==UpdateAll){
        dependsloveptompt->updatemode=UpdateAll;
    }else if(updatemode==UpdatePart){
        dependsloveptompt->updatemode=UpdatePart;
    }else if(updatemode==UpdateSystem){
        dependsloveptompt->updatemode=UpdateSystem;
        dependsloveptompt->conflictpromptTab->setText(tr("Dependency conflict exists in this update,need to be completely repaired!"));
    }
    dependsloveptompt->uninstallinfoTab->setText(pkgnamestr +" "+ tr("packages are going to be removed,Please confirm whether to accept!"));
    dependsloveptompt->show();
}

void TabWid::slotReconnTimes(int times) {
    qDebug() << "更新模板失败 ， 重新连接 " << times << "次";
    versionInformationLab->setText(tr("trying to reconnect ") + QString::number(times) + tr(" times"));
}

void TabWid::showDetails()
{
    if(detailLabel->isHidden())
    {
        detailLabel->show();
        progressLabel->show();
        //        updatedetaileInfo->setText(tr("收起"));
        updatedetaileInfo->setText(tr("back"));
    }
    else
    {
        detailLabel->hide();
        progressLabel->hide();
        //        updatedetaileInfo->setText(tr("详情"));
        updatedetaileInfo->setText(tr("details"));
    }
}

void TabWid::checkUpdateBtnClicked()
{
    updateMutual = UpdateDbus::getInstance();

    if(checkUpdateBtn->text() == tr("Cancel"))
    {
        updateMutual->interface->call("CancelDownload");
        isCancel=true;
        return;
    }

    /*加入自动更新功能安装时在systemwidget中的显示*/
    QFile file("/var/lib/unattended-upgrades/unattended-upgrade.conf");
    qDebug()<<"进入button click";
    if(file.exists())
    {
        qDebug()<<"标志位文件存在";
        QSettings settings("/var/lib/unattended-upgrades/unattended-upgrade.conf",QSettings::IniFormat);
        QString ret = settings.value("UNATTENDED_UPGRADE/autoupdate_run_status").toString();

        if (!ret.compare("backup"))
        {
//            versionInformationLab->setText("后台自动更新进程正在备份中......");
            versionInformationLab->setText(tr("Auto-Update is backing up......"));
            checkUpdateBtn->hide();
            QDBusConnection::systemBus().connect(QString(),QString("/"),"com.kylin.install.notification","InstallStart",this,SLOT(autoupgradestatusshow()));
            QDBusConnection::systemBus().connect(QString(),QString("/"),"com.kylin.install.notification","BackupFailure",this,SLOT(backupfailure()));
            return;
        }
        if (!ret.compare("install"))
        {
            qDebug()<<"开始安装";
            autoupgradestatusshow();
            return;
        }
        if (!ret.compare("download"))/*需要杀掉程序*/
        {
            /*如果自动更新在下载中，调用dbus去kill掉下载程序，继续原流程，不进行多余操作*/
            QString pid;
            QFile pidfile("/var/run/unattended-upgrades.pid");
            if (pidfile.exists())
            {
                pidfile.open(QIODevice::ReadOnly);//| QIODevice::Text | QIODevice::Append
                pid = pidfile.readAll();
                pidfile.close();
                if(pid == nullptr)
                {
                    return;
                }
//                pidfile.close();
                updateSource->killProcessSignal(pid.toInt(), 10);
            }

            checkUpdateBtn->setEnabled(true);
            checkUpdateBtn->setText(tr("Check Update"));
        }
        if (!ret.compare("preinstall"))/*需要杀掉程序*/
        {
            /*如果自动更新在下载中，调用dbus去kill掉下载程序，继续原流程，不进行多余操作*/
            QString pid;
            QFile pidfile("/var/run/unattended-upgrades.pid");
            if (pidfile.exists())
            {
                pidfile.open(QIODevice::ReadOnly);//| QIODevice::Text | QIODevice::Append
                pid = pidfile.readAll();
                pidfile.close();
                if(pid == nullptr)
                {
                    return;
                }
//                pidfile.close();
                updateSource->killProcessSignal(pid.toInt(), 10);
            }

            checkUpdateBtn->setEnabled(true);
            checkUpdateBtn->setText(tr("Check Update"));
        }
        if (!ret.compare("idle")) {
            /*如果没有进行自动更新，那就不需要操作 */
//            checkUpdateBtn->setEnabled(true);
//            checkUpdateBtn->setText(tr("Check Update"));
        }
    }
    QDBusReply<qint32> obj_reply = updateMutual->interface->call("GetBackendStatus",getLanguageEnv());
    int backendStatus = obj_reply.value();
    switch(backendStatus)
    {
        case ACTION_DEFUALT_STATUS:
        break;
        case ACTION_UPDATE:
            versionInformationLab->setText(tr("The progress is downloading..."));
            connect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDetectStatusChanged(int,QString)),this,SLOT(slotUpdateCacheProgress(int,QString)));
            return;
        break;
        case ACTION_INSTALL:
            isAllUpgrade = true;
            isContinueUpgrade = true;
            versionInformationLab->setText(tr("The progress is installing..."));
            checkUpdateBtn->hide();
            connect(updatedetaileInfo,&QPushButton::clicked,this,&TabWid::showDetails);
            connect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));
            connect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
            return;
        break;
        default:
        break;
    }
    if(checkUpdateBtn->text() == tr("Check Update"))
    {
        //progressLabel->hide();
        widgetList.clear();

        connect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
        connect(updateMutual->interface,SIGNAL(UpdateDetectStatusChanged(int,QString)),this,SLOT(slotUpdateCacheProgress(int,QString)));
        updateMutual->failedList.clear();
        QList<AppUpdateWid*> list = this->findChildren<AppUpdateWid*>();
        for(AppUpdateWid* tmp:list)        {
            tmp->deleteLater();
        }
        updateMutual->callDBusUpdateSource();
        versionInformationLab->setText(tr("Updating the software source")+"...");
        checkUpdateBtn->start();
        checkUpdateBtn->setEnabled(false);
    }
    else if(checkUpdateBtn->text() == tr("UpdateAll"))
    {
        if (!get_battery()) {
            QMessageBox msgBox;
            msgBox.setText(tr("The battery is below 50% and the update cannot be downloaded"));
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setButtonText(QMessageBox::Ok,tr("OK"));
            msgBox.exec();
            return ;
        }


        QMessageBox msgBox(qApp->activeModalWidget());
        msgBox.setText(tr("Please back up the system before all updates to avoid unnecessary losses"));
        msgBox.setWindowTitle(tr("Prompt information"));
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.addButton(tr("Only Update"), QMessageBox::YesRole);
        msgBox.addButton(tr("Back And Update"), QMessageBox::AcceptRole);
        msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);

        connect(updatedetaileInfo,&QPushButton::clicked,this,&TabWid::showDetails);
        connect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));
        isAllUpgrade = true;
        int ret = msgBox.exec();
        switch (ret) {
        case 0:
            qDebug() << "全部更新。。。。。。";
            foreach (AppUpdateWid *wid, widgetList) {
//                if(wid->updateAPPBtn->text() == tr("Update"))
                    wid->updateAPPBtn->setEnabled(false);
            }
            checkUpdateBtn->setEnabled(false);
            checkUpdateBtn->start();
            updateMutual->isPointOutNotBackup = false;   //全部更新时不再弹出单个更新未备份提示
            emit updateAllSignal(false);
            break;
        case 1:
            bacupInit(true);
            backupCore();
            qDebug() << "否，立即备份";
            break;
        case 2:
//            foreach (AppUpdateWid *wid, widgetList) {
//                //disconnect(wid, &AppUpdateWid::sendProgress, this, &TabWid::getAllProgress);
//                wid->updateAPPBtn->show();
//            }
            disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
            isAllUpgrade = false;
            qDebug() << "Close 暂不更新!";
            break;
        default:
//            foreach (AppUpdateWid *wid, widgetList) {
//                wid->updateAPPBtn->show();
//            }
            disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
            isAllUpgrade = false;
            qDebug() << "Close 暂不更新!";
            break;
        }
    }

}

void TabWid::DownloadLimitValueChanged(const QString &value)
{
    if(DownloadHBtn->isChecked()==false)
    {
        updateMutual->SetDownloadLimit(0,false);
        updateMutual->insertInstallStates("download_limit_value","0");
    }
    else if (DownloadHBtn->isChecked()==true)
    {
        //int dlimit = DownloadLimitValue->value();
        updateMutual->SetDownloadLimit(value,true);
        updateMutual->insertInstallStates("download_limit_value",value);
    }
    else
    {
        qDebug()<<"Download Limit Changed";
        updateMutual->SetDownloadLimit(0,false);
        updateMutual->insertInstallStates("download_limit_value","0");
    }
}


void TabWid::DownloadLimitChanged()
{
    if(DownloadHBtn->isChecked() == false)
    {
        qDebug()<<"download limit disabled";
//        DownloadHValue->hide();
        DownloadHValue->setEnabled(false);
        updateMutual->SetDownloadLimit(0,false);
        updateMutual->insertInstallStates("download_limit","false");

    }
    else if(DownloadHBtn->isChecked() == true)
    {
        updateMutual->insertInstallStates("download_limit","true");
        qDebug()<<"download limit enabled";
//        DownloadHValue->show();
        DownloadHValue->setEnabled(true);
        QString dlimit = DownloadHValue->currentText();
        updateMutual->SetDownloadLimit(dlimit,true);
    }
}

void TabWid::slotCancelDownload()
{
    checkUpdateBtn->setEnabled(true);
    //    checkUpdateBtn->setText("全部更新");
    checkUpdateBtn->setText(tr("UpdateAll"));
    checkUpdateBtn->adjustSize();
    checkUpdateBtn->setCheckable(true);
    versionInformationLab->setText(tr("update has been canceled!"));
//    versionInformationLab->setToolTip("");
    foreach (AppUpdateWid *wid, widgetList) {
        wid->updateAPPBtn->setEnabled(true);
    }
}

void TabWid::startoneappupdateslot()
{
    isAllUpgrade=false;
//    connect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));
}

void TabWid::hideUpdateBtnSlot(bool state,QStringList pkgname,QString error,QString reason)
{
        Q_UNUSED(pkgname);
        bacupInit(false);
        isAllUpgrade = false;
        alldownloadstart = false;
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->stop();
        //        checkUpdateBtn->setText(tr("检查更新"));
        checkUpdateBtn->setText(tr("Check Update"));
        if(state) {
            if(isContinueUpgrade)
            {
                versionInformationLab->setText(tr("This update has been completed！"));
            }else{
                versionInformationLab->setText(tr("Your system is the latest!"));
            }
            isContinueUpgrade=false;
//            versionInformationLab->setToolTip("");
            systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/normal.png").scaled(96,96));
            checkUpdateBtn->hide();
            allProgressBar->hide();
            progressLabel->hide();
            updatedetaileInfo->hide();
            detailLabel->hide();
//            updateMutual->fileUnLock();
        }
        else {

            if(isCancel)
            {
                isCancel=false;
                versionInformationLab->setText(tr("update has been canceled!"));
//                versionInformationLab->setToolTip("");
                checkUpdateBtn->hide();
//                return;
            }else{
                versionInformationLab->setText(tr("Part of the update failed!"));
                versionInformationLab->setToolTip(tr("Failure reason:")+ "\r\n"+error);
                checkUpdateBtn->hide();
            }
//            updateMutual->fileUnLock();
            allProgressBar->hide();
            updatedetaileInfo->hide();
            detailLabel->hide();
            progressLabel->setText(error);
        }
        disconnect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
        QString updatetime = tr("No Information!");
        QSqlQuery queryInstall(QSqlDatabase::database("A"));
        queryInstall.exec("select * from updateinfos order by id desc");
        while (queryInstall.next()) {
            QString statusType = queryInstall.value("keyword").toString();
            if(statusType == "" || statusType =="1") {
                updatetime = queryInstall.value("date").toString();
                break;
            }
        }
        lastRefreshTime->setText(tr("Last refresh:")+updatetime);
        lastRefreshTime->show();
        disconnect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
        disconnect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));

}

void TabWid::oneappUpdateresultSlot(bool state,QStringList pkgname,QString error,QString reason)
{
    Q_UNUSED(pkgname);
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->stop();
        //        checkUpdateBtn->setText(tr("检查更新"));
        checkUpdateBtn->setText(tr("UpdateAll"));
        checkUpdateBtn->adjustSize();
        foreach (AppUpdateWid *wid, widgetList) {
            wid->updateAPPBtn->setEnabled(true);
        }
        if(state) {
            versionInformationLab->setText(tr("Part of the update success!"));
//            versionInformationLab->setToolTip("");
            systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/normal.png").scaled(96,96));
            checkUpdateBtn->show();
            allProgressBar->hide();
            progressLabel->hide();
            updatedetaileInfo->hide();
            detailLabel->hide();
//            updateMutual->fileUnLock();
        }
        else {

            if(isCancel)
            {
                isCancel=false;
                versionInformationLab->setText(tr("update has been canceled!"));
//                versionInformationLab->setToolTip("");
                checkUpdateBtn->show();
                return;
            }else{
                versionInformationLab->setText(tr("Part of the update failed!"));
                versionInformationLab->setToolTip(tr("Failure reason:")+ "\r\n"+error);
                checkUpdateBtn->show();
            }
//            updateMutual->fileUnLock();
            allProgressBar->hide();
            updatedetaileInfo->hide();
            detailLabel->hide();
            progressLabel->setText(error);
        }
        if(updateMutual->importantList.size() == 0 && updateMutual->failedList.size()==0)
        {
            checkUpdateBtn->setEnabled(true);
            checkUpdateBtn->stop();
            //        checkUpdateBtn->setText(tr("检查更新"));
            checkUpdateBtn->setText(tr("Check Update"));
            versionInformationLab->setText(tr("Your system is the latest!"));
//            versionInformationLab->setToolTip("");
            systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/normal.png").scaled(96,96));
            checkUpdateBtn->hide();
            allProgressBar->hide();
            progressLabel->hide();
            updatedetaileInfo->hide();
            detailLabel->hide();
//            updateMutual->fileUnLock();
        }
        disconnect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
        QString updatetime = tr("No Information!");
        QSqlQuery queryInstall(QSqlDatabase::database("A"));
        queryInstall.exec("select * from updateinfos order by id desc");
        while (queryInstall.next()) {
            QString statusType = queryInstall.value("keyword").toString();
            if(statusType == "" || statusType =="1") {
                updatetime = queryInstall.value("date").toString();
                break;
            }
        }
        lastRefreshTime->setText(tr("Last refresh:")+updatetime);
        lastRefreshTime->show();
        disconnect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));

}

void TabWid::updatecancel(void)
{
    isAllUpgrade=false;
    checkUpdateBtn->setEnabled(true);
    checkUpdateBtn->stop();
    //        checkUpdateBtn->setText(tr("检查更新"));
    checkUpdateBtn->setText(tr("UpdateAll"));
    checkUpdateBtn->adjustSize();
    versionInformationLab->setText(tr("update has been canceled!"));
//    versionInformationLab->setToolTip("");
    checkUpdateBtn->show();

    foreach (AppUpdateWid *wid, widgetList) {
//        if(wid->updateAPPBtn->text() == tr("Update"))
            wid->updateAPPBtn->setEnabled(true);
    }

    disconnect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
    disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
    disconnect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
    disconnect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
}

void TabWid::changeUpdateAllSlot(bool isUpdate)
{
    if (isUpdate) {
        isAllUpgrade=false;
        checkUpdateBtn->setEnabled(false);
        versionInformationLab->setText(tr("Being updated..."));
//        versionInformationLab->setToolTip("");
        foreach (AppUpdateWid *wid, widgetList) {
                wid->updateAPPBtn->setEnabled(false);
        }
    } else  {
        if(checkUpdateBtn->isEnabled() == false)
        {
            checkUpdateBtn->setText(tr("UpdateAll"));
            checkUpdateBtn->setEnabled(true);
            checkUpdateBtn->adjustSize();
        }
    }
}


void TabWid::waitCrucialInstalled()
{
    if(fileLockedStatus == false)
    {
        QString msg = tr("An important update is in progress, please wait.");
        //        QString msg = tr("正在进行一项重要更新，请等待。");
        updateMutual->onRequestSendDesktopNotify(msg);
        versionInformationLab->setText(msg);
//        versionInformationLab->setToolTip("");
        fileLockedStatus = true;
    }
}

void TabWid::getReplyFalseSlot()
{
    isConnectSourceSignal = true;
}

void TabWid::receiveBackupStartResult(int result)
{
    switch (result) {
    case int(backuptools::backup_result::BACKUP_START_SUCCESS):
        //        versionInformationLab->setText(tr("开始备份，正在获取进度")+"...");
        versionInformationLab->setText(tr("Start backup,getting progress")+"...");
//        versionInformationLab->setToolTip("");
        checkUpdateBtn->start();
        checkUpdateBtn->setEnabled(false);
        foreach (AppUpdateWid *wid, widgetList) {
//            if(wid->updateAPPBtn->text() == tr("Update"))
                wid->updateAPPBtn->setEnabled(false);
        }
        return;
    case int(backuptools::backup_result::WRITE_STORAGEINFO_ADD_ITEM_FAIL):
    case int(backuptools::backup_result::WRITE_STORAGEINFO_UPDATE_ITEM_FAIL):
        //        backupMessageBox(tr("写入配置文件失败，本次更新不会备份系统！"));
        backupMessageBox(tr("Failed to write configuration file, this update will not back up the system!"));
        break;
    case int(backuptools::backup_result::BACKUP_CAPACITY_IS_NOT_ENOUGH):
        //        backupMessageBox(tr("备份空间不足，本次更新不会备份系统！"));
        backupMessageBox(tr("Insufficient backup space, this update will not backup your system!"));
        break;
    case int(backuptools::backup_result::INC_NOT_FOUND_UUID):
        //        backupMessageBox(tr("麒麟备份还原工具无法找到UUID，本次更新不会备份系统!"));
        backupMessageBox(tr("Kylin backup restore tool could not find the UUID, this update will not backup the system!"));
        break;
    default:
        backupMessageBox(tr("The backup restore partition is abnormal. You may not have a backup restore partition.For more details,see /var/log/backup.log"));
        break;
    }
    disconnect(backup, &BackUp::backupStartRestult, this, &TabWid::receiveBackupStartResult);
}

void TabWid::whenStateIsDuing()
{
    versionInformationLab->setText(tr("Calculating Capacity..."));
//    versionInformationLab->setToolTip("");
    checkUpdateBtn->start();
    checkUpdateBtn->setEnabled(false);
}

bool TabWid::get_battery()
{
    QStringList users;
    int battery_value = 0;
    QDBusInterface m_interface1( "org.freedesktop.UPower",
                                 "/org/freedesktop/UPower",
                                 "org.freedesktop.UPower",
                                 QDBusConnection::systemBus() );
    if (!m_interface1.isValid()) {
        qDebug() << "电源管理器dbus接口初始化失败";
        return true;
    }

    QDBusReply<QList<QDBusObjectPath>> obj_reply = m_interface1.call("EnumerateDevices");

    if (obj_reply.isValid()) {
        for (QDBusObjectPath op : obj_reply.value())
            users << op.path();
        if (users.size()==1 || users.isEmpty()) {
            qDebug()<<"无法获取电量值,判断此电脑为台式电脑";
            return true;
        }
        foreach (QString str, users) {
            if (str == users.at(0) || str == users.at(users.size() - 1)) {
                continue ;
            }
            QDBusInterface m_interface( "org.freedesktop.UPower",
                                        str,
                                        "org.freedesktop.DBus.Properties",
                                        QDBusConnection::systemBus());

            if (!m_interface.isValid()) {
                qDebug() << "电源管理器dbus接口初始化失败";
                return true;
            }

            QDBusReply<QVariant> obj_reply = m_interface.call("Get","org.freedesktop.UPower.Device","Percentage");
            int Ele_surplus = obj_reply.value().toInt();
            battery_value += Ele_surplus;
            qDebug() << "battery value : " << Ele_surplus;
        }
        return true;
    }
    /*如果电池总电量小于50不可升级*/
    if (battery_value < 50)
        return false;
    return true;
}

void TabWid::fileLock()
{
    QDir dir("/tmp/auto-upgrade/");
    if(! dir.exists()) {
        dir.mkdir("/tmp/auto-upgrade/");//只创建一级子目录，即必须保证上级目录存在
        chmod("/tmp/auto-upgrade/",0777);
    }
    umask(0000);
    int fd = open("/tmp/auto-upgrade/ukui-control-center.lock", O_RDONLY | O_CREAT,0666);
    if (fd < 0) {
        qDebug()<<"解锁时文件锁打开异常";
        return;
    }
    flock(fd, LOCK_EX | LOCK_NB);
}

void TabWid::fileUnLock()
{
    QDir dir("/tmp/auto-upgrade/");
    if(! dir.exists()) {
        dir.mkdir("/tmp/auto-upgrade/");//只创建一级子目录，即必须保证上级目录存在
        chmod("/tmp/auto-upgrade/",0777);
    }
    umask(0000);
    int fd = open("/tmp/auto-upgrade/ukui-control-center.lock", O_RDONLY | O_CREAT,0666);
    if (fd < 0) {
        qDebug()<<"解锁时文件锁打开异常";
        return;
    }
    flock(fd, LOCK_UN);
}

void TabWid::updateAllApp(bool status)
{
        if(!status){
            checkUpdateBtn->setText(tr("Calculating"));
            connect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));

        }else{
            foreach (AppUpdateWid *wid, widgetList) {
                wid->updateAPPBtn->hide();
                wid->hide();
            }
            connect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
            connect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
        }
        qDebug()<<"updateAllApp";
        updateMutual->DistUpgradeAll(status);

}

void TabWid::disupdateallaccept()
{
    distUpgradeAllApp(true);
}

bool TabWid::distUpgradeAllApp(bool status)
{
    if(!status){
        connect(updateMutual->interface,SIGNAL(DistupgradeDependResloveStatus(bool,bool,QStringList,QStringList,QString,QString)),this,SLOT(DistupgradeDependResloveResult(bool,bool,QStringList,QStringList,QString,QString)));
    }else{

        foreach (AppUpdateWid *wid, widgetList) {
            wid->hide();
        }
        connect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
        connect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
        connect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
    }
    updateMutual->DistUpgradeSystem(status);
}

void TabWid::autoupgradestatusshow()
{
    qDebug()<<"接收到备份完成信号";
    filewatcher = new QFileSystemWatcher();
    filewatcher->addPath("/var/run/unattended-upgrades.progress");
    connect(filewatcher,SIGNAL(fileChanged(QString)),this,SLOT(progresschanged()));
    QStringList path = filewatcher->files();
    qDebug()<<path;
//    return;
}

void TabWid::progresschanged()
{
    qDebug()<<"进度变化";
    QFile progressfile("/var/run/unattended-upgrades.progress");
    QFile lockfile("/tmp/auto-upgrade/ukui-control-center.lock");
    QFile pidfile("/var/run/unattended-upgrades.pid");
    progressfile.open(QIODevice::ReadOnly);
    QString prgs= progressfile.readAll();
//    versionInformationLab->setText("系统自动更新功能正在安装新文件："+prgs);
    versionInformationLab->setText(tr("Auto-Update progress is installing new file：")+prgs+"%");
    checkUpdateBtn->hide();
    filewatcher->addPath("/var/run/unattended-upgrades.progress");//加此句的意义：电脑的修改文件的方式不一样，有可能会删除该文件导致只能够监测一次文件的变化
    QDBusConnection::systemBus().connect(QString(),QString("/"),"com.kylin.install.notification","InstallFinish",this,SLOT(autoinstallfinish()));
    return;


    /*不能通过判断文件是否还存在判断安装结束，这只能作为一种异常处理，可以让沈亚峰引入信号*/
//    if(!pidfile.exists())//文件pid不存在就是程序
//    {
//        versionInformationLab->setText("系统更新完成！");
//        qDebug()<<"断开开关链接";
//        disconnect(filewatcher,SIGNAL(fileChanged(QString)),this,SLOT(progresschanged()));
//        return;
//    }
//    return;
}

void TabWid::autoinstallfinish()
{
//    versionInformationLab->setText("系统自动更新完成！");
    versionInformationLab->setText(tr("Auto-Update progress finished!"));
    disconnect(filewatcher,SIGNAL(fileChanged(QString)),this,SLOT(progresschanged()));
    return;
}

void TabWid::backupfailure()
{
//    versionInformationLab->setText("自动更新安装时备份失败！");
    versionInformationLab->setText(tr("Auto-Update progress fail in backup!"));
    return;
}

QString TabWid::getLanguageEnv()
{
    QStringList environment = QProcess::systemEnvironment();
    QString language_default="zh_CN.UTF-8";
    QString str="";
    foreach(str,environment)
    {
        if (str.startsWith("LANG="))
        {
            return str.mid(5);
        }
    }
    return language_default;
}
