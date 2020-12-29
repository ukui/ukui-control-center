#include "tabwidget.h"

TabWid::TabWid(QWidget *parent):QWidget(parent)
{
    updateMutual = UpdateDbus::getInstance();
//    this->resize(620,580);
    allComponents();
    createUI();
    getAllDisplayInformation();

}


void TabWid::allComponents()
{
    mainTabLayout = new QVBoxLayout();  //整个页面的主布局

    updateWidget = new QTabWidget(this);   //标签页
//    updateWidget->resize(500,580);
    scrollArea = new QScrollArea(this);

    updateTab = new QWidget(this);  //更新页面
    AppMessage = new QVBoxLayout();
    updateTab->setLayout(AppMessage);
    systemWidget = new QFrame(updateTab);
    systemWidget->setFrameShape(QFrame::Box);
    systemPortraitLab = new QLabel(systemWidget);

    AppMessage->addWidget(systemWidget,0,Qt::AlignTop);
//    tab1VLayout->addWidget(appUpdateListWidget);
//    updateTab->setLayout(tab1VLayout);

    scrollArea->setWidget(updateTab);
    scrollArea->setFrameStyle(0);
//    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidgetResizable(true);

//    tab1VLayout->addWidget(appWid1,0,Qt::AlignTop);
//    tab1VLayout->addWidget(appWid2,1,Qt::AlignTop);

    systemWidget->setFixedHeight(120);

    updateSettingTab = new QWidget(this);  //更新设置界面
    tab2VLayout = new QVBoxLayout();
    updateSettingTab->setLayout(tab2VLayout);
//    tab2HLayout = new QHBoxLayout(updateSettingTab);
    isAutoCheckWidget = new QFrame();
    isAutoCheckWidget->setFrameShape(QFrame::Box);
    isAutoCheckedLab = new QLabel();
    isAutoCheckWidget->setFixedHeight(50);
    isAutoCheckedLab->setText("自动检查更新");

    isAutoCheckSBtn = new SwitchButton(isAutoCheckWidget);
    QHBoxLayout *autoCheckLayout = new QHBoxLayout();
    isAutoCheckWidget->setLayout(autoCheckLayout);
    autoCheckLayout->addWidget(isAutoCheckedLab,0,Qt::AlignLeft);
    autoCheckLayout->addWidget(isAutoCheckSBtn,1,Qt::AlignRight);
    autoCheckLayout->setSpacing(0);


    tab2VLayout->addWidget(isAutoCheckWidget,0,Qt::AlignTop);

    mainTabLayout->addWidget(updateWidget);
    mainTabLayout->setContentsMargins(0,0,0,0);  //存在问题，设置右边边距为28px时会截掉右边部分窗口

    updateWidget->addTab(scrollArea,"更新");
    updateWidget->addTab(updateSettingTab,"更新设置");

    this->setLayout(mainTabLayout);
    connect(isAutoCheckSBtn,&SwitchButton::checkedChanged,this,&TabWid::isAutoCheckedChanged);
    connect(updateMutual,&UpdateDbus::updatelist,this,&TabWid::slotGetImportant);


}

