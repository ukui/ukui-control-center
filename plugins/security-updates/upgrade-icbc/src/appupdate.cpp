#include "appupdate.h"
#include <QPixmap>
#include <stdio.h>
#include <QLocale>
#include <QScrollBar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


#define JSON_FILE_PATH "/usr/share/kylin-update-desktop-config/data/"
#define CONFIG_FILE_PATH "/usr/share/ukui-control-center/upgrade/"

AppUpdateWid::AppUpdateWid(AppAllMsg msg,QWidget *parent):QWidget(parent)
{
    m_updateMutual = UpdateDbus::getInstance(this);
    qRegisterMetaType<AppMsg>("AppMsg"); //注册信号槽类型
    qRegisterMetaType<AppAllMsg>("AppAllMsg"); //注册信号槽类型
    appAllMsg = msg;
    updateAppUi(appAllMsg.name);
    this->setObjectName("AppUpdateWid");
    initConnect();
}

AppUpdateWid::~AppUpdateWid()
{
    qDebug() << "delete->"<< appAllMsg.name;

}

void AppUpdateWid::initConnect()
{
    connect(detaileInfo,&QPushButton::clicked,this,&AppUpdateWid::showDetails);
    connect(updatelogBtn,&QPushButton::clicked,this,&AppUpdateWid::showUpdateLog);
    connect(updateAPPBtn,&QPushButton::clicked,this,&AppUpdateWid::cancelOrUpdate);
}

/* remove enter */
void AppUpdateWid::remove_last_enter(char *p_src_in_out)
{
    if (p_src_in_out == NULL) {
        return;
    }

    char *p_tmp = p_src_in_out + strlen(p_src_in_out) - 1;
    if (*p_tmp == '\n') {
        *p_tmp = '\0';
    }

    return;
}

/* analysis config file */
QStringList AppUpdateWid::analysis_config_file(char *p_file_path)
{
    FILE *fd = NULL;
    char p_buf[1024];

    QStringList dst;
    dst.clear();

    memset(p_buf , 0x00 , sizeof(p_buf));

    //printf("Info : p_file_path = [%s]\n" , p_file_path);
    fd = fopen(p_file_path , "r");
    if (fd == NULL) {
        printf("Error : open reboot config file fail\n");
        return dst;
    }

    while (fgets(p_buf , sizeof(p_buf) , fd) != NULL) {
        remove_last_enter(p_buf);
        if (!strlen(p_buf)) {
            memset(p_buf , 0x00 , sizeof(p_buf));
            continue;
        }

        //printf("Info : config file data [%s]\n" , p_buf);

        dst << QString(p_buf);

        memset(p_buf , 0x00 , sizeof(p_buf));
    }

    return dst;
}

//修改界面状态值
void AppUpdateWid::showInstallStatues(QStringList appAptName,int progress,QString status,QString detailinfo)
{
    qDebug() << appAptName[0];
    if(QString::compare(appAllMsg.name,appAptName[0]) == 0)
    {
        /* 临时解决方案 , 获取系统语言环境 , 英文加悬浮框 , 中文不加 */
//        m_updateMutual->fileLock();
        int pgs = progress;
        //        appVersion->setText(tr("正在安装")+"("+QString::number(pgs)+"%)");
        if (!isUpdateAll && progress>50 && !isCancel) {
            updateAPPBtn->hide();
                //updateAPPBtn->setEnabled(false);
            if(Cancelfail)
            {
                appVersion->setText(tr("Cancel failed,Being installed"));
                return;
            }
            appVersion->setText(tr("Being installed"));
            appVersion->setToolTip("");
        }

    }
}

