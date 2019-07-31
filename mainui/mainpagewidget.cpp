#include "mainpagewidget.h"
#include "ui_mainpagewidget.h"

#include "mainwindow.h"
#include "component/custdomlabel.h"
#include "component/custdomframe.h"

MainPageWidget::MainPageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainPageWidget)
{
    ui->setupUi(this);
    this->setParent(parent);
    pmainWindow = (MainWindow *)parentWidget();

    logoSize = QSize(62, 62);

    initUI();
}

MainPageWidget::~MainPageWidget()
{
    for (int i = 0; i < delLabelList.count(); i++){
        delete(delLabelList.at(i));
    }
    delLabelList.clear();

    for (int i = 0; i < delFrameList.count(); i++){
        delete(delFrameList.at(i));
    }
    delFrameList.clear();

    delete ui;
}

void MainPageWidget::initUI(){

    //-----------------start----------------
    QPalette labelPalette;
    labelPalette.setColor(QPalette::WindowText, Qt::blue);
//    QFont font;
//    font.setPointSize(18);

    //system
    ui->systemLogo->setPixmap(QPixmap("://homePage/system.png").scaled(logoSize));
    ui->systemTitle->installEventFilter(this);
//    ui->systemTitle->setFont(font);
//    ui->systemTitle->setStyleSheet("color:#000000");
//    ui->systemHLayout->setSpacing(10);
//    ui->systemHLayout->setContentsMargins(110, 10, 0, 10);

//    pmainWindow->systemStringList << "display" << "defaultapp" << "power" << "autoboot";
    pmainWindow->subfuncList[SYSTEM] << "display" << "defaultapp" << "power" << "autoboot";

    QStringList systemStringList;
    systemStringList = pmainWindow->subfuncList[SYSTEM];

    QSignalMapper * systemSignalMapper = new QSignalMapper(this);
    QMap<QString, QObject *> systemMaps;
    systemMaps = pmainWindow->export_module(SYSTEM);

    bool systemfirst = false;
    for (int num = 0; num < systemStringList.size(); num++){
        if (!systemMaps.contains(systemStringList.at(num)))
            continue;
        CustdomLabel * label = new CustdomLabel(systemStringList.at(num));
        delLabelList.append(label);
        CustdomFrame * line = new CustdomFrame(systemStringList.at(num));
        delFrameList.append(line);
        connect(label, SIGNAL(clicked()), systemSignalMapper, SLOT(map()));
        systemSignalMapper->setMapping(label, systemMaps[systemStringList.at(num)]);
        if (systemfirst) //分割线首次不添加
            ui->optionsSystemHLayout->addWidget(line);
        systemfirst = true;
        ui->optionsSystemHLayout->addWidget(label);
    }
    connect(systemSignalMapper, SIGNAL(mapped(QObject *)), pmainWindow, SLOT(pluginClicked_cb(QObject*)));

//    if (systemMaps.isEmpty()){
//        ui->optionsSystemHLayout->addWidget(unavailableLable);
//        qDebug() << "Get System plugins Failed";
//    }
//    else{
//        QMap<QString, QObject *>::iterator it;
//        bool first = false;
//        for (it = systemMaps.begin(); it != systemMaps.end(); ++it){
//            CustdomLabel * label = new CustdomLabel(it.key());
//            delLabelList.append(label);
//            CustdomFrame * line = new CustdomFrame(it.key());
//            delFrameList.append(line);
//            connect(label, SIGNAL(clicked()), systemSignalMapper, SLOT(map()));
//            systemSignalMapper->setMapping(label, it.value());
//            if (first) //分割线首次不添加
//                ui->optionsSystemHLayout->addWidget(line);
//            first = true;
//            ui->optionsSystemHLayout->addWidget(label);
//        }
//        connect(systemSignalMapper, SIGNAL(mapped(QObject *)), pmainWindow, SLOT(pluginClicked_cb(QObject*)));
//    }

    //devices
    ui->devicesLogo->setPixmap(QPixmap("://homePage/system.png").scaled(logoSize));
    ui->devicesTitle->installEventFilter(this);
//    ui->devicesTitle->setFont(font);
//    ui->devicesTitle->setStyleSheet("color: #000000");
//    ui->devicesHLayout->setSpacing(10);
//    ui->devicesHLayout->setContentsMargins(40, 10, 0, 10);

    pmainWindow->subfuncList[DEVICES] << "printer" << "mouse" << "keyboard" << "audio";

    QStringList devicesStringList;
    devicesStringList = pmainWindow->subfuncList[DEVICES];

    QSignalMapper * devicesSignalMapper = new QSignalMapper(this);
    QMap<QString, QObject *> devicesMaps;
    devicesMaps = pmainWindow->export_module(DEVICES);

    bool devicesfirst = false;
    for (int num = 0; num < devicesStringList.size(); num++){
        if (!devicesMaps.contains(devicesStringList.at(num)))
            continue;

        CustdomLabel * label = new CustdomLabel(devicesStringList.at(num));
        delLabelList.append(label);
        CustdomFrame * line = new CustdomFrame(devicesStringList.at(num));
        delFrameList.append(line);
        connect(label, SIGNAL(clicked()), devicesSignalMapper, SLOT(map()));
        devicesSignalMapper->setMapping(label, devicesMaps[devicesStringList.at(num)]);
        if (devicesfirst)
            ui->optionsDevicesHLayout->addWidget(line);
        devicesfirst = true;
        ui->optionsDevicesHLayout->addWidget(label);
    }
    connect(devicesSignalMapper, SIGNAL(mapped(QObject*)), pmainWindow, SLOT(pluginClicked_cb(QObject*)));

    //personalized
    ui->personalizedLogo->setPixmap(QPixmap("://homePage/system.png").scaled(logoSize));
    ui->personalizedTitle->installEventFilter(this);
//    ui->personalizedTitle->setFont(font);
//    ui->personalizedTitle->setStyleSheet("color: #000000");
//    ui->personalizedHLayout->setSpacing(10);
//    ui->personalizedHLayout->setContentsMargins(110, 10, 0, 10);

    pmainWindow->subfuncList[PERSONALIZED] << "wallpaper" << "theme" << "screenlock" << "fonts" << "screensaver" << "start" << "panel";

    QStringList personalizedStringList;
    personalizedStringList = pmainWindow->subfuncList[PERSONALIZED];

    QSignalMapper * personalizedSignalMapper = new QSignalMapper(this);
    QMap<QString, QObject *> personalizedMaps;
    personalizedMaps = pmainWindow->export_module(PERSONALIZED);

    bool personalizedfirst = false;
    for (int num = 0; num < personalizedStringList.size(); num++){
        if (!personalizedMaps.contains(personalizedStringList.at(num)))
            continue;
        CustdomLabel * label = new CustdomLabel(personalizedStringList.at(num));
        delLabelList.append(label);
        CustdomFrame * line = new CustdomFrame(personalizedStringList.at(num));
        delFrameList.append(line);
        connect(label, SIGNAL(clicked()), personalizedSignalMapper, SLOT(map()));
        personalizedSignalMapper->setMapping(label, personalizedMaps[personalizedStringList.at(num)]);
        if (personalizedfirst)
            ui->optionsPersonalizedHLayout->addWidget(line);
        personalizedfirst = true;
        ui->optionsPersonalizedHLayout->addWidget(label);
    }
    connect(personalizedSignalMapper, SIGNAL(mapped(QObject*)), pmainWindow, SLOT(pluginClicked_cb(QObject*)));


    //network
    ui->networkLogo->setPixmap(QPixmap("://homePage/system.png").scaled(logoSize));
    ui->networkTitle->installEventFilter(this);
//    ui->networkTitle->setFont(font);
//    ui->networkTitle->setStyleSheet("color: #000000");
//    ui->networkHLayout->setSpacing(10);
//    ui->networkHLayout->setContentsMargins(40, 10, 0, 10);


    pmainWindow->subfuncList[NETWORK] << "netconnect" << "vpn" << "proxy";

    QStringList networkStringList;
    networkStringList = pmainWindow->subfuncList[NETWORK];

    QSignalMapper * networkSignalMapper = new QSignalMapper(this);
    QMap<QString, QObject * > networkMaps;
    networkMaps = pmainWindow->export_module(NETWORK);

    bool networkfirst = false;

    for (int num = 0; num < networkStringList.size(); num++){
        if (!networkMaps.contains(networkStringList.at(num)))
            continue;
        CustdomLabel * label = new CustdomLabel(networkStringList.at(num));
        delLabelList.append(label);
        CustdomFrame * line = new CustdomFrame(networkStringList.at(num));
        delFrameList.append(line);
        connect(label, SIGNAL(clicked()), networkSignalMapper, SLOT(map()));
        networkSignalMapper->setMapping(label, networkMaps[networkStringList.at(num)]);
        if (networkfirst)
            ui->optionsNetworkHLayout->addWidget(line);
        networkfirst = true;
        ui->optionsNetworkHLayout->addWidget(label);
    }
    connect(networkSignalMapper, SIGNAL(mapped(QObject*)), pmainWindow, SLOT(pluginClicked_cb(QObject*)));

    //account
    ui->accountLogo->setPixmap(QPixmap("://homePage/system.png").scaled(logoSize));
    ui->accountTitle->installEventFilter(this);
//    ui->accountTitle->setFont(font);
//    ui->accountTitle->setStyleSheet("color: #000000");
//    ui->accountHLayout->setSpacing(10);
//    ui->accountHLayout->setContentsMargins(110, 10, 0, 10);

    pmainWindow->subfuncList[ACCOUNT] << "userinfo" << "loginoptions";

    QStringList accountStringList;
    accountStringList = pmainWindow->subfuncList[ACCOUNT];

    QSignalMapper * accountSignalMapper = new QSignalMapper(this);
    QMap<QString, QObject * > accountMaps;
    accountMaps = pmainWindow->export_module(ACCOUNT);

    bool accountfirst = false;
    for (int num = 0; num < accountStringList.size(); num++){
        if (!accountMaps.contains(accountStringList.at(num)))
            continue;
        CustdomLabel * label = new CustdomLabel(accountStringList.at(num));
        delLabelList.append(label);
        CustdomFrame * line = new CustdomFrame(accountStringList.at(num));
        delFrameList.append(line);
        connect(label, SIGNAL(clicked()), accountSignalMapper, SLOT(map()));
        accountSignalMapper->setMapping(label, accountMaps[accountStringList.at(num)]);
        if (accountfirst)
            ui->optionsAccountHLayout->addWidget(line);
        accountfirst = true;
        ui->optionsAccountHLayout->addWidget(label);
    }
    connect(accountSignalMapper, SIGNAL(mapped(QObject*)), pmainWindow, SLOT(pluginClicked_cb(QObject*)));

    //time and lanuage
    ui->time_lanLogo->setPixmap(QPixmap("://homePage/system.png").scaled(logoSize));
    ui->time_lanTitle->installEventFilter(this);
//    ui->time_lanTitle->setFont(font);
//    ui->time_lanTitle->setStyleSheet("color: #000000");
//    ui->time_lanHLayout->setSpacing(10);
//    ui->time_lanHLayout->setContentsMargins(40, 10, 0, 10);

    pmainWindow->subfuncList[TIME_LANGUAGE] /*<< "language"*/ << "datetime" << "area" ;

    QStringList tlStringList;
    tlStringList = pmainWindow->subfuncList[TIME_LANGUAGE];

    QSignalMapper * tlSignalMapper = new QSignalMapper(this);
    QMap<QString, QObject* > tlMaps;
    tlMaps = pmainWindow->export_module(TIME_LANGUAGE);

    bool tlfirst = false;
    for (int num = 0; num < tlStringList.size(); num++){
        if (!tlMaps.contains(tlStringList.at(num)))
            continue;
        CustdomLabel * label = new CustdomLabel(tlStringList.at(num));
        delLabelList.append(label);
        CustdomFrame * line = new CustdomFrame(tlStringList.at(num));
        delFrameList.append(line);
        connect(label, SIGNAL(clicked()), tlSignalMapper, SLOT(map()));
        tlSignalMapper->setMapping(label, tlMaps[tlStringList.at(num)]);
        if (tlfirst)
            ui->optionsTLHLayout->addWidget(line);
        tlfirst = true;
        ui->optionsTLHLayout->addWidget(label);
    }
    connect(tlSignalMapper, SIGNAL(mapped(QObject*)), pmainWindow, SLOT(pluginClicked_cb(QObject*)));

    //Security and updates
    ui->security_updatesLogo->setPixmap(QPixmap("://homePage/system.png").scaled(logoSize));
    ui->security_updatesTitle->installEventFilter(this);
//    ui->security_updatesTitle->setFont(font);
//    ui->security_updatesTitle->setStyleSheet("color: #000000");
//    ui->security_updatesHLayout->setSpacing(10);
//    ui->security_updatesHLayout->setContentsMargins(110, 10, 0, 10);

    pmainWindow->subfuncList[SECURITY_UPDATES] << "update" << "recovery" << "backup";

    QStringList suStringList;
    suStringList = pmainWindow->subfuncList[SECURITY_UPDATES];

    QSignalMapper * suSignalMapper = new QSignalMapper(this);
    QMap<QString, QObject * > suMaps;
    suMaps = pmainWindow->export_module(SECURITY_UPDATES);

    bool sufirst = false;
    for (int num=0; num < suStringList.size(); num++){
        if (!suMaps.contains(suStringList.at(num)))
            continue;
        CustdomLabel * label = new CustdomLabel(suStringList.at(num));
        delLabelList.append(label);
        CustdomFrame * line = new CustdomFrame(suStringList.at(num));
        delFrameList.append(line);
        connect(label, SIGNAL(clicked()), suSignalMapper, SLOT(map()));
        suSignalMapper->setMapping(label, suMaps[suStringList.at(num)]);
        if (sufirst)
            ui->optionsSUHLayout->addWidget(line);
        sufirst = true;
        ui->optionsSUHLayout->addWidget(label);
    }
    connect(suSignalMapper, SIGNAL(mapped(QObject*)), pmainWindow, SLOT(pluginClicked_cb(QObject*)));

    //messages and task
    ui->messages_taskLogo->setPixmap(QPixmap("://homePage/system.png").scaled(logoSize));
    ui->messages_taskTitle->installEventFilter(this);
//    ui->messages_taskTitle->setFont(font);
//    ui->messages_taskTitle->setStyleSheet("color: #000000");
//    ui->messages_taskHLayout->setSpacing(10);
//    ui->messages_taskHLayout->setContentsMargins(40, 10, 0, 10);


    pmainWindow->subfuncList[MESSAGES_TASK] << "notice-operation" << "multitask" << "about";

    QStringList messagesStringList;
    messagesStringList = pmainWindow->subfuncList[MESSAGES_TASK];

    QSignalMapper * messagesSignalMapper = new QSignalMapper(this);
    QMap<QString, QObject *> messagesMaps;
    messagesMaps = pmainWindow->export_module(MESSAGES_TASK);

    bool messagesfirst = false;
    for (int num = 0; num < messagesStringList.size(); num++){
        if (!messagesMaps.contains(messagesStringList.at(num)))
            continue;
        CustdomLabel * label = new CustdomLabel(messagesStringList.at(num));
        delLabelList.append(label);
        CustdomFrame * line = new CustdomFrame(messagesStringList.at(num));
        delFrameList.append(line);
        connect(label, SIGNAL(clicked()), messagesSignalMapper, SLOT(map()));
        messagesSignalMapper->setMapping(label, messagesMaps[messagesStringList.at(num)]);
        if (messagesfirst)
            ui->optionsMTHLayout->addWidget(line);
        messagesfirst = true;
        ui->optionsMTHLayout->addWidget(label);
    }
    connect(messagesSignalMapper, SIGNAL(mapped(QObject*)), pmainWindow, SLOT(pluginClicked_cb(QObject*)));
}

