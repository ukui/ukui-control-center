#include "netconnect.h"
#include "ui_netconnect.h"

NetConnect::NetConnect()
{
    ui = new Ui::NetConnect;
    pluginWidget = new QWidget;
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    ui->setupUi(pluginWidget);

    pluginName = tr("netconnect");
    pluginType = NETWORK;

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

QWidget * NetConnect::get_plugin_ui(){
    return pluginWidget;
}

void NetConnect::begin_timer(){
    timerid = startTimer(3000);
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

    QSize size(64, 64);
    //本地、因特网图标的设置
    QPixmap localpixmap("://local.png");
    ui->localLabel->setPixmap(localpixmap.scaled(size));

    QPixmap netpixmap("://internet.png");
    ui->interLabel->setPixmap(netpixmap.scaled(size));

    //网络设备
    acquire_cardinfo();
    for (int num = 0; num < cardinfoQList.count(); num++){
        CardInfo current = cardinfoQList.at(num);
        QString pic;
        if (current.type == ETHERNET)
            if (current.status)
                pic = "://eth.png";
            else
                pic = "://eth_disconnect.png";
        else
            if (current.status)
                pic = "://wifi.png";
            else
                pic = "://wifi_disconnect.png";
        QIcon cardicon(pic);

        QWidget * netdeviceWidget = new QWidget();
        netdeviceWidget->setAttribute(Qt::WA_DeleteOnClose);
        QVBoxLayout * netdeviceVerLayout = new QVBoxLayout(netdeviceWidget);
        QToolButton * netdeviceToolBtn = new QToolButton(netdeviceWidget);
        netdeviceToolBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        netdeviceToolBtn->setIcon(cardicon);
        netdeviceToolBtn->setIconSize(QSize(42,42));
        netdeviceToolBtn->setText(current.name);

        netdeviceVerLayout->addWidget(netdeviceToolBtn);
        netdeviceWidget->setLayout(netdeviceVerLayout);

        QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
        item->setSizeHint(QSize(120, 64));
        ui->listWidget->setSpacing(5);
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
    switch (netstatus) {
    case DISCONNECTED:
        statuspic = "://disconnect.png";
        break;
    case NOINTERNET:
        statuspic = "://nonet.png";
        break;
    case CONNECTED:
        statuspic = "://connect.png";
    default:
        break;
    }
    QPixmap statuspixmap(statuspic);
    ui->statusLabel->setPixmap(statuspixmap);

}

void NetConnect::timerEvent(QTimerEvent *event){
    if (event->timerId() == timerid){
        if (lookhostid != -1){
            QHostInfo::abortHostLookup(lookhostid);
            netstatus = NOINTERNET;
            reset_lookuphostid();
        }
        refreshUI();
        killTimer(timerid);
    }
}

void NetConnect::reset_lookuphostid(){
    lookhostid = -1;
}