void AppUpdateWid::showInstallFinsih(bool state,QStringList pkgname,QString error,QString reason)
{
    QLocale locale;
    char p_path[1024];
    memset(p_path , 0x00 , sizeof(p_path));
    sprintf(p_path , "%s%s" , CONFIG_FILE_PATH , "kylin-need-reboot.conf");
    QStringList reboot = analysis_config_file(p_path);
    qDebug() << "Info : need reboot pkg :" << reboot;

    memset(p_path , 0x00 , sizeof(p_path));
    sprintf(p_path , "%s%s" , CONFIG_FILE_PATH , "kylin-need-logout.conf");
    QStringList logout = analysis_config_file(p_path);
    qDebug() << "Info : need logout pkg :" << logout;
    if(QString::compare(appAllMsg.name,pkgname[0]) == 0)
    {
        if(state)
        {
            downloadstart = false;
            Cancelfail=false;
            updateAPPBtn->hide();
//            m_updateMutual->fileUnLock();
            //            appVersion->setText(tr("更新成功！"));

            if (reboot.contains(pkgname[0])) {
                if (locale.language() == QLocale::Chinese) {
                    appVersion->setText(tr("Update succeeded , It is recommended that you restart later!"));
                } else {
                    appVersion->setText(tr("Update succeeded , It is recommended that you restart later!"));
                    appVersion->setToolTip(tr("Update succeeded , It is recommended that you restart later!"));
                }
            } else if (logout.contains(pkgname[0])) {
                if (locale.language() == QLocale::Chinese) {
                    appVersion->setText(tr("Update succeeded , It is recommended that you log out later and log in again!"));
                } else {
                    appVersion->setText(tr("Update succeeded , It is recommended that you log out later and log in again!"));
                    appVersion->setToolTip(tr("Update succeeded , It is recommended that you log out later and log in again!"));
                }
            } else {
                appVersion->setText(tr("Update succeeded!"));
            }

            QIcon icon = QIcon::fromTheme("ukui-dialog-success");
            QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(16, 16)));
            appVersionIcon->setPixmap(pixmap);
            m_updateMutual->importantList.removeOne(appAllMsg.name);
            m_updateMutual->failedList.removeOne(appAllMsg.name);
            //            QString message = QString("%1"+tr("更新成功！")).arg(dispalyName);
            QString message = QString("%1"+tr("Update succeeded!")).arg(dispalyName);
            m_updateMutual->onRequestSendDesktopNotify(message);
            detaileInfo->hide();
            largeWidget->hide();
            emit oneappUpdateResultSignal(true,pkgname,"","");
        }
        else
        {
//            m_updateMutual->fileUnLock();
            if(isCancel)
            {
                appVersion->setText(tr("Update has been canceled!"));
                updateAPPBtn->show();//是否需要加入updateAPPBtn==更新？
                isCancel=false;
                emit appupdateiscancel();
            }else{
            Cancelfail=false;
            detaileInfo->hide();
            updateAPPBtn->hide();
            //            appVersion->setText(tr("更新失败！"));
            appVersion->setText(tr("Update failed!"));
            //            appVersion->setToolTip(tr("失败原因：")+(appNameLab->dealMessage(errormsg)));
            appVersion->setToolTip(tr("Failure reason:")+ "\r\n"+(appNameLab->dealMessage(error)));
            m_updateMutual->importantList.removeOne(appAllMsg.name);
            m_updateMutual->failedList.append(appAllMsg.name);
            QIcon icon = QIcon::fromTheme("dialog-error");
            QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(16, 16)));
            appVersionIcon->setPixmap(pixmap);
            //            QString message = QString("%1"+tr("更新失败！")).arg(dispalyName);
            QString message = QString("%1"+tr("Update failed!")).arg(dispalyName);
            m_updateMutual->onRequestSendDesktopNotify(message);
            emit oneappUpdateResultSignal(false,pkgname,error,reason);
            }
      }
        disconnect(m_updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(OneAppDependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
        disconnect(m_updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(showInstallFinsih(bool,QStringList,QString,QString)));
        disconnect(m_updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(showInstallStatues(QStringList,int,QString,QString)));
    }
}
void AppUpdateWid::OneAppDependResloveResult(bool ResloveStatus,bool ResloveResult,QStringList DeletePkgList, QStringList DeletePkgDeslist,QStringList DeletePkgReasonlist,QString Error,QString Reason)
{
    if(!ResloveStatus)
    {
        qDebug()<<Error<<Reason;
        QMessageBox msgBox(qApp->activeModalWidget());
        msgBox.setText(tr("There are unresolved dependency conflicts in this update，Please select update all"));
        msgBox.setWindowTitle(tr("Prompt information"));
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.addButton(tr("Update ALL"), QMessageBox::YesRole);
        msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);

        int ret = msgBox.exec();
        switch (ret) {
        case 0:
            qDebug() << "全部更新。。。。。。";
            emit changeupdateall();
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
                distUpgradePartial(true);
        }else{
            oneappshowDependSlovePtompt(DeletePkgList,DeletePkgDeslist,DeletePkgReasonlist);
        }
    }
    disconnect(m_updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(OneAppDependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));

}

