#include "netconnect.h"
#include "ui_netconnect.h"

NetConnect::NetConnect()
{
    ui = new Ui::NetConnect;
    pluginWidget = new CustomWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("netconnect");
    pluginType = NETWORK;

    timer = new QTimer();
    timer->setInterval(3000);
    connect(timer, SIGNAL(timeout()), this, SLOT(updatevalue()));

    begin_timer();
    component_init();
    status_init();
}

NetConnect::~NetConnect()
{
    delete ui;
    delete nmg;
}

QString NetConnect::get_plugin_name(){
    return pluginName;
}

int NetConnect::get_plugin_type(){
    return pluginType;
}

CustomWidget *NetConnect::get_plugin_ui(){
    return pluginWidget;
}

void NetConnect::plugin_delay_control(){

}

void NetConnect::begin_timer(){
//    timerid = startTimer(3000);
    timer->start();
    lookhostid = QHostInfo::lookupHost("www.ubuntukylin.com", this, SLOT(internet_status_slot(QHostInfo)));
}

void NetConnect::acquire_cardinfo(){
    QList<QNetworkInterface> network = QNetworkInterface::allInterfaces();
    for (QList<QNetworkInterface>::const_iterator it = network.constBegin(); it != network.constEnd(); it++){
        if ((*it).type() == QNetworkInterface::Loopback)
            continue;

        CardInfo ci;
        if ((*it).type() == QNetworkInterface::Ethernet)
            ci.type = ETHERNET;
        else if ((*it).type() == QNetworkInterface::Wifi)
            ci.type = WIFI;
        ci.name = (*it).humanReadableName();

        QList<QNetworkAddressEntry> addressList = (*it).addressEntries();
        if (addressList.count() == 0)
            ci.status = false;
        else
            ci.status = true;

        cardinfoQList.append(ci);
//        for (QList<QNetworkAddressEntry>::const_iterator itj = addressList.constBegin(); itj != addressList.constEnd(); itj++){
//            qDebug() << "***********";
//            qDebug() << "IP:" << (*itj).ip().toString();
//            qDebug() << "netmask : " << (*itj).netmask().toString();
//        }
    }
}

void NetConnect::component_init(){

//    QSize size(64, 64);
    //本地、因特网图标的设置
    QPixmap localpixmap("://netconnect/local.svg");
    ui->localLabel->setPixmap(localpixmap/*.scaled(size)*/);

    QPixmap netpixmap("://netconnect/Internet.svg");
    ui->interLabel->setPixmap(netpixmap/*.scaled(size)*/);

    ui->leftLabel->setPixmap(QPixmap("://netconnect/aided.png"));
    ui->rightLabel->setPixmap(QPixmap("://netconnect/aided.png"));

    //网络设备
    ui->listWidget->setStyleSheet("border: 0px slide");

    acquire_cardinfo();
    for (int num = 0; num < cardinfoQList.count(); num++){
        CardInfo current = cardinfoQList.at(num);
        QString pic;
        QString statustip;
        if (current.type == ETHERNET)
            if (current.status){
                pic = "://netconnect/eth.png";
                statustip = tr("Connect");
            }
            else{
                pic = "://netconnect/eth_disconnect.png";
                statustip = tr("Disconnect");
            }
        else
            if (current.status){
                pic = "://netconnect/wifi.png";
                statustip = tr("Connect");
            }
            else{
                pic = "://netconnect/wifi_disconnect.png";
                statustip = tr("Disconnect");
            }
        QIcon cardicon(pic);

        QWidget * netdeviceWidget = new QWidget();
        netdeviceWidget->setAttribute(Qt::WA_DeleteOnClose);
        QVBoxLayout * netdeviceVerLayout = new QVBoxLayout(netdeviceWidget);
        QToolButton * netdeviceToolBtn = new QToolButton(netdeviceWidget);
        netdeviceToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        netdeviceToolBtn->setIcon(cardicon);
        netdeviceToolBtn->setIconSize(QSize(48,48));
        netdeviceToolBtn->setText(current.name + "\n" + statustip);

        netdeviceVerLayout->addWidget(netdeviceToolBtn);
        netdeviceWidget->setLayout(netdeviceVerLayout);

        QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
        item->setSizeHint(QSize(160, 64));
//        ui->listWidget->setSpacing(0);
        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, netdeviceWidget);
        ui->listWidget->setViewMode(QListView::IconMode);
    }
}


void NetConnect::status_init(){
    //获取当前状态
    nmg  = new QNetworkConfigurationManager();
    if (nmg->isOnline())
        netstatus = CONNECTED;
    else
        netstatus = DISCONNECTED;

    //
    connect(nmg, SIGNAL(onlineStateChanged(bool)), this, SLOT(internet_status_changed_slot(bool)));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(run_external_app_slot()));
}

void NetConnect::run_external_app_slot(){
    QString cmd = "nm-connection-editor";
    QProcess process(this);
    process.startDetached(cmd);
}

void NetConnect::internet_status_changed_slot(bool status){
    if (status){
        netstatus = CONNECTED;
        begin_timer();
    }
    else{
        netstatus = DISCONNECTED;
        refreshUI();
    }

}

void NetConnect::internet_status_slot(QHostInfo host){
    if (host.error() != QHostInfo::NoError && netstatus == CONNECTED){
//        qDebug() << "Internet status faild" << host.errorString();
        netstatus = NOINTERNET;
        return;
    }
    reset_lookuphostid();
}

void NetConnect::refreshUI(){
    //网络状态的设置
    QString statuspic;
    QString statustip;
    switch (netstatus) {
    case DISCONNECTED:
        statuspic = "://netconnect/disconnect.png";
        statustip = tr("Disconnect");
        break;
    case NOINTERNET:
        statuspic = "://netconnect/nonet.png";
        statustip = tr("Restricting access");
        break;
    case CONNECTED:
        statuspic = "://netconnect/connect.png";
        statustip = tr("Connect");
    default:
        break;
    }
    QPixmap statuspixmap(statuspic);
    ui->statusLabel->setPixmap(statuspixmap);
    ui->statustipLabel->setText(statustip);

}

//void NetConnect::timerEvent(QTimerEvent *event){
//    if (event->timerId() == timerid){
//        if (lookhostid != -1){
//            QHostInfo::abortHostLookup(lookhostid);
//            netstatus = NOINTERNET;
//            reset_lookuphostid();
//        }
//        refreshUI();
//        killTimer(timerid);
//    }
//}

void NetConnect::updatevalue(){
    if (lookhostid != -1){
        QHostInfo::abortHostLookup(lookhostid);
        netstatus = NOINTERNET;
        reset_lookuphostid();
    }
    refreshUI();
    timer->stop();
}

void NetConnect::reset_lookuphostid(){
    lookhostid = -1;
}