bool MainPageWidget::eventFilter(QObject *watched, QEvent *event){
    if (watched == ui->systemTitle){
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton){
                qDebug() << ui->systemTitle->text() << "click";
                return true;
            }
            else
                return false;
        }
    }
    else if (watched == ui->devicesTitle){
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton){
                qDebug() << ui->devicesTitle->text() << "click";
                return true;
            }
            else
                return false;
        }
    }
    else if (watched == ui->personalizedTitle){
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton){
                qDebug() << ui->personalizedTitle->text() << "click";
                return true;
            }
            else
                return false;
        }
    }
    else if (watched == ui->networkTitle){
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton){
                qDebug() << ui->networkTitle->text() << "click";
                return true;
            }
            else
                return false;
        }
    }
    else if (watched == ui->accountTitle){
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton){
                qDebug() << ui->accountTitle->text() << "click";
                return true;
            }
            else
                return false;
        }
    }
    else if (watched == ui->time_lanTitle){
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton){
                qDebug() << ui->time_lanTitle->text() << "click";
                return true;
            }
            else
                return false;
        }
    }
    else if (watched == ui->security_updatesTitle){
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton){
                qDebug() << ui->security_updatesTitle->text() << "click";
                return true;
            }
            else
                return false;
        }
    }
    else if (watched == ui->messages_taskTitle){
        if (event->type() == QEvent::MouseButtonPress){
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton){
                qDebug() << ui->messages_taskTitle->text() << "click";
                return true;
            }
            else
                return false;
        }
    }
    return QWidget::eventFilter(watched, event);
}