void AppUpdateWid::oneappshowDependSlovePtompt(QStringList pkgname,QStringList description,QStringList deletereason)
{
    oneappdependsloveptompt = updatedeleteprompt::GetInstance(this);
    //在屏幕中央显示
    //QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    //historyLog->move((availableGeometry.width()-historyLog->width())/2,(availableGeometry.height()- historyLog->height())/2);
    connect(oneappdependsloveptompt,&updatedeleteprompt::updatedependsolvecancelsignal,this,&AppUpdateWid::updatecancel);
    connect(oneappdependsloveptompt,&updatedeleteprompt::updatedependsolveacceptsignal,this,&AppUpdateWid::updateaccept);
    int pkgnum = oneappdependsloveptompt->updatedeletepkglist(pkgname,description,deletereason);//更新列表
    QString pkgnamestr = QString::number(pkgnum);
    int UpdatePart=2;
    oneappdependsloveptompt->updatemode=UpdatePart;

    oneappdependsloveptompt->uninstallinfoTab->setText(pkgnamestr +" "+ tr("pkg will be uninstall!"));
    oneappdependsloveptompt->show();
}

void AppUpdateWid::updateaccept()
{
    distUpgradePartial(true);
    qDebug() << "accept distUpgradePartial! ";
}

void AppUpdateWid::updatecancel()
{
    appVersion->setText(tr("Update has been canceled!"));
    updateAPPBtn->setText(tr("Update"));
//    updateAPPBtn->adjustSize();
    updateAPPBtn->show();
    detaileInfo->show();
    isCancel=false;
    emit appupdateiscancel();
}