//往tabwidget中插入listwidget信息
void TabWid::getAppUpdateMsg(QStringList arg)
{
//    updateMutual->ConnectSlots();
//    int cnum = updateMutual->cnameList.size();
    int inum = arg.size();

    if(inum != 0)
    {
        QStringList thisInameList = arg;
        QApt::Backend *m_backend = new QApt::Backend();
        m_backend->init();
        int checkNum = 0;
        for(inumber = 0; inumber < inum; inumber++)
        {

            QString app_name = thisInameList[inumber];
            if(app_name.isEmpty())
                continue;
            QApt::Package *m_package = m_backend->package(app_name);
            if(m_package == nullptr)
                continue;
            if(!m_package->isInstalled() || (m_package->state() & QApt::Package::Upgradeable)) //判断软件包是否已安装或者可升级
            {
                qDebug() << app_name;
                checkNum++;
                updateMutual->importantList.append(app_name);   //重要更新列表中添加appname
                AppUpdateWid *appWidget = new AppUpdateWid(app_name, this);
                connect(appWidget, &AppUpdateWid::cancel, this, &TabWid::slotCancelDownload);
                connect(checkUpdateBtn, &QPushButton::clicked, appWidget, &AppUpdateWid::updateAllApp);
                connect(appWidget,&AppUpdateWid::hideUpdateBtnSignal,this,&TabWid::hideUpdateBtnSlot);
                connect(appWidget,&AppUpdateWid::changeUpdateAllSignal,this,&TabWid::changeUpdateAllSlot);

                AppMessage->addWidget(appWidget,0,Qt::AlignTop);
            }
            delete m_package;
            m_package = NULL;
        }
        AppMessage->addStretch();

        checkUpdateBtn->setText("全部更新");
        disconnect(checkUpdateBtn,&QPushButton::clicked,updateMutual,&UpdateDbus::startTray);

        qDebug() << "num:" << checkNum;
        if(checkNum == 0)
        {
            checkUpdateBtn->hide();
            versionInformationLab->setText("您的系统已是最新！");
        }
        m_backend = NULL;
    }
    else
    {
        checkUpdateBtn->hide();
        versionInformationLab->setText("您的系统已是最新！");
    }
}

void TabWid::getAllDisplayInformation()
{
    QSqlQuery query(QSqlDatabase::database("A"));
    QStringList displayinfo;

    query.exec("select * from display");
    while(query.next())
    {
        displayinfo.append(query.value(1).toString());
    }
    QString updatetime = displayinfo[0];
    QString checkedtime = displayinfo[1];
    QString checkedstatues = displayinfo[3];

    lastRefreshTime->setText("上次更新: "+updatetime);

    versionInformationLab->setText("上次检测: "+checkedtime);

    checkUpdateBtn->setText("开始检测");


//    if(checkedstatues == "false")
//    {
//        isAutoCheckSBtn->setChecked(false);
//        checkUpdateBtn->setText("开始检测");
//    }
//    else if (checkedstatues == "true")
//    {

////        connect(updateMutual,&UpdateDbus::updatelist,this,&TabWid::getAppUpdateMsg);
//        isAutoCheckSBtn->setChecked(true);
//        checkUpdateBtn->setText("全部更新");
////        getAppUpdateMsg();]
//        qDebug() << "调用。。。。";
////        updateMutual->initTrayD_bus(true);
//        qDebug() << "调用。。。fffff。";

//    }

}
void TabWid::showHistoryWidget()
{
    historyLog = new m_updatelog();
    historyLog->updatesql();
    historyLog->show();
    qDebug()<<"ok";
}

void TabWid::checkUpdateBtnClicked()
{
    if(checkUpdateBtn->text() == "开始检测")
    {
        checkUpdateBtn->setText("正在检测...");

//        QDateTime nowtime = QDateTime::currentDateTime();
//        QString current_date = nowtime.toString("yyyy.MM.dd hh:mm:ss");
//        QSqlQuery query(QSqlDatabase::database("A"));
//        query.exec(QString("update display set item = '%1' where info = '%2'").arg(current_date).arg("上次检测时间"));


    }
    else if(checkUpdateBtn->text() == "全部更新")
    {
        checkUpdateBtn->setText("正在更新...");
        checkUpdateBtn->setEnabled(false);
    }
}