void AppUpdateWid::updateAppUi(QString name)
{
    iconNameLayout = new QHBoxLayout();
    smallHLayout = new QHBoxLayout();
    largeVLayout = new QVBoxLayout();
    appTitleWid = new QWidget();
    largeWidget = new QWidget();
    mainVLayout = new QVBoxLayout();
    QHBoxLayout *frameLayout = new QHBoxLayout();
    AppFrame = new QFrame(this);  //最外层窗口适配主题样式
    AppFrame->setFrameShape(QFrame::Box);
    AppFrame->setMinimumSize(QSize(550,0));
    AppFrame->setMaximumSize(QSize(16777215, 16777215));

    frameLayout->addWidget(AppFrame);
    frameLayout->setContentsMargins(0,0,0,0);   //更新信息之间的间距
    frameLayout->setSpacing(0);
    this->setLayout(frameLayout);

    appIconName = new QLabel(this);
//    appIconName->setMinimumWidth(130);
//    appIconName->setMaximumWidth(400);
    appIcon = new FixLabel(appIconName);
    appNameLab = new MyLabel(appIconName);
    //    appNameLab->setMinimumWidth(140);
    appIconName->setLayout(iconNameLayout);
    appIcon->setFixedSize(32,32);
    iconNameLayout->setAlignment(Qt::AlignLeft);
    iconNameLayout->addSpacing(6);
    iconNameLayout->addWidget(appIcon,1);
    iconNameLayout->setSpacing(0);
    iconNameLayout->addSpacing(8);
    iconNameLayout->addWidget(appNameLab,10);
    //    iconNameLayout->addStretch();

    appVersion = new FixLabel(this);
    appVersionIcon = new QLabel(this);
    appVersionIcon->setFixedSize(16,16);
    appVersionIcon->setPixmap(QPixmap());
//    appVersion->setMinimumWidth(300);
    //    QIcon icon = QIcon::fromTheme("dialog-error");
    //    QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(14, 14)));
    //    appVersionIcon->setPixmap(pixmap);

    detaileInfo = new QPushButton(this);
    //    detaileInfo->setText(tr("详情"));
    detaileInfo->setText(tr("details"));
    //detaileInfo->setFixedSize(60,30);
//    detaileInfo->adjustSize();
//    detaileInfo->setMaximumWidth(95);
    detaileInfo->setFlat(true);

    updateAPPBtn = new QPushButton(this);
    //    updateAPPBtn->setText(tr("更新"));
    updateAPPBtn->setText(tr("Update"));
//    updateAPPBtn->setFixedWidth(95);
    //updateAPPBtn->setFixedSize(70,40);
//    updateAPPBtn->adjustSize();


    versionlabLayout = new QHBoxLayout();  //版本号布局
    versionlabLayout->setSpacing(0);
    versionlabLayout->setMargin(0);
    versionLab = new QLabel(this);
    versionLab->setMargin(0);
//    versionLab->setMinimumWidth(250);
//    versionLab->setMaximumWidth(450);
    versionLab->setFixedHeight(60);

    versionlabLayout->setAlignment(Qt::AlignLeft);
    versionlabLayout->addWidget(appVersionIcon);
    versionlabLayout->addWidget(appVersion);
    versionLab->setLayout(versionlabLayout);

    otherBtnLayout = new QHBoxLayout();  //详情、更细按钮布局
//    otherBtnLayout->setSpacing(0);
//    otherBtnLayout->setMargin(0);
    otherBtnLab = new QLabel(this);
    otherBtnLab->setMargin(0);
//    otherBtnLab->setMinimumWidth(135);
//    otherBtnLab->setMaximumWidth(400);
    otherBtnLab->setFixedHeight(60);


//    otherBtnLayout->addWidget(appVersionIcon);
//    otherBtnLayout->addWidget(appVersion);
    otherBtnLayout->addWidget(detaileInfo,1,Qt::AlignRight);
    otherBtnLayout->addSpacing(1);
    otherBtnLayout->addWidget(updateAPPBtn,1,Qt::AlignRight);
    updateAPPBtn->setFixedWidth(100);
//    otherBtnLayout->setAlignment(Qt::AlignLeft);
    otherBtnLab->setLayout(otherBtnLayout);

    smallHLayout->addWidget(appIconName,1);
    smallHLayout->addStretch(0);
    smallHLayout->addWidget(versionLab,1);
    smallHLayout->addStretch(0);
    smallHLayout->addWidget(otherBtnLab,1);
    smallHLayout->setSpacing(0);
    smallHLayout->setContentsMargins(0,0,0,0);
    appTitleWid->setLayout(smallHLayout);

    mainVLayout->addWidget(appTitleWid);
    mainVLayout->setMargin(0);
    mainVLayout->setSpacing(0);
    someInfoEdit = new QTextEdit(this);   //部分changelog  缩略显示
    someInfoEdit->setReadOnly(true);
    someInfoEdit->verticalScrollBar()->setProperty("drawScrollBarGroove" , false);
    QPalette pl = someInfoEdit->palette();
    someInfoEdit->setFixedHeight(120);
    pl.setBrush(QPalette::Base,QBrush(QColor(255,0,0,0)));
    someInfoEdit->setPalette(pl);

    //当前语言环境
    QString locale = QLocale::system().name();
    if (locale == "zh_CN")
        environment=zh_cn;

    updatelogBtn = new QPushButton(this);
    //    updatelogBtn->setText(tr("更新日志"));
    updatelogBtn->setText(tr("Update log"));
    updatelogBtn->setFlat(true);
    updatelog1 = new UpdateLog(this);

    largeVLayout->addWidget(someInfoEdit);
    largeVLayout->addWidget(updatelogBtn,0,Qt::AlignLeft);
    largeVLayout->setSpacing(5);
    largeVLayout->setContentsMargins(50,0,50,5);
    largeWidget->setLayout(largeVLayout);
    //    largeWidget->setFixedHeight(80);
    mainVLayout->addWidget(largeWidget);
    largeWidget->hide();
    //    largeWidget->setFixedHeight(120);
    //    appTitleWid->setFixedHeight(60)
    //    this->setLayout(mainVLayout);
    AppFrame->setLayout(mainVLayout);
    QMap<QString, QString> map = getNameAndIconFromJson(name);
    if (!map.value("name").isNull())
        dispalyName = map.value("name");
    else
        dispalyName = translationVirtualPackage(name);

    appNameLab->setText(dispalyName);
    /*判断图标，优先级: JSON文件指定 > qrc资源文件中 > 主题 > 默认*/
    if (!map.value("icon").isNull()) {
        haveThemeIcon = true;
        appIcon->setPixmap(QPixmap(map.value("icon")));
    } else if (name.contains("kylin-update-desktop-")||name == "linux-generic") {
        haveThemeIcon = true;
        pkgIconPath = QString(":/img/upgrade/%1.png").arg(name);
        appIcon->setPixmap(QPixmap(pkgIconPath));
    } else if (QIcon::fromTheme(name).hasThemeIcon(name)) {    //判断是否有主题图标并输出
        haveThemeIcon = true;
        QIcon icon = QIcon::fromTheme(name);
        QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(32, 32)));
        appIcon->setPixmap(pixmap);
    } else {
        QIcon icon = QIcon::fromTheme("application-x-desktop");
        QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(32, 32)));
        appIcon->setPixmap(pixmap);
    }
    QString newStrMsg = appAllMsg.availableVersion;

    if(newStrMsg.size()>16)
    {
        //        appVersion->setText(tr("最新：")+newStrMsg);
        //        appVersion->setToolTip(tr("最新：")+newStrMsg);
        appVersion->setText(tr("Newest:")+newStrMsg);
        appVersion->setToolTip(tr("Newest:")+newStrMsg);
    }
    else
    {
        //        appVersion->setText(tr("最新：")+newStrMsg);
        appVersion->setText(tr("Newest:")+newStrMsg);
        appVersion->setToolTip("");
    }

    //获取并输出changelog
    chlog = setDefaultDescription(appAllMsg.longDescription);
    updatelog1->logContent->append(chlog);

    QTextCursor tmpCursor = updatelog1->logContent->textCursor();
    tmpCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    updatelog1->logContent->setTextCursor(tmpCursor);

    //    updatelog1->logAppName->setText(dispalyName+tr("更新日志"));
    //    updatelog1->logAppVerson->setText(tr("最新:")+appAllMsg.availableVersion);
    updatelog1->logAppName->setText(dispalyName+tr("Update log"));
    updatelog1->logAppVerson->setText(tr("Newest:")+appAllMsg.availableVersion);

    //获取并输出description
    description = setDefaultDescription(appAllMsg.shortDescription);

    QFontMetrics fontWidth(someInfoEdit->font());//得到每个字符的宽度
//    QString StrMsg = fontWidth.elidedText(description, Qt::ElideRight,600);//最大宽度description, Qt::ElideRight,600
    QString StrMsg = description;
    someInfoEdit->append(StrMsg);

    someInfoEdit->append(tr("Download size:")+QString(modifySizeUnit(appAllMsg.msg.allSize)));
    someInfoEdit->append(tr("Install size:")+QString(modifySizeUnit(appAllMsg.msg.installSize)));
    if (!map.value("icon").isNull()) {
        haveThemeIcon = true;
        updatelog1->logAppIcon->setPixmap(QPixmap(map.value("icon")));
    } else if(name.contains("kylin-update-desktop")||name == "linux-generic") {
        pkgIconPath = QString(":/img/upgrade/%1.png").arg(name);
        updatelog1->logAppIcon->setPixmap(QPixmap(pkgIconPath));
    } else {
        if(QIcon::fromTheme(name).hasThemeIcon(name))
        {
            QIcon icon = QIcon::fromTheme(name);
            QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(32, 32)));
            updatelog1->logAppIcon->setPixmap(pixmap);
        }
        else
        {
            QIcon icon = QIcon::fromTheme("application-x-desktop");
            QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(32, 32)));
            updatelog1->logAppIcon->setPixmap(pixmap);
        }
    }
    //获取并输出包大小、当前版本号
    QString currentVersion = appAllMsg.version;
    if(currentVersion != "")
    {
        //        someInfoEdit->append(tr("当前版本：")+currentVersion);
        someInfoEdit->append(tr("Current version:")+currentVersion);
    }

}