void TabWid::isAutoCheckedChanged()     //自动检测按钮绑定的槽函数
{
    QSqlQuery query(QSqlDatabase::database("A"));
    if(isAutoCheckSBtn->isChecked() == false)
    {
        QString Str = QString("update display set item = '%1' where info = '%2'").arg("false").arg("自动检测");
        query.exec(Str);

    }
    else if(isAutoCheckSBtn->isChecked() == true)
    {
        QString Str = QString("update display set item = '%1' where info = '%2'").arg("true").arg("自动检测");
        query.exec(Str);
    }
}
void TabWid::createUI()
{
    systemPortraitLab = new QLabel();
    tab1HLayout = new QHBoxLayout();
    systemWidget->setLayout(tab1HLayout);
//    systemWidget->setBaseSize(QSize(400,120));

    historyUpdateLog = new QPushButton(this); // 历史日志弹出窗口控制按钮
    historyUpdateLog->setText("历史更新");
    historyUpdateLog->setFlat(true);

    connect(historyUpdateLog,&QPushButton::clicked,this,&TabWid::showHistoryWidget);




    systemPortraitLab->setFixedSize(68,68);
    systemPortraitLab->setPixmap(QPixmap(":/img/plugins/upgrade/refresh.png").scaled(QSize(64,64)));


    inforLayout = new QVBoxLayout();
    lastRefreshTime = new QLabel(systemWidget);
    lastRefreshTime->setText("上次更新时间：0000.00.00 00:00");


    versionInformationLab = new QLabel(systemWidget);
    versionInformationLab->setText("上次检测时间：0000.00.00 00:00");



    inforLayout->addWidget(versionInformationLab);
    inforLayout->addWidget(lastRefreshTime);
    QWidget *inforWidget = new QWidget(systemWidget);
    inforWidget->setLayout(inforLayout);

    checkUpdateBtn = new QPushButton(systemWidget);
    checkUpdateBtn->setText("检测更新");
    checkUpdateBtn->setFixedSize(120,36);
    connect(checkUpdateBtn,&QPushButton::clicked,this,&TabWid::checkUpdateBtnClicked);
    connect(checkUpdateBtn,&QPushButton::clicked,updateMutual,&UpdateDbus::startTray);




    tab1HLayout->addWidget(systemPortraitLab,0,Qt::AlignLeft);
    tab1HLayout->addWidget(inforWidget,Qt::AlignLeft|Qt::AlignTop);
    tab1HLayout->addWidget(historyUpdateLog);
    tab1HLayout->addWidget(checkUpdateBtn);

    tab1HLayout->setMargin(20);
    tab1HLayout->setSpacing(20);




}

void TabWid::tabUI2()
{
    systemPortraitLab = new QLabel();
    tab1HLayout = new QHBoxLayout();
    systemWidget->setLayout(tab1HLayout);

    systemPortraitLab->setFixedSize(68,68);

    systemPortraitLab->setPixmap(QPixmap(":/image/refresh.png").scaled(QSize(64,64)));


    historyUpdateLog = new QPushButton(this); // 历史日志弹出窗口控制按钮
    historyUpdateLog->setText("历史更新");
    historyUpdateLog->setFlat(true);


    inforLayout = new QVBoxLayout();
    lastRefreshTime = new QLabel(systemWidget);


    versionInformationLab = new QLabel(systemWidget);

    inforLayout->addWidget(versionInformationLab);
    inforLayout->addWidget(lastRefreshTime);
    QWidget *inforWidget = new QWidget(systemWidget);
    inforWidget->setLayout(inforLayout);

    updateAllBtn = new QPushButton(systemWidget);
    updateAllBtn->setText("全部更新");
    updateAllBtn->setFixedSize(120,36);


    tab1HLayout->addWidget(systemPortraitLab,0,Qt::AlignLeft);
    tab1HLayout->addWidget(inforWidget,Qt::AlignLeft|Qt::AlignTop);
    tab1HLayout->addWidget(updateAllBtn);

    tab1HLayout->setMargin(20);
    tab1HLayout->setSpacing(20);

}

void TabWid::slotCancelDownload()
{
    checkUpdateBtn->setText("全部更新");
    checkUpdateBtn->setCheckable(true);
}

void TabWid::slotGetImportant(QStringList args)
{
    QStringList itList = args;
    getAppUpdateMsg(itList);

}

void TabWid::hideUpdateBtnSlot()
{
    qDebug() << updateMutual->importantList;
    QDateTime nowtime = QDateTime::currentDateTime();
    QString current_date = nowtime.toString("yyyy.MM.dd hh:mm:ss");
    lastRefreshTime->setText("上次更新时间："+current_date);
    if(updateMutual->importantList.size() == 0)
    {
        checkUpdateBtn->hide();
        versionInformationLab->setText("您的系统已是最新！");
    }
}

void TabWid::changeUpdateAllSlot()
{

    if(checkUpdateBtn->isEnabled() == false)
    {
        checkUpdateBtn->setText("全部更新");
        checkUpdateBtn->setEnabled(true);
    }
}

//void TabWid::recieveUpgradList(QStringList args)
//{

//}