QMap<QString, QString> AppUpdateWid::getNameAndIconFromJson(QString pkgname)
{
    QMap <QString, QString> nameIconList;
    /*判断json文件是否存在*/
    QString filename = QString(JSON_FILE_PATH) +pkgname +".json";
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)){
        qDebug() << "JSON file open failed! ";
        return nameIconList;
    }
    QByteArray jsonData = file.readAll();

    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(jsonData, &err_rpt); // 字符串格式化为JSON

    if (!root_Doc.isNull() && (err_rpt.error == QJsonParseError::NoError)) {  // 解析未发生错误
        if (root_Doc.isObject()) { // JSON 文档为对象
            QJsonObject object = root_Doc.object();  // 转化为对象
            if (QLocale::system().name() == "zh_CN"){
                QString name  = object.value("name").toObject().value("zh_CN").toString();
                if (!name.isNull()) {
                    nameIconList.insert("name", name);
                }
            }else {
                QString name  = object.value("name").toObject().value("en_US").toString();
                if (!name.isNull()) {
                    nameIconList.insert("name", name);
                }
            }
            QString iconPath = object.value("icon").toString();
            if (!iconPath.isNull())
                nameIconList.insert("icon", iconPath);
        }
    }else{
        qDebug() << "JSON文件格式错误！";
        return nameIconList;
    }

    return nameIconList;
}

void AppUpdateWid::showDetails()
{
    if(largeWidget->isHidden())
    {
        largeWidget->show();
        //        detaileInfo->setText(tr("收起"));
        detaileInfo->setText(tr("back"));
    }
    else
    {
        largeWidget->hide();
        //        detaileInfo->setText(tr("详情"));
        detaileInfo->setText(tr("details"));
    }
}

void AppUpdateWid::showUpdateLog()
{
    //QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    //updatelog1->move((availableGeometry.width()-updatelog1->width())/2,(availableGeometry.height()- updatelog1->height())/2);
    updatelog1->exec();
}

void AppUpdateWid::cancelOrUpdate()
{
    if(updateAPPBtn->text() == tr("Update"))
    {
//        emit changeUpdateAllSignal(true);
        /*判断电量是否支持更新*/
        if (!get_battery()) {
            QMessageBox msgBox;
            msgBox.setText(tr("The battery is below 50% and the update cannot be downloaded"));
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setButtonText(QMessageBox::Ok,tr("OK"));
            msgBox.exec();
            return ;
        }
        if (isAutoUpgrade) {
            updateOneApp(false);
            return ;
        }
        if(m_updateMutual->isPointOutNotBackup == true)
        {
            QMessageBox msgBox(qApp->activeModalWidget());
            msgBox.setText(tr("A single update will not automatically backup the system, if you want to backup, please click Update All."));
            msgBox.setWindowTitle(tr("Prompt information"));
            msgBox.setIcon(QMessageBox::Icon::Warning);
//            msgBox.setStandardButtons(QMessageBox::YesAll
//                                      | QMessageBox::NoToAll|QMessageBox::Cancel);
//            msgBox.setButtonText(QMessageBox::YesAll, tr("Do not backup, continue to update"));
//            msgBox.setButtonText(QMessageBox::NoToAll, tr("Cancel"));
//            msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel update"));

            msgBox.addButton(tr("Do not backup, continue to update"), QMessageBox::YesRole);
            msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
            QCheckBox *cb = new QCheckBox(&msgBox);
            msgBox.setCheckBox(cb);
            msgBox.checkBox()->setText(tr("This time will no longer prompt"));
            msgBox.checkBox()->show();

            int ret = msgBox.exec();
            if(msgBox.checkBox()->checkState() == Qt::Checked) {
                m_updateMutual->isPointOutNotBackup = false;
            }
            if(ret == 0) {
                emit changeUpdateAllSignal(true);
                qDebug() << "立即更新!";
                updateOneApp(false);
            } else if(ret == 1) {
                emit changeUpdateAllSignal(false);
                m_updateMutual->isPointOutNotBackup = true;
                qDebug() << "不进行更新。";
            }
            qDebug() << "m_updateMutual->isPointOutNotBackup = " << m_updateMutual->isPointOutNotBackup;
        }
        else{
            emit changeUpdateAllSignal(true);
            updateOneApp(false);
        }
    }
    else {
        updateAPPBtn->setText(tr("Update"));
//        updateAPPBtn->adjustSize();
        detaileInfo->show();
//        QString newStrMsg = appAllMsg.availableVersion;
//        if(newStrMsg.size() > 16) {
//            appVersion->setText(tr("Newest:") + newStrMsg);
//            appVersion->setToolTip(tr("Newest:") + newStrMsg);
//        } else {
//            appVersion->setText(tr("Newest:") + newStrMsg);
//            appVersion->setToolTip("");
//        }

        QDBusReply<bool> reply = m_updateMutual->interface->call("CancelDownload");
        if(!reply.isValid())
        {
            qDebug()<<"取消下载接口异常";
            return ;
        }
        bool cancelreply = reply.value();
        if(cancelreply)
        {
            isCancel = true;
            emit appupdateiscancel();
        }
        else
        {
            Cancelfail = true;  //取消失败
        }
    }
}

void AppUpdateWid::updateOneApp(bool status)
{
    distUpgradePartial(status);

}

void AppUpdateWid::distUpgradePartial(bool status)
{
    if(status){
        connect(m_updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(showInstallStatues(QStringList,int,QString,QString)));
        connect(m_updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadStatues(QStringList,int,int,uint,uint,int)));
        connect(m_updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(showInstallFinsih(bool,QStringList,QString,QString)));
        updateAPPBtn->setText(tr("Cancel"));
//        updateAPPBtn->adjustSize();
        updateAPPBtn->setEnabled(false);
        //updateAPPBtn->setFixedWidth(110);
/*11.25*/
        detaileInfo->hide();

        appVersionIcon->setPixmap(QPixmap());
        appVersion->setText(tr("Ready to update"));
        emit startoneappupdate();
    }else{
         connect(m_updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(OneAppDependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
    }
    QStringList pkgname;
    pkgname.append(appAllMsg.name);
    m_updateMutual->interface->asyncCall("DistUpgradePartial",status,pkgname);
}
//转换包大小的单位
QString AppUpdateWid::modifySizeUnit(long long size)
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

//当前下载速度单位换算
QString AppUpdateWid::modifySpeedUnit(long size, float time)
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

void AppUpdateWid::showDownloadStatues(QStringList pkgname,int currentserial,int total,uint downloadedsize,uint totalsize,int downloadSpeed)
{

    if(downloadSpeed > 0)
        downloadstart=true;
//    connect(m_updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(enableupdateAPPBtn));
    QString downloadspeed = modifySpeedUnit(downloadSpeed, 1);
    if(QString::compare(appAllMsg.name,pkgname[0]) == 0)
    {
        updateAPPBtn->setEnabled(true);
    //    updateAPPBtn->adjustSize();
//        updateAPPBtn->setFixedWidth(110);
        if (!isUpdateAll){
            if((downloadedsize == totalsize) && (totalsize == 0))
            {
                if(isCancel)
                {
                    return;
                }
                else
                {
                    appVersion->setText(tr("downloaded"));
                    return;
                }
            }
            if(downloadSpeed==0 && !downloadstart)
            {
                appVersion->setText(tr("downloading")+":"+"("+tr("calculating")+")"+modifySizeUnit(downloadedsize)+"/"+modifySizeUnit(totalsize));
                return;
            }
            appVersion->setText(tr("downloading")+":"+"("+downloadspeed+")"+modifySizeUnit(downloadedsize)+"/"+modifySizeUnit(totalsize));
            appVersion->setToolTip("");
        }
    }
    if(currentserial==total)
    {
        disconnect(m_updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadStatues(QStringList,int,int,uint,uint,int)));
//        disconnect(m_updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(enableupdateAPPBtn));
    }
}

void AppUpdateWid::showUpdateBtn()
{
    if (!isUpdateAll)
        updateAPPBtn->show();
    updateAPPBtn->setText(tr("Update"));
//    updateAPPBtn->adjustSize();
}
void AppUpdateWid::hideOrShowUpdateBtnSlot(int result)
{
    if(result == 0)  //0表示备份还原开始
    {
        updateAPPBtn->hide();
    }
    else if(result == 99 ||result == -20)  //99时备份成功 -20时备份还原进程被中断
    {
        if (!isUpdateAll)
            updateAPPBtn->show();
    }

}

QString AppUpdateWid::translationVirtualPackage(QString str)
{
    if(QLocale::system().name()!="zh_CN")
        return str;
    if(str == "kylin-update-desktop-app")
        return "基本应用";
    if(str == "kylin-update-desktop-security")
        return "安全更新";
    if(str == "kylin-update-desktop-support")
        return "系统基础组件";
    if(str == "kylin-update-desktop-ukui")
        return "桌面环境组件";
    if(str == "linux-generic")
        return "系统内核组件";
    if(str == "kylin-update-desktop-kernel")
        return "系统内核组件";
    if(str == "kylin-update-desktop-kernel-3a4000")
        return "系统内核组件";
    if(str == "kylin-update-desktop-kydroid")
        return "kydroid补丁包";
    return str;
}
bool AppUpdateWid::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->metaObject()->className() == QStringLiteral("QLabel"))
    {
        if(event->type() == QEvent :: ToolTip){
            QToolTip::hideText();
            event->ignore();
            return true;
        }
        else
        {
            return false;
        }
    }
}

QString AppUpdateWid::setDefaultDescription(QString str)
{
    if(str == "")
    {
        //        str = tr("暂无内容");
        str = tr("No content.");
    }
    return str;
}

type AppUpdateWid::checkSourcesType()
{
    QFile soucesFile(SOURCESLIST);
    soucesFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QString result = soucesFile.readAll();
    if(result.contains("http://") || result.contains("https://")){
        qDebug() << "当前源为http源";
        return http;
    }else if(result.contains("ftp://")){
        qDebug() << "当前源为ftp源";
        return ftp;
    }else if(result.contains("file://")){
        qDebug() << "当前源为本地源";
        return file;
    }
    return http;
}

bool AppUpdateWid::get_battery()
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

//void AppUpdateWid::enableupdateAPPBtn(bool enabled)
//{
//    if(updateAPPBtn->text()=="Cancel")
//    {
//        if(enabled)
//        {
//            updateAPPBtn->setEnabled(true);
//            updateAPPBtn->adjustSize();
//        }
//        else
//        {
//            updateAPPBtn->setEnabled(false);
//            updateAPPBtn->adjustSize();
//        }
//    }
//}

